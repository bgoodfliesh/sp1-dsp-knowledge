# SP-1 Storage System

## eMMC Hardware

**VERIFIED** (see hardware.md for pin assignments)

```
Device:     Toshiba THGBMNG5 4 GB eMMC
Interface:  1-bit SPI-style (not standard eMMC mode)
Frequency:  up to 32 MHz
CRC:        hardware
Controller: SPIM3
```

Pins:
```
CLK  = P0.06
CMD  = P0.08
DAT0 = P0.07
RST  = P1.08
VCCQ = P0.14
```

## Storage Ownership

**CRITICAL**: eMMC is owned by a dedicated **streamer thread**.

Only the streamer thread should:
- Initiate eMMC reads/writes
- Manage eMMC state
- Handle eMMC errors
- Perform initialization/reset

Audio thread must **never** access eMMC directly. Other modules should not compete for the bus.

## Ring Buffer Architecture

**VERIFIED** from the firmware storage design

### Ring Size

```
RING_SAMPLES = 8192 samples per direction
RRING_SAMPLES = 8192 samples per direction (recording ring)
```

Sample size: typically 16-bit or 24-bit (depends on stem format).

In bytes:
```
8192 samples × 2 channels × 2 bytes = 32,768 bytes (play ring)
8192 samples × 2 channels × 2 bytes = 32,768 bytes (record ring)
```

### Ring Contract

**VERIFIED** by the storage ring contract

```
Producer: Streamer thread (fills from eMMC)
Consumer: Audio thread (reads samples)
```

Producer-consumer contract includes:
- Write pointer atomicity
- Read pointer atomicity
- Ordering guarantees
- Overflow/underflow handling

Pre-allocation:
```
static int16_t ring[8192];           // play ring (stereo interleaved)
static int16_t rring[8192];          // record ring
static volatile uint16_t write_pos;  // streamer updates
static volatile uint16_t read_pos;   // audio reads
```

Do **not** use dynamic allocation for ring buffers. Buffers must be pre-allocated and deterministic.

### Streamer Responsibilities

Streamer thread:

1. **Reads eMMC sectors** (8192 bytes per sector, ≈ 340 audio frames)
2. **Unpacks audio frames** (handles unconventional byte layout; see audio.md)
3. **Decodes/decompresses** if necessary (codec-specific)
4. **Writes to ring buffer** (manages write pointer, avoids overflow)
5. **Reports state** (current position, availability, errors)
6. **Handles errors** (read failures, CRC errors, recovery)

### Audio Thread Responsibilities

Audio thread:

1. **Reads from ring buffer** via safe API (never direct pointer access)
2. **Manages read pointer** (consuming samples deterministically)
3. **Detects underruns** (ring emptied before streamer refills)
4. **Processes audio** (effects, mixing, tape effects)
5. **Signals streamer** (via volatile mailbox if position feedback needed)

## Sector Layout

**VERIFIED**

Physical eMMC sector: 8192 bytes

```
Offset      Size    Purpose
0–2039      2040 B  Audio frames (≈340 frames × 6 bytes/frame)
2040–2041   2 B     Timing/synchronization info
2042–2043   2 B     Tempo
2044–2047   4 B     LED information
```

Audio frame format: see `audio.md`.

## Recording Format

**INFERRED** (should match playback format)

Recording stores audio in the same tape stem format:
- 24-bit or 16-bit samples (context-dependent)
- 48 kHz sample rate
- 8 channels (4 stereo stems)
- Unconventional byte layout (must be preserved exactly)
- Same sector structure

Do not invent recording format differences without evidence.

## Storage Path: Play

Conceptually:

```
1. Application: "play file X from position Y"
2. Streamer: reads eMMC sectors starting at position Y
3. Streamer: unpacks audio frames with unconventional byte layout
4. Streamer: writes unpacked stereo samples to ring buffer
5. Audio: reads from ring, applies effects, outputs I2S
6. Codec: outputs to headphones/speaker
```

Streamer must keep ring buffer filled to prevent underruns.

## Storage Path: Record

Conceptually:

```
1. Audio: receives I2S input or internal loop mix
2. Audio: writes to record ring buffer
3. Streamer: reads from record ring (managed by separate pointer)
4. Streamer: packs samples into audio frame format
5. Streamer: writes audio frames to eMMC sectors
6. Application: saves completed recording to file
```

Record buffer management must account for:
- Streamer latency in consuming record ring
- Audio input rate (48 kHz, deterministic)
- Scheduler jitter
- eMMC write latency (UNMEASURED)

