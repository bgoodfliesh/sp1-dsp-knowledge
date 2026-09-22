# Real-Time Memory-Efficient Architecture

## Core Principles

Deterministic real-time audio requires eliminating dynamic allocation and CPU-bus contention entirely.

**Key constraint:** No allocations in audio callback. Ever.

## DMA "Ping-Pong" Buffering

The optimal real-time pattern for moving audio data:

### Setup

- Pre-allocate one array: `int32_t buffer[2 * BLOCK_SIZE]`
- Configure DMA in Circular Mode
- Codec writes directly to hardware DMA destination

### Execution

```
Hardware DMA:
  Writes to buffer[0..255]         ← Ping
  Triggers Half-Transfer Interrupt
  
  Writes to buffer[256..511]       ← Pong
  Triggers Full-Transfer Interrupt

CPU Audio ISR:
  HT Interrupt → Process buffer[0..255]
  FT Interrupt → Process buffer[256..511]
```

### Result

- **Zero CPU cycles** spent moving data (DMA does it)
- **Zero allocations** (single pre-allocated buffer)
- **Deterministic latency** (interrupt-driven, not polled)
- **No secondary buffers** (process in-place)

## In-Place Processing

All DSP must modify buffers destructively:

```c
// CORRECT: In-place (half the RAM)
void effect_process(q16_t* buffer, size_t samples, state_t* state) {
    for (size_t i = 0; i < samples; i++) {
        buffer[i] = apply_filter(buffer[i], state);
    }
}

// WRONG: Separate output array (double RAM, no benefit)
void effect_process(const q16_t* input, q16_t* output, size_t samples) {
    for (size_t i = 0; i < samples; i++) {
        output[i] = apply_filter(input[i]);
    }
}
```

**Benefits:**
- ✓ 50% RAM savings
- ✓ Better cache locality
- ✓ Simpler state management

## Lock-Free IPC (Interprocess Communication)

### SPSC Ring Buffers

For passing UI events to audio thread without locks:

```c
// Lock-free Single-Producer/Single-Consumer ring buffer
typedef struct {
    q16_t buffer[256];
    volatile uint16_t write_pos;  // Only producer writes
    volatile uint16_t read_pos;   // Only consumer reads
} spsc_ringbuffer_t;

// In UI thread
void queue_parameter_change(spsc_ringbuffer_t* rb, q16_t value) {
    // No locks; producer owns write_pos
    rb->buffer[rb->write_pos] = value;
    rb->write_pos = (rb->write_pos + 1) & 0xFF;  // Wrap at 256
}

// In audio ISR
void audio_callback(spsc_ringbuffer_t* rb, state_t* state) {
    // No locks; consumer owns read_pos
    while (rb->read_pos != rb->write_pos) {
        state->parameter = rb->buffer[rb->read_pos];
        rb->read_pos = (rb->read_pos + 1) & 0xFF;
    }
    // Process audio with updated state
}
```

**Why this works:**
- No locks (avoids priority inversion)
- No race conditions (each thread owns its pointer)
- Bounded latency (ring buffer is pre-allocated)
- Atomic pointer updates (on 32-bit ARM)

### Fast-memory placement

Some targets provide tightly coupled or otherwise lower-latency memory.
Placement is target-specific and must be confirmed in the consumer firmware's
linker configuration:

```c
// Force critical buffers into fast memory
__attribute__((section(".dtcmram")))
static q16_t filter_state[MAX_FILTERS];

__attribute__((section(".dtcmram")))
static q16_t delay_line[8192];
```

Measure the impact rather than assuming that a section attribute improves
latency or determinism.

## Constraint: No RTOS

Real-time audio does not need a full RTOS. Instead:

1. **Audio ISR** at highest priority (interrupt-driven)
2. **Streaming/background worker** at medium priority (if the product needs it)
3. **MIDI/Control** at lower priority (UI events)
4. **Main loop** at background priority (setup, monitoring)

All shared data is **pre-allocated** and **lock-free**.

## Memory Layout

Typical pre-allocation for a reverb effect on SP-1:

```c
// Global audio buffers (never freed)
static q16_t ping_pong[2 * BLOCK_SIZE];      // 512 samples × 4 B = 2 KB
static q16_t delay_line[8192];               // 8K samples × 4 B = 32 KB
static reverb_state_t reverb_state = {0};    // ~500 B

// Per-effect state
static filter_state_t filters[4] = {0};      // 4 × ~100 B = 400 B

// Ring buffers for UI→Audio communication
static spsc_ringbuffer_t parameter_queue;    // 256 × 4 B = 1 KB

// TOTAL for reverb: ~35 KB (leaves ~220 KB for other effects)
```

No malloc. No free. No fragmentation. Deterministic.

## Key Takeaway

**Architecture first, algorithms second.**

A well-designed memory architecture:
- ✓ Prevents glitches and dropouts
- ✓ Makes CPU budget predictable
- ✓ Enables lock-free communication
- ✓ Allows safe nested interrupts
- ✓ Simplifies testing and debugging

A poorly-designed architecture (dynamic allocation, locking, blocking I/O in ISR):
- ✗ Causes unpredictable latency
- ✗ Leads to dropouts under load
- ✗ Makes timing analysis impossible
- ✗ Introduces subtle bugs

---

## References

See `/algorithms/` for specific algorithm choices (fixed-point, WDFs, SVFs) that work within this architecture.

See `/sources/` for reference implementations (DaisySP, CMSIS-DSP) that follow these patterns.

See `sp1-dsp-engineering/references/realtime.md` for SP-1-specific constraints.
