# SP-1 Public Firmware API

## Purpose

The firmware should expose a **thin, stable public API** that:

1. Abstracts hardware details from feature modules
2. Decouples transport (USB, BLE, etc.) from musical logic
3. Provides a clear boundary between firmware and DSP
4. Remains independent of specific audio effects or processing algorithms

**Do not** expose raw hardware registers or low-level details to feature modules.

**Do not** hard-wire effects or features into the core firmware.

## Core Abstractions

### Sample Type

```c
typedef struct {
    int16_t left;   // L channel
    int16_t right;  // R channel
} sp1_sample_t;
```

Audio samples are:
- Q16 fixed-point (internally; may be Q16 or Q24 depending on context)
- Stereo pairs (left/right)
- 48 kHz sample rate
- Single block = 256 samples

Modules process samples through DSP callbacks (see below).

### Time Reference

```c
typedef uint32_t sp1_millis_t;

// Get current system time in milliseconds since boot
sp1_millis_t sp1_millis(void);
```

Used for:
- Timestamping events
- Measuring elapsed time
- Debouncing inputs
- Scheduler timing

Do **not** use wall-clock time (no RTC required).

### Module Registration

Modules should self-register using:

```c
// In Zephyr device tree or using STRUCT_SECTION_ITERABLE
struct sp1_module {
    const char *name;
    int priority;
    int (*init)(void);
    int (*fini)(void);
};

STRUCT_SECTION_ITERABLE(sp1_module, my_effect, ...)
```

The firmware iterates registered modules at startup and shutdown.

Do **not** create a growing switch statement with hard-coded module names.

## Control Input API

### Fader Reading

```c
typedef uint16_t sp1_fader_t;  // 0–4095

sp1_fader_t sp1_fader_read(int fader_index);  // 0–3
```

Returns:
- Raw 12-bit ADC value
- Updated every ~10 ms
- Hysteresis/filtering applied by firmware (if configured)

### Track Selection

```c
typedef enum {
    SP1_TRACK_1 = 0,
    SP1_TRACK_2 = 1,
    SP1_TRACK_3 = 2,
    SP1_TRACK_4 = 3,
    SP1_TRACK_PLAY = 4,
    SP1_TRACK_UNKNOWN = -1,
} sp1_track_t;

sp1_track_t sp1_track_get(void);
void sp1_track_set_listener(sp1_track_change_callback cb);
```

Returns:
- Current track position (debounced)
- Detected as solid, not intermediate ADC values

Callback fired on transition.

### Rocker / Volume

```c
typedef struct {
    float rocker;   // -1.0 (down) to +1.0 (up), 0.0 center
    float volume;   // 0.0 (quiet) to 1.0 (loud)
} sp1_rocker_state_t;

sp1_rocker_state_t sp1_rocker_get(void);
void sp1_rocker_set_listener(sp1_rocker_change_callback cb);
```

Returns:
- Decoded rocker position and volume level
- Updated continuously (smooth analog values)
- Suitable for DSP parameter modulation

### Function Button

```c
typedef enum {
    SP1_BUTTON_PRESS = 0,
    SP1_BUTTON_RELEASE = 1,
} sp1_button_event_t;

void sp1_button_set_listener(sp1_button_event_callback cb);
```

Callback fired on press/release (or hold, depending on firmware implementation).

## LED Control API

```c
typedef enum {
    SP1_LED_PLAYBACK_1 = 0,
    SP1_LED_PLAYBACK_2 = 1,
    SP1_LED_PLAYBACK_3 = 2,
    SP1_LED_PLAYBACK_4 = 3,
    SP1_LED_TRACK_1 = 4,
    SP1_LED_TRACK_2 = 5,
    SP1_LED_TRACK_3 = 6,
    SP1_LED_TRACK_4 = 7,
} sp1_led_t;

void sp1_led_set(sp1_led_t led, int on);
void sp1_led_toggle(sp1_led_t led);
void sp1_led_pattern(sp1_led_t led, const uint8_t *pattern, int pattern_len);
```