## Buffering and Latency

### Play Buffering

Required to prevent underruns:

```
eMMC read latency
   + scheduler jitter
   + audio consumption rate
   = minimum ring buffer depth
```

**UNMEASURED** on real hardware.

Typical embedded systems require 50–100 ms of buffering for storage-based playback. SP-1 likely similar, but measure to confirm.

Current ring size (8192 samples @ 48 kHz ≈ 170 ms) should provide adequate margin if streamer has consistent priority.

### Record Buffering

Record ring prevents:
- Audio glitches if streamer is delayed
- Lost samples if streamer cannot keep up with audio input

Same buffering strategy applies.

### Latency-Critical Operations

Do **not** introduce large buffering on critical paths:
- Tape speed changes
- Effect parameter updates
- Playback position changes

These should be low-latency where possible, or explicitly documented if delayed.

## Streaming Priorities

**CRITICAL**: Audio callback > storage convenience > UI convenience

Audio is hard real-time. Storage is best-effort with buffering.

If storage and audio both need CPU simultaneously:
- Audio always wins (cannot be delayed)
- Storage defers to next scheduler opportunity
- Ring buffer absorbs latency

Interrupt priorities should reflect this:
- Audio I2S: very high
- Storage/streamer: high but not preempting audio
- Bluetooth/MIDI: lower
- UI/logging: lowest

Do not invent exact priority values without hardware evidence. The principle is clear; the implementation depends on Zephyr configuration and total firmware load.

## Error Handling

### eMMC Read Errors

Possible scenarios:
- Sector CRC failure (hardware CRC reports error)
- Timeout (device does not respond)
- Position out of bounds (seek beyond file end)
- Uninitialized device

Recovery:
- Log error (with position, sector, CRC status)
- Pause playback
- Return to safe state
- Wait for user intervention or retry

Do **not** silently skip bad sectors. The user should be aware.

### Ring Underrun

If audio thread consumes samples faster than streamer refills:
- Output zeroes (silence) for missed samples
- Log underrun event
- Increase ring buffer size if consistent (re-compile firmware)
- Investigate streamer latency if occasional

### Ring Overflow (Record)

If streamer cannot consume record ring fast enough:
- Drop oldest samples (oldest overwritten)
- Log overflow event
- Investigate eMMC write latency
- Check streamer thread priority

## Storage Configuration

**INFERRED** (generation size TBD)

Configuration should specify:
- File system type (if applicable)
- Sector allocation
- File layout (tape stems, metadata)
- Maximum file size
- Session/project structure

Configuration should be separable from implementation. Use YAML or binary format to define storage layout.

## Boot and Storage State

During firmware boot:

1. Initialize eMMC (reset, configure, detect)
2. Verify eMMC is responsive
3. Load configuration / file metadata
4. Prepare streamer thread
5. Do **not** allocate ring buffers in bootloader (keep minimal)

On shutdown:

1. Stop audio path (may flush ring)
2. Stop streamer thread (no new eMMC access)
3. Flush any pending writes
4. Power down eMMC (VCCQ control)
5. Clear streamer state

## Known Limitations

- eMMC is SPI-style 1-bit interface (slower than standard eMMC or modern faster buses)
- 4 GB capacity (fixed; no expansion)
- Ring buffer sizes fixed at compile-time (not dynamically configurable)
- No TRIM/garbage collection currently used (storage may fragment over time)

## Unverified

```yaml
unknown:
  - exact_eMMC_initialization_sequence
  - exact_eMMC_read_latency_distribution
  - exact_eMMC_write_latency_distribution
  - fragment_handling_and_optimization
  - power_consumption_of_eMMC_active_vs_idle
  - thermal_behavior_during_sustained_read_write
  - worst_case_sector_error_rate
  - ring_buffer_size_adequacy_without_measurement
```

Record discoveries with measurement methodology and source.

## Storage Checklist

When initializing storage:

- [ ] eMMC hardware responds (reset sequence, device detect)
- [ ] eMMC CRC operational (hardware reports errors)
- [ ] First sector readable (basic I/O test)
- [ ] Streamer thread created and running
- [ ] Ring buffers allocated (static, pre-compiled sizes)
- [ ] Streamer reads first block successfully
- [ ] Audio reads from ring successfully
- [ ] No glitches during playback from eMMC
- [ ] Record path tested (ring→streamer→eMMC)
- [ ] Underrun/overflow handled gracefully
- [ ] Shutdown sequence clears state properly
