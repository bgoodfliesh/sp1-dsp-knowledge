# Algorithms

Reusable DSP building blocks live here. An algorithm is not necessarily a
user-facing effect; keep composition and product-specific wiring in
`effects/` when appropriate.

Each component should have an engineering record, provenance metadata,
reference tests, and a clearly earned status. Category directories:

| Directory | Scope |
| --- | --- |
| [`filters/`](filters/) | EQ, resonant, state-variable, ladder, and related filters |
| [`delays/`](delays/) | Delay lines, interpolation, feedback, and modulated delay primitives |
| [`dynamics/`](dynamics/) | Envelopes, compressors, expanders, gates, and limiters |
| [`modulation/`](modulation/) | LFOs, clocks, smoothing, and modulation utilities |
| [`synthesis/`](synthesis/) | Oscillators, waveshaping primitives, and voice building blocks |
| [`granular/`](granular/) | Grain scheduling, windows, and granular buffers |
| [`spectral/`](spectral/) | FFT, STFT, spectral transforms, and phase processing |
| [`nonlinear/`](nonlinear/) | Saturation, wavefolding, rectification, and anti-aliasing |
