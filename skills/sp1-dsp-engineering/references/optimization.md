# DSP Optimization

## Foundational Principle

**Measure first. Identify the actual bottleneck. Optimize only what is proven expensive on real hardware.**

Do not optimize code that "looks expensive." Do not assume a technique is faster without benchmarking it.

## Optimization Hierarchy

1. **Remove the work** (dead code, redundant operations, unnecessary processing, unnecessary copies)
2. **Share the work** (combine voices, reuse a processor, cache shared results, avoid duplicate work)
3. **Move the work** (initialization, load time, block rate, control rate, background worker, offline preprocessing)
4. **Reduce the work** (reduce update frequency, reduce precision, shorten state, simplify algorithm)
5. **Measure** actual bottleneck on SP-1 hardware (DWT cycle counters, real listening tests for clicks/glitches)
6. **Choose a better algorithm** if justified by measurement (trade accuracy or latency for CPU; validate tradeoff sonically)
7. **Improve numerical representation** if space/precision justify it (e.g., 16-bit vs 32-bit; on SP-1, audio is Q16, not float)
8. **Lookup tables / precomputation** for expensive functions (sin, cos, exp, sqrt; validate error bounds and target cost)
9. **Exploit target architecture** (Cortex-M4F accumulate instructions, hardware multiply-accumulate, cache-friendly layout)
10. **Historical low-level techniques** (bit manipulation, loop unrolling, inline assembly; measure before doing; risk maintainability)
11. **Re-measure** on SP-1 hardware after each change to confirm improvement (do not assume)

## Structural optimization before inner-loop optimization

The strongest general finding from the Schwung case studies is that optimization should usually proceed from the topology outward. Before tuning a hot inner loop, ask whether an expensive processor or operation can be eliminated, shared, moved, or reduced in frequency.

```text
1. DOES THIS WORK NEED TO EXIST?
2. DOES IT NEED TO OCCUR THIS MANY TIMES?
3. DOES IT NEED TO HAPPEN IN REALTIME?
4. DOES IT NEED THIS UPDATE RATE OR PRECISION?
5. CAN THE STATE OR MEMORY LIFETIME BE REDUCED?
6. ONLY THEN: optimize the remaining implementation
```

### Topological example

```text
stem 1 ─┐
stem 2 ─┤
stem 3 ─┼→ PREMIX → ONE PROCESSOR
stem 4 ─┘
```

This can be preferable to four instances of an expensive processor in parallel when the architecture permits a single shared processor. This is not a free optimization: it changes routing semantics, nonlinear interaction, and downstream flexibility.

### Generalized rule

> Reduce how many times expensive work occurs before reducing the cost of each instance.

## Evidence discipline for optimization claims

The following should not be normalized into general SP-1 doctrine unless they are measured on the actual target and clearly sourced:

- exact silence-bypass percentage savings
- exact control-rate and sub-block percentages
- exact RAM savings from scratch pooling or streaming
- exact cache alignment or SIMD assumptions
- fixed thresholds from another target
- percentage-based claims without target workload and measurement context

Promote the mechanism, not an unverified numeric claim.

## Decision tree

```text
1. Remove it?                     -> yes: eliminate it
2. Share it?                      -> yes: combine / cache / reuse
3. Move it?                      -> yes: init / load / block / control / offline
4. Reduce it?                    -> yes: lower rate / precision / state
5. Make remaining work faster?   -> only after structural adjustments
```

## Techniques for Fixed-Point Embedded Audio

Common optimizations relevant to Q16 fixed-point on constrained hardware:

### Lookup Tables for Expensive Functions

**Candidates**: sin, cos, exp, sqrt, reciprocal, nonlinear transfer functions

**Trade-off**:
- CPU: target-dependent; a lookup, interpolation, or arithmetic approximation
  may win depending on memory access, branches, compiler output, and update rate
- Memory: table size and placement are measurable costs
- Accuracy: limited by table resolution and interpolation; quantify error

