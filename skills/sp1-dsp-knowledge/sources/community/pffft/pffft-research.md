# pffft Research

## Overview

pffft is a single-file C FFT library optimized for performance on modern CPUs, using SIMD intrinsics without FFTW's overhead.

**Repository:** https://github.com/marton78/pffft  
**License:** BSD-like; inspect bundled FFTPACK and file notices  
**Target:** Portable C with SIMD (SSE/NEON/Altivec)  
**Language:** C (single header + implementation)  
**Dependency:** None (optional; SIMD via compiler intrinsics)

## Why We Study pffft

**Fast FFT without bloat or external dependencies.**

- Single `.c` and `.h` file (easy integration)
- SIMD optimized (uses NEON on ARM)
- No FFTW dependency (much smaller)
- Modest memory overhead
- Competitive with FFTW for typical block sizes

## Characteristics

### Performance
- Speed: Comparable to FFTW for 256–4096 point FFTs
- Memory: ~8× smaller than FFTW
- Setup: Fast, suitable for multiple FFT lengths

### Limitations
- Not optimized for very small FFTs (<64)
- Real FFT (RFFT) not provided (must use complex with zeros)
- No multi-dimensional FFT

## Potentially useful uses

Offline frequency-domain analysis (for development, not production):
- ✓ Grain spectral analysis
- ✓ Algorithm verification
- ✓ Coefficient generation

Example:
```c
// NOT in audio callback; background process
pffft_perform_fft(setup, input_buffer, output_fft);
// Analyze magnitude spectrum for distortion, aliasing, etc.
```

## Implementation Notes

### Target feature selection

PFFFT selects SIMD paths according to the consuming target and build flags.
Do not transfer a desktop SIMD result to an embedded target without measuring
the scalar or target-specific path. CMSIS-DSP may be a better ARM-specific
dependency, but that is a selection decision for the consumer.

### 2. Complex vs Real FFT

pffft provides complex FFT:
```c
pffft_transform(...);  // Complex-in, complex-out
```

For real-valued audio, must pad with zeros:
```c
// Input: [x0, x1, x2, ..., xN]
// Pad:   [x0, x1, x2, ..., xN, 0, 0, 0, ...]
// FFT outputs N/2 unique frequency bins
```

### 3. Fixed-Point Conversion

pffft uses `float` internally:
```c
typedef float pffft_complex[2];  // [real, imag]
```

For Q16 analysis offline:
```c
// Convert audio to float
float* audio_f32 = convert_q16_to_float(audio_q16);

// FFT
pffft_perform_fft(setup, audio_f32, fft_output);

// Analyze (convert back if needed)
```

## Algorithms Studied

- [ ] `pffft_new_setup()` — Initialize FFT plan for given size
- [ ] `pffft_perform_fft()` — In-place complex FFT
- [ ] `pffft_zreorder()` — Reorder output for magnitude spectrum

## Potential Adaptations

None planned for SP-1 real-time audio path.

### Offline Use: Spectral Analysis

```c
// Offline: measure grain spectral content
for each grain {
    float grain_f32[256];  // Convert from Q16
    pffft_perform_fft(setup, grain_f32, spectrum);
    
    // Analyze spectrum for aliasing, distortion, etc.
    float nyquist_energy = measure_high_frequency_content(spectrum);
}
```

**Status:** EXPERIMENTAL — Not required for initial SP-1 release

## References

See `sources/community/pffft-record.yaml` for provenance and
`sources/cmsis-dsp/` for the separate ARM-oriented reference.

## Status

**REFERENCE** — Studied as a compact FFT and convolution reference. Target
selection, realtime suitability, and performance remain consumer-specific.
