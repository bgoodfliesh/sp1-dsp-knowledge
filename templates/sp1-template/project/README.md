# {{ project_name }}

Generated from the SP-1 plugin development template.

- Author: {{ author }}
- MIDI selection: {{ midi_support }}
- Example gain: {{ "included" if include_example_gain else "not included" }}
- DSP selections: {{ dsp_modules | join(", ") }}

## Development status

This is a development template, not production-ready SP-1 firmware. Audio
hardware, ADC/button ladder calibration, MIDI transport drivers, LED hardware,
and BQ24232 power integration remain board-specific seams. Replace those seams
only when backed by device evidence and regression tests.

## Tests

Run the generated standalone unit/functional suite:

```bash
make -C tests/unit run
```

Read `AGENTS.md`, `tests/SKILL.md`, and `docs/knowledge-sources.yml` before
changing firmware or adding a plugin.

## Plugin development

Keep DSP reusable and hardware-independent where possible. Register modules
with `SP1_MODULE(...)`, document control mappings and resource limits, and
contribute broadly useful improvements back to the template repository.
