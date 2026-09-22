# SP-1 Test-writing skill

Use this guide when adding or reviewing tests in this template. Prefer tests
that are deterministic, hardware-independent, and explicit about the boundary
they verify.

## Choose the right test layer

### Host tests

Use the root `tests/` Makefile for pure C and DSP code that does not require
Zephyr:

```bash
make -C tests run
make -C tests wav
```

Host tests should cover numerical behavior, buffer ownership, clipping,
parameter ranges, reset determinism, dry/wet behavior, and long-running
stability. Keep generated artifacts out of the repository.

### Generated unit/functional tests

Use the generated project's `tests/unit/` for firmware logic that normally includes
Zephyr headers but can run without Zephyr, a kernel, or a board:

```bash
make -C tests/unit run
```

The test build compiles production C sources with `SP1_HOST_TEST`, replacing
only Zephyr initialization and iterable-section plumbing with deterministic
test shims. Add production code through a narrow hardware seam, then replace
that seam in the test with a deterministic fake. Do not include the whole
application just to reach one function.

### Hardware or integration tests

Use a separate `native_sim` or real-board test only when behavior depends on
kernel scheduling, devicetree, drivers, DMA, timing, or electrical behavior.
The unit test board cannot validate those properties.

## Hardware seam rules

Hardware-facing code should be behind small functions or interfaces. A test
double should be able to:

- Set ADC/fader values directly.
- Set individual GPIO/button states.
- Capture LED writes and verify brightness/state transitions.
- Capture MIDI bytes and received callbacks.
- Report I2S/audio buffer events without real DMA.
- Return injected driver errors.
- Record watchdog install, setup, and feed calls.

Keep fake state reset in a suite `before` hook. Never depend on test order,
wall-clock time, random input, host audio devices, or uninitialized memory.

Prefer testing reusable plugin contracts over application-specific wiring.
Every reusable plugin should have a pure host test path and a functional path
through the shared SP-1 API. Keep hardware adapters thin so the same plugin
can be validated in this template and, later, by an independent plugin host.

Tests are also documentation for contributors with different levels of
technical experience. Use descriptive names and failure messages, explain
non-obvious device assumptions, and make the shortest useful command easy to
find. The goal is confidence that helps people make music, not ceremony for
its own sake.

## SP-1-specific practices

- Test fader values at `0`, `1`, midpoint, just inside/outside each clamp, and
  invalid enum values.
- Test every button through idle → pressed → held → released, including
  repeated scans while held.
- Verify button edge events are emitted once per transition.
- Test the ADC ladder with values at the center and both sides of every
  acceptance window; include values exactly on the boundary.
- Verify LED brightness is clamped and invalid LED indices do not write
  hardware.
- Verify MIDI status bytes mask the channel correctly and data bytes stay in
  the `0..127` range.
- Test MIDI callback registration, replacement, and null callbacks.
- Test audio modules with silence, an impulse, maximum positive/negative
  samples, zero frames, and blocks that are not a multiple of the DSP period.
- Verify module registration names are stable and modules do not write outside
  their output buffer.
- Test watchdog setup failure and feed failure as observable error paths.
- Keep audio-thread tests allocation-free and avoid logging from tight loops.

## Test review checklist

Before adding a test, identify the contract it protects and the smallest
source set needed to exercise it. A good test:

1. Names the behavior rather than the implementation.
2. Arranges all inputs and fake hardware state locally.
3. Makes one meaningful action.
4. Asserts outputs and important interactions.
5. Cleans up or restores shared state.

Use `zassert_*` for Zephyr unit tests and the existing assertions in the host
runner. Prefer exact assertions for protocol/state values and tolerance-based
assertions for floating-point calculations.

## Device-specific expansion points

This guide intentionally starts with stable contracts. Add measured values and
board-specific procedures here as they become available:

- Exact ADC channel, reference voltage, resolution, and ladder resistor values.
- Fader calibration curve, dead zones, and acceptable tolerances.
- Button ladder voltage table and debounce interval.
- GPIO polarity, pull configuration, and boot-time pin states.
- LED driver topology, PWM frequency, polarity, and safe startup state.
- I2S sample rate, word size, channel order, DMA period, and underrun behavior.
- Codec reset/address sequence and required power-up delays.
- USB/BLE MIDI transport configuration and cable/disconnect behavior.
- Battery/charger thresholds and safe low-power transitions.
- Watchdog timeout, bootloader entry sequence, and recovery procedure.
- Which tests can run through the board runner and what telemetry proves pass.

Record the source of each value (schematic, datasheet, measurement, or
firmware observation) and add boundary tests whenever a value changes.

## Feed findings back to the knowledgebase

Tests and skills are living engineering artifacts. When a test exposes a new
device constraint, DSP invariant, timing rule, calibration value, or reliable
workaround:

1. Keep the regression test in the project.
2. Update this guide or the appropriate device-specific section with the
   reusable practice.
3. Update the active external knowledgebase listed in
   `docs/knowledge-sources.yml` when the finding applies to other projects.
4. Open a focused pull request to that central knowledge repository, including
   the evidence and affected test, so the improvement can be reviewed and
   reused.

Do not copy an unverified assumption into shared skills. Mark uncertain
behavior as a hypothesis until it is confirmed by a datasheet, schematic,
measurement, or repeatable firmware observation.
