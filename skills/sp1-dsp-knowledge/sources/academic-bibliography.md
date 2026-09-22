# Extended DSP reading list

This list contains additional references that complement the canonical records
in `research-papers.md`. Repeated papers are intentionally not duplicated here.
No entry is an implementation approval; record exact editions and local
evidence before relying on a result.

## Filter design

- Oppenheim, A. V., & Schafer, R. W. (2009). *Discrete-Time Signal
  Processing* (3rd ed.). Pearson. Fundamental FIR/IIR design, bilinear
  transforms, and frequency warping.
- Parks, T. W., & McClellan, J. H. (1972). “Chebyshev Approximation for
  Nonrecursive Digital Filters with Linear Phase.” *IEEE Transactions on
  Circuit Theory*. Remez-exchange FIR design and linear-phase tradeoffs.

## Time-frequency and granular processing

- Laroche, J., & Dolson, M. (1999). “Improved Phase Vocoder Time-Stretching
  at Lower Computational Cost.” *ICASSP*. Phase-vocoder alternatives and
  phase-continuity issues; measure latency and artifacts before considering
  embedded use.
- Roads, C. (2004). *Microsound*. MIT Press. Granular synthesis concepts,
  grain density, windows, and perceptual tradeoffs.

## Nonlinear and analog modeling

- Huovilainen, A. (2004). “Nonlinear Digital Implementation of the Moog Ladder
  Filter.” *DAFx*. Nonlinear ladder modeling and oversampling considerations.
  Verify the exact edition and citation before implementation.
- Schroeder, M. R. (1962). “Natural Sounding Artificial Reverberation.”
  *Journal of the Audio Engineering Society*, 10(3). Comb/all-pass
  reverberator foundations.
- Dattorro, J. (1997). “Effect Design, Part 1: Reverberator and Other
  Filters.” *Journal of the Audio Engineering Society*, 45(9). Practical
  reverberator topology and modulation ideas.

## Architecture and numerical methods

- Hennessy, J. L., & Patterson, D. A. (2011). *Computer Architecture: A
  Quantitative Approach* (5th ed.). Morgan Kaufmann. Memory hierarchy,
  bandwidth, and performance-measurement context.
- Volder, J. E. (1959). “The CORDIC Trigonometric Computing Technique.”
  *IRE Transactions on Electronic Computers*, EC-8(3). Shift/add
  trigonometric computation and convergence.
- Trefethen, L. N. (2013). *Approximation Theory and Approximation Practice*.
  SIAM. Polynomial and rational approximation, error bounds, and tradeoffs.

## Use

Start with `sources/research-papers.md` for papers with structured metadata
and explicit local-evidence requirements. Add a dedicated record when a
reference becomes the basis for an implementation.
