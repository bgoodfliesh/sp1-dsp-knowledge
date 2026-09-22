# Generic DSP architecture notes

Document generic processing contracts, ownership and lifetime of state, block
scheduling, parameter transport, channel conventions, and real-time safety
rules here. Do not duplicate the SP-1 firmware audio graph or board contract.
Link to implementation records and consumer integration notes as those become
available.

## Open questions

- Processing granularity assumptions: document per-sample, block, or hybrid behavior
- Channel/interleaving convention: document per component
- Parameter update and smoothing contract: document per component
- Allocation and locking policy in the real-time path: no hidden allocation or blocking
- Numeric format and SIMD assumptions: document portable baseline and optional paths
