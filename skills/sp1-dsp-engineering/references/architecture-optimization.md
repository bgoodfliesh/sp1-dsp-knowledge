# Structural optimization and execution context

## Principle

Before optimizing an inner loop, reduce the number of expensive operations or the number of times they execute.

This is the most portable general lesson from the Schwung case studies: not a specific DSP algorithm, but a design pattern.

```text
1. REMOVE THE WORK
2. SHARE THE WORK
3. MOVE THE WORK
4. REDUCE THE WORK
5. MAKE THE REMAINING WORK FASTER
```

The cheapest DSP operation is the operation the architecture makes unnecessary.

## Topological optimization

Topological optimization changes the signal-flow structure so expensive work happens fewer times.

Example:

```text
stem 1 ─┐
stem 2 ─┤
stem 3 ─┼→ PREMIX → ONE PROCESSOR
stem 4 ─┘
```

This is preferable to running an expensive processor per stem and then mixing later if the downstream semantics permit a shared processor. The benefit is structural, not merely a micro-optimization.

Document the tradeoffs explicitly:

- independent downstream processing lost
- altered signal semantics
- nonlinear shared interactions
- reduced routing flexibility
- increased state coupling

Treat topology changes as architectural decisions, not free optimizations.

## Execution context classification

Every realtime DSP function should have a known execution context.

Document each routine or callback as one of:

```text
REALTIME
CONTROL
LOAD / INITIALIZATION
BACKGROUND / WORKER
BUILD / OFFLINE
```

This matters because realtime safety depends on knowing where code executes, not just what it appears to do.

Examples:

- parameter snapshot: CONTROL or BLOCK-BOUNDARY
- gain curve generation: CONTROL or LOAD
- asset decoding: LOAD / OFFLINE
- DSP processing: REALTIME
- logging/UI state: never in the realtime path unless explicitly bounded and proven safe

## Realtime safety

A realtime callback should have a predictable upper execution bound.

Avoid operations whose runtime depends unpredictably on external state, including:

- allocation or free
- blocking mutexes or waits
- file or network access
- parsing or config interpretation
- unbounded iteration
- dynamic container growth
- lazy initialization
- logging with unpredictable backpressure
- background synchronization that can stall the audio thread

The underlying rule is not simply "never call X"; it is:

> operations with externally dependent or nondeterministic runtime threaten the block deadline.

## Allocation and lifetime strategy

The safest general model is:

```text
INIT
  ↓
allocate state
allocate delay buffers
allocate scratch memory
construct tables
parse configuration

PREPARE
  ↓
configure for sample rate / block size

PROCESS
  ↓
bounded realtime execution only

DEINIT
  ↓
release resources
```

Prefer preallocation before processing begins whenever practical, and allocate according to simultaneous lifetime rather than total algorithm count.

## Shared scratch and buffer design

Instead of giving every processor its own private scratch, consider a shared scratch arena when lifetimes do not overlap:

```text
PREALLOCATED SCRATCH ARENA
A borrows → A finishes → B borrows → B finishes
```

This is correct only when:

- lifetimes do not overlap
- there is no reentrant call path
- an async consumer cannot retain stale pointers
- subsequent work does not rely on overwritten data

This is a general memory strategy, not a universal rule.

## Buffer copies are work

Before adding an intermediate buffer, ask whether the transformation can happen:

- in place
- directly into the destination
- into an accumulation bus
- by writing to the same buffer with explicit alias safety

Prefer:

```text
READ → MODIFY → WRITE SAME BUFFER
```

over:

```text
READ → TEMP BUFFER → COPY → OUTPUT BUFFER
```

when the algorithm permits it. Buffer copies, clears, and mixing passes all consume CPU and memory bandwidth and should be treated as DSP work.

## Activity-aware suspension

Do not encode a blanket rule such as "silence means stop DSP". Many processors maintain state that must be released cleanly.

Instead define suspension only when both external activity and relevant internal state are safely inactive.

Useful states include:

- delay tail / reverb tail
- oscillator autonomy
- resonator energy
- envelope or modulation state
- note or event queue status
- feedback energy or tail release

A safe model is:

```text
ACTIVE → RELEASE / TAIL → SLEEPING
```

with explicit wake conditions such as input audio, note events, parameter changes, or internal energy rises.

## Control-rate and precomputation rules

Many expensive values should not be computed at audio rate if they are not required to change every sample.

Prefer:

```text
read control
  ↓
compute target
  ↓
interpolate / slew
  ↓
audio DSP
```

Possible examples:

- expensive nonlinear maps
- filter coefficient generation
- control curves
- gain and saturation coefficients
- modulation envelopes
- precomputed asset conversion

The correct update interval depends on modulation bandwidth, coefficient sensitivity, algorithm stability, and audible stepping; there is no universal sub-block size.

## Precomputation and asset baking

If a value does not depend on realtime state, move the work upward when possible.

Examples:

- lookup tables
- waveform tables
- control maps
- preset or routing translations
- asset normalization
- resampling at load time
- parameter compilation

This is especially valuable for expensive source representations and large assets.

## Evidence and limits

The strongest general findings are structural and lifecycle-oriented:

- remove unnecessary work first
- share expensive work when semantics permit
- move work out of the realtime path when safe
- reduce update rate or precision only after structure is examined
- optimize only the remaining work

They should not be converted into exact percentages, fixed thresholds, or target-specific values unless measured and sourced for the actual SP-1 target.

This file documents the general principle; remaining numeric values stay subject to target measurement and evidence labeling.

Status: `REFERENCE`
