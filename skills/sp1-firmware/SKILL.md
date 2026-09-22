# SP-1 Firmware Engineering

## Purpose

This is the **machine-truth and firmware-architecture layer** for the Teenage Engineering SP-1 Stem Player.

Its role is to teach an AI what the SP-1 **actually is**—its hardware, peripherals, verified behavior, constraints, and architecture—so that firmware modifications or extensions respect real physical limitations rather than inventing capabilities.

The central principle is:

> **The SP-1 hardware and verified firmware behavior are ground truth. Never substitute assumptions for evidence.**

## What This Is Not

This is **not**:
- A generic Zephyr/embedded-C/Nordic textbook
- Architecture patterns independent of the SP-1
- DSP implementation guidance (that is `sp1-dsp-engineering`)
- A library of DSP implementations (that is `sp1-dsp`)

## What This Is

This **is**:
- A compact engineering reference for what the SP-1 hardware contains
- Pin assignments, I2C addresses, codec sequences, verified formats
- Real-time constraints and ownership rules
- Firmware architecture that respects hardware topology
- The boundary between firmware and DSP layers

## Evidence Model

Every technical statement belongs to one of these categories:

### VERIFIED
- Directly established from hardware inspection
- Reverse engineering and protocol captures
- Working firmware evidence
- Datasheets and authoritative documentation
- Hardware measurement

### INFERRED
- Strongly supported by observed behavior
- Not directly verified but well-established through repeated evidence
- Mark as such; do not present as VERIFIED

### UNKNOWN
- Not currently established
- Preferred over fabricated answers

### CONFLICTING
- Two sources disagree
- Record the conflict explicitly
- Identify competing claims
- Prefer direct hardware evidence
- Mark unresolved behavior clearly

**Never convert INFERRED or UNKNOWN into VERIFIED because an assumption seems reasonable.**

## Hardware: Verified Facts

### Main SoC

**VERIFIED**: Nordic Semiconductor nRF52840
- ARM Cortex-M4F
- 256 KB RAM
- 1 MB flash
- Zephyr firmware environment (v4.3.1 as of Phase 0)
- Integrated Bluetooth radio (not usable in current SP-1 hardware configuration; antenna connection absent)

**Do not** design firmware assuming usable nRF BLE unless hardware configuration is independently verified.

### Audio Codec

**VERIFIED**: Cirrus Logic CS42L42
- I2C address: `0x48`
- Reset: `P0.15` (active-low)
- Audio I2C: SCL = `P1.11`, SDA = `P1.07`
- Operates in 48 kHz audio architecture
- Acts as I2S master

**Do not** invent codec register sequences. Use verified CS42L42 behavior and established SP-1 firmware initialization.

### Speaker Amplifier

**VERIFIED**: TI TAS2505
- I2C address: `0x18`
- Reset: `P0.09` (active-low)
- Expects SP-1-specific configuration; do not assume generic TAS25xx initialization is sufficient

### Audio Clocking

**VERIFIED**: External 3.072 MHz oscillator
- Controlled by: `P0.13`
- CS42L42 acts as I2S master (clock authority)
- nRF52840 and TAS2505 are I2S slaves
- Sample rate: **48 kHz**

I2S pins:
```text
DOUT  = P1.09
LRCLK = P0.11
SCLK  = P0.12
```

**Do not** redesign clock architecture without explicit hardware evidence.

### eMMC Storage

**VERIFIED**: Toshiba THGBMNG5 4 GB eMMC
- 1-bit SPI-style interface (not standard eMMC mode)
- Up to 32 MHz, hardware CRC
- Uses SPIM3

Pins:
```text
CLK  = P0.06
CMD  = P0.08
DAT0 = P0.07
RST  = P1.08
VCCQ = P0.14
```

**Critical**: eMMC must be treated as a dedicated streamer resource. A dedicated streamer thread should own the storage bus rather than allowing arbitrary modules to compete for it.

### Audio Storage Format

**VERIFIED** (from tape-looper codebase):

**24-bit, 48 kHz, 8 channels (4 stereo stems)**

Storage sector size: **8192 bytes** ≈ 340 audio frames per sector

Sector layout:
```text
0–2039     audio data
2040–2041  timing/synchronization
2042–2043  tempo
2044–2047  LED information
```

Audio block order: `0, 2, 1, 3` with frame indices progressing as: `0, 4, 8, ..., 2, 6, 10, ..., 1, 5, 9, ..., 3, 7, 11, ...`

**One audio frame = 24 bytes** containing four stereo stem frames.

**Critical**: Stem frame byte reconstruction is **unconventional**:

