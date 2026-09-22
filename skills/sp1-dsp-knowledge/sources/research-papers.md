# DSP research papers

These papers are research references for future SP-1 work. They are not
evidence that an algorithm is suitable for the SP-1, and they do not replace
reference implementations, numerical analysis, target tests, or benchmarks.
Record the exact paper version consulted when extracting an algorithm.

## Fixed-point finite-precision analysis

**Fang, C. F., Rutenbar, R. A., & Chen, T. (2003).** “Fast, accurate static
analysis for fixed-point finite-precision effects in DSP designs.”
*ICCAD-2003*, 275–282. DOI:
[10.1109/ICCAD.2003.159701](https://doi.org/10.1109/ICCAD.2003.159701).

The DOI may require IEEE Xplore access. An open preprint or author manuscript
has not yet been pinned in this repository.

- **Repository relevance:** `knowledge/numeric.md`,
  `tools/templates/validation.md`, and future fixed-point test tooling.
- **Research question:** how to estimate finite-precision effects, including
  overflow and quantization/precision loss, before hardware deployment.
- **SP-1 use:** inform range analysis, Q-format selection, worst-case input
  vectors, and static checks for recursive filters.
- **Do not infer:** a paper-level analysis does not prove a particular SP-1
  implementation safe; coefficient quantization, scaling, saturation, state
  initialization, and compiler behavior still require local evidence.

## Fixed-point lattice WDFs

**Agarwal, M., & Rawat, T. K. (2016).** “VLSI Implementation of Fixed-Point
Lattice Wave Digital Filters for Increased Sampling Rate.”
*Radioengineering*, 25, 821–829. DOI:
[10.13164/re.2016.0821](https://doi.org/10.13164/re.2016.0821).
Open-access PDF:
[radioeng.cz/fulltexts/2016/16_04_0821_0829.pdf](https://www.radioeng.cz/fulltexts/2016/16_04_0821_0829.pdf).

- **Repository relevance:** `knowledge/numeric.md`,
  `knowledge/architecture.md`, `algorithms/nonlinear/`, and future WDF
  implementation records.
- **Research question:** fixed-point lattice WDF structures intended to
  improve throughput and preserve useful stability characteristics.
- **SP-1 use:** compare lattice/topology choices, delay-loop structure, fixed
  point scaling, and hardware throughput.
- **Do not infer:** stability or low coefficient sensitivity in one topology
  does not guarantee stability under SP-1 word length, saturation policy,
  parameter modulation, or nonlinear elements.

## Efficient embedded DSP architectures

**Chiper, D.-F., & Paleologu, C. (2023).** “Efficient Algorithms and
Architectures for DSP Applications.” *Electronics*, 12(4), 1012. DOI:
[10.3390/electronics12041012](https://doi.org/10.3390/electronics12041012).
Open-access article:
[mdpi.com/2079-9292/12/4/1012](https://www.mdpi.com/2079-9292/12/4/1012).

- **Repository relevance:** `knowledge/architecture.md`,
  `knowledge/realtime.md`, and `knowledge/optimization.md`.
- **Research question:** efficient algorithms and architectures for embedded
  DSP where deterministic throughput and low overhead matter.
- **SP-1 use:** frame boundaries between audio processing, event/control
  handling, memory ownership, and scheduling; identify optimization ideas to
  test on target hardware.
- **Do not infer:** “no RTOS” is not itself a design requirement. Any SP-1
  architecture must document interrupt priorities, bounded work, buffering,
  synchronization, and failure behavior.

## Citation and evidence policy

Paper summaries must distinguish:

1. what the paper explicitly evaluates;
2. what is hypothesized to transfer to the SP-1; and
3. what local test or benchmark would confirm the transfer.

Avoid copying equations or figures without checking publication rights.
Prefer a concise technical summary plus a citation and an independent
implementation.

## Real-time audio thread architecture

**Bencina, R.** “Real-time audio programming 101: time waits for nothing.”
[Article](http://www.rossbencina.com/code/real-time-audio-programming-101-time-waits-for-nothing).

- **Repository relevance:** `knowledge/realtime.md`,
  `knowledge/architecture.md`, and future `tools/` checks for callback-safe
  code.
- **Research question:** how to keep an audio callback within its deadline and
  avoid blocking, allocation, logging, locks, and unsafe cross-thread state
  mutation.
- **SP-1 use:** define the audio-thread contract, separate control/event work
  from sample processing, and evaluate bounded SPSC queues or equivalent
  message handoff.
- **Do not infer:** a lock-free queue is not automatically correct; ownership,
  memory ordering, overflow policy, reset behavior, and interrupt priority
  still require an SP-1 design record and tests.

## Fixed-point limit cycles

**Smith, J. O. III.** “Introduction to Digital Filters: Limit Cycles.”
[CCRMA reference](https://ccrma.stanford.edu/~jos/filters/Limit_Cycles.html).

- **Repository relevance:** `knowledge/numeric.md` and recursive filter
  validation records.
- **Research question:** how finite-word-length truncation in recursive
  filters can create zero-input oscillations and other limit-cycle behavior.
- **SP-1 use:** define zero-input, impulse-decay, quantization-noise, and
  saturation tests; compare state rounding and error-feedback strategies.
- **Do not infer:** mitigation techniques must be evaluated against SP-1
  precision, coefficient updates, denormal policy, and audible behavior.
- **Ingestion note:** the supplied CCRMA URL returned 404 during retrieval;
  citation remains a research lead pending URL/version confirmation.

## Trapezoidal-integrated state-variable filters

**Simper, A.** “Solving the continuous SVF equations using trapezoidal
integration.” [Cytomic PDF](https://www.cytomic.com/files/dsp/SvfLinearTrapOptimised2.pdf).

- **Repository relevance:** `algorithms/filters/`, `knowledge/numeric.md`,
  and filter validation records.
- **Research question:** optimized linear state-variable-filter equations using
  trapezoidal integration, with attention to stability and numerical behavior.
- **SP-1 use:** compare SVF topologies, coefficient update cost, modulation
  behavior, and state scaling against existing filter candidates.
- **Do not infer:** the paper’s equations do not establish fixed-point safety;
  test resonance limits, coefficient quantization, saturation, and rapid
  parameter changes on target.

## Wave Digital Filters

**Smith, J. O. III.** “Wave Digital Filters,” in *Physical Audio Signal
Processing*. [CCRMA chapter](https://ccrma.stanford.edu/~jos/pasp/Wave_Digital_Filters.html).

- **Repository relevance:** `algorithms/nonlinear/`, `knowledge/numeric.md`,
  and `sources/community/chowdsp-wdf-record.yaml`.
- **Research question:** converting physical networks into wave variables,
  bilinear/trapezoidal integration, and scattering junctions.
- **SP-1 use:** understand passivity-oriented circuit modeling and compare
  WDF implementations for distortion, filters, and analog-inspired effects.
- **Do not infer:** passivity in the ideal mathematical structure does not
  guarantee safe finite-word-length, nonlinear, or parameter-modulated SP-1
  behavior.

## WSOLA time-stretching

**Verhelst, W., & Roelands, M. (1993).** “An Overlap-Add Technique Based on
Waveform Similarity (WSOLA).” *ICASSP-1993*.
[Supplied PDF](http://www.etro.vub.ac.be/research/dssp/PUB_FILES/int_conf/ICASSP-1993.pdf).

- **Repository relevance:** `algorithms/granular/`, `effects/experimental/`,
  and future time-stretch validation records.
- **Research question:** waveform-similarity-guided overlap-add alignment for
  time stretching without changing nominal pitch.
- **SP-1 use:** research frame sizing, cross-correlation search bounds,
  overlap/window choices, latency, RAM, and artifact tradeoffs.
- **Do not infer:** WSOLA is not automatically low-cost; correlation search and
  buffering can dominate CPU/RAM and introduce latency.
- **Ingestion note:** the supplied PDF host failed retrieval during ingestion;
  preserve the citation and re-pin an accessible copy or DOI before
  implementation.
