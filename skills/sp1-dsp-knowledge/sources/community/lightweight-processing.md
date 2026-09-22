# Lightweight processing source map

This map ingests the additional references supplied for SP-1. It describes
where each source may help and what must be proven locally. It is not an
endorsement of direct reuse.

| Source | Language | Relevant SP-1 use | Important boundary |
| --- | --- | --- | --- |
| CMSIS-DSP | C | ARM filtering, transforms, vector math, fixed-point kernels | Measure exact core, datatype, alignment, compiler, and memory configuration |
| liquid-dsp | C | FIR/polyphase resampling, filters, oscillators, SDR math | Isolate create/configure from realtime processing and audit optional dependencies |
| chowdsp_wdf | C++14+ | Wave Digital Filter circuit models and nonlinear analog research | Validate code size, compile time, stack/RAM, stability, and C++ ABI fit |
| Faust | Faust, generated C/C++/other targets | Compact algorithm specifications and generated reference implementations | Preserve the Faust source and compiler version; generated output is target-dependent |
| libsamplerate | C | High-quality streaming sample-rate conversion | Measure latency, state RAM, CPU, and callback behavior |
| DaisySP | C++ | Small embedded oscillators, filters, delays, reverbs, and effects | Independently verify every selected module |
| KFR | C++20 | SIMD expression pipelines, filters, FFT, and convolution | GPLv2+ or commercial licensing is a major product constraint |
| PFFFT | C | Small FFT and fast convolution kernels | Audit SIMD selection, alignment, scratch buffers, precision, and notices |
| math_approx | C++17/20 | Fast scalar/SIMD approximations for nonlinear inner loops | Validate domain, error, denormals, and audible artifacts |
| EarLevel Biquad | C/C++ reference | Minimal biquad coefficient/state reference | Supplied path returned 404; license and canonical source remain unresolved |

## Suggested research order

For a lightweight SP-1 component, first compare CMSIS-DSP, DaisySP, and
liquid-dsp. Use PFFFT for spectral work, libsamplerate for conversion, and
math_approx only after a measured bottleneck is established. Treat KFR and WDF
as higher-risk architecture references until target and licensing constraints
are resolved.
