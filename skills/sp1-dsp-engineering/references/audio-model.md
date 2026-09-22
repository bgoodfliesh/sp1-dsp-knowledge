# SP-1 Audio Model

## Verified Hardware Context

- **Sample rate**: 48 kHz (working assumption; I2S transport confirms audio rate, specific value to be verified from device firmware)
- **Channels**: Stereo DSP path (confirmed in REFACTOR_NOTES.md)
- **Transport**: I2S, 256-frame audio blocks
- **Firmware**: Zephyr-based (4.3.1 as of baseline)
- **Threading**: Audio thread (I2S, highest app priority, RAM-only); separate streamer/eMMC/MIDI/main threads
- **Execution**: Real-time embedded, no dynamic allocation in audio path

## Non-Negotiable Audio-Path Constraint

**Audio DSP uses Q16 fixed-point arithmetic.** From REFACTOR_PLAN.md: *"There is no float on the audio path. Speed is already Q16."*

When implementing or adapting DSP:
- Preserve Q16 scaling and saturation.
- Document bit width, rounding, and overflow behavior.
- If a reference implementation uses floating-point, establish the fixed-point equivalent with validated error bounds.
- Do not introduce floating-point math on the audio thread, even though the Cortex-M4F has an FPU.

## Required Per-Algorithm Definition

Every DSP component must explicitly document:

- **Sample rate** — assume 48 kHz unless proven otherwise; document if rate is variable
- **Block size** — explicit (e.g., 256-frame blocks); do not assume undocumented sizes
- **Channels** — stereo expected; document per-channel state vs shared state
- **Numerical format** — Q16 fixed-point with bit width, rounding, saturation strategy
- **Input/output format** — Q16 range, initialization, discontinuities
- **Latency** — algorithmic, buffering, filter group delay (if any); do not assume zero
- **State** — persistent, temporary, per-channel, shared, initialization values
- **Parameter domains** — when do parameters update (initialization, control-rate, block-rate, audio-rate)?
- **CPU cost** — theoretical estimate or measured (label appropriately)
- **RAM cost** — state size, temporary buffers, no dynamic allocation in audio path
- **Reset behavior** — state on reset, bypass behavior, edge cases

Status: REFERENCE (audio model); Q16 constraint verified from REFACTOR_PLAN.md; sample rate TBD (likely 48 kHz, see cpumd for measurement discipline)
