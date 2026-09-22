# SP-1 knowledge skills

This directory contains reusable knowledge blocks for developing the Teenage
Engineering SP-1. Each skill is a focused set of guidance documents, checklists,
constraints, and reference material for use during firmware and DSP work.

## Available skills

### [`sp1-firmware`](sp1-firmware/)

Firmware and hardware knowledge for the SP-1:

- Hardware components, pins, buses, and peripheral ownership
- Audio transport, storage formats, streaming, and eMMC constraints
- Controls, Bluetooth, bootloader, watchdog, and recovery behavior
- Firmware architecture, threading, APIs, and error handling

Start with [`sp1-firmware/SKILL.md`](sp1-firmware/SKILL.md), then consult the
relevant files in [`sp1-firmware/references/`](sp1-firmware/references/).

### [`sp1-dsp-engineering`](sp1-dsp-engineering/)

Engineering guidance for implementing DSP within the SP-1 firmware boundary:

- Real-time audio-thread safety and deterministic execution
- Q16 fixed-point arithmetic and numerical validation
- State ownership, parameter mapping, CPU budgeting, and memory budgeting
- Optimization methodology and validation status

Start with [`sp1-dsp-engineering/SKILL.md`](sp1-dsp-engineering/SKILL.md), then
use the focused checklists in
[`sp1-dsp-engineering/references/`](sp1-dsp-engineering/references/).

### [`sp1-ui-ux`](sp1-ui-ux/)

Relational control grammar for designing coherent SP-1 instrument interfaces:

- Stable physical semantics across pages
- Functional and opposing control relationships
- FN interaction tiers and tactile learnability
- Experiential parameter naming and layout validation

Start with [`sp1-ui-ux/SKILL.md`](sp1-ui-ux/SKILL.md), then use the focused
guidance in [`sp1-ui-ux/references/`](sp1-ui-ux/references/).

## How to use the skills

1. Read the relevant `SKILL.md` before proposing an architecture or changing
   an audio or firmware path.
2. Use the reference documents as design and review checklists.
3. Treat claims as evidence-based: distinguish verified facts, estimates,
   compiled results, and hardware measurements.
4. Preserve existing behavior unless a change is intentional, documented, and
   validated.
5. Keep cross-references relative to this repository so the skills remain
   usable in a fresh checkout.

The skills describe engineering constraints and practices; they do not replace
source code, hardware testing, or review of the current firmware implementation.