LED state is managed by firmware, not scattered throughout feature modules.

LED patterns can represent:
- Mode status (recording, looping, etc.)
- Level indicators
- User feedback

## MIDI Transport Abstraction

**CRITICAL**: MIDI should not be hard-wired to a single transport.

### Transport Backend

```c
typedef struct {
    const char *name;
    int (*send_note_on)(uint8_t channel, uint8_t note, uint8_t velocity);
    int (*send_note_off)(uint8_t channel, uint8_t note, uint8_t velocity);
    int (*send_cc)(uint8_t channel, uint8_t controller, uint8_t value);
    int (*send_program_change)(uint8_t channel, uint8_t program);
    // etc.
} sp1_midi_transport_t;

int sp1_midi_transport_register(const sp1_midi_transport_t *transport);
```

Transports:
- USB MIDI
- BLE-MIDI
- UART MIDI (future)
- Internal MIDI (loopback for testing)

### Public MIDI API

```c
// Receive side: register callback
typedef void (*sp1_midi_callback_t)(const sp1_midi_event_t *event);

int sp1_midi_set_rx_callback(sp1_midi_callback_t cb);

// Send side: modules use this interface
int sp1_midi_send_note_on(uint8_t channel, uint8_t note, uint8_t velocity);
int sp1_midi_send_note_off(uint8_t channel, uint8_t note);
int sp1_midi_send_cc(uint8_t channel, uint8_t controller, uint8_t value);
int sp1_midi_send_program_change(uint8_t channel, uint8_t program);
int sp1_midi_send_clock(void);      // MIDI clock pulse (24 per quarter)
int sp1_midi_send_start(void);      // Start playback
int sp1_midi_send_stop(void);       // Stop playback
int sp1_midi_send_continue(void);   // Resume playback
```

Modules call public API functions. Firmware routes to active transport.

## Audio DSP Boundary

### Sample Block Callback

```c
typedef int (*sp1_process_callback_t)(
    sp1_sample_t *input,    // 256 samples (or block_size)
    sp1_sample_t *output,
    int block_size,
    void *context
);

int sp1_dsp_set_processor(sp1_process_callback_t cb, void *context);
```

The callback is invoked by the audio ISR (interrupt service routine).

The callback **must** be:
- Deterministic (fixed execution time or bounded worst-case)
- Non-blocking (no I/O, no malloc)
- Q16-aware (input/output are Q16 fixed-point)
- Fast (~5 ms for 256 samples @ 48 kHz)

Input samples:
- Come from tape playback, USB audio, Bluetooth A2DP, or line input
- Are pre-decoded and placed in the ring buffer by the streamer

Output samples:
- Go to I2S output (headphones, speaker)
- May also be recorded to eMMC (if recording)

Context pointer allows the callback to maintain private state.

### Parameter Passing

Parameters to DSP should be pre-computed on the control boundary and passed via:

```c
// Before calling DSP block:
struct my_effect_params {
    float frequency;
    float resonance;
    float gain;
};

// Compute expensive operations (exp, sqrt) at control rate
my_effect_params->frequency = 1000.0;  // Hz
my_effect_params->resonance = 0.9;
compute_filter_coefficients(my_effect_params);

// During audio callback, use precomputed coefficients
sp1_dsp_set_processor(my_effect_process, my_effect_params);
```

See `skills/sp1-dsp-engineering/references/parameters.md` for parameter engineering discipline.

## Storage API

```c
// Read audio from tape at position
int sp1_storage_read(uint64_t position, sp1_sample_t *buffer, int samples);

// Write audio to tape (recording)
int sp1_storage_write(uint64_t position, const sp1_sample_t *buffer, int samples);

// Get current playback position
uint64_t sp1_tape_position_get(void);

// Seek to position
int sp1_tape_seek(uint64_t position);
```

These APIs abstract:
- eMMC hardware (pins, protocol, timing)
- Storage format (sector layout, byte reconstruction)
- Ring buffer implementation (pre-fetching, buffering)

