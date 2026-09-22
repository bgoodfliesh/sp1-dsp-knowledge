# SP-1 DSP Engineering

## Purpose

This skill is the engineering knowledge layer for developing DSP appropriate to the Teenage Engineering SP-1 firmware.

It is **not** a generic DSP textbook and it is **not** the accumulated DSP implementation library. Its job is to teach an AI to turn DSP algorithms, research, reference implementations, and sonic requirements into efficient, maintainable implementations appropriate for the SP-1's actual constraints.

The primary goal is to prevent **vibe-coded DSP**: plausible-looking code that has not been reasoned about for real-time execution, numerical behavior, memory, CPU usage, latency, state, or sonic correctness.

## Sourcing Constraints

Constraints documented here come from:
- **Tape-looper firmware source** (`sp1-tape-looper` repo, `REFACTOR_PLAN.md` / `REFACTOR_NOTES.md`)
- **Hardware design** (Teenage Engineering SP-1 device)
- **Measured or compiled evidence** (when available; labeled as such)

Consult the tape-looper repo's `REFACTOR_PLAN.md` (especially "Non-negotiable invariants" and "Rejected shapes" sections) before proposing architecture or algorithm changes. A shape already rejected there should not be re-proposed.

The separate `sp1-dsp` repository contains validated algorithms, implementations, tests, benchmarks, provenance, and accumulated DSP knowledge independent of the firmware. Consult it before creating new implementations.

## Verified SP-1 Hardware Context

The following may be treated as verified hardware facts:

- **Processor**: Nordic nRF52840, ARM Cortex-M4F (has FPU, but see "Fixed-point constraint" below)
- **Firmware**: Zephyr-based (4.3.1 as of Phase 0)
- **Audio transport**: I2S, 256-frame audio blocks to the DSP
- **Audio system**: Stereo path (confirmed in REFACTOR_NOTES.md)
- **Threading model**: audio thread (I2S, highest app priority, RAM-only), streamer (eMMC sole owner), MIDI, main/control threads
- **Constraints**: Real-time embedded execution, no dynamic allocation in audio path, constrained CPU and RAM alongside storage/controls/MIDI/UI

**Do not invent additional performance characteristics.** If data has not been measured or established by a trusted source, label it `UNMEASURED`.

## Critical Audio-Path Constraint: Fixed-Point Arithmetic

The SP-1 firmware's audio DSP uses **Q16 fixed-point arithmetic by design**, not floating-point.

From `REFACTOR_PLAN.md` ("Rejected shapes"): *"There is no float on the audio path. Speed is already Q16."*

**Implication**: Even though the Cortex-M4F has an FPU, do not introduce floating-point math on the audio path. Q16 fixed-point is a non-negotiable invariant. Existing DSP implementations (reverb, codecs, tape effects) preserve this representation.

When adapting existing DSP or proposing new DSP:
- Preserve fixed-point representation and scaling.
- If floating-point reference code exists, establish fixed-point equivalent before implementation.
- Document the scaling, saturation, and rounding strategy.
- Validate against floating-point reference to confirm bit-identical or acceptable error bounds.

## Audio-Path Invariants (Non-negotiable)

From `REFACTOR_PLAN.md`:

- Audio path stays deterministic and non-blocking.
- Audio **never** accesses eMMC (streamer is the sole eMMC owner).
- Storage layout (`meta_blk`, `x3_tab`, `grid_ext`) stays compatible with existing saved sessions.
- DSP constants, buffer sizes, and timing remain stable — do not change as "refactor" work.
- Watchdog and Track 1+4 bootloader recovery stay intact.
- Code remains idiomatic C; no C++, no framework.

## Measurement Discipline

Always distinguish between:

- **Theoretical estimate** — calculation or modeling, never on hardware
- **Desktop benchmark** — measured on development machine, not SP-1
- **Simulated embedded benchmark** — emulated or simulated nRF52840, not real hardware
- **Compiled SP-1 implementation** — built with `west build`, but not flashed/measured on actual device
- **Hardware-measured SP-1 performance** — collected from running code on real device

**Never collapse these into a single performance claim.**

If CPU, RAM, latency, instruction-count, throughput, or similar performance data has not been measured or established by a trusted source, label it:

`UNMEASURED`

## Core Engineering Rules

DSP implementations for the SP-1 should:

1. **Real-time safe** — no blocking, no unbounded work, no dynamic allocation in audio path
2. **Deterministic** — predictably bounded execution, especially on the audio thread
3. **Explicit about state** — ownership, initialization, lifetime, per-channel vs shared
4. **Numerically intentional** — Q16 fixed-point with documented scaling, saturation, and rounding; not "plausible"
5. **Audio-path aware** — respect the streamer/eMMC boundary; audio never accesses eMMC
6. **Resource-aware** — CPU, RAM, flash budgets measured or estimated; not invented
7. **Block-size aware** — explicit about block size (e.g., 256-frame audio blocks); never assume undocumented sizes
8. **Parameter-deliberate** — document when parameters change (initialization, control rate, block rate, audio rate) and why
9. **Reset and bypass explicit** — define what bypass means, reset behavior, edge cases, failure modes
10. **Measured before optimization** — identify the actual bottleneck; do not optimize what merely looks expensive
11. **Validated before declared correct** — compilation and plausible sound are not proof
12. **Conservative with existing implementations** — preserve proven DSP; understand before changing

**Do not** optimize for code elegance. Do not rewrite functioning DSP solely to look cleaner—subtle behavior may be part of the sound.

### Workflow for Adapting Existing DSP

**First preserve. Then isolate. Then adapt. Then optimize.**

When working with proven implementations (e.g., reverb, codecs already in the firmware):

1. Preserve the existing algorithm, scaling, saturation, and rounding.
2. Isolate the component in its own boundary (header-only in `.inc` phase, or eventual `.c`/`.h` module).
3. Adapt only what is necessary for the new ownership/API boundary.
4. Optimize only when a bottleneck is identified on real hardware.

Avoid simultaneously changing algorithm, numerical representation, state design, CPU optimization, and parameter behavior unless there is a specific reason to do all at once. Each change should be isolable so that sonic regressions can be identified.

## Shapes Already Rejected

The tape-looper refactor (`REFACTOR_PLAN.md`, "Rejected shapes" section) has already evaluated and rejected the following approaches for this firmware. Do not re-propose them:

- **Hermite interpolation for tape speed** — the existing rocker uses slewed linear interpolation (`posb`/`fracb`). Hermite is a sound change; existing behavior is intentional.
- **Floating-point on audio path** — Q16 fixed-point is non-negotiable. The FPU exists but is not used for sample-rate DSP.
- **`TapeHead` objects + RAM buffer** — proposed redesign deletes the eMMC streamer. The current architecture (streamer + ring) is preserved.
- **64-sample per-track send FX bus** — FX already run once per 256-frame post-mix block. Echo/reverb share one 4608-sample line (9,216 B shared buffer); a second line does not fit the original budget without dropping something else.
- **Five-state UI FSM** — UI is concurrent per-track + pages + chords, not a sequential PLAYBACK/RECORD/OVERDUB state machine.
- **`k_msgq` in I2S callback** — audio already uses `volatile` mailboxes. Do not introduce Zephyr message queues in the audio callback.
- **Retune ring alignment, refill watermarks, or `__SSAT` as architecture work** — these are performance details, not refactor scope.
- **CMSIS `q15_t` representation** — Q16 is the chosen fixed-point format, not CMSIS's 1.15 `q15_t`.

If you find a reference implementation or algorithm that aligns with one of these rejected shapes, adapt it away from that shape before proposing implementation.

## DSP Implementation Model

Every DSP implementation should conceptually define:

- Inputs
- Outputs
- Sample rate
- Block size
- State
- Parameters
- Parameter update rate
- Latency
- CPU cost
- RAM cost
- Flash cost
- Numerical representation
- Reset behavior
- Bypass behavior
- Failure/edge behavior

Make clear which work occurs:

- at initialization
- when a parameter changes
- at control rate
- once per block
- once per sample

Before moving a calculation to audio rate, ask:

> Does this value actually need to change every sample?

Before moving an operation out of audio rate, ask:

> Does reducing its update rate create audible or numerical problems?

## Control Domain vs Audio Domain

Human controls and audio streams operate at fundamentally different rates.

Prefer:

```text
physical control
      ↓
normalize / quantize
      ↓
parameter mapping
      ↓
DSP coefficient/state
      ↓
audio-rate processing
```

Expensive parameter calculations should preferably occur:

1. Once when a parameter changes.
2. At control/update rate.
3. Once per audio block.
4. At audio rate only when genuinely necessary.

