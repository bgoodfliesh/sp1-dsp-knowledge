# libsamplerate Research

## Overview

libsamplerate (SRC) is an industry-standard library for high-quality, real-time sample-rate conversion. Used in professional audio software, DAWs, and mastering.

**Repository:** https://github.com/libsndfile/libsamplerate  
**License:** See `sources/community/libsamplerate-record.yaml` and upstream notices  
**Target:** Portable C (ARM compatible)  
**Language:** C with optional Rust bindings  
**Documentation:** http://www.mega-nerd.com/SRC/

## Why We Study libsamplerate

**Gold standard for audio resampling.**

- Industry-standard quality (used in Audacity, SoX)
- Multiple quality modes (fast to ultra-high-quality)
- Well-documented source code
- Proven on real-time systems
- BSD licensed

## Core Concept: Polyphase Filters

libsamplerate uses polyphase FIR filter banks to change sample rate without artifacts:

```
Input Stream (48 kHz)
    ↓
Polyphase Filter Bank (multiple parallel FIRs)
    ↓
Resample via interpolation/decimation
    ↓
Output Stream (44.1 kHz, 96 kHz, etc.)
```

## Quality Modes

libsamplerate offers trade-offs:

| Quality | CPU Cost | Use Case |
|---|---|---|
| Linear | Low | Rough pitch shifts |
| Zero-Order Hold | Low | Draft mode |
| **Sinc Medium** | Medium | Default, balance |
| Sinc Fast | High | Real-time, good quality |
| **Sinc Best** | Very High | Mastering, offline |

## Possible uses

Pre-resample grains before playback:
```
Workflow:
1. Load a source grain from storage
2. Offline: Resample to target pitch using libsamplerate
3. Cache resampled grain in SRAM
4. Playback in real-time at 1.0 speed
```

**Cost:** Must be measured for the selected converter, ratio, and target.
Quality and artifacts must be tested with representative material.

## Code Structure

```c
// libsamplerate API (simplified)
SRC_STATE* src_new(int converter_type, int channels, int* error);
int src_process(SRC_STATE *state, SRC_DATA *data);
SRC_STATE* src_delete(SRC_STATE *state);

// converter_type options
#define SRC_SINC_BEST       0   // Highest quality
#define SRC_SINC_MEDIUM     1   // Good default
#define SRC_SINC_FASTEST    2   // Lowest quality
```

## Realtime suitability

Do not infer realtime suitability from desktop CPU percentages. Separate
converter creation from `src_process`, measure worst-case callback cost, and
document state RAM, latency, supported ratios, and allocation behavior.

## Fixed-Point Considerations

libsamplerate uses `float`:
```c
typedef float* pbuf_t;  // Internal buffers are float
```

For SP-1 Q16 integration:
1. Convert Q16 audio → float
2. Resample (offline)
3. Convert float → Q16 result
4. Store in SRAM

**Accuracy loss:** Must be measured against the selected reference and
tolerance; no universal RMS claim is made here.

## Algorithms Studied

- [ ] `src_sinc_medium_converter` — Medium-quality Sinc filter
- [ ] `src_process()` — Main processing loop
- [ ] Polyphase filter bank structure
- [ ] Phase accumulation for pitch control

## Potential Adaptations

### 1. Offline Grain Library Preprocessing

```c
// Pseudocode: prepare grain library
for each pitch_ratio in [0.5, 0.75, 1.0, 1.5, 2.0] {
    for each grain in grain_library {
        // Load grain at 48 kHz (original)
        float* grain_48k = load_grain(grain_id);
        
        // Resample to target pitch
        float* grain_resampled = libsamplerate_resample(
            grain_48k,
            grain_length,
            pitch_ratio,
            SRC_SINC_MEDIUM  // Good quality/speed trade-off
        );
        
        // Convert to Q16 and store
        q16_t* grain_q16 = convert_float_to_q16(grain_resampled);
        store_preprocessed_grain(pitch_ratio, grain_id, grain_q16);
    }
}
```

**Result:** Grain library with pre-pitched versions for all playback rates

**Cost:** One-time offline preprocessing (hours, not real-time)

### 2. Custom Q16 Lightweight Resampler

Rather than adapt libsamplerate directly for real-time, implement a lightweight Q16 resampler:

```c
// Simple linear interpolation (fast, good enough for grain pitch)
q16_t resample_linear_q16(q16_t* source, size_t length,
                          q16_t pitch_ratio, q16_t* output) {
    q16_t read_head = 0;
    size_t write_idx = 0;
    
    while (read_head < length << 16) {
        size_t pos0 = read_head >> 16;
        size_t pos1 = (pos0 + 1) & length;
        q16_t frac = read_head & 0xFFFF;
        
        // Linear interpolation
        q16_t y0 = source[pos0];
        q16_t y1 = source[pos1];
        output[write_idx++] = y0 + ((q16_t)((int64_t)(y1 - y0) * frac >> 16));
        
        read_head += pitch_ratio;
    }
    return write_idx;
}
```

**Status:** EXPERIMENTAL — Needs validation against the selected resampling
reference and target processing budget.

## References

**Polyphase Filter Banks:**
See `/knowledge/fixed-point-guide.md` for Q16 multiply-accumulate patterns.

**Real-Time Constraints:**
See `knowledge/realtime-architecture.md` for generic realtime constraints.

**Grain Processing:**
See `/algorithms/granular/` for grain synthesis architecture.

## Status

**REFERENCE** — Resampling reference requiring local quality, latency, memory,
and realtime measurements before selection.
