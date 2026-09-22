# Numerical Engineering

## Scope

Numerical representation, stability, precision, and failure behavior for Q16 fixed-point DSP on the SP-1.

**Audio-path DSP must use Q16 fixed-point.** (See audio-model.md.)

## Q16 Fixed-Point Format

Q16 representation: 16 integer bits + 16 fractional bits (signed 32-bit int).

- **Range**: −32,768.0 to +32,767.999... (in units of 1/65536)
- **Typical audio range**: −1.0 to +1.0 maps to −65536 to +65536 (using headroom for intermediate calculations)
- **Resolution**: 1 LSB = 1/65536 ≈ 15.3 µ V per sample @ ±1 V audio reference
- **Saturation**: must clip to prevent wraparound; use `__SSAT(value, 32)` or similar

Existing SP-1 code uses Q16 throughout (reverb, codecs, tape effects). Preserve this format in all audio-path math.

## Design-Time Checklist

For each audio DSP component:

- [ ] **Range and units defined** — what does Q16 value mean? (audio samples, coefficients, accumulators)
- [ ] **Precision justified** — why Q16? Is 16 bits of fractional precision sufficient for this component? (usually yes for audio, but justify if not)
- [ ] **Feedback stability considered** — if the component has feedback (IIR filter, delay line), does the Q16 scaling cause instability? (accumulated errors, coefficient denormalization)
- [ ] **Overflow/underflow considered** — what happens when Q16 value exceeds ±1 (in audio units)? Saturate, wrap, or pass through?
- [ ] **Accumulation error** — if you sum many Q16 values, do they lose precision? Do you need to upscale to 64-bit intermediate, then downscale?
- [ ] **Denormal/subnormal behavior** — fixed-point has no denormals; very small values just become 0 (no gradual underflow like float)
- [ ] **Initialization and reset deterministic** — state initialized to a known value, not garbage
- [ ] **Parameter discontinuities analyzed** — when a parameter changes (e.g., filter coefficient), does the change cause an audible click or discontinuity? If so, is it smoothed?

## Approximation for Expensive Operations

When implementing operations expensive in fixed-point (e.g., exp, sqrt, sin/cos), consider approximation:

### Example: Exponential (used in filter coefficient calculation)

**Original**: `exp(x)` floating-point math library

**Approximation options**:
1. **Lookup table** — precomputed table of exp values for likely coefficient ranges (e.g., 256-entry table)
2. **Polynomial approximation** — fit a low-degree polynomial to exp(x) over a range; trade accuracy for speed
3. **Hybrid** — lookup + linear interpolation between table entries

**Documentation required**:
```
Operation: exp(x) for filter coefficient α = 1 - exp(−2π * fc / fs)
  where fc = cutoff frequency, fs = sample rate

Reference: floating-point exp(x) from math library

Approximation: 256-entry lookup table, log-spaced
  Input range: x ∈ [−10, 0] (typical audio filter ranges)
  Error: max ±0.01 in α (acceptable for perceptual audio filtering)
  Cost: 1 table lookup + interpolation vs 1 exp call (estimated 10× faster)
  Validation: frequency response vs reference (sweep 20 Hz–20 kHz)

Result: (UNMEASURED on real SP-1; estimated speedup valid if measured)
```

Do not claim "should be faster" without benchmarking the swap on real hardware.

## Validation Against Float Reference

When adapting an algorithm from floating-point reference code:

1. **Establish the float reference** — run original float code on known test signals (sine, noise, impulse)
2. **Implement Q16 version** — scale inputs/outputs, implement with Q16 math, saturation, rounding
3. **Compare outputs** — Q16 vs float on same test signals; measure error (RMS, max, spectral)
4. **Determine acceptable error** — is an RMS error of 0.1% audible? 0.01%? Depends on component (filter coefficients less sensitive than delay-line samples)
5. **Document the error** — e.g., "IIR filter pole location error ±0.5% due to Q16 coefficient quantization; inaudible in listening tests"

## Known Fixed-Point Implementations

- **Reverb and codecs**: Preserve the existing implementation and validate any adaptation against a stable reference.

Preserve Q16 format when extracting or adapting these.

Status: REFERENCE (Q16 format documented and exemplified; validation discipline established; Reverb/Codec examples provided)