```c
int32_t left =
    (data[1] << 24) |
    (data[0] << 16) |
    (data[3] << 8);

int32_t right =
    (data[2] << 24) |
    (data[5] << 16) |
    (data[4] << 8);
```

This representation **must not be "corrected"** into conventional PCM layout without evidence. The unusual layout is part of the known SP-1 storage format and audio streaming contract.

## Control Input Hardware

### Faders and Ladder Inputs

**VERIFIED**: Four faders + two resistive ladder controls via SAADC

Ladder power enable: `P1.10` (must drive high before reading)

SAADC assignments:
```text
AIN0 = Track ladder
AIN1 = Volume / rocker ladder
AIN2 = Fader 3
AIN3 = Fader 1
AIN4 = Battery
AIN6 = Fader 2
AIN7 = Fader 4
```

Track ladder 12-bit targets (±3% acceptance window):
```text
Track 1 = 240
Track 2 = 450
Track 3 = 816
Track 4 = 1357
Play    = 2037
```

Rocker/volume ladder includes intermediate positions; simultaneous positions produce intermediate voltages.

**Do not** treat a resistive ladder as a collection of independent digital switches.

### Function Button

**VERIFIED**: `P0.27`

Treat button behavior separately from ADC ladder behavior. Do not infer debounce timing or gesture semantics unless verified.

## LED Hardware

**VERIFIED**: Eight playback/track LEDs, two separate GPIO banks

Playback LEDs:
```text
P1.13
P0.00
P1.12
P0.01
```

Track LEDs:
```text
P0.29
P0.26
P1.15
P1.14
```

**Architecture rule**: LED behavior should be represented as firmware state, not scattered GPIO manipulation throughout feature modules.

## Power Management

### Charger

**VERIFIED**: BQ24232 charger

Signals:
```text
nCE    = P0.21
nCHG   = P0.22
nPGOOD = P0.24
ISET   = P1.00
Battery ADC = P0.28 / AIN4
```

**Do not** infer battery percentage from voltage alone without explicitly documenting the approximation.

### System Shutdown

Shutdown must leave hardware in a known safe state before `SYSTEM_OFF`.

Audio, storage, Bluetooth, peripherals, and GPIO states must be handled according to their established shutdown requirements.

**Do not** assume that entering `SYSTEM_OFF` alone constitutes a complete shutdown sequence.

## Bluetooth Module

**VERIFIED**: Infineon CYBT-353027-02 (CYW20706A2 SoC)

The module is **independent embedded hardware**:
- Cortex-M3 SoC
- Bluetooth radio + antenna
- 512 KB SPI flash
- Communicates with nRF52840 over UART

Default UART:
```text
115200 baud
8 data bits
1 stop bit
no parity
logic level: 1.8 V
```

Stock firmware:
```text
Classic Bluetooth A2DP sink
```

**INFERRED**: BLE-MIDI requires custom AIROC/WICED application; stock firmware does not support it.

### Bluetooth Safety and Recovery

**CRITICAL**:
- **Never** use `CHIP_ERASE` during normal firmware development
- **Always** preserve Static Section (contains BD_ADDR, keys, RF calibration)

Known recovery sequence:
1. Hold CTS low
2. Pulse `RST_N` low (~10 ms)
3. Wait ~10 ms
4. Release CTS
5. Repeatedly issue HCI reset until acknowledgement

**Never** put Bluetooth flashing/recovery into the audio execution path.

### BLE-MIDI

Service UUID:
```text
03B80E5A-EDE8-4B33-A751-6CE34EC4C700
```

I/O characteristic:
```text
7772E5DB-3868-4112-A1A9-F2669D106BF3
```

Implement BLE-MIDI as a transport backend, not hard-wired into musical modules.

## Real-Time Audio Constraints

The audio path is hard real-time.

**Avoid in audio callback**:
- Blocking operations
- Filesystem access
- eMMC access
- Bluetooth operations
- Arbitrary logging
- Dynamic allocation
- Unbounded loops
- Unpredictable synchronization
- Long critical sections

Audio ISR should remain as small and deterministic as practical. Non-audio work moves to threads or deferred processing.

## Memory and Allocation

**CRITICAL**: The audio path must avoid dynamic allocation.

Prefer:
- Static allocation
- Fixed-size buffers
- Deterministic state
- Compile-time configuration
- Bounded queues
- Explicit ownership

Heap allocation must not be introduced into time-critical audio processing for convenience.

## Firmware Architecture Principle

Firmware should follow a layered architecture:

