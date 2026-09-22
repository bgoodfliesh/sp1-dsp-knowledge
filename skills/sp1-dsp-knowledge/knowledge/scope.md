# Repository scope

## What belongs here

- Generic lightweight DSP algorithms and effects
- Small, understandable reference implementations
- Provenance and license records
- Reference equations and implementation notes
- Deterministic tests and golden-vector comparisons
- Reproducible CPU, RAM, latency, and artifact measurements
- Known failure modes and rejected approaches
- Optional notes describing how a consumer such as SP-1 firmware may adapt a component

## What does not belong here

- Board drivers, codec/DMA code, interrupt handlers, or device startup
- Product UI, app state, preset management, or firmware release logic
- Device-specific build systems or private hardware assumptions
- Large copied upstream repositories
- Unexplained generated code
- One-off abstractions whose only purpose is to connect a single app

## Relationship to companion repositories

| Repository | Owns |
| --- | --- |
| `sp1-dsp` | Reusable DSP knowledge, examples, evidence, and provenance |
| `sp1-firmware` | Device integration, audio graph, drivers, product behavior, and target builds |
| `sp1-dsp-engineering` | Operating manual, workflows, review gates, and procedures |

If a change requires all three repositories, land the generic DSP record here
first, then implement the consumer-specific integration in the firmware
repository using the engineering repository's process.

## Lightweight means explicit

“Lightweight” is not a claim that a component is fast on every target. Record
the algorithmic complexity, state and scratch memory, expected processing
granularity, latency, allocation behavior, and measured conditions. A small
source file can still hide expensive searches, cache behavior, numerical
instability, or unbounded work.
