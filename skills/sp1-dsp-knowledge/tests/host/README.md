# Host regression tests

These tests run without Zephyr or SP-1 hardware. They prove deterministic
algorithm behavior against a frozen reference; they do not prove target CPU,
RAM, latency, or audio quality.

## CloudVerb

From this directory, compile and run:

```sh
cc -std=c11 -O2 -Wall -Wextra -Werror -o test_cloudverb \
  test_cloudverb.c
./test_cloudverb
rm test_cloudverb
```

The test compares the checked-in cloudverb kernel with
`cloudverb_oracle.h` across zero, impulse, constant, full-scale, ramp, room, and
hall vectors. It also checks coefficients, clipping, state, and delay-line
contents.