This is especially important for nonlinear mappings.

## Control-Domain Lookup Tables

Lookup tables are a first-class optimization technique when an expensive mapping does not need arbitrary precision.

```text
control value
      ↓
table index
      ↓
precomputed coefficient
      ↓
audio processing
```

Potential strategies:

- direct lookup
- lookup + linear interpolation
- logarithmically spaced tables
- piecewise approximations
- quantized controls mapped to precomputed coefficients
- coefficient recalculation only on parameter change
- coefficient recalculation once per audio block

Table resolution must be driven by perceptual and sonic requirements, not automatically by ADC resolution.

A 12-bit physical control does not necessarily require 4096 unique mathematical evaluations. Likewise, a 48 kHz audio stream does not imply that every parameter mapping must be evaluated 48,000 times per second.

### Slew / Portamento Example

A one-pole slew process can commonly be represented as:

```text
y[n] = y[n-1] + a × (target[n] - y[n-1])
```

where `a` is derived from the desired response time.

If deriving `a` requires an expensive operation such as `exp()`, consider precomputing the mapping:

```text
time control
     ↓
lookup table
     ↓
slew coefficient
     ↓
one multiply + one add per sample
```

The lookup approximates the mathematical mapping while preserving the intended perceptual response. It does not need to reproduce the mathematical function perfectly.

## Perceptual Resolution

When a DSP parameter is human-controlled, ask whether adjacent parameter values are actually perceptible.

High mathematical resolution may provide little sonic benefit when:

- the physical control is noisy
- the parameter is naturally logarithmic
- hearing is relatively insensitive in part of the range
- DSP subsequently smooths the parameter
- another stage already quantizes it
- the control changes slowly

Nonlinear tables may allocate resolution where it matters.

**Spend precision where the user or signal can actually perceive it.**

## Approximation

Approximation is permitted and often desirable on constrained hardware when its error is characterized and appropriate.

Possible candidates:

- `sin`
- `cos`
- `exp`
- `log`
- reciprocal
- square root
- nonlinear transfer functions
- parameter curves
- envelope mappings
- oscillator calculations
- filter coefficient calculations

For nontrivial approximations document:

- original operation
- replacement approximation
- expected error or error bound
- input/parameter range
- whether error is deterministic
- whether error accumulates
- audible consequences
- CPU impact
- RAM/flash impact
- limitations
- validation method

A small error may be irrelevant in a control mapping but highly significant in an audio-rate nonlinear process.

Validate approximations numerically and sonically where appropriate.

## Embedded Efficiency and Historical Optimization

Use the long history of constrained-system optimization as a toolbox: embedded audio, game consoles, trackers, demoscene software, and classic game engines.

Potential techniques include:

- fixed-point or reduced-precision representations
- lookup tables
- interpolation
- precomputed coefficients
- approximate mathematical functions
- reciprocal/inverse approximations
- power-of-two buffer sizes
- masking instead of modulo where appropriate
- efficient circular buffers
- bit-level representations and operations
- block-rate instead of sample-rate computation
- shared calculations
- shared state where mathematically valid
- state compression
- reduced oversampling
- simplified models
- sparse or reduced-rate modulation
- branch reduction when actually beneficial
- memory-layout optimization
- appropriate flash/RAM placement
- DMA/peripheral assistance
- architecture-specific DSP instructions
- numerical techniques to avoid pathological floating-point behavior

These are tools, not commandments.

### Historical Optimization Case Study

Fast inverse square root, popularized by Quake III Arena, is a case study in constrained optimization.

Its lesson is not to copy the exact hack. The lesson is:

1. Identify the expensive operation.
2. Determine the precision actually required.
3. Understand the numerical representation.
4. Construct a cheaper approximation.
5. Measure its error.
6. Determine whether the error matters.
7. Benchmark the real implementation on target hardware.

A technique optimal on another processor is not automatically optimal on the nRF52840.

## Optimization Hierarchy

When DSP is too expensive:

1. Measure the actual bottleneck.
2. Remove unnecessary work.
3. Reduce computation frequency where mathematically valid.
4. Reuse calculations.
5. Choose a more appropriate algorithm.
6. Improve numerical representation.
7. Use lookup/precomputation.
8. Exploit architecture-specific capabilities.
9. Consider lower-level or historical optimization techniques.
10. Re-measure on actual SP-1 hardware.

