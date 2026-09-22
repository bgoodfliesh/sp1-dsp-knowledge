# Status definitions

Statuses are ordered evidence gates, not quality labels. A component may use
the highest status it has earned and should list missing evidence explicitly.

Evidence labels are orthogonal to lifecycle status:

| Evidence label | Meaning |
| --- | --- |
| `VERIFIED` | Supported by reliable technical evidence or target measurement. |
| `DERIVED` | Follows from established theory but is not target-validated. |
| `ESTIMATED` | Plausible estimate, not a measurement. |
| `HYPOTHESIS` | Proposed behavior worth testing. |
| `UNKNOWN` | Insufficient evidence. |

| Status | Meaning |
| --- | --- |
| `RESEARCH` | References and problem understanding are being collected; no implementation claim. |
| `REFERENCE` | A reference algorithm or implementation is identified and understood, but not validated for SP-1. |
| `EXPERIMENTAL` | A local exploratory implementation exists; behavior, numerical limits, or resource use may be incomplete. |
| `REFERENCE-VALIDATED` | The implementation agrees with a trusted reference under documented test conditions. |
| `SP1-COMPILES` | The implementation builds for the SP-1 target/toolchain. |
| `SP1-TESTED` | Target integration tests pass on SP-1 or its documented target-equivalent environment. |
| `SP1-BENCHMARKED` | Reproducible SP-1 CPU/RAM/latency measurements are recorded. |
| `PRODUCTION` | Review, validation, integration, failure behavior, and benchmark evidence meet project release criteria. |

## Provenance kinds

Each component must identify one kind:

- `original-algorithm`
- `adapted-algorithm`
- `derivative-implementation`
- `independent-reimplementation`
- `reference-only`
