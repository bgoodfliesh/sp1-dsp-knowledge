# Lightweight DSP experiment queue

This queue records hypotheses for small, stateful DSP systems. It is not a
production roadmap. Each experiment needs a record with implementation,
parameters, numerical format, evidence status, measured resource use, sonic
observations, and failure regions.

Use these evidence labels:

- `VERIFIED`: supported by reliable technical evidence or target measurement
- `DERIVED`: follows from established theory but is not target-validated
- `ESTIMATED`: plausible estimate, not a measurement
- `HYPOTHESIS`: proposed behavior worth testing
- `UNKNOWN`: insufficient evidence

## Queue

1. **Short feedback delay** — sweep short delay lengths, damping, excitation,
   feedback, and interpolation; measure resonance, pitch, and stability.
2. **Nonlinearity inside feedback** — compare hard clip, polynomial, and
   rational saturation for boundedness, harmonic growth, and cost.
3. **Allpass scattering** — compare damping alone, allpass alone, and combined
   phase scattering; do not assume allpass is inherently metallic.
4. **Tiny coupled network** — compare one, two, and four delay/resonator nodes
   with explicit coupling matrices and stability limits.
5. **LUT versus arithmetic** — compare nearest/interpolated LUTs, polynomial,
   and piecewise approximations using identical error and target timing tests.
6. **Rational saturation cost** — isolate division/reciprocal strategies and
   measure cost, error, branch behavior, and behavior near singular regions.
7. **Phase distortion oscillator** — vary phase warp, table resolution,
   interpolation, and smoothing while measuring aliasing and control response.
8. **Discrete maps** — evaluate circle, logistic, tent, and Hénon-like maps for
   cycles, DC, spectra, controllability, and fixed-point sensitivity.
9. **Shift-register coupled oscillators** — compare clock relationships,
   feedback routing, correlation, pattern length, and lockups.
10. **Micro-loop collapse** — shrink captured transient loops toward a few
    samples and evaluate fractional indexing, clicks, crossfades, and resonance.

## Failure data

Keep instability onset, aliasing onset, mode locking, self-oscillation,
saturation lock, dead states, short cycles, and parameter cliffs as experiment
data. A noise-like signal is not evidence of randomness, and a bounded
mathematical recurrence is not automatically safe in fixed point.