Do not begin by replacing readable mathematical code with opaque micro-optimizations.

## Optimization Tradeoffs

Every meaningful optimization should consider:

```text
CPU ↓
RAM ↓
FLASH ↓
LATENCY ↓
PRECISION ↓
COMPLEXITY ↑
MAINTAINABILITY ↓
```

Not every optimization improves every resource. Document the actual tradeoff.

Example:

```yaml
optimization:
  technique: lookup_table
  cpu: reduced
  ram: increased
  flash: increased
  precision: slightly_reduced
  latency: unchanged
  measured: false
  status: UNMEASURED
```

Do not claim a measured result until it is benchmarked.

## Maintainability Rule

Clever optimizations must never become unexplained magic.

For unusual approximations, bit manipulation, lookup strategies, precision reductions, architecture-specific optimizations, or numerical shortcuts, document:

- why it exists
- what it replaces
- what resource it saves
- what tradeoff it introduces
- how it was validated

The objective is not clever-looking code.

The objective is to make constrained hardware perform exactly as much work as necessary—and no more.

## Numerical Engineering

Explicitly consider:

- floating-point range
- quantization
- precision
- overflow
- underflow
- denormals/subnormals
- accumulation error
- coefficient stability
- feedback stability
- DC buildup
- NaN/Inf propagation
- initialization
- reset state
- parameter discontinuities

Floating-point support does not make numerical behavior irrelevant.

## Parameter Handling

Every parameter should define:

- range
- units
- default
- scaling
- mapping
- update rate
- smoothing behavior
- valid/invalid states
- reset behavior

Do not automatically smooth everything.

Depending on role, a parameter may need:

- immediate changes
- linear smoothing
- exponential smoothing
- slew limiting
- block interpolation
- zero-crossing changes
- state transitions

Justify the choice.

## State

DSP state must be explicit.

Document:

- persistent state
- temporary state
- per-channel state
- shared state
- initialization state
- reset behavior
- bypass behavior
- state required for parameter changes
- state required for sample-rate changes

Avoid hidden global state. Prefer deterministic initialization.

## Block Processing

DSP should behave correctly for reasonable audio block sizes unless a fixed block size is intentionally required and documented.

Never silently assume:

```text
block_size == X
```

unless that requirement is enforced.

Parameter changes at block boundaries must not create unintended discontinuities.

## Latency

Identify:

- algorithmic latency
- buffering latency
- lookahead
- filter/group delay where relevant
- parameter-dependent latency

Do not assume zero latency.

## Validation

Compilation and plausible sound are not proof of correctness.

Where appropriate validate against:

- mathematical reference
- trusted reference implementation
- offline reference output
- impulse response
- frequency response
- step response
- known test signals
- numerical error measurements
- edge cases
- parameter sweeps
- reset/bypass behavior
- CPU benchmark
- RAM usage
- hardware execution

For approximations compare:

```text
reference mathematics
        vs
approximation
```

and, when applicable:

```text
reference audio
        vs
SP-1 implementation
```

## Research and Existing Implementations

For known DSP techniques, research before reinventing.

Potential sources include:

- academic references
- open-source implementations
- embedded DSP libraries
- Mutable Instruments code
- DaisySP
- Faust
- CMSIS-DSP
- STK
- appropriate repositories
- existing SP-1 implementations

Determine:

- algorithm lineage
- license
- assumptions
- numerical representation
- CPU/RAM characteristics
- sample-rate assumptions
- state model
- portability
- known limitations

Do not blindly copy source code.

Preserve provenance.

## Implementation Preservation

When adapting an existing implementation:

**First preserve. Then isolate. Then adapt. Then optimize.**

Avoid simultaneously changing:

- algorithm
- numerical representation
- state design
- CPU optimization
- parameter behavior
- architecture

unless there is a specific reason.

Separate changes enough that sonic regressions can be identified.

## Engineering Record

Every substantial DSP implementation should have an engineering record:

```yaml
name:
purpose:
category:
inputs:
outputs:
sample_rate:
block_size:
latency:
state:
parameters:
numeric_representation:
cpu_cost:
ram_cost:
flash_cost:
optimization:
  technique:
  rationale:
  tradeoffs:
reference_algorithm:
reference_implementation:
provenance:
validation:
benchmark:
status:
```

Possible statuses:

