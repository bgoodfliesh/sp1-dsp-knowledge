# SP-1 Control Inputs

## SAADC Overview

**VERIFIED**

Nordic nRF52840 SAADC (Successive Approximation ADC) reads:
- Four faders (analog inputs)
- Two resistive ladder controls (track selector, rocker/volume)
- Battery voltage
- All 12-bit resolution

Before reading ladders, enable power:
```
P1.10 = 1 (LADDER_POWER)
```

## SAADC Channel Assignments

**VERIFIED**

```
AIN0 = Track ladder selector
AIN1 = Rocker / volume ladder
AIN2 = Fader 3
AIN3 = Fader 1
AIN4 = Battery voltage
AIN6 = Fader 2
AIN7 = Fader 4
```

Typical SAADC configuration:
- 12-bit resolution
- Internal reference (~0.6 V)
- Gain of 1/6 (for 3.6 V input range)
- Single-ended measurement (0–3.6 V or 0–VDDH)

Exact SAADC settings should match established SP-1 firmware.

## Track Ladder Selector

**VERIFIED** (12-bit ADC targets)

The track ladder is a resistive voltage divider with five positions:

```
Position    ADC Value    ±3% Window
Track 1     240         233–247
Track 2     450         437–463
Track 3     816         792–840
Track 4     1357        1317–1397
Play        2037        1976–2098
```

### Ladder Behavior

The ladder produces discrete output voltages at specific knob positions. Intermediate positions produce intermediate voltages.

**Do not** treat as:
- Independent digital switches
- Precise analog measurement (±3% window is typical)

**Do** treat as:
- Coarse position detection
- Debounce around target values
- Detect transitions between positions

### Implementation Pattern

```c
// Pseudo-code
uint16_t raw_adc = read_ladder(AIN0);

if (in_window(raw_adc, 240, 233, 247)) {
    track = TRACK1;
} else if (in_window(raw_adc, 450, 437, 463)) {
    track = TRACK2;
} else if (in_window(raw_adc, 816, 792, 840)) {
    track = TRACK3;
} else if (in_window(raw_adc, 1357, 1317, 1397)) {
    track = TRACK4;
} else if (in_window(raw_adc, 2037, 1976, 2098)) {
    track = PLAY;
} else {
    track = UNKNOWN;  // mid-position or noise
}
```

Debounce by confirming same position over multiple reads (e.g., 3 consecutive stable readings).

## Rocker / Volume Ladder

**INFERRED** (approximate 12-bit targets)

The rocker/volume control produces discrete positions along a resistive ladder.

Approximate targets (INFERRED):
```
Rocker down    ≈ 450
Volume down    ≈ 816
Rocker center  ≈ 1200 (estimate)
Volume center  ≈ 1500 (estimate)
Rocker up      ≈ 1800 (estimate)
Volume up      ≈ 2037 (estimate)
```

Actual exact values depend on the mechanical design and resistor network. Measure on hardware.

The rocker and volume functions may be time-multiplexed or position-encoded. Determine from firmware behavior.

## Faders

**VERIFIED** (four analog inputs)

```
Fader 1 = AIN3 (12-bit, 0–4095)
Fader 2 = AIN6 (12-bit, 0–4095)
Fader 3 = AIN2 (12-bit, 0–4095)
Fader 4 = AIN7 (12-bit, 0–4095)
```

Faders produce smooth continuous values from bottom (0) to top (4095).

Implementation pattern:
```c
uint16_t fader1_raw = read_fader(AIN3);
float fader1_normalized = fader1_raw / 4095.0;  // 0.0–1.0

// Map to parameter (e.g., volume -inf to 0 dB)
float fader1_db = -60.0 + 60.0 * fader1_normalized;  // -60 to 0 dB
```

Faders should support:
- Smooth fading (no jumps from quantization)
- Hysteresis (small noise immunity)
- Ramping (DSP parameter updates slewed over time)

Do **not** treat fader changes as step-quantized; they are continuous analog measurements.

## Function Button

**VERIFIED** (GPIO P0.27)

The function button is a push-button input. Behavior:
- Press: P0.27 goes low (active-low)
- Release: P0.27 goes high (inactive)

