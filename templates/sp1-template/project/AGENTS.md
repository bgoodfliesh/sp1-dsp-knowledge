# SP-1 project agent guide

Before changing firmware or plugin code, read:

1. `docs/knowledge-sources.yml`
2. Every source entry with `status: active` that applies to the requested
   subsystem
3. `tests/SKILL.md` before adding or changing tests

The knowledge registry is the source of truth for external engineering
references. Keep repository URL, ref, and path separate when updating it. If a
source cannot be fetched, do not silently use a different revision.

## Keep knowledge and tests current

When implementation or hardware work reveals a reusable fact, update the
closest durable artifact in the same change:

- Add a regression or boundary test for the behavior.
- Update `tests/SKILL.md` with the testing practice or device-specific case.
- Update the applicable external knowledgebase through a focused pull request
  when the finding is useful beyond this project.
- Keep the source registry and any local instructions synchronized with the
  accepted central reference.

Prefer small, reviewable pull requests to central knowledge repositories.
Include the evidence source for device-specific values and distinguish
measured facts from working assumptions.

## Contribute reusable improvements

This project is generated from an evolving SP-1 development template. When a
change is useful to more than this project, propose the generalized version
back to the template repository in a focused pull request. This includes new
test cases, hardware seams, plugin scaffolding, API fixes, DSP utilities, and
build or documentation improvements.

Keep one-off experiments local until they have a clear reusable contract.
When opening a template PR, describe the impact on newly generated projects
and include regression coverage where practical.

## Keep the creative goal visible

The purpose of this project is to help technical contributors share useful
knowledge with musicians and artists so more people can turn ideas into music.
Keep instructions kind and practical, explain assumptions instead of
gatekeeping them, and favor defaults that let someone make sound quickly.
Preserve rigor while keeping the path from code to creative result welcoming.

## Reusable plugin architecture

Prefer shared code and independently reusable plugins over project-local
copies. New plugin code should:

- Depend on the stable SP-1 API, not private application internals.
- Keep DSP separate from hardware integration.
- Expose explicit parameters and document control mappings.
- Include host tests plus standalone functional tests.
- State CPU, RAM, latency, sample-rate, and licensing constraints.

The included reverb is a usable, verified reference plugin. Improve it as a
real plugin and preserve its boundary so it can later move to an independent
plugin host or shared catalog. Do not turn reference plugins into throwaway
examples or silently fork shared implementations.

## Project conventions

- Keep hardware access behind narrow seams so it can be replaced by a
  deterministic test stub.
- Preserve the public API in `src/sp1_api.h` for plugin authors.
- Register audio modules with `SP1_MODULE(...)`.
- Add functional coverage in `tests/unit/` for every new module or behavior.
- Keep DSP host-testable without Zephyr where practical.

## Validation

```bash
make -C tests/unit run
```

When changing pure DSP code, also run the repository host suite if available:

```bash
make -C ../../tests run
```