- `EXPERIMENTAL`
- `REFERENCE`
- `REFERENCE-VALIDATED`
- `SP1-COMPILES`
- `SP1-TESTED`
- `SP1-BENCHMARKED`
- `PRODUCTION`

Never imply a higher status than the evidence supports.

## Anti-Vibe-Coding Workflow

For significant DSP:

```text
desired sonic behavior
        ↓
identify candidate algorithms
        ↓
research/reference existing implementations
        ↓
understand state + parameter behavior
        ↓
estimate CPU/RAM/latency
        ↓
choose numerical representation
        ↓
implement simplest correct version
        ↓
validate against reference
        ↓
benchmark
        ↓
identify actual bottlenecks
        ↓
optimize selectively
        ↓
revalidate
        ↓
document
```

Do not start with optimization.
Do not start by copying code.
Do not invent an algorithm when an established one matches the requirement.

## Repository Relationship

`sp1-dsp-engineering/` is the engineering knowledge layer.

It teaches:

- DSP reasoning
- algorithm evaluation
- implementation adaptation
- optimization
- validation
- documentation

It is not the implementation library.

The separate `sp1-dsp` repository contains validated algorithms, implementations, source references, tests, benchmarks, provenance, and accumulated institutional knowledge.

Consult it before creating new implementations.

## Reference Files: How to Use Them

Each reference file is a checklist, framework, and discipline guide for a specific DSP concern. They work together:

1. **`audio-model.md`** — Start here: understand the hardware (48 kHz, stereo, I2S blocks, Q16 fixed-point requirement). This sets the foundation.

2. **`realtime.md`** — Rules for audio-thread safety: no eMMC access, no dynamic allocation, bounded execution. Check against this for every audio-path function.

3. **`state.md`** — Define ownership and lifetime for every piece of state (persistent, temporary, per-channel, shared). Prevents hidden globals and race conditions.

4. **`parameters.md`** — Document every user-facing or algorithmic parameter: range, mapping, update rate, smoothing, reset behavior. Separates control-rate computation from audio-rate processing.

5. **`numeric.md`** — Establish Q16 scaling, saturation, rounding, and validation. When adapting float reference code, use this to prove fixed-point equivalence.

6. **`cpu.md`** — Estimate and measure CPU cost. Distinguish THEORETICAL ESTIMATE vs HARDWARE-MEASURED; measure actual bottleneck before optimizing.

7. **`memory.md`** — Size all state, buffers, and lookup tables. Account for RAM/flash budget. No dynamic allocation in audio path.

8. **`optimization.md`** — Methodology for making DSP faster: measure bottleneck → reduce frequency → reuse calculations → lookup tables → exploit architecture. Do not optimize blindly.

9. **`validation.md`** — Test against reference implementation, measure numerical error, confirm no clicks/glitches, benchmark on real hardware. Document status (EXPERIMENTAL, REFERENCE-VALIDATED, SP1-TESTED, etc.).

### Workflow Example: Adding a New Filter

1. **Audio model**: Is 48 kHz, stereo, 256-frame blocks correct for this component? Do I need different block sizes?
2. **Real-time**: Does filter run on audio thread? No eMMC access, no malloc, bounded execution—confirmed.
3. **State**: IIR state is per-channel (2 channels) or shared (1 set of coefficients)? Initialize deterministically.
4. **Parameters**: User-exposed cutoff frequency? Control rate or block rate? Linear or log scale? Smooth on change?
5. **Numeric**: Implement in Q16; validate against float reference on test signals (sine sweep, noise).
6. **CPU**: Estimate cycles per sample (e.g., 2 mul + 3 add ≈ 6 cycles). Measure on real hardware if available.
7. **Memory**: Coefficient storage (4 values = 16 B), state (2×2 values = 16 B per channel = 32 B total). RAM budget OK?
8. **Optimization**: Is it the bottleneck? If not, leave readable. If yes, consider lookup table for expensive (exp, sqrt) operations.
9. **Validation**: Run `test_filter` against scipy.signal reference output; confirm bit-identical or error < 0.1%; flash to real SP-1 and listen for artifacts.

### These files begin as structured frameworks and grow from real engineering work

Do not turn them into speculative DSP encyclopedias. Each section should be filled in as you encounter an actual design decision, measurement, or validation result.

Example: The first time you need a lookup table for exp(), add details to `optimization.md` about how you chose table size, validated accuracy, and measured speedup. The next person (or future you) can then reference that example instead of starting from scratch.
