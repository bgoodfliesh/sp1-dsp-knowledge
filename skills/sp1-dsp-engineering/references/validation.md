# DSP Validation

## Validation Is Not Optional

- **Compilation is not validation.** Code that compiles is not proven correct.
- **Plausible sound is not validation.** Hearing output that "sounds right" does not prove numerical correctness, stability, or sonic accuracy.
- **UNMEASURED must remain UNMEASURED** until evidence exists.

## Test Categories

Depending on the DSP algorithm, prepare evidence from some or all of these categories:

### Numerical / Reference Algorithm

- **Mathematical reference**: Does the implementation match the published algorithm? (e.g., a biquad filter matches the canonical difference equation)
- **Trusted reference implementation**: Compare against known-good code (e.g., open-source DSP library, academic implementation, previous SP-1 version)
- **Offline reference output**: Run the reference on known test signals (sine waves, noise, silence, impulse), capture output, compare to SP-1 implementation

### Frequency / Time Domain

- **Impulse response**: excite with a single sample, measure output envelope and decay
- **Frequency response**: measure magnitude and phase across 20 Hz–20 kHz (or relevant range) at multiple levels
- **Step response**: sudden parameter change (e.g., gain jump), confirm no clicks/glitches and proper settling time
- **Phase response**: measure group delay, latency, any nonlinear phase distortion

### Signal-Level and Edge Cases

- **Silence**: pass silence through the component; confirm output is silence (no creep, clicks, noise generation)
- **DC offset**: introduce DC bias, confirm component does not amplify or accumulate it
- **Known test signals**: sine sweep, pink noise, chirp, impulse train
- **Parameter sweeps**: vary each parameter across full range while playing test signal; listen and measure for artifacts
- **Extreme values**: maximum positive, maximum negative, very small values near zero, very large values (near saturation)
- **Discontinuities**: rapid parameter changes (knob tweaks); confirm smooth transition or intended click behavior

### State and Lifecycle

- **Reset behavior**: confirm reset clears state and leaves component in known initial condition
- **Bypass behavior**: if component has bypass, confirm audio is identical when bypassed vs when engaged with unity gain
- **Session load/save**: if state is persistent, confirm state survives save/load without corruption
- **Thread safety**: if state is shared (audio + control threads), confirm no race conditions under rapid parameter changes

### Numerical / Fixed-Point Specific

- **Quantization error**: for Q16 implementations adapted from float, measure RMS error vs reference; is it < 0.1%? < 0.01%?
- **Overflow behavior**: for accumulators or feedback paths, confirm saturation (not wraparound) when exceeding range
- **Accumulation drift**: for processes that sum many samples, confirm error is bounded (not accumulating indefinitely)

### CPU and RAM

- **CPU cost** (HARDWARE-MEASURED on real SP-1): peak and average cycle count per sample, per block
- **RAM usage**: static state size, temporary buffer size, stack depth
- **Flash size**: code + read-only data from `-Os` build

### SP-1 Hardware Execution

- **Flash and listen**: compile with `west build`, flash to real nRF52840 device, play through audio system, listen for clicks/glitches/artifacts
- **Load comparison**: if the component is a new version of an existing SP-1 effect, A/B compare new vs old on same session

## Example Validation: IIR Filter

```
Component: Two-pole lowpass IIR filter

Reference: Directly derive difference equation from transfer function H(z)
   H(z) = b0 / (1 - a1*z^-1 - a2*z^-2)
   y[n] = b0*x[n] + a1*y[n-1] + a2*y[n-2]

Reference implementation: scipy.signal.lfilter (Python) or MATLAB butter(2, fc/(fs/2))

Test signals:
  1. 1 kHz sine @ −20 dBFS (quiet, to avoid saturation)
  2. Pink noise @ −15 dBFS
  3. Chirp 20 Hz → 20 kHz over 10 sec
  4. Silence (confirm no creep)

Expected results (from theory):
  - Magnitude at 1 kHz: depends on fc and Q; e.g., 0 dB if fc = 1 kHz, −3 dB if fc = 1.4 kHz
  - Phase at 1 kHz: e.g., −90° if 2nd-order lowpass
  - Roll-off above fc: 40 dB/decade (2nd-order)

Measurement:
  - Compute FFT of output from SP-1 implementation on same test signals
  - Compare magnitude vs reference: error < 0.5 dB?
  - Compare phase vs reference: error < 5°?
  - Confirm roll-off slope matches theory

Sonic validation:
  - Listen to chirp sweep; confirm bright high-frequency content is reduced
  - Listen to noise with rapid fc sweeps; confirm smooth transition, no clicks

Status: (example; fill in actual results once measured on SP-1)
```

## Existing SP-1 Validated Components

- **Reverb**: Validate against a stable reference implementation
  - Validation: Host test against a stable reference, with bit-identical output where required
  - Status: `SP1-TESTED` (host-only; hardware listen test pending)

- **Codecs**: SP1-ADPCM7, G.711 µ-law, IMA ADPCM
  - Validation: Host tests with exhaustive vectors appropriate to each codec
  - Status: `SP1-TESTED` (host-only; hardware listen test pending)

When adapting or reusing these, inherit their validation: run the same host tests, confirm bit-identical output.

## Validation Status Labels

Use these consistently:

- `EXPERIMENTAL` — implemented, not yet validated
- `REFERENCE` — matches published algorithm or trusted reference
- `REFERENCE-VALIDATED` — reference algorithm + comparison against reference output
- `SP1-COMPILES` — compiles with `west build`, not yet flashed
- `SP1-TESTED` — flashed to real nRF52840, audio path passes functional testing
- `SP1-BENCHMARKED` — CPU/RAM measured on real hardware
- `PRODUCTION` — validated, benchmarked, deployed in released firmware

Never claim a higher status than the evidence supports. If you've measured on a desktop or simulator, label it `REFERENCE-VALIDATED`, not `SP1-TESTED`.

Status: REFERENCE (validation framework and SP-1-specific examples documented; validation discipline established)
