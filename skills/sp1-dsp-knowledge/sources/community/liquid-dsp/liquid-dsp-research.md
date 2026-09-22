# Liquid-DSP research notes

Liquid-DSP is a portable C DSP library originally focused on software-defined
radio. It is useful here as a reference for small filters, filter design,
oscillators, synchronizers, complex math, and polyphase resampling.

**Repository:** https://github.com/jgaeddert/liquid-dsp  
**License:** See the pinned source record and upstream license  
**Observed revision:** `d61cf506d427439f2ce38bf556362ad7a7aa8fa0`

## Relevant concepts

- Polyphase interpolators and decimators
- FIR filter design and filter objects
- Explicit create/process/destroy object lifetimes
- Optional SIMD, FFTW, threading, logging, tests, and benchmarks

## Reuse boundary

Treat object construction, coefficient generation, and processing as separate
operations. Any realtime consumer must prove that setup and processing paths
have bounded work and that optional dependencies are not accidentally required.
Use the algorithms as reference material or isolate a small component; do not
assume the complete library is lightweight for every target.

## Validation questions

- What state and scratch memory does the selected primitive require?
- Can setup and teardown be kept off the realtime path?
- What ratio, filter length, and block-size limits apply?
- What is the measured CPU and latency under the consuming firmware's target
  conditions?

## Status

`REFERENCE`: source inspected for algorithm and architecture ideas; no local
implementation or target benchmark is implied.
