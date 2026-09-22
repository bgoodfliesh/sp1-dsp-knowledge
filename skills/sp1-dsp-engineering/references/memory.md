# Memory Engineering

## Scope

RAM, flash, buffers, state, lookup tables, and memory lifetime on the nRF52840 with constrained audio + firmware overhead.

The nRF52840 has 256 KB RAM, shared by:
- Zephyr kernel + threading
- Audio thread + DSP state
- Streamer thread + ring buffers
- MIDI, UI, control, debug
- Flash and EEPROM metadata

Audio-path components cannot dynamically allocate and compete for a finite pool.

## Design-Time Checklist

For each audio DSP component:

- [ ] **Persistent state sized** — reverb tails, delay buffers, coefficient history; measured in bytes
- [ ] **Temporary state sized** — accumulators, interpolation state, per-block scratch; bounded and pre-allocated
- [ ] **Per-channel state identified** — if stereo, is state duplicated per channel or shared?
- [ ] **Shared state identified** — overlapping buffers (e.g., echo + reverb on one 4608-sample line per REFACTOR_PLAN.md), lookup tables, coefficients
- [ ] **Lookup table cost identified** — table size, precision, update frequency (static or dynamic recomputation)
- [ ] **Buffer lifetime identified** — does buffer exist for the lifetime of the component, for a session, or per-block?
- [ ] **Stack requirements identified** — how much stack does the audio function use? Does it call other functions? (audio thread is RAM-only; stack space is limited)
- [ ] **Flash cost identified** — code size, read-only data (e.g., coefficient tables); measured via `-Os` build
- [ ] **No dynamic allocation** — all memory is pre-allocated or on stack; no `malloc()` in audio path

## Known Constraints from REFACTOR_PLAN.md

- **Shared reverb/echo buffer**: 4,608 samples (9,216 B), 16-bit samples, shared between reverb and echo effects; a second delay line does not fit the original budget without dropping something else (see "Rejected shapes")
- **Ring buffers**: RING_SAMPLES = RRING_SAMPLES = 8192 per direction, sized for play/rec buffering; do not retune alignment or size as refactor work
- **Streamer thread alignment**: 2048-byte alignment (`__aligned(2048)`) for DMA efficiency; do not change
- **No float on audio path**: Q16 fixed-point; saves memory compared to 32-bit float per sample, but requires explicit scaling and saturation

## Example: Reverb State (Phase 2b)

From `REFACTOR_PLAN.md`:

```
Reverb struct:
  - 5 state fields × 32-bit = 20 B
  - (shared 4608-sample echo/reverb line, not owned by reverb)
  
Total reverb RAM: ~40 B (expected, Phase 2b)
Expected change from float: replace g_rv_w/lp/pl/pr/live (~24 B) with Reverb struct (~40 B) = net +16 B
```

This is acceptable because the old code had persistent state anyway; the refactor just isolated it.

## Measurement Discipline

**Do not invent available RAM or flash budgets.** Use:

- Actual measurements from the compiled binary (`west build` output, ELF sections)
- Zephyr-reported RAM/stack usage (if profiling tools available)
- Hardware-measured peak usage (if device is available)

Until a value is measured, mark it `UNMEASURED`. If you estimate, label it clearly: `(estimate: ~500 B for delay line)`.

## Optimization Priority

1. Remove unnecessary state (do not preserve history unless required).
2. Reuse buffers (e.g., shared reverb/echo line).
3. Compress representation (e.g., 16-bit samples vs 32-bit, Q16 vs float).
4. Reduce buffer size where sonic requirements allow (e.g., is a 4096-sample delay line sufficient, or do you need 8192?).
5. Precompute lookup tables (one-time cost, fast runtime access).

Status: UNMEASURED (RAM/flash allocation not profiled in this environment; consult `west build` artifact or real device for actual memory usage)
