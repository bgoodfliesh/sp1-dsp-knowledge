# Source selection notes

These are research leads, not approvals to copy code. Every implementation
must create its own engineering record and file-level provenance.

| Need | First sources to inspect | What to extract | What must be revalidated |
| --- | --- | --- | --- |
| Small filters, oscillators, effects | DaisySP | State layout, parameter update behavior, composable module boundaries | SP-1 sample-rate assumptions, reset, CPU, RAM, and artifacts |
| Mutable Instruments algorithm research | Mutable Instruments `eurorack` and `stmlib` | Algorithm behavior, fixed-point/float choices, MCU constraints, control-rate assumptions | License path, hardware I/O separation, SP-1 numerical and timing behavior |
| FFT and low-level math | CMSIS-DSP | Datatype variants, transform APIs, vectorization and compiler requirements | SP-1 core/ABI support, memory placement, flags, exact benchmark |
| Declarative DSP references | Faust libraries | Equations, canonical structures, parameter semantics | Per-file license, generated-code target, realtime allocation and latency |
| Physical modeling and synthesis | STK | Unit-generator structure, modal/physical-model concepts, educational references | Patent notices, stability, target cost, and production suitability |

## Evidence rule

An upstream repository being mature or popular does not advance an SP-1
component beyond `REFERENCE`. The component must earn
`REFERENCE-VALIDATED`, `SP1-COMPILES`, `SP1-TESTED`, and
`SP1-BENCHMARKED` through local evidence.