### Button Behavior

**Do not infer** exact debounce timing or gesture semantics without evidence.

Firmware should:
- Read button state at regular intervals (e.g., every 10 ms)
- Debounce (confirm same state over 3+ reads)
- Detect transitions (press, hold, release)
- Map transitions to actions (e.g., function menu, power-off)

Established behavior in working firmware should be preserved.

## Battery Voltage

**VERIFIED** (SAADC AIN4)

Battery voltage is measured via:
```
AIN4 = P0.28
```

12-bit ADC reading corresponds to voltage via:
```
V_battery = ADC_value / 4095.0 × V_ref × gain
```

Exact conversion depends on SAADC configuration (V_ref, gain).

**Do not** assume battery percentage from voltage without explicit approximation:
- Lithium chemistry has nonlinear voltage-to-capacity curve
- Discharge curve depends on load current
- Temperature affects voltage measurement
- Charging curve differs from discharge curve

If battery percentage is needed:
1. Document the cell type and voltage range
2. Create a lookup table or curve-fitting function
3. Validate on real hardware across temperature/load conditions
4. Mark approximation as such

## Control Update Rate

**RECOMMENDED** (not verified in firmware)

```
Ladder update:     ~10 ms (detect track changes)
Fader update:      ~10 ms (smooth parameter control)
Button update:     ~10 ms (debounce)
Battery reading:   ~100 ms (slow-changing, low priority)
SAADC sampling:    ~1 kHz (fast, accumulate and downsample)
```

Actual rates depend on firmware design. Read from working firmware or tune based on responsiveness measurements.

## Control-to-DSP Boundary

Controls update at control rate (e.g., ~10 ms per call).

Control values are mapped to audio parameters on the control boundary:

```
Physical control (ladder/fader/button)
    ↓
Normalized value (0.0–1.0)
    ↓
Parameter mapping (control → audio space, e.g., log scale for frequency)
    ↓
Audio coefficient/state update
    ↓
Audio path (DSP uses precomputed coefficients)
```

See `skills/sp1-dsp-engineering/references/parameters.md` for parameter engineering discipline.

The audio thread should **not** read raw ADC values or perform expensive parameter calculations. Those happen on the control boundary.

## SAADC Configuration Checklist

When initializing SAADC:

- [ ] ADC peripheral enabled
- [ ] Channels configured (0–7)
- [ ] Reference voltage set
- [ ] Gain/attenuation set
- [ ] Sample rate configured (e.g., 1 kHz)
- [ ] Resolution set (12-bit)
- [ ] Single-ended or differential mode chosen
- [ ] Interrupt/DMA configuration (if used)
- [ ] First valid reading obtained
- [ ] Ladder power control available
- [ ] Battery voltage readings reasonable (3–4.2 V for lithium)
- [ ] Fader readings span full range (0–4095 on move)

## Debounce and Hysteresis

**RECOMMENDED** (specific algorithm not verified)

For ladder positions, use:
- Acceptance window: ±3% of target (documented above)
- Debounce: require 3+ consecutive stable readings
- Hysteresis: small bias (e.g., ±20 ADC counts) toward last-known position

For faders:
- Noise floor: <10 ADC counts typical
- Hysteresis: optional (continuous values tolerate small noise)
- Ramping: damp rapid changes in DSP with slew filter

For button:
- Debounce: 10–20 ms after state change
- Edge detection: react on press, not on release (or both, depending on action)

## Known Limitations

- SAADC is single-ended, not differential (susceptible to ground noise)
- 12-bit resolution limits continuous fader precision (~4000 distinguishable positions)
- Ladder reading requires enabling/disabling power (power consumption optimization)
- Faders lack mechanical detents (positions not quantized)

## Unverified

```yaml
unknown:
  - exact_SAADC_sample_rate_on_hardware
  - exact_fader_linearity_across_range
  - exact_control_update_latency
  - exact_button_debounce_timing
  - worst_case_ADC_noise_spectrum
  - power_consumption_of_ladder_reads
  - temperature_drift_of_ADC_measurements
```

Measure on hardware and document.
