# CPU Engineering

## Scope

CPU budgeting, bottleneck identification, and target-hardware measurement for the SP-1 (ARM Cortex-M4F @ nRF52840).

Audio-path CPU cost directly competes with control/streamer/MIDI threads. CPU overage on the audio path causes real-time underruns (clicks, glitches) on a device with limited headroom.

## Measurement Discipline

**Never collapse measurement classes into a single claim.** Always label the source:

- **THEORETICAL ESTIMATE**: math on paper, cycle calculations, no hardware involved; used for initial feasibility, not proof
- **DESKTOP BENCHMARK**: measured on a development machine (x86, optimized toolchain); gives performance *order*, not actual target performance
- **SIMULATED EMBEDDED BENCHMARK**: emulated Cortex-M4F or similar (e.g., QEMU), not real hardware; closer to target but still not authoritative
- **COMPILED SP-1 IMPLEMENTATION**: built with `west build`, binary exists, not flashed to device; proves compilation/linking, not runtime performance
- **HARDWARE-MEASURED SP-1 PERFORMANCE**: flashed to real nRF52840, benchmarked on-device via cycle counters, DWT, or instrumentation; this is ground truth

Until a value is HARDWARE-MEASURED, label it `UNMEASURED` and note what measurement class it actually is.

## Workflow: Bottleneck-First Optimization

1. **Measure** on real SP-1 hardware. If not available, compile and estimate; label estimate clearly.
2. **Identify actual bottleneck** via profiling (e.g., DWT cycle counter in Zephyr, CPU load, real-world listening tests for clicks/glitches).
3. **Remove unnecessary work** (dead code, redundant calculations, unnecessary copying).
4. **Reduce computation frequency** where mathematically valid (control-rate updates instead of audio-rate, block-rate instead of sample-rate).
5. **Reuse calculations** (cache coefficients, share lookups, avoid redundant exp/log calls).
6. **Change algorithm** if justified (e.g., replace expensive operation with LUT approximation; benchmark the swap, do not assume it is faster).
7. **Improve numerical representation** if necessary (e.g., Q15 vs Q16, fixed-point vs float—on SP-1, audio path is Q16 fixed-point only; see audio-model.md).
8. **Exploit architecture-specific capabilities** (e.g., Cortex-M4F accumulate instructions, hardware multiply-accumulate).
9. **Benchmark again** after each change on real hardware to confirm the change actually helped.

Do not begin by micro-optimizing code that "looks expensive." Measure first.

## Block and Sample Rate Context

- **Audio block**: 256 frames per I2S interrupt (@ 48 kHz ≈ 5.3 ms per block)
- **Per-block CPU budget** (UNMEASURED): depends on phase offset, total firmware load, priority; estimated ~0.5–2 ms CPU per block available for audio DSP (highly variable; measure on target)
- **Per-sample CPU budget** (UNMEASURED): derived from per-block budget (e.g., if 1 ms available per 5.3 ms block, ~188 CPU cycles per sample @ 48 kHz; **do not rely on this estimate without hardware validation**)

Do not commit to a per-sample cycle budget without measuring the actual firmware on real hardware.

## Example: Cost Accounting

When implementing a new filter or effect:

```
Component: Two-pole IIR filter

ESTIMATE (theoretical):
  - per-sample: 2 multiply + 3 add + 1 shift (operation count only; runtime is UNMEASURED)
  - per block: coefficient recalc (if parameter change) = exp() + 2× sqrt() (runtime is UNMEASURED)

COMPILED:
  - `-Os` binary size: ~40 B instruction + 16 B state (verified via `size` / ELF)

HARDWARE-MEASURED:
  - (pending real SP-1 device)
```

Document what you have. Do not claim a measured result until it exists.

Status: UNMEASURED (no real SP-1 device available in this environment; as of 2026-09-21, the firmware compiles end-to-end but has not been flashed/measured on hardware)
