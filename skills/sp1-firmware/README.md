# SP-1 Firmware Knowledge Block

**Machine-truth and firmware-architecture layer for the Teenage Engineering SP-1 Stem Player.**

This knowledge block teaches what the SP-1 **actually is**—its hardware, verified behavior, real-time constraints, and firmware architecture—so AI (and developers) can work with the device without inventing hardware capabilities or breaking invariants.

## Purpose

To provide:
- Verified hardware facts (pins, peripherals, addresses, formats)
- Firmware architecture (layers, thread model, resource ownership)
- Clear boundaries (firmware ↔ DSP, hardware ↔ application)
- Real-time constraints (audio hard-deadline rules)
- Safe engineering practices (no destructive assumptions, preserve evidence)

To prevent:
- Inventing GPIO assignments, codec sequences, or timing behavior
- Guessing at storage formats or audio processing parameters
- Breaking audio real-time constraints with blocking operations
- Creating firmware that assumes generic embedded-system conventions

## File Structure

```
sp1-firmware/
├── SKILL.md                    # Core skill: purpose, evidence model, principles
├── README.md                   # This file
└── references/
    ├── hardware.md             # Verified pins, peripherals, addresses
    ├── audio.md                # Sample rate, storage format, streaming, Q16
    ├── storage.md              # eMMC, ring buffers, streamer thread
    ├── controls.md             # SAADC, ladders, faders, button, debouncing
    ├── bluetooth.md            # CYBT module, HCI, BLE-MIDI, recovery
    ├── bootloader.md           # Boot vectors, reboot, watchdog, DFU
    ├── api-design.md           # Public APIs, MIDI abstraction, DSP boundary
    └── architecture.md         # Layers, threads, initialization, ownership
```

## How to Use This Knowledge Block

### For Hardware Questions

**Start with `hardware.md`**
- What pins are used?
- What's the I2C address?
- Where is the reset signal?
- What are the voltage levels?

Example: "The CS42L42 codec is at I2C address 0x48, SCL=P1.11, SDA=P1.07, reset=P0.15 (active-low)."

### For Audio System Questions

**Start with `audio.md`**
- What is the sample rate?
- How do samples flow from eMMC to I2S output?
- Why is the byte layout unconventional?
- What are Q16 constraints?
- How does the ring buffer prevent underruns?

Cross-reference `sp1-dsp-engineering/audio-model.md` for DSP implications.

### For Storage and Streaming

**Start with `storage.md`**
- Who owns eMMC?
- How big is the ring buffer?
- What does a sector layout look like?
- How are stems encoded?
- What happens on underrun?

Understand that audio thread **never** directly accesses eMMC.

### For Control Inputs

**Start with `controls.md`**
- How are track selection and faders read?
- What are the SAADC channels?
- How should debouncing work?
- What is the ADC window for Track 1?

Cross-reference with `sp1-dsp-engineering/parameters.md` for parameter mapping.

### For Bluetooth

**Start with `bluetooth.md`**
- What module is used?
- How does it communicate?
- Is BLE-MIDI supported?
- What is the recovery sequence?
- Why is Static Section preservation critical?

### For Boot and Reset

**Start with `bootloader.md`**
- How does the device boot?
- How do you reboot to bootloader?
- What does watchdog do?
- What does RESETREAS track?

### For Firmware Design

**Start with `architecture.md`**
- How should firmware be layered?
- What threads are needed?
- Who owns which resources?
- How should initialization be ordered?
- What are real-time constraints?

### For Public API Design

**Start with `api-design.md`**
- How should modules expose their APIs?
- How should MIDI abstraction work?
- How does DSP interface with firmware?
- What are recommended API functions?
- How should modules register themselves?

## Cross-References Between Knowledge Blocks

### `sp1-firmware` → `sp1-dsp-engineering`

`sp1-firmware` provides **the stable boundary**; `sp1-dsp-engineering` defines **how DSP operates on that boundary**.

Examples:

| Firmware Reference | DSP Reference | Topic |
|---|---|---|
| `audio.md` (48 kHz stereo, Q16) | `sp1-dsp-engineering/audio-model.md` | Hardware assumptions |
| `audio.md` (hard real-time) | `sp1-dsp-engineering/realtime.md` | Real-time safety |
| `controls.md` (parameter mapping) | `sp1-dsp-engineering/parameters.md` | Parameter engineering |
| `storage.md` (unconventional bytes) | `sp1-dsp-engineering/numeric.md` | Numerical handling |
| `architecture.md` (layers, APIs) | `sp1-dsp-engineering/implementation-patterns/` | Module structure |

### `sp1-firmware` → `sp1-dsp`

`sp1-dsp` contains **validated implementations and algorithms** for components that run on the firmware boundary.

Examples:

- Reverb algorithm → `sp1-dsp/dsp/reverb/` (source, tests, benchmarks)
- Codec implementations → `sp1-dsp/storage/codecs/` (A7, µ-law, IMA)
- Filter libraries → `sp1-dsp/filters/` (state, coefficients, validation)

## Evidence Model

Every technical statement is one of:

### VERIFIED
- From hardware inspection, reverse engineering, datasheets, working code
- Can be relied upon with confidence
- Example: "CS42L42 at I2C address 0x48" (from hardware traces and firmware)

### INFERRED
- Strongly supported by observed behavior or architecture, but not directly verified
- Marked as such; not presented as VERIFIED
- Example: "Stock Bluetooth firmware does not support BLE-MIDI" (inferred from firmware capabilities, not confirmed in source code)

### UNKNOWN
- Not currently established
- Preferred over fabricated answers
- Example: "Exact eMMC read latency" (timing depends on many factors; should be measured)

### CONFLICTING
- Two sources disagree
- Recorded explicitly with both claims and preferred evidence source

When uncertain, preserve the uncertainty. Do not convert INFERRED or UNKNOWN into VERIFIED because an assumption seems reasonable.

## Confidence Levels

Each reference includes confidence markers:

- ✓ VERIFIED (hardware evidence, protocol captures, working code)
- ≈ INFERRED (strongly supported, architectural evidence)
- ? UNKNOWN (not established; measure to verify)
- ⚠ UNVERIFIED (sourced but not yet confirmed on hardware)

## Key Principles

### 1. Hardware is Ground Truth

Never assume generic embedded-system conventions. The SP-1 has specific hardware with specific quirks.

The unconventional byte layout of tape storage must be preserved **exactly**, not "corrected" into standard PCM.

### 2. Real-Time Constraints Are Non-Negotiable

Audio is hard real-time. Blocking operations, dynamic allocation, or unbounded loops on the audio path cause audible glitches.

The 48 kHz → 5.3 ms / block → ~2 ms DSP budget is strict.

### 3. Resource Ownership Is Explicit

eMMC is owned by the streamer thread. Audio thread never accesses it. Other modules should not compete for the bus.

Do not allow multiple modules to independently configure shared peripherals (codec, I2S, ADC).

### 4. Q16 Fixed-Point Is Non-Negotiable

Audio path uses Q16 fixed-point, not floating-point, even though the Cortex-M4F has an FPU.

Do not introduce float into audio processing. It is explicitly rejected in `sp1-dsp-engineering/SKILL.md`.

### 5. Preserve Proven Behavior

If audio playback works, do not "improve" the clock architecture without hardware evidence.

If a codec initialization sequence works, do not "clean it up" without understanding side effects.

Change incrementally and verify each step.

### 6. Measure Before Optimizing

Do not assume what is expensive. Measure CPU, latency, and power on real hardware.

Do not introduce micro-optimizations without evidence they solve real bottlenecks.

## How to Contribute

New discoveries should be recorded with:

1. **What was observed** (e.g., "CS42L42 register X controls...")
2. **How it was observed** (protocol capture, reverse engineering, measurement)
3. **Confidence** (VERIFIED, INFERRED, etc.)
4. **Hardware/software version** (nRF52840 rev, Zephyr 4.3.1, firmware v1.0.0)
5. **Expected behavior** (why this matters)
6. **Known limitations** (edge cases, dependencies)

When a discovery changes a previous assumption:
- Identify the old assumption
- Record the new evidence
- Update the affected reference
- Note the change in repository history

## Absolute Anti-Hallucination Rules

The AI must **never invent**:

- GPIO assignments (must be verified from schematics or hardware)
- I2C addresses (must be confirmed by protocol analysis or firmware)
- Codec initialization sequences (must come from datasheets or working code)
- Bluetooth capabilities (must be verified from module specification)
- eMMC behavior (must match actual SPI-style 1-bit interface)
- Memory addresses or timings (must be measured or sourced)
- Performance limits (must be benchmarked on hardware)
- Bootloader procedures (must be tested on real device)

If required information is not in this knowledge block or authoritative source:
- Say "UNKNOWN"
- Say "UNVERIFIED"
- Do not guess

## File Descriptions

### SKILL.md

**Core philosophy and framework.**

Defines:
- Purpose and scope
- Evidence model (VERIFIED/INFERRED/UNKNOWN/CONFLICTING)
- Hardware facts (nRF52840, Q16, real-time constraints)
- Critical constraints (audio, storage, Bluetooth safety)
- Decision procedure for AI working with SP-1 firmware
- Relationship to DSP engineering and DSP libraries

**Read this first.** It sets the tone for how to use all other references.

### hardware.md

**Verified pin assignments, peripherals, addresses.**

Includes:
- Main SoC (nRF52840, flash layout, RAM)
- Oscillator (3.072 MHz)
- Audio codec (CS42L42, I2C 0x48)
- Speaker amplifier (TAS2505, I2C 0x18)
- I2S interface (pins, topology, master/slave)
- eMMC (Toshiba THGBMNG5, SPIM3, pins)
- SAADC (channels, ladder power, ladder targets)
- Function button
- LEDs (playback, track)
- Power (charger BQ24232, battery ADC)
- Bluetooth module (CYBT-353027-02, UART 115200)
- GPIO summary (quick reference table)

**Consult when working with hardware.** Every GPIO number, I2C address, and register comes from here.

### audio.md

**Sample rate, storage format, streaming architecture, real-time constraints.**

Includes:
- Audio sample rate (48 kHz) and format (stereo, 256-frame blocks)
- I2S interface details (topology, clock authority)
- Tape storage format (24-bit, 8 channels, sector layout, **unconventional byte reconstruction**)
- Streamer architecture (eMMC → ring → audio)
- Ring buffer contract (producer-consumer rules)
- Codec initialization (CS42L42, TAS2505)
- Real-time audio path constraints (what can/cannot happen in audio callback)
- Cross-reference to DSP engineering (audio-model.md)

**Essential for understanding how audio flows.** The unconventional byte layout **must** be preserved exactly.

### storage.md

**eMMC, ring buffers, streamer thread, buffering strategy.**

Includes:
- eMMC hardware (4 GB, SPI-style, SPIM3)
- Storage ownership (streamer thread owns eMMC exclusively)
- Ring buffer architecture (8192 samples, play + record)
- Sector layout (8192 bytes, audio frames + metadata)
- Recording format (matches playback)
- Buffering requirements (underrun/overflow handling)
- Streaming priorities (audio > storage > UI)
- Error handling (read failures, underruns)
- Storage checklist

**Critical for understanding how audio persists.** Audio thread must never directly access eMMC.

### controls.md

**SAADC, ladder inputs, faders, button, debouncing.**

Includes:
- SAADC overview (12-bit ADC, channels 0–7)
- Track ladder (5 positions with ±3% windows)
- Rocker/volume ladder (approximate targets)
- Faders (four analog inputs, continuous values)
- Function button (P0.27, debouncing)
- Battery voltage reading
- Control update rate recommendations
- Control-to-DSP boundary (parameter mapping)
- Debounce and hysteresis patterns

**Consult for control input behavior.** Ladders are resistive voltage dividers, not binary switches.

### bluetooth.md

**CYBT module, HCI, BLE-MIDI, safety procedures.**

Includes:
- Module hardware (CYBT-353027-02, CYW20706A2, independent Cortex-M3)
- UART communication (115200, H4 HCI)
- Stock firmware (A2DP sink)
- BLE-MIDI service UUID and characteristics
- HCI commands (RESET, VERSION, HOST BUFFER SIZE)
- Module recovery procedure (CTS hold, RST_N pulse)
- **Static Section preservation** (critical: BD_ADDR, keys, RF calibration)
- Bluetooth safety rules (never CHIP_ERASE, use evaluation board for development)

**Critical for safety.** Destroying Static Section renders device inoperable.

