# DSP State

## State Categories

- **Persistent**: Survives reset, session save/load (e.g., reverb tail in shared buffer, tape reels position)
- **Temporary**: Ephemeral per-block or per-sample (e.g., intermediate filter state, accumulator)
- **Per-channel**: Duplicated per stereo channel if the component is multichannel (e.g., delay line feedback per channel)
- **Shared**: One instance across all channels (e.g., shared reverb buffer, common LUT)
- **Initialization**: Set once at startup; may be preserved across reset or recomputed
- **Reset-related**: Behavior when reset is triggered; what clears, what persists, what discontinuity results
- **Bypass-related**: What happens when audio is bypassed; does state freeze, continue, or reset?
- **Parameter-transition state**: State required to change parameters smoothly (e.g., slew accumulators, crossfade state)
- **Sample-rate-transition state**: State required if sample rate changes (unlikely in SP-1, but document if relevant)

## Design Rules

- **Explicit ownership**: Every piece of state belongs to exactly one component/thread. Do not hide global mutable state. If state is shared (e.g., a ring buffer accessed by audio and streamer threads), document the contract (producer/consumer, atomicity, ordering).
- **Deterministic initialization**: State has a defined reset value, not garbage or "whatever was there before." Initialize in the component's `_init()` function, not implicitly.
- **No hidden global state**: Avoid `static` variables whose lifetime and ownership are unclear. If a component must have persistent state, make it a struct field or a parameter.
- **Define bypass semantics**: If the component can be bypassed, is output muted, passed through unchanged, or does internal state continue to evolve?
- **Define reset semantics**: Does reset clear state, preserve history, or depend on context?

## Preservation Rule

When adapting existing, proven DSP (e.g., reverb from Phase 2b):

- Preserve the state struct and initialization.
- Preserve the state layout and sizing (for cache/RAM efficiency).
- Preserve the lifetime and owner.
- Change state design only when a new ownership boundary requires it (e.g., promoting `.inc` to `.c`/`.h` module might require exposing formerly-private state via a public API).

## Example: Ring Buffer State

The tape-looper streamer ring exemplifies shared state design:

- **Owner**: `streamer.inc` / future `streamer.c`
- **State**: ring write/read pointers, sample count, aligned start address
- **Contract**: defined in `storage/ring_contract.h` (producer-consumer rules, atom ordering, no types/code)
- **Access from audio thread**: audio reads via a narrow API (`streamer_thread_handle()`, not raw globals)
- **Access from streamer thread**: sole owner, full access

This design preserves the audio-never-accesses-eMMC invariant while allowing audio to read play head via a volatile read.

Status: REFERENCE (design patterns documented; SP-1 tape-looper examples)
