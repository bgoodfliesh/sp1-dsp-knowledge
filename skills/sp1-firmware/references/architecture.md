# SP-1 Firmware Architecture

## Layered Architecture

**VERIFIED** (principle; specific implementation may vary)

The firmware should follow a clear layered architecture:

```
┌─────────────────────────────────────────────────────┐
│  Feature Modules (reverb, filters, effects, etc.)  │
│  (consumes public APIs, implements DSP)             │
└─────────────────────────────────────────────────────┘
                         ↑
                   (public API)
                         ↑
┌─────────────────────────────────────────────────────┐
│  User / MIDI / Control Interaction                 │
│  (state machines, gesture handling, routing)        │
└─────────────────────────────────────────────────────┘
                         ↑
┌─────────────────────────────────────────────────────┐
│  Public SP-1 API                                    │
│  (MIDI abstraction, control input, LEDs, storage)  │
└─────────────────────────────────────────────────────┘
                         ↑
┌─────────────────────────────────────────────────────┐
│  Core Services (Audio, Storage, Controls, Power)   │
│  (owns globally coherent resources)                 │
└─────────────────────────────────────────────────────┘
                         ↑
┌─────────────────────────────────────────────────────┐
│  Peripheral / Driver Layer                          │
│  (I2C, SPI, ADC, GPIO, UART drivers)               │
└─────────────────────────────────────────────────────┘
                         ↑
┌─────────────────────────────────────────────────────┐
│  Hardware (nRF52840, codecs, eMMC, Bluetooth)      │
└─────────────────────────────────────────────────────┘
```

Hardware-specific behavior stays in lower layers. Feature modules consume services through public APIs rather than directly manipulating hardware.

## Core Service Responsibilities

### Audio Core

Owns:
- I2S peripheral (nRF52840 I2S interface)
- CS42L42 codec configuration and control
- TAS2505 speaker amplifier configuration
- 3.072 MHz oscillator control
- Audio ISR and callback management
- Sample format standardization (Q16 fixed-point, 48 kHz, stereo)

Provides:
- Audio process callback registration (DSP entry point)
- Sample I/O (input from tape/USB/BLE, output to I2S)

### Storage Core

Owns:
- eMMC hardware (SPIM3, pins, protocol)
- Streamer thread (sole eMMC accessor)
- Ring buffers (play and record)
- Storage-layer error handling and recovery