**Validation**: Compare table output vs reference (e.g., exp(x) in range [−10, 0]) on a test sweep; measure error and confirm inaudible

### Block-Rate vs Sample-Rate Computation

**Example**: IIR filter coefficient is constant across a block. Compute once per block, not per sample.

```c
// Control-rate (before audio block):
alpha = 1 - exp(-2 * M_PI * fc / fs);  // expensive

// Audio-rate (during block processing):
for (int i = 0; i < BLOCK_SIZE; i++) {
    y[i] = y[i-1] + alpha * (x[i] - y[i-1]);  // fast: 1 mul, 1 add
}
```

If a parameter changes mid-block (rare), defer the update to the next block boundary.

### Shared Buffers and State

**Example**: Reverb and echo may share a delay line when the architecture and resource budget permit it.

- CPU: one write, multiple reads from shared line
- Memory: 9,216 B shared vs 18,432 B if separate buffers
- Tradeoff: slightly more complex bookkeeping, major RAM saving

### Avoid Dynamic Allocation

Pre-allocate all buffers and state at startup. No `malloc()` in audio path.

- CPU: allocation overhead avoided
- Memory: predictable, no fragmentation, no heap searching
- Maintainability: explicit buffer sizes, easier to understand memory footprint

### Q16 Saturation and Rounding

Use the repository's established saturation helper or an explicit clamp. Do not
assume `__SSAT()` is faster than a manual clamp until the exact compiled target
path is measured.

```c
int32_t q16_accumulator = acc + (a * b);  // might overflow
int32_t q16_clipped = __SSAT(q16_accumulator, 32);
```



### Inline Assembly (Use Sparingly)

Cortex-M4F has hardware accumulate (`SMLABB`, `SMLATT`). If profiling shows multiply-accumulate is the bottleneck, inline assembly for hand-optimized MAC loops may help.

**But**: only if measurement proves it is necessary, and only for the hot inner loop. Keep the rest readable C.

## Tradeoff Record Template

Every optimization should document what it gains and what it costs:

```yaml
optimization:
  name: "Lookup table for exp() in filter coefficient"
  technique: "256-entry log-spaced lookup + linear interpolation"
  rationale: "exp() is expensive in filter design; coefficient computation happens at block rate; 256 entries sufficient for perceptual audio"
  
  tradeoff:
    cpu: "reduced (1 LUT + interp vs 1 exp() ≈ 3–5× speedup)"
    ram: "increased (256 × 4 B = 1 KB for table)"
    flash: "increased (table + interpolation code ≈ 100 B)"
    latency: "unchanged (coefficient computed before block, not on audio thread)"
    precision: "slightly reduced (LUT error ±0.01 vs exact exp; inaudible)"
    complexity: "increased (interp logic, table generation)"
    maintainability: "reduced (magic table, less obvious than exp())"
  
  measured: false
  measurement_class: "THEORETICAL ESTIMATE + COMPILED SP-1 (code size verified; runtime speedup not measured on hardware)"
  status: "UNMEASURED"
```

If you claim "should be 3× faster," label it ESTIMATE. If you've benchmarked on real SP-1, label it HARDWARE-MEASURED.

## Historical Reference

Optimization techniques from game consoles, trackers, and demoscene work often inspired by resource constraints similar to SP-1:

- **Fast approximate trig** (e.g., Cordic, polynomial approximations) — common in game physics; validate error before audio use
- **Bit-reversal tricks** for FFT or block indexing — useful for DSP but CPU cost depends on target architecture
- **Cache-friendly memory layout** — relevant if profiling shows cache misses; less relevant on Cortex-M4 (small L1/no L2 on nRF52840)
- **Branch reduction** — Cortex-M4 branch penalty is modest; usually not the bottleneck
- **Inline multiply-accumulate** — powerful if the hot inner loop is MUL+ADD heavy; measure first

Treat these as ideas to investigate, not cargo-cult solutions.

Status: REFERENCE (hierarchy and techniques documented; tradeoff discipline established; measurement-first principle emphasized)