Modules call these, firmware handles streamer coordination.

## Power Management API

```c
typedef enum {
    SP1_POWER_ACTIVE,
    SP1_POWER_IDLE,
    SP1_POWER_SLEEP,
    SP1_POWER_OFF,
} sp1_power_state_t;

int sp1_power_set_state(sp1_power_state_t state);
sp1_power_state_t sp1_power_get_state(void);
int sp1_battery_read_percent(void);  // 0–100%
int sp1_charger_is_connected(void);
```

Power state transitions:
- ACTIVE → IDLE (after timeout, no user interaction)
- IDLE → SLEEP (optional, depends on implementation)
- ACTIVE/IDLE → OFF (user power button)
- OFF → ACTIVE (power button, Bluetooth connection, etc.)

Battery percentage should be an approximation (see hardware.md); do not rely on exact accuracy.

## Configuration API

```c
typedef struct {
    const char *name;
    void *data;
    int size;
} sp1_config_t;

int sp1_config_load(const char *name, sp1_config_t *config);
int sp1_config_save(const char *name, const sp1_config_t *config);
int sp1_config_reload(void);
```

Configuration is stored in eMMC or NVS (Non-Volatile Storage).

Configurations should not require runtime heap allocation.

Use code generation (from YAML or binary format) to produce compile-time constants where practical.

## System Control API

```c
// Reboot to bootloader (for DFU)
int sp1_reboot_to_bootloader(void);

// Reboot normally
int sp1_system_reboot(void);

// Get system uptime
uint32_t sp1_system_uptime_ms(void);

// Get reset reason from last boot
uint32_t sp1_reset_reason(void);
```

These are control-plane operations (not time-critical).

## Error Codes

Common error codes across all APIs:

```c
#define SP1_OK              0
#define SP1_ERROR           -1
#define SP1_EBUSY           -2
#define SP1_ENODEV          -3
#define SP1_ENOMEM          -4
#define SP1_ENOTIMPL        -5
#define SP1_EINVAL          -6
```

APIs return `SP1_OK` on success, negative error code on failure.

## Example Module Integration

A hypothetical reverb effect module:

```c
struct reverb_context {
    sp1_dsp_reverb_t state;  // DSP state
    float wet_level;          // Parameter
    float room_size;          // Parameter
};

int reverb_init(void) {
    sp1_dsp_reverb_init(&context.state);
    sp1_rocker_set_listener(reverb_rocker_callback);
    sp1_dsp_set_processor(reverb_process, &context);
    return SP1_OK;
}

int reverb_process(sp1_sample_t *in, sp1_sample_t *out,
                   int block_size, void *ctx) {
    struct reverb_context *c = (struct reverb_context *)ctx;
    
    // Process block using precomputed parameters
    for (int i = 0; i < block_size; i++) {
        out[i] = sp1_dsp_reverb_process(&c->state, in[i],
                                        c->wet_level, c->room_size);
    }
    return SP1_OK;
}

void reverb_rocker_callback(const sp1_rocker_state_t *rocker) {
    // Rocker moves → update DSP parameter
    // This runs on control thread, not audio thread
    context.wet_level = rocker->volume;
}

STRUCT_SECTION_ITERABLE(sp1_module, reverb_module,
    .name = "reverb",
    .init = reverb_init,
);
```

Advantages:
- DSP code is isolated (sp1-dsp-engineering/sp1-dsp)
- Parameter updates deferred to control rate
- Module registers itself
- MIDI/USB/etc. are abstracted (module doesn't care how audio arrives)

## Implementation Priority

1. **Core audio path** (playback, basic mixing)
2. **USB MIDI** (easiest to implement; PC connectivity)
3. **Recording** (write to eMMC)
4. **BLE-MIDI** (requires custom AIROC firmware)
5. **Advanced DSP** (reverb, filters, effects)

Do not implement advanced features before core stability is verified on real hardware.
