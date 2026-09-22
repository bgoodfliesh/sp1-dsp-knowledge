# Contributing to SP-1 DSP

The goal is to preserve generic, reusable DSP knowledge, not merely add code.
A contribution is complete only when its algorithm, assumptions, provenance,
validation status, and measurable costs are understandable to someone who did
not write it.

This repository is deliberately separate from `sp1-firmware` and
`sp1-dsp-engineering`. Do not move device drivers, product UI, app wiring,
interrupt setup, or firmware release procedures here. Add a generic algorithm
or a clearly bounded reference example, then document the integration
questions for the consuming firmware repository.

## Contribution lifecycle

1. **Idea and research** — record the problem, alternatives, references, and
   licensing constraints in `sources/` or `knowledge/`.
2. **Algorithm selection** — explain why the selected approach fits SP-1 and
   identify sample-rate, numerical, latency, and resource assumptions.
3. **Implementation** — keep components small and composable. Add an
   engineering record based on `tools/templates/implementation/`.
4. **Reference validation** — compare against a trusted mathematical or
   independent reference before claiming SP-1 integration.
5. **Consumer integration** — document optional SP-1 adaptations, build
   assumptions, and target interfaces without importing firmware ownership into
   this repository.
6. **Hardware testing and benchmarking** — use the standard templates and
   record reproducible conditions, not unexplained CPU percentages.
7. **Documentation** — update indexes and status as evidence improves.

## Source and licensing rules

Do not copy upstream modules wholesale. For each source, record the canonical
URL, project, pinned revision, license, and exactly what was learned or reused.
If copying is not permitted, retain links and technical notes only. Derived
code must preserve required notices and clearly identify its relationship to
the source.

## Required engineering record

An implementation should document purpose, algorithm, provenance, license,
sample-rate assumptions, inputs and outputs, state, parameters, latency, CPU
and RAM cost, numerical considerations, known artifacts, SP-1 adaptations,
validation status, and hardware benchmark status. Use explicit `TBD` values
when evidence is not yet available; do not infer or invent measurements.

## Status discipline

Use only the statuses in `knowledge/status-definitions.md`. Move a component
forward only when the corresponding evidence exists. A passing compile or
unit test does not imply hardware validation or production readiness.

## Tests and benchmarks

Tests must state what they prove and include deterministic inputs where
practical. Benchmarks must include firmware commit, implementation version,
sample rate, block size, channel count, compiler/toolchain, optimization,
measurement method, RAM, artifacts, and hardware configuration.

## AI-assisted changes

AI-generated work follows the same evidence requirements as any other change.
Ask the repository before implementing a new component, prefer existing
validated primitives, preserve provenance, and document uncertainty and failed
approaches. Never upgrade a status to make an incomplete implementation appear
finished. If an app needs firmware-specific glue, keep that glue out of this
repository and record only the reusable DSP boundary.
