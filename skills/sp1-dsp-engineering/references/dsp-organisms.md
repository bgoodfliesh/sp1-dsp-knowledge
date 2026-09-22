# Stateful DSP systems

## Principle

A small deterministic computational system can produce disproportionately rich
behavior when input, state, nonlinear transformation, feedback, and parameter
movement interact. A useful abstract shape is:

```text
INPUT
  ↓
STATE / MEMORY
  ↓
TRANSFORM
  ↓
OUTPUT
  ↑
FEEDBACK
```

Not every system needs every stage. Examples include one-sample feedback,
integrators, short delays, resonators, shift registers, coupled state
variables, and nonlinear recursive maps.

Before adding independent DSP blocks, consider whether a smaller stateful
structure can produce the desired behavior with less memory and simpler
control. This is a design option, not an absolute rule: state also introduces
history dependence and failure modes.

## Engineering record

Every stateful system should specify:

- state ownership, size, initialization, reset, and bypass behavior;
- numerical range, rounding, saturation, and overflow behavior;
- feedback stability and bounded operating region;
- transient behavior, long-term drift, and parameter sensitivity;
- update rate and whether parameter movement is smoothed;
- reproducibility, seed/initial-state requirements, and failure recovery;
- CPU, RAM, latency, and stack costs with their evidence class.

For nonlinear feedback, test silence, impulse, sine, noise, sustained input,
parameter sweeps, startup from multiple states, and recovery after saturation.
Record instability, self-oscillation, mode locking, saturation lock, and
numerical collapse as observations—not automatically as desirable behavior.

## Stateful complexity vocabulary

Do not conflate these categories:

| Behavior | Meaning |
| --- | --- |
| Random | Driven by an external random source |
| Pseudo-random | Deterministic sequence with an intended long period |
| Chaotic | Deterministic state evolution with sensitive parameter/state behavior |
| Correlated wandering | Random or complex values shaped over time |
| Noise-like | A perceptual description, not a statistical proof |
| Repeating pattern | Deterministic behavior with an observed finite period |

Finite-precision systems can exhibit short cycles, dead states, fixed points,
overflow, or coefficient-dependent collapse. For recursive maps, run a fixed
number of samples and detect repetition where practical; record cycle length,
dead seeds, state distribution, and parameter sensitivity.

Status: `REFERENCE` for the engineering pattern; individual systems remain
`EXPERIMENTAL` until their behavior and costs are measured.