Provides:
- Read/write abstractions (modules don't know about sectors or byte ordering)
- Position tracking
- Buffering and pre-fetching

### Control Core

Owns:
- SAADC peripheral (all ADC channels)
- Ladder power control
- Button input
- Debouncing and filtering

Provides:
- Track selection (debounced)
- Fader readings (0–4095)
- Rocker/volume (smooth continuous values)
- Button events (press/release)

### Power Core

Owns:
- Power state transitions (ACTIVE, IDLE, SLEEP, OFF)
- Watchdog service
- Battery monitoring
- Charger interface
- Peripheral power gating

Provides:
- Power mode requests
- Battery status
- Reset reason
- System uptime

## Thread Model

**RECOMMENDED** (not all details verified in firmware)

```
Threads:

1. Audio thread (highest priority, real-time)
   - I2S interrupt handler
   - DSP callback execution
   - Ring buffer management (read side)
   - Deterministic timing

2. Streamer thread (high priority, best-effort)
   - eMMC read/write
   - Ring buffer management (write side)
   - Storage error handling

3. MIDI thread (medium priority)
   - HCI communication with Bluetooth module
   - USB MIDI polling
   - Event routing to application

4. Control thread (medium priority)
   - SAADC reads
   - Button debouncing
   - Parameter computation
   - State machine updates

5. UI/Application thread (lowest priority)
   - User interaction
   - Logging
   - Configuration management
   - Non-critical updates

6. Zephyr kernel threads
   - Timer/scheduler
   - Memory management
   - Kernel internal tasks
```

Thread priorities:
```
Audio ISR:      ≥ everything (interrupt, not thread)
Streamer:       very high (must keep buffers full)
MIDI:           high
Control:        normal
Application:    normal (or lower)
Zephyr:         background
```

Do **not** invent exact priority values without hardware evidence. The principle is clear: audio deadline > storage > UI.

## Initialization Sequence

Boot must follow dependency order:

```
1. Hardware reset
   - CPU boots at 0x00000 (bootloader)
   - Bootloader jumps to 0x20000 (application)

2. Zephyr kernel initialization
   - Clocks enabled
   - Memory management
   - Scheduler

3. Device drivers (SYS_INIT priority)
   - GPIO/UART/I2C/SPI drivers
   - SAADC ADC driver
   - Watchdog driver

4. Hardware initialization (low-level)
   - Oscillator (3.072 MHz) enabled
   - CS42L42 reset and I2C initialization
   - TAS2505 reset and I2C initialization
   - eMMC detection and identification
   - UART to Bluetooth module

5. Core service initialization
   - Audio ring buffers allocated
   - I2S peripheral configured (I2S slave mode)
   - SAADC configured
   - Watchdog started
   - Power management initialized

6. Thread creation
   - Streamer thread starts (begins reading eMMC)
   - MIDI thread starts
   - Control thread starts
   - Application thread starts

7. Feature module registration and initialization
   - Modules self-register (SYS_INIT or explicit)
   - Each module initializes its DSP state
   - Each module registers callbacks

8. Zephyr kernel starts
   - Main thread yields control to scheduler
   - Threads begin executing at assigned priorities

9. Application main() runs
   - User code
   - Feature-specific initialization
```

Exact ordering must be verified against actual hardware dependencies. Use Zephyr `SYS_INIT` where dependencies require deterministic startup.

## Interrupt Priorities

**RECOMMENDED** (specific values depend on Zephyr configuration)

Interrupt priority hierarchy:

```
Highest:  I2S audio interrupt
          (must never be blocked; hard real-time)

High:     eMMC/storage interrupts
          (keep ring buffer filled; time-sensitive but can tolerate brief delays)

Medium:   UART (Bluetooth, USB)
          (should not block audio, but important for connectivity)

Low:      SAADC/GPIO (controls, LEDs)
          (can tolerate delays; no audible impact)

Lowest:   Timer, system tick
          (background)
```

Do **not** perform heavy work in ISRs. Defer non-critical work to threads.

Do **not** call blocking operations (malloc, I/O, mutex lock) from high-priority ISRs.

## Resource Ownership

**CRITICAL**: Define ownership explicitly.

### Audio I2S and Codecs

Owner: **Audio core**
- nRF52840 I2S peripheral
- CS42L42 codec (I2C address 0x48)
- TAS2505 amplifier (I2C address 0x18)
- 3.072 MHz oscillator

No other module should:
- Toggle codec pins
- Issue codec I2C commands
- Reconfigure I2S

Access through public audio API only.

### eMMC Storage

Owner: **Storage core / Streamer thread**

No other module should:
- Initiate eMMC reads/writes
- Manage eMMC state
- Access sectors directly

Access through public storage API (read/write abstractions).

### SAADC and Controls

Owner: **Control core**

No other module should:
- Read ADC channels directly
- Change ADC configuration
- Manage control debouncing

Access through public control API (fader readings, track selection, etc.).

### LEDs

Owner: **UI core** (or application, depending on design)

Multiple modules may:
- Request LED state changes
- Have LEDs indicate their status

Central LED manager coordinates (prevents conflicts, implements patterns).

### Watchdog

Owner: **Power core**

Application and feature modules must:
- Call watchdog feed at regular intervals
- Report if system becomes unresponsive

Watchdog manager detects hung subsystems and resets system.

### Bluetooth Module

Owner: **MIDI/connectivity core** (or application)

Firmware provides:
- UART driver (transparent communication)
- HCI packet framing (if needed)

Bluetooth firmware (AIROC/CYW20706A2) is independent. Communicate via HCI over UART.

## Configuration and Customization

### Device Tree

Zephyr device tree defines:
- GPIO assignments
- I2C bus configuration
- SPI bus configuration
- ADC channel mapping
- UART configuration
- Interrupt assignments

Example (pseudo-code):
```dts
/ {
    soc {
        i2c1 {
            cs42l42@48 {
                status = "okay";
                reg = <0x48>;
            };
            tas2505@18 {
                status = "okay";
                reg = <0x18>;
            };
        };
    };
};
```

Firmware should **not** hard-code GPIO numbers or I2C addresses. Use device tree.

### CMakeLists and Build Configuration

```cmake
cmake_minimum_required(VERSION 3.20)

project(sp1_firmware)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

target_sources(app PRIVATE
    src/main.c
    src/audio.c
    src/storage.c
    src/controls.c
    # ... module files
)

target_include_directories(app PRIVATE
    include
    drivers
)

# Board and configuration
set(BOARD sp1)  # (or specific evaluation board)
```

Build should be reproducible and deterministic.

## Error Recovery

Firmware should recover gracefully from:

- Transient control input noise (filtering, debouncing)
- Codec initialization delays (retry with timeout)
- eMMC read errors (retry, log, continue or pause)
- Audio underruns (mute and refill)
- Bluetooth disconnection (reconnect, user notification)
- Watchdog timeout (reset, investigate logs)

Do **not** silently ignore critical failures.

Do **not** leave the system in an inconsistent state.

## Testing Strategy

### Unit Tests

Test individual modules in isolation:
- DSP algorithms (against reference implementation)
- Storage format unpacking
- Control debouncing
- Configuration loading

Run on desktop or simulator (fast iteration).

### Integration Tests

Test modules together:
- Audio + streamer (underrun handling)
- Controls + features (gesture recognition)
- MIDI transport abstraction (multiple backends)

Run on real SP-1 hardware or realistic emulation.

### System Tests

Full firmware on real hardware:
- Boot sequence
- Audio playback/recording
- Parameter changes
- Error recovery
- Power transitions
- Watchdog functionality
- DFU procedure

Do **not** ship firmware without system testing on real hardware.

## Performance Budgets

**UNMEASURED** (establish through measurement on real hardware)

Estimated budgets (for planning purposes):

```
Audio block (256 samples @ 48 kHz): ~5.3 ms total
├── I2S DMA completion:             ~5.3 ms (synchronous)
├── DSP processing:                 ~2.0 ms (estimate, should be < 3 ms)
├── Ring buffer management:         ~0.3 ms (estimate)
└── Margin:                         ~1.7 ms

Streamer (eMMC read):               ~20–50 ms per sector (UNMEASURED)
├── eMMC read latency:              ~10–30 ms
├── Ring write:                     ~0.1 ms
├── Data copy/unpack:               ~1–2 ms
└── (Must keep buffer ahead of audio)

Control update (debounce/filter):   ~5–10 ms per update
├── SAADC read:                     ~1–2 ms
├── Debounce/filter:                ~1–2 ms
└── Callback invocation:            ~1–2 ms
```

Measure on real hardware and adjust budgets accordingly.

## Cross-Reference to DSP Engineering

The DSP boundary between firmware and DSP modules is defined by:

1. **Sample interface**: Audio core provides 256-sample blocks @ 48 kHz, Q16 fixed-point
2. **Parameter interface**: Control core computes expensive DSP parameters at control-rate; audio thread applies precomputed values
3. **Real-time constraints**: DSP callbacks must finish within ~5 ms (see audio.md, realtime.md)
4. **Numerical constraints**: Q16 fixed-point only (no float on audio path)
5. **Storage format**: Unconventional byte layout (must be preserved exactly)

See `sp1-dsp-engineering/` for DSP-specific design guidance.

Firmware **does not** define DSP algorithms. Firmware provides the stable boundary on which DSP operates.

## Known Limitations

- Single audio path (one output at a time; no simultaneous playback + recording in v1 firmware)
- Ring buffers fixed at compile-time (not dynamically configurable)
- Bluetooth module is independent (AIROC firmware updates require special procedure)
- eMMC is SPI-style, not high-speed interface (limits simultaneous playback + recording bandwidth)
- Zephyr overhead varies with configuration (exact real-time margin depends on kernel tuning)

## Architecture Checklist

When implementing new features:

- [ ] Identify which core service owns affected resources
- [ ] Request access through public API (do not directly manipulate hardware)
- [ ] Confirm API provides required functionality (if not, extend API)
- [ ] Verify feature respects real-time constraints (if audio-related)
- [ ] Confirm feature has clear shutdown path (no resource leaks)
- [ ] Test error cases (connectivity lost, eMMC failure, etc.)
- [ ] Verify feature does not block audio or critical services
- [ ] Document new API additions
- [ ] Add feature to initialization sequence if needed
