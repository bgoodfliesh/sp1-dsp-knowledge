# Numerical considerations

Capture floating-point or fixed-point formats, range analysis, denormal
handling, coefficient interpolation, stability bounds, quantization, clipping,
NaN/Inf behavior, and reset/initial-state requirements. Link each rule to
tests that exercise it.

## Fixed-point research leads

For recursive or nonlinear fixed-point work, consult
`sources/research-papers.md` and the machine-readable records in
`sources/research-papers.yaml`. Treat limit cycles, overflow, coefficient
quantization, and truncation as claims requiring explicit test vectors and
range evidence. Do not promote a fixed-point component based on host audio
quality alone.

The Smith limit-cycle, Simper SVF, and Smith WDF records in
`sources/research-papers.yaml` are useful starting points for recursive filter,
finite-precision, and topology research. They do not replace SP-1 range,
stability, or artifact tests.
