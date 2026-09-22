# Real-Time DSP

## Scope

Rules for code executed in the SP-1 audio path (I2S callback, audio thread).

## Non-Negotiable Rules

From `REFACTOR_PLAN.md` ("Non-negotiable invariants"):

- **Audio never accesses eMMC.** The streamer thread is the sole eMMC owner. Audio operates on in-RAM ring buffers.
- **No filesystem/storage/UI/MIDI operations in audio callback.** Exception: explicit, documented, bounded cross-thread communication via `volatile` mailboxes (not `k_msgq`), existing in Phase 0 architecture.
- **No dynamic allocation in the audio path.** All state, buffers, and coefficients are pre-allocated or stack-based.
- **Deterministic/bounded execution.** No unbounded loops, no variable-depth recursion, no blocking operations.
- **Explicit block-size awareness.** 256-frame audio blocks (@ 48 kHz = ~5.3 ms per block). Document behavior for other block sizes if supported; do not assume undocumented sizes.

## Audit Checklist

For each audio-path component:

- [ ] What executes per sample? (e.g., one multiply-add)
- [ ] What executes per block? (e.g., envelope update, parameter interpolation)
- [ ] What executes only on parameter change? (e.g., filter coefficient recalculation)
- [ ] What is the worst-case path? (total CPU cycles, considering branching, loop depth)
- [ ] Are there any cross-thread accesses? If so, are they via `volatile` mailbox (existing architecture) or Zephyr primitives?
- [ ] Does the implementation read from or depend on external state (eMMC, control state, MIDI state)? If so, is it pre-fetched/cached on the control-rate boundary?
- [ ] Is all state pre-allocated? No `malloc`, no variable-size buffers.
- [ ] Can execution time vary based on data? If so, by how much?

## Cross-Thread Communication Pattern

The audio thread communicates with control/UI/streamer threads via existing `volatile` mailboxes (not Zephyr message queues). Examples in Phase 0 code:

- Audio reads lookahead intent from control thread.
- Audio signals the streamer to prefetch / flush.
- Audio reports play head position to UI.

New audio-path additions should follow this pattern: pre-fetch or pre-compute on the control boundary, pass immutable data to audio thread via `volatile` struct, audio reads and processes. No blocking, no dynamic work on audio thread.

## Measurement

Record target-hardware benchmark results here as they become available. Use the measurement discipline in `cpu.md`: distinguish theoretical estimate, desktop benchmark, compiled SP-1, and hardware-measured results.

Status: UNMEASURED (no hardware benchmarks available in this environment; consult real SP-1 device once `west build` artifact is validated on hardware)
