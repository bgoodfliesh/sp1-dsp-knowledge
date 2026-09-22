# SP-1 Firmware Template

Guided project generator for custom Teenage Engineering SP-1 firmware using the community `sp1_api` framework.

## Create a new project

```bash
# once
pipx install copier

# from a local checkout
copier copy /path/to/basic-firmware-template my-project

# once this template is published
copier copy <template-repository-url> my-project
```

Answer the short questionnaire (name, MIDI options, which DSP modules to include).
You get a ready-to-build Zephyr application that already contains:

- Stable `sp1_api.h`
- Core skeleton (audio, controls, system)
- Optional example modules and DSP (e.g. Clouds-derived reverb)

## Build & flash

```bash
cd my-project
west build -b sp1
# then use https://solderless.engineering (hold Track 1 + Track 4)
```

## Adding your own DSP

1. Drop a pure header in `lib/dsp/` (or keep it next to the module).
2. Write a thin `src/modules/my_fx.c` that uses `SP1_MODULE(...)`.
3. Rebuild — the module registers automatically via iterable sections.

See `dsp/reverb/` in this repo for a complete example extracted from the tape-looper firmware.

## Host-side tests

The dependency-free host test suite exercises the reusable DSP without Zephyr or
an SP-1:

```bash
make -C tests run
make -C tests wav  # also writes tests/reverb_test.wav
```

Generated projects also include a standalone unit/functional test application:

```bash
make -C tests/unit run
```

It links the real controls and audio logic under test, with Zephyr initialization
and iterable-section behavior replaced by small deterministic test shims.
Hardware reads are stubbed, covering control input edges and iterable
audio-module processing without requiring Zephyr, a board, or an SP-1. The
generated baseline also checks invalid control indices, zero-frame audio calls,
output-buffer guards, module metadata, and one-shot button edges.

Test-writing guidance, including hardware seams and SP-1-specific test cases,
is in [`tests/SKILL.md`](tests/SKILL.md).

## Knowledge sources

Coding agents should read `AGENTS.md` first. It directs them to the generated
knowledge registry and test-writing guidance before implementation work.

Each generated project has a configurable reference registry at
`docs/knowledge-sources.yml`. Add skills or knowledgebases there instead of
scattering URLs through source files or instructions:

```yaml
knowledge_sources:
  - id: example-source
    name: Example engineering knowledge
    url: https://github.com/example/project
    ref: main
    path: skills
    purpose: What this source should be used for
    status: active
```

Keep `url`, `ref`, and `path` separate. When a repository or branch moves,
update the registry entry and leave the source-specific path and purpose
explicit. Use `status: archived` for references that should remain documented
but no longer guide new development.

Bundled DSP may retain separate upstream licensing or attribution
requirements. Check each file's header before redistributing or extracting a
plugin; the repository license does not override third-party notices.

## Contributing improvements

This repository is intended to grow into a reusable SP-1 plugin-development
template. If a project uncovers a broadly useful test, API improvement, module
scaffold, build fix, or documented hardware behavior, contribute the
generalized change back here through a focused pull request. Keep experiments
that are specific to one plugin in that project until they can be generalized
and verified.

## Reusable plugins and shared code

The template favors a shared ecosystem over isolated firmware forks. Build new
DSP and device features as reusable plugins or libraries behind stable API and
hardware-adapter boundaries. The included reverb is a usable reference plugin
and a candidate for eventual promotion into an independent host or verified
plugin catalog; it is not merely example code.

When extracting a verified plugin, preserve its public contract, tests,
resource documentation, control mapping, licensing, and compatibility with
projects generated from this template. Prefer sharing improvements upstream
over maintaining private copies.

## The goal

This project exists to help people with deep technical knowledge share that
knowledge in approachable, usable forms with people who want to make music.
The point is not to make everyone become an embedded-systems expert; it is to
lower the barrier between an idea and a playable instrument so more people can
make art.

Keep contributions welcoming, practical, and positive. Explain the why,
document the sharp edges, celebrate working creative results, and prefer
helpful abstractions over unnecessary gatekeeping. Technical rigor and
encouragement belong together.

## License

Template and API: community / MIT-style.
Individual DSP modules may carry their own notes (see file headers).
