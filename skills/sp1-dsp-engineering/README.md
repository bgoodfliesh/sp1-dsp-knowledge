# SP-1 DSP Engineering Skill

Engineering knowledge layer for developing DSP appropriate to the Teenage Engineering SP-1 firmware.

This skill teaches how to turn DSP algorithms, research, reference implementations, and sonic requirements into efficient, maintainable implementations appropriate for the SP-1's actual constraints. It prevents **vibe-coded DSP**: plausible-looking code that has not been reasoned about for real-time execution, numerical behavior, memory, CPU usage, latency, state, or sonic correctness.

## File Structure

```
sp1-dsp-engineering/
├── SKILL.md              # Core skill: purpose, constraints, rejected shapes, engineering rules
├── README.md             # This file
└── references/
    ├── audio-model.md    # Hardware context, Q16 requirement, per-algorithm checklist
    ├── realtime.md       # Audio-thread safety, non-negotiable invariants, cross-thread patterns
    ├── state.md          # State ownership, lifecycle, preservation rules
    ├── parameters.md     # Parameter records, control-rate separation, perceptual resolution
    ├── numeric.md        # Q16 fixed-point, approximations, validation against float
    ├── cpu.md            # Measurement discipline, bottleneck-first optimization, cycle budgeting
    ├── memory.md         # RAM/flash budgeting, pre-allocation, known SP-1 constraints
    ├── optimization.md   # Optimization hierarchy, LUT strategies, tradeoff documentation
    └── validation.md     # Testing framework, reference comparison, status labels
```

## How to Use This Skill

### Start Here: `SKILL.md`

This is the foundation document. It covers:
- Purpose and scope
- SP-1 hardware verified facts (nRF52840, Q16 fixed-point, threading model)
- Critical audio-path constraints (no eMMC access, deterministic execution, no float)
- Non-negotiable invariants from the tape-looper firmware
- Shapes already rejected (Hermite interpolation, `TapeHead` redesign, floating-point, etc.)
- Measurement discipline (ESTIMATE vs HARDWARE-MEASURED)
- Core engineering rules (real-time safe, numerically intentional, resource-aware, validated)
- Workflow for adapting existing DSP

### Reference Files: Checklists and Frameworks

Each reference file is a discipline guide and checklist for a specific DSP concern. Use them together:

1. **`audio-model.md`** — Understanding the hardware
   - Start here to confirm sample rate, block size, stereo path, Q16 requirement
   - Define what every new component must specify

2. **`realtime.md`** — Audio-thread safety
   - No eMMC access, no dynamic allocation, bounded execution
   - Cross-thread communication patterns (volatile mailboxes, not `k_msgq`)
   - Audit checklist for every audio-path function

3. **`state.md`** — State ownership and lifecycle
   - Define persistent vs temporary, per-channel vs shared
   - Explicit initialization, reset behavior, bypass semantics
   - Preserve state when adapting existing DSP

4. **`parameters.md`** — Parameter engineering
   - Document range, scaling, mapping, update rate, smoothing for every parameter
   - Separate control-rate computation from audio-rate processing
   - Allocate resolution where perceptual (not by control precision)

5. **`numeric.md`** — Fixed-point arithmetic
   - Q16 format, range, precision, saturation, rounding
   - Approximation strategies (e.g., lookup tables for expensive functions)
   - Validation against floating-point reference

6. **`cpu.md`** — CPU budgeting and measurement
   - Measurement discipline: distinguish ESTIMATE, COMPILED, HARDWARE-MEASURED
   - Bottleneck-first optimization: measure → identify → remove unnecessary → reduce frequency → reuse → LUT → benchmark
   - Do not assume what is expensive; measure on real hardware

7. **`memory.md`** — RAM and flash budgeting
   - Pre-allocate all audio-path state and buffers
   - Known SP-1 constraints (256 KB RAM, shared reverb buffer, ring buffers, streamer alignment)
   - No dynamic allocation in audio path

8. **`optimization.md`** — Optimization methodology
   - Hierarchy: measure → remove unnecessary → reduce frequency → reuse → better algorithm → improved representation → LUT → exploit architecture
   - Fixed-point embedded techniques (lookup tables, block-rate computation, shared buffers)
   - Tradeoff record template: document what each optimization gains and costs

9. **`validation.md`** — Testing and proof
   - Compilation is not validation; plausible sound is not validation
   - Test categories: numerical/reference, frequency/time domain, signal-level, state, CPU/RAM, hardware execution
   - Status labels (EXPERIMENTAL, REFERENCE-VALIDATED, SP1-TESTED, SP1-BENCHMARKED, PRODUCTION) — never claim higher than evidence supports

## Example Workflow: Adding a New Audio Filter

1. **Read `audio-model.md`**: Confirm 48 kHz, stereo, 256-frame blocks, Q16 fixed-point
2. **Check `realtime.md`**: Filter runs on audio thread, no eMMC access, bounded execution
3. **Use `state.md`**: Define IIR state (per-channel or shared), initialization values
4. **Use `parameters.md`**: Cutoff frequency — control-rate or block-rate? Smooth on change?
5. **Use `numeric.md`**: Implement in Q16, validate against float reference (scipy.signal) on test signals
6. **Use `cpu.md`**: Estimate cycles per sample, measure on real hardware if available
7. **Use `memory.md`**: Size coefficients and state, confirm RAM budget OK
8. **Use `optimization.md`**: Is it a bottleneck? If not, keep readable. If yes, consider LUT for expensive operations
9. **Use `validation.md`**: Test against reference output, confirm bit-identical or error < 0.1%, flash to real SP-1 and listen

## Key Principles

- **Measure first.** Do not optimize code that merely "looks expensive." Identify the actual bottleneck on real hardware.
- **Preserve existing DSP.** When adapting proven implementations (reverb, codecs), preserve algorithm, scaling, saturation, rounding. Change only what the new ownership boundary requires.
- **Q16 is non-negotiable.** Audio-path DSP on SP-1 uses fixed-point, not floating-point, even though the Cortex-M4F has an FPU.
- **Audio never accesses eMMC.** The streamer is the sole eMMC owner. Audio operates on in-RAM ring buffers.
- **Explicit over implicit.** Every parameter, state variable, and execution path has documented behavior. No hidden globals, no vague "should be fast."
- **Validation before declaration.** Compilation and plausible sound are not proof. Test against reference, measure error, benchmark on hardware, listen for artifacts.

## Sourcing

- **Hardware and threading**: nRF52840, Zephyr-based firmware, I2S audio transport, 256-frame blocks
- **Constraints and non-negotiables**: The firmware's documented real-time, storage, and DSP invariants
- **Proven implementations**: Existing firmware DSP and codec implementations, when available
- **Measurement discipline**: Distinction between THEORETICAL ESTIMATE, DESKTOP BENCHMARK, COMPILED SP-1, HARDWARE-MEASURED

## About This Skill

This skill is **not** a generic DSP textbook. It is **not** the accumulated DSP implementation library.

Its job is to teach an AI (or a human) to reason about DSP design on constrained, real-time hardware; to respect the actual architecture and invariants of the SP-1 firmware; and to validate work before claiming it is correct.

### Status

All reference files are established frameworks (`REFERENCE` status) with examples from the tape-looper codebase. They will grow with actual engineering work: the first time you validate a filter against a reference, tune a lookup table, or measure CPU on real hardware, add those results to the appropriate reference file so the next person (or future you) has concrete precedent to work from.

Reference files begin as structured placeholders and should grow from actual engineering work rather than speculative DSP encyclopedias.