### bootloader.md

**Boot vectors, reboot procedures, watchdog, memory layout.**

Includes:
- Memory layout (bootloader at 0x00000, application at 0x20000)
- Boot vector and ARM Cortex-M initialization
- Reboot to bootloader (Track 1+4, Function button, programmatic)
- Reset reason register (RESETREAS)
- Watchdog (< 5 second timeout)
- System Off and wake sources
- DFU (Device Firmware Update) procedure
- Flash protection

**Consult for boot behavior and safety.** Watchdog is non-negotiable.

### api-design.md

**Public firmware APIs, MIDI abstraction, DSP boundary.**

Includes:
- Sample type (Q16 fixed-point stereo)
- Time reference (sp1_millis())
- Module registration (self-registering modules)
- Control input API (faders, track, rocker, button)
- LED control API (per-LED, patterns)
- MIDI transport abstraction (USB, BLE, internal)
- Audio DSP boundary (process callback, parameter passing)
- Storage API (read/write abstractions)
- Power management API
- Configuration API
- System control API
- Error codes

**Design blueprint for firmware/application interface.** Modules should use these APIs, not directly manipulate hardware.

### architecture.md

**Firmware layers, thread model, initialization, resource ownership.**

Includes:
- Layered architecture (hardware → drivers → core services → API → features)
- Core service responsibilities (audio, storage, control, power ownership)
- Thread model (audio, streamer, MIDI, control, UI threads)
- Initialization sequence (dependency order)
- Interrupt priorities
- Resource ownership (explicit, non-competing)
- Configuration and customization (device tree, CMakeLists)
- Error recovery strategies
- Testing strategy (unit, integration, system)
- Performance budgets (estimated, UNMEASURED)
- Cross-reference to DSP engineering

**Foundational for firmware structure.** Defines how everything fits together.

## Technology Stack

- **Processor**: Nordic nRF52840 (ARM Cortex-M4F)
- **RTOS**: Zephyr 4.3.1
- **Audio**: 48 kHz stereo, Q16 fixed-point
- **Storage**: Toshiba THGBMNG5 4 GB eMMC, SPI-style 1-bit interface
- **Bluetooth**: Infineon CYBT-353027-02 (external module, independent CYW20706A2)
- **Audio Codec**: Cirrus Logic CS42L42
- **Amplifier**: TI TAS2505
- **Oscillator**: External 3.072 MHz

## Related Knowledge Blocks

### sp1-dsp-engineering

Covers DSP implementation strategy, real-time engineering, numerical behavior, validation, and optimization **for the SP-1 firmware boundary**.

See `sp1-dsp-engineering/SKILL.md` for:
- Q16 fixed-point constraints
- Real-time audio path rules
- Numerical stability and approximation
- Optimization hierarchy
- Validation discipline

### sp1-dsp

Accumulated DSP knowledge, algorithms, implementations, tests, benchmarks, and provenance independent of the firmware.

Consult before implementing:
- Reverb
- Filters
- Codecs
- Tape effects
- Any other DSP algorithm

## When to Consult This Knowledge Block

You should consult `sp1-firmware`:

- ✓ "What GPIO controls the codec reset?"
- ✓ "How big is the ring buffer?"
- ✓ "What is the I2C address of the amplifier?"
- ✓ "Why is the byte layout unconventional?"
- ✓ "How should I recover from Bluetooth module failure?"
- ✓ "What are the real-time constraints?"
- ✓ "How should firmware be layered?"
- ✓ "What does the RESETREAS register track?"

You should consult `sp1-dsp-engineering` instead:

- ✗ "How do I implement a filter with Q16 arithmetic?"
- ✗ "What is a lookup table approximation?"
- ✗ "How do I optimize an IIR filter?"
- ✗ "What is the correct way to validate DSP?"

(Different knowledge domains; firmware knows the machine; DSP engineering knows how to use it.)

## Final Principle

> **The machine is the source of truth.**

When uncertain, preserve the uncertainty.

When evidence exists, preserve the evidence.

When behavior is already working, preserve the behavior before attempting to improve it.

Do not invent hardware capabilities. Do not break invariants. Do not guess about timing, addresses, or constraints.

The SP-1 is not a generic embedded board. Treat it with the specificity it deserves.
