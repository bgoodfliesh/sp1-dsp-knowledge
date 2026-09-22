# Parameter Engineering

## Parameter Record

For every user-facing or algorithmic parameter, define:

- **Range**: min–max, inclusive
- **Units**: Hz, dB, semitones, %, or dimensionless
- **Default**: reset value
- **Scaling**: how does user control (0–100 knob) map to parameter range? (linear, logarithmic, piecewise)
- **Mapping function**: control value → mathematical coefficient or state update (e.g., knob position → cutoff frequency → filter pole)
- **Update rate**: when does the parameter take effect? (initialization, on control change, control-rate, block-rate, audio-rate)
- **Smoothing**: does the parameter change abruptly (click risk) or slew gradually? If smooth, what is the time constant?
- **Valid/invalid states**: are all values in range valid, or are some forbidden? (e.g., Q value must be > 0 for stability)
- **Reset behavior**: what happens when the user resets or loads a new session? (parameter value changes, internal state clears, or both?)

## Preferred Domain Separation

Minimize work on the audio thread:

```
User physical control (rocker, knob, button)
        ↓
Normalize / quantize (0–4095 from ADC, or discrete button event)
        ↓
Control-domain mapping (quantized knob → parameter space)
        ↓
Coefficient / state computation (expensive math: exp, sqrt, trig; produce audio-rate values)
        ↓
Audio-thread processing (multiply, add, feedback; use precomputed coefficients)
```

**Implications**:
- Parameter changes happen *on the control thread*, not the audio thread.
- Audio thread reads pre-computed coefficients via `volatile` mailbox or atomic load.
- Expensive operations (exp, sqrt, filter design) happen at control rate, not audio rate.
- Audio thread does only fast arithmetic (multiply, add, feedback).

## Optimization Hierarchy

Before audio-rate evaluation of an expensive function (exp, sqrt, sin, etc.):

1. **Compute at initialization** — if parameter never changes (constant cutoff frequency), compute once at startup
2. **Compute on parameter change** — if parameter changes rarely (user tweaks knob), recompute only then
3. **Compute at control rate** — if parameter changes regularly (LFO modulation, envelope), recompute every ~10 ms (Zephyr control tick)
4. **Compute at block rate** — if parameter must update more frequently, recompute every ~5 ms (audio block boundary)
5. **Lookup table or approximation** — if computation is still expensive, compare a table, interpolation, piecewise mapping, and polynomial against the target cost and error budget
6. **Compute at audio rate** — only if truly necessary (e.g., very high-rate FM modulation); measure the cost on real hardware

## Perceptual Resolution

Allocate resolution where it is audible, not where the control is precise.

**Example**: A 12-bit ADC (4096 levels) on a cutoff-frequency knob does not mean you need 4096 unique filter designs.

- If frequencies are logarithmic (octaves), allocate table entries log-spaced, not linearly
- If the ear is less sensitive in some range (e.g., very high frequency), use coarser resolution there
- If the original SP-1 firmware accepts a lower-resolution control (e.g., 256 frequency steps), preserve that resolution; do not artificially increase it

**Test**: does moving the knob one step produce an audible change? If not, reduce resolution.

## Example Parameter: Filter Cutoff Frequency

```
Control: rocker DC knob, 12-bit ADC, range [0, 4095]
Parameter: cutoff frequency fc, range [20 Hz, 20 kHz]

Scaling: logarithmic
  input_norm = ADC_value / 4095
  fc = 20 * 10^(input_norm * 2)  # 10^2 = 100 for 20 Hz → 20 kHz span

Update rate: on control change + 100 Hz smooth slew
  - Control thread: ADC reads fc_target every ~10 ms
  - Smooth slew: fc_current += 0.01 * (fc_target - fc_current)
  - Block-rate coefficient update: every 5.3 ms, compute filter pole from fc_current

Smoothing: 1-pole slew filter with ~100 Hz update rate prevents clicks

Coefficient computation:
  α = 1 - exp(−2π * fc / fs)  # fs = 48 kHz
  (expensive: candidate for lookup or another approximation; profile the alternatives)

Audio-rate usage:
  y[n] = y[n-1] + α * (x[n] - y[n-1])  # one multiply, one add per sample

Validation: frequency response on 1 kHz sine sweep; compare to reference
```

## Cross-Reference to Tape-Looper Architecture

The tape-looper Phase 6 ("Peel gestures out of main() into semantic events") separates physical controls from semantic intent. When implementing DSP parameters:

- Physical control (rocker, button press) → semantic event (speed change, FX engage)
- Semantic event → DSP parameter update (filter coefficient, delay time, reverb wet)
- DSP parameter → audio-thread coefficient

This layering keeps audio-path code simple and deterministic. Do not add new parameters directly to the audio thread; route them through the semantic-event layer.

Status: REFERENCE (discipline and examples documented; cross-reference to tape-looper Phase 6 architecture)
