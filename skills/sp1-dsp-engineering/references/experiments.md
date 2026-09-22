# DSP experiment records

Experiments preserve hypotheses without promoting them to architecture or
production facts. Use the evidence vocabulary consistently:

- `VERIFIED` — supported by reliable technical evidence or target measurement
- `DERIVED` — follows from established theory but is not target-validated
- `ESTIMATED` — plausible implementation or hardware estimate
- `HYPOTHESIS` — proposed behavior worth testing
- `UNKNOWN` — insufficient evidence

## Record template

```text
Question:
Hypothesis:
Implementation:
Evidence status:
Target and build:
Sample rate / block size / channels:
State and numerical format:
Parameters and update rate:
Measured CPU / RAM / latency:
Numerical results:
Spectral or time-domain results:
Sonic observations:
Failure regions:
Conclusion:
Promotion decision:
```

Record both useful and failed regions: instability onset, aliasing onset, mode
locking, self-oscillation, saturation lock, subharmonic transitions, and
sensitivity cliffs. These observations constrain future designs even when they
are not promoted into production behavior.

## Candidate queue

The following are high-value hypotheses, not commitments:

1. Short feedback delay/waveguide: compare roughly 5–200 samples as an
   exploratory range, with damping, excitation, feedback, and optional
   interpolation.
2. Nonlinearity inside feedback: compare hard clip, polynomial, and rational
   saturation for boundedness, useful range, harmonic growth, and cost.
3. Allpass scattering: compare damping alone, allpass alone, and combined
   phase scattering inside resonant feedback.
4. Tiny coupled networks: compare one, two, and four delay/resonator nodes with
   structured sum/difference or Hadamard-like coupling.
5. Approximation profiling: compare nearest and interpolated LUTs, polynomial,
   and piecewise approximations using identical error criteria.
6. Phase-distortion oscillator: vary phase warp, table resolution,
   interpolation, and smoothing while measuring aliasing.
7. Discrete maps: evaluate logistic, tent, circle, and Hénon-like maps for
   cycles, DC, spectra, controllability, and fixed-point sensitivity; begin at
   control rate unless audio-rate usefulness is demonstrated.
8. Shift-register coupling: compare clock relationships, feedback routing,
   pattern length, correlation, and lockups.
9. Micro-loop collapse: shrink a captured transient loop toward a few samples
   and evaluate fractional indexing, clicks, crossfades, and resonance.

Do not describe a candidate as musical, metallic, wooden, chaotic, stable, or
cheap without specifying the parameterization and evidence behind the claim.