```text
Hardware (pins, peripherals, silicon)
    ↓
Peripheral / Driver Layer (I2C, SPI, ADC, GPIO drivers)
    ↓
Core Audio / Storage / Controls / Power (owns shared resources)
    ↓
Public SP-1 API (transport-independent abstractions)
    ↓
Feature Modules (reverb, filters, effects)
    ↓
User / MIDI / Control Interaction (UI state machines)
```

Hardware-specific behavior stays in lower layers. Feature modules consume services through public APIs rather than directly manipulating hardware.

## Resource Ownership

Core firmware owns globally coherent resources:
- I2S and codec configuration
- Speaker amplifier
- eMMC and streamer
- ADC and control acquisition
- Power state and watchdog
- System lifecycle

Feature modules consume these services. Do not allow arbitrary modules to independently configure shared peripherals.

## Initialization Order

Conceptually:

```text
hardware reset
    ↓
clocks
    ↓
power / peripherals
    ↓
storage
    ↓
audio clock / codec
    ↓
controls
    ↓
MIDI / Bluetooth
    ↓
feature modules
    ↓
application
```

Exact ordering must be verified against actual hardware dependencies. Use Zephyr `SYS_INIT` where dependencies require deterministic startup.

## Reset and Watchdog

**Watchdog timeout**: < 5 seconds

Firmware must service watchdog deliberately. **Do not** use watchdog feeding as a substitute for fixing blocked/deadlocked subsystems.

**Reset reason**: Clear `RESETREAS` during normal boot. Clear/reset before `SYSTEM_OFF` so subsequent resets can be interpreted correctly.

**Reboot mechanisms** (verified in working firmware):
- Track 1 + Track 4 hold
- Function power-off (when applicable)

## Boot and Memory Layout

Application firmware begins at: `0x20000`

Bootloader owns lower flash region.

Firmware must always preserve a path back to bootloader (via reboot mechanisms listed above).

## Module Registration and Configuration

Modules should self-register using Zephyr mechanisms (e.g., `STRUCT_SECTION_ITERABLE`) where practical.

Architecture should allow new modules to be added without hard-coded switch statements.

Configuration should be separable from implementation. Use YAML or equivalent to generate flash-resident constant tables. Configuration must not require runtime heap allocation.

## Relationship to DSP Engineering

`sp1-firmware` defines **what the machine is** (hardware, peripherals, storage formats, real-time constraints, resource ownership).

`sp1-dsp-engineering` defines **how DSP should be engineered** for that machine (numerical representation, optimization, validation, approximation).

`sp1-dsp` contains **accumulated DSP knowledge and validated implementations** for the SP-1.

**The DSP layer should not need to know**:
- Which GPIO controls a fader
- Which I2C address belongs to the codec
- How eMMC sectors are accessed
- Which UART carries Bluetooth
- How bootloader entry is triggered

**Firmware should not need to know**:
- Internal mathematical details of effects
- Algorithm implementation specifics
- Codec design details

The boundary between firmware and DSP is explicit and well-defined.

## Error Handling

Errors should be explicit and recoverable where practical.

Distinguish between:
- Transient errors
- Recoverable peripheral errors
- Storage errors
- Audio underruns
- Invalid configuration
- Hardware initialization failure
- Fatal firmware errors

**Do not** silently continue after critical hardware initialization failure.

**Do not** allow error handlers to block the audio path indefinitely.

## Discovery and Updates

New discoveries should be recorded:
- What was observed
- How it was observed
- Confidence level (VERIFIED/INFERRED/UNKNOWN)
- Relevant hardware/software version
- Expected behavior
- Known limitations

If a discovery changes a previous assumption, update the affected reference and record the change in history. **Do not** silently rewrite institutional knowledge.

## AI Decision Procedure

When asked to modify SP-1 firmware:

```text
1. Identify requested behavior
2. Identify which hardware/resources it touches
3. Consult relevant SP-1 reference
4. Determine VERIFIED vs UNKNOWN
5. Identify real-time constraints
6. Identify ownership of affected peripherals/state
7. Determine appropriate firmware layer
8. Prefer existing public APIs over direct hardware
9. Implement smallest coherent change
10. Validate compilation and behavior
11. Measure where performance matters
12. Document new assumptions or discoveries
```

**If required information is missing, do not invent it.** Use `UNKNOWN` or `UNVERIFIED`.

## Final Principle

The firmware knowledge block prevents treating the SP-1 like a generic embedded board.

When uncertain, preserve the uncertainty.

When evidence exists, preserve the evidence.

When behavior is already working, preserve the behavior before attempting to improve it.

**The machine is the source of truth.**
