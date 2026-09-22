# Validation strategy

Validation is layered: deterministic unit tests, mathematical/reference
comparisons, property and edge-case tests, integration tests, target tests,
and reproducible benchmarks. A result must state its scope and limitations.

Use `tools/templates/validation.md` for component evidence and keep failures
and known artifacts documented rather than silently excluding them.
