# SP-1 DSP Knowledge

Accumulated, reusable DSP knowledge and implementation reference repository for
the SP-1 ecosystem.

This repository is institutional memory, not primarily an AI skill and not a
copy of the SP-1 firmware. It collects generic lightweight DSP algorithms,
proven examples, reference implementations, provenance, tests, benchmarks,
implementation notes, and known failures that can speed up new firmware work.
Device-specific integration belongs in `sp1-firmware`; operating procedures
belong in `sp1-dsp-engineering`.

## Scope boundary

This repository answers: **what DSP approach exists, how does it work, what
are its tradeoffs, can we trust the example, and what would need adapting?**

It does not own the SP-1 audio graph, board drivers, product UI, interrupt
configuration, release process, or device-specific build contract. A component
may include an optional SP-1 adaptation note, but it must remain understandable
and useful as generic DSP knowledge.

## Repository flow

```text
sources/       upstream research and provenance
      ↓
knowledge/     algorithm understanding and selection notes
      ↓
algorithms/    reusable, lightweight algorithm implementations
effects/       composed effect implementations and examples
      ↓
tests/         correctness and regression evidence
      ↓
benchmarks/    reproducible performance measurements
```

Upstream source is not copied here by default. Record links, pinned revisions,
licenses, and technical notes in `sources/`; vendor or submodule code only when
licensing and maintenance requirements are explicit.

## Status and confidence

Every component must declare a status from `knowledge/status-definitions.md`.
Status is evidence-based: compilation alone is not validation. In particular,
`EXPERIMENTAL` must never be presented as equivalent to hardware-validated
code. Provenance must also identify whether the work is an original algorithm,
adaptation, derivative implementation, or independent reimplementation.

## Finding existing work

Use the metadata in each component's `provenance.yaml` and overview document
to answer:

- whether a resonant filter or granular delay already exists;
- which upstream algorithms have been adapted;
- what sample-rate, CPU, RAM, and latency assumptions apply;
- whether an implementation has been tested or benchmarked on SP-1 hardware;
- what artifacts and failure modes are already known.

## Adding a component

Start with `tools/templates/implementation/`, copy only the files needed by
the component, and complete the engineering record before calling the work
complete. Use `tools/templates/benchmark.md` for every measured result and
`tools/templates/validation.md` for reference, integration, and hardware
evidence. Read `CONTRIBUTING.md` before opening a change.

Source entries begin with `sources/source-record-template.yaml`; test and
benchmark records have machine-readable templates in `tests/` and
`benchmarks/`. These templates are deliberately explicit so an AI agent can
query status, provenance, and evidence without reconstructing it from prose.

## Avoiding spaghetti

Prefer small components with explicit state, bounded processing, stable
interfaces, and reproducible evidence. Do not add firmware wiring, hidden
global state, copied upstream modules, or one-off app-specific abstractions to
make an example work. When a component needs device integration, document the
boundary and leave the integration to the firmware repository.

## Experimental knowledge

The candidate queue in
[`knowledge/experiments.md`](knowledge/experiments.md) is intentionally
hypothesis-driven. It separates reusable questions from verified components
such as CloudVerb. Failure regions, finite-precision cycles, and unsupported
performance claims should be recorded rather than silently promoted.

## Current scope

This is an initial architecture skeleton. It intentionally contains no large
DSP implementation or copied upstream source. Its first useful contents should
be concise research records and proven reference examples, not a second
firmware repository.
