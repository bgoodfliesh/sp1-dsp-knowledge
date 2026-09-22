# CloudVerb effects

Add composed CloudVerb implementations here.

Record topology, memory, latency, decay behavior, numerical format, CPU cost,
artifacts, and reference or hardware validation.

## Current implementation

[`implementation/cloudverb.h`](implementation/cloudverb.h) is the working SP-1
CloudVerb extracted from the SP-1 tape-looper firmware. It is a Clouds-derived
12 kHz stereo network with four input diffusers, two damped feedback loops,
Q8 controls, and four-frame wet interpolation for 48 kHz blocks.

The implementation is intentionally header-only and `static inline`: its
12 kHz network step runs 64 times per 256-frame block, and keeping it in the
audio translation unit preserves inlining. It uses about 40 bytes of state
plus a shared 4,608-sample `int16_t` line. Echo and CloudVerb are mutually
exclusive users of that line.

### Provenance and status

- **Origin:** `sp1-tape-looper-3-2/firmware/src/dsp/reverb.h`
- **License:** MIT, from the source project
- **Status:** `REFERENCE-VALIDATED`; host regression source is in
  [`tests/host/`](../../tests/host/)
- **Known boundary:** the kernel expects a 4,608-sample shared line and
  firmware-owned lifecycle/engagement behavior; it is not a drop-in effect
  wrapper for arbitrary hosts

Do not change delay lengths, bases, mask, fixed-point scaling, clamp behavior,
interpolation, or shared-line ownership without updating the regression
oracle and documenting the reason.
