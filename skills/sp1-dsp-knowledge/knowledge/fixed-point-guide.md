# Fixed-Point Mathematics for Audio DSP

## Why Fixed-Point?

- **No FPU overhead** — Integer-only math on fast paths
- **Predictable performance** — No floating-point exceptions or stalls
- **Deterministic rounding** — No surprise results across platforms
- **Tight control** — Every bit matters; no hidden precision loss

## Q16 format example

Q-format choice belongs to the consuming implementation. The examples below
use Q16.16 as a concrete format for discussion; they do not define a device
standard.

Q16 = 16 integer bits + 16 fractional bits, signed 32-bit:

```
     1 bit     16 bits     16 bits
   [Sign] [Integer] [Fraction]
   ───────────────────────────
       -32768.0 to +32767.999...
```

### Conversion

```c
// Float → Q16
q16_t q16_from_float(float x) {
    return (q16_t)(x * 65536.0f);
}

// Q16 → Float
float q16_to_float(q16_t q) {
    return (float)q / 65536.0f;
}

// Examples
q16_t one = Q16(1.0f);        // 1.0 * 65536 = 65536
q16_t half = Q16(0.5f);       // 0.5 * 65536 = 32768
q16_t tiny = Q16(0.001f);     // 0.001 * 65536 ≈ 65
```

### Saturation

Always guard against overflow:

```c
// Multiply two Q16 numbers
int64_t acc = (int64_t)a * b;  // Intermediate is 64-bit
acc >>= 16;                     // Shift back to Q16
q16_t result = q16_saturate(acc);

// Saturation function
q16_t q16_saturate(int64_t value) {
    if (value > 0x7FFFFFFF) return 0x7FFFFFFF;    // Max: ~32767
    if (value < -0x80000000) return -0x80000000;  // Min: ~-32768
    return (q16_t)value;
}

// Or use ARM SIMD: __SSAT(value, 32)
```

## Limit Cycle Oscillations

Recursive filters in fixed-point suffer from **fractional truncation**.

### Problem

```c
// Simple all-pass filter in Q16
q16_t y = q16_saturate(((int64_t)a * x + (int64_t)b * state) >> 16);
state = x;  // Quantization error lost here
```

The rounding error accumulates → DC bias → audible low-frequency tone.

### Solution: Fraction Saving

Keep the fractional part and feed it back:

```c
// Maintain full-precision state internally
int64_t state_frac = 0;  // Keep fractional bits

q16_t filter_step(q16_t input, q16_t coeff) {
    // Multiply in full precision
    int64_t product = (int64_t)coeff * input;
    
    // Add previous state (including fraction)
    state_frac += product;
    
    // Extract output (upper 32 bits)
    q16_t output = (q16_t)(state_frac >> 16);
    
    // Lower 16 bits stay in state_frac for next iteration
    // This prevents DC bias buildup
    
    return output;
}
```

**Result:** Truncation noise moves to Nyquist (20 kHz), inaudible.

## CORDIC: Trigonometry Without Lookup Tables

CORDIC (Coordinate Rotation DIgital Computer) computes sin/cos/atan using **only bit-shifts and addition**.

### Why

- LUT-based sine wastes SRAM (even quarter-wave: ~16 KB)
- CORDIC needs ~20 iterations of shift+add per sample
- Result: accurate to ~15 bits with zero memory

### Algorithm Sketch

```c
// Simplified CORDIC for sin(angle)
// angle in range [0, 2*pi] as unsigned 32-bit
typedef struct {
    q16_t x, y;
} cordic_result_t;

cordic_result_t cordic_sin_cos(uint32_t angle) {
    // Precomputed atan(2^-i) table (very small)
    static const q16_t atan_table[16] = {
        0x80000,  // atan(2^0) in Q16
        0x4AEB0,  // atan(2^-1) in Q16
        // ... 14 more values
    };
    
    q16_t x = 0x4DFC0;  // Start with ~0.618
    q16_t y = 0;
    uint32_t z = angle;
    
    // 16 iterations of rotation
    for (int i = 0; i < 16; i++) {
        int d = (z >= 0x80000000) ? -1 : 1;
        
        // Rotate: (x', y') = R(angle) * (x, y)
        q16_t x_new = x - ((d * y) >> i);
        q16_t y_new = y + ((d * x) >> i);
        x = x_new;
        y = y_new;
        
        // Update angle
        z -= d * atan_table[i];
    }
    
    // Normalize to unit circle
    x = (q16_t)((int64_t)x * 0xB505 >> 16);  // Magic normalization
    y = (q16_t)((int64_t)y * 0xB505 >> 16);
    
    return (cordic_result_t){x, y};
}
```

**Trade-off:**
- ✓ Zero SRAM (only small atan table)
- ✗ ~20 iterations per call
- ✗ Slower than LUT for frequent calls

**Use CORDIC when:** Memory is critical (granular synthesis, many oscillators)  
**Use LUT when:** You have spare SRAM and tight CPU budget

## Lookup Table Compression

If you must use LUTs, compress aggressively:

### Quarter-Wave Symmetry

Periodic functions repeat every 90°. Store only 1/4:

```c
// Storage: only quadrant 1 (90 degrees)
static const q16_t sine_quarter[256] = {
    0x0000,  // sin(0°) in Q16
    0x0324,  // sin(1.4°)
    // ... 254 more values
};

// Retrieve any quadrant
q16_t sine_lookup(uint32_t angle_deg_u32) {
    // angle_deg_u32: 0..359 degrees (fixed-point, 32-bit)
    uint32_t quadrant = (angle_deg_u32 >> 24) & 3;  // 0..3
    uint32_t phase = angle_deg_u32 & 0xFFFFFF;      // Within quadrant
    uint16_t index = (phase >> 16) & 0xFF;          // 0..255
    
    q16_t value = sine_quarter[index];
    
    // Apply sign and mirror based on quadrant
    if (quadrant == 1) return value;                // Q1: sin(x)
    if (quadrant == 2) return value;                // Q2: sin(180-x)
    if (quadrant == 3) return -value;               // Q3: -sin(x)
    return -value;                                   // Q4: -sin(180-x)
}

// Result: 256 entries instead of 1024, 75% savings
```

### Padé Approximants

Replace LUTs with polynomial curves:

```c
// Instead of storing saturation curve in LUT (512 entries)
// Use rational polynomial approximation:
// tanh(x) ≈ (x*(3 + 0.1*x^2)) / (1 + 0.1*x^2)

q16_t soft_saturate(q16_t x) {
    // Coefficients pre-computed from Padé table
    const q16_t a0 = 0x30000;  // 3.0 in Q16
    const q16_t a1 = 0x01999;  // 0.1 in Q16
    
    q16_t x2 = (q16_t)((int64_t)x * x >> 16);  // x^2 in Q16
    
    q16_t numerator = (q16_t)((int64_t)x * (a0 + (a1 * x2 >> 16)) >> 16);
    q16_t denominator = 0x10000 + ((a1 * x2) >> 16);  // 1 + 0.1*x^2
    
    return (q16_t)((int64_t)numerator << 16) / denominator;
}

// Result: ~4 coefficients, zero LUT, good accuracy
```

## Filter Topologies: WDF and SVF

Standard Direct Form IIR filters are unstable in fixed-point:

```c
// UNSTABLE in Q16: tiny coefficient errors explode
// a[1] ≈ 2*cos(freq); quantization error → instability
typedef struct {
    q16_t a1, a2, b0, b1, b2;
    q16_t z1, z2;  // State
} biquad_direct_form_t;
```

Use **State-Variable Filter (SVF)** topology instead:

```c
// SVF: Structurally stable even when coefficients are quantized
typedef struct {
    q16_t f, q;      // Frequency, resonance
    q16_t ic1, ic2;  // Integrator states
} svf_state_t;

void svf_process_step(q16_t input, svf_state_t* s, 
                      q16_t* low, q16_t* band, q16_t* high) {
    // Trapezoidal integration
    q16_t in_l = input - s->ic2;
    q16_t out_l = (q16_t)((int64_t)s->f * s->ic1 >> 16);
    
    q16_t in_b = out_l - s->ic1;
    q16_t out_b = (q16_t)((int64_t)s->f * in_b >> 16);
    
    // Update states
    s->ic1 += (q16_t)((int64_t)s->f * in_b >> 16);
    s->ic2 += (q16_t)((int64_t)s->f * in_l >> 16);
    
    *low = out_l;
    *band = out_b;
    *high = input - (q16_t)((int64_t)s->q * out_b >> 16) - out_l;
}
```

**SVF advantages:**
- ✓ Passivity preserved (stable even with 8-bit quantization)
- ✓ Simultaneous multi-mode output (low, band, high)
- ✓ Smooth parameter changes (no discontinuities)

## Static Analysis: Predicting Overflow

Before deployment, identify worst-case overflow:

```c
// How big can intermediate values grow?
// Input: ±1.0 (max audio level)
// Coefficient: ±1.0 (max feedback)
// Cascade: filter → delay → saturate → filter

// Manual analysis
q16_t worst_case_gain = 0;
for (int i = 0; i < FILTER_ORDER; i++) {
    q16_t stage_gain = estimate_filter_gain(filters[i]);
    worst_case_gain = q16_saturate((int64_t)worst_case_gain * stage_gain >> 16);
}

if (worst_case_gain > 0x10000) {  // > 1.0
    // Risk of overflow; add pre-gain reduction
}
```

Tools like FAUST do this automatically. Hand-rolled C requires careful measurement.

## Checklist: Q16 Implementation

Before shipping a Q16 algorithm:

- [ ] All multiplications use `(int64_t)a * b >> 16`
- [ ] Saturation applied after multiply-accumulates
- [ ] Intermediate calculations stay within int64_t
- [ ] Limit cycle analysis done (or fraction-saving implemented)
- [ ] Tested at extreme levels (-1.0, +1.0)
- [ ] Tested with DC offset input (detects limit cycles)
- [ ] RMS error vs float reference < 0.1%
- [ ] Coefficients quantized and tested (not just float coefficients)

---

## References

- **Agarwal & Rawat (2016):** "VLSI Implementation of Fixed-Point Lattice Wave Digital Filters"
- **Fang, Rutenbar, & Chen (2003):** "Fast, accurate static analysis for fixed-point finite-precision effects in DSP designs"
- **Simper (2013):** "Solving the continuous SVF equations using trapezoidal integration"

See `/sources/` for detailed papers and implementations.
