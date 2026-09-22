# SP-1 template agent guide

Read this file before changing the Copier template.

## Required references

1. Read the generated project's `docs/knowledge-sources.yml` when working on
   firmware, DSP, controls, MIDI, codec, power, or board-specific behavior.
2. Follow the source entry whose `status` is `active`. Its `url`, `ref`, and
   `path` identify the intended version of the engineering knowledgebase.
3. Read `tests/SKILL.md` before adding or changing tests.

If a reference is unavailable, do not silently substitute a different branch or
source. Note the limitation and keep implementation-specific assumptions
explicit.

## Knowledge and test maintenance

Treat implementation findings as reusable project knowledge. When you discover
an ADC value, timing constraint, hardware behavior, DSP invariant, workaround,
or other fact that future developers will need:

- Update the relevant local skill, test guidance, or device-specific expansion
  point.
- Add or strengthen a deterministic test that protects the finding whenever
  practical.
- If the finding belongs in an external active knowledgebase, prepare a
  focused pull request against that source repository rather than keeping the
  improvement local only.
- Keep the local registry's URL, ref, and path accurate if the central source
  moves.

Do not claim a hardware fact from an unverified assumption. Record whether it
came from a schematic, datasheet, measurement, or firmware observation.

## Contribute improvements back

Treat this repository as an expanding template, not only a starter snapshot.
When a change is broadly useful to future SP-1 projects, propose it here in a
focused pull request. Good candidates include:

- Reusable tests, test seams, and test-writing guidance
- Stable API improvements and module scaffolding
- Corrected build, Copier, or project-layout behavior
- General DSP helpers and documented performance constraints
- Verified device-specific knowledge that should ship in every project

Keep project-specific experiments and unverified hardware work in the
generated project until they are generalized and supported by tests or
evidence. In template PRs, explain what future projects gain and include the
smallest validation that demonstrates the improvement.

## Keep the creative goal visible

This template helps technically experienced contributors share their knowledge
with musicians and artists who should not need to understand every embedded
systems detail to make something expressive. Optimize for clear explanations,
approachable defaults, reusable abstractions, and positive collaboration.
Technical precision should expand creative access, not become a barrier to it.

## Prefer reusable plugins and shared code

Design new functionality as a reusable plugin or shared library whenever
possible rather than embedding it in one generated application. Keep the
plugin boundary explicit:

- Stable public API and configuration
- No direct dependence on unrelated application state
- Hardware access through shared seams or adapters
- Host-testable DSP and deterministic functional tests
- Documentation of controls, performance, memory, and licensing

The shipped reverb is an intentionally usable reference plugin, not disposable
sample code. Treat verified plugins as candidates for a future independent
plugin host or shared catalog. Promote a plugin only after its behavior,
resource limits, hardware assumptions, and tests are documented. Preserve
compatibility when moving a verified plugin out of this template.

## Template layout

- `copier.yml` defines generator questions and rendering.
- `project/` is the template source directory whose contents become the
  generated project root.
- The generated project's `docs/knowledge-sources.yml` is its reference
  registry.
- `tests/` contains host-side tests for the template and DSP.
- The generated project's `tests/unit/` contains standalone tests.

Keep template-only files outside `{{project_slug}}` unless they are intended
to be copied into every generated project. Update README instructions when
paths or commands change.

## Validation

Run the host suite after source or DSP changes:

```bash
make -C tests run
```

Run the generated standalone unit suite from a rendered project:

```bash
make -C tests/unit run
```
