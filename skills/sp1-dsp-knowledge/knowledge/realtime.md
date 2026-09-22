# Real-time constraints

Record constraints that affect correctness in the audio callback: bounded
execution, no allocation or blocking, denormal handling, reset behavior,
parameter smoothing, underrun policy, and instrumentation overhead.

All numerical limits and timing budgets must be measured or explicitly marked
`TBD`; do not treat desktop behavior as evidence of SP-1 real-time safety.

Research leads on deterministic embedded architectures are cataloged in
`sources/research-papers.md`. A proposed no-RTOS or low-overhead design must
still specify interrupt boundaries, buffering, synchronization, bounded work,
and failure behavior.

See the Bencina research record in `sources/research-papers.yaml` for the
audio-callback constraints and the evidence required before adopting an SPSC
control/event handoff.
