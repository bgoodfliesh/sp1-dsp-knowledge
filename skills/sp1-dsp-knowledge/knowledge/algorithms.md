# Algorithm knowledge index

Use this file as a searchable index of algorithm families and selection
criteria. Add links to component records rather than embedding large code
blocks. Record alternatives, tradeoffs, and known failure modes.

| Family | Components | Selection notes | Status |
| --- | --- | --- | --- |
| Filters | TBD | Cutoff, resonance, stability, oversampling, modulation behavior | RESEARCH |
| Delays | Short feedback delay; TBD | Interpolation, feedback bounds, memory, modulation artifacts | RESEARCH |
| Dynamics | TBD | Envelope timing, detector topology, gain smoothing | RESEARCH |
| Modulation | TBD | LFO quality, sync, rate limits, parameter smoothing | RESEARCH |
| Synthesis | Phase-distortion oscillator; discrete maps; TBD | Oscillator quality, aliasing, phase/state behavior | RESEARCH |
| Granular | TBD | Grain scheduling, windowing, memory, density | RESEARCH |
| Spectral | TBD | FFT size, overlap, latency, memory, numerical behavior | RESEARCH |
| Nonlinear | Feedback saturation; rational saturation; TBD | Oversampling, anti-aliasing, stability, level dependence | RESEARCH |

Related exploratory candidates are tracked in
[`experiments.md`](experiments.md), including allpass scattering, tiny coupled
networks, shift-register coupling, and micro-loop collapse.
