/*
 * Clouds-derived stereo CloudVerb — 12 kHz, shared echo/CloudVerb delay line.
 *
 * Extracted from SP-1 firmware 3.0 (CLOUDVERB-676 / RV2-680) without algorithm
 * changes. This is the firmware's implementation, not a re-import of upstream
 * Mutable Instruments Clouds.
 *
 * Topology (Clouds FxEngine, one write pointer, 4096-sample window):
 *   4 input allpasses (diffusers)
 *   loop 1: feedback + damping LP + allpass(-) + allpass(+) + delay
 *   loop 2: feedback + damping LP + allpass(+) + allpass(-) + delay
 * Lengths are Clouds' 32 kHz set scaled to 12 kHz then x0.57 to fit 4096
 * (sum 4088). kap = 160/256 = 0.625. Input is >> 2 (-12 dB); wet is << 2
 * back out in the 48 kHz interpolator (>> 6 after wet q8).
 *
 * Ownership:
 *   Echo and CloudVerb share one int16 line of 4608 samples (384 ms at 12 kHz).
 *   Echo uses the full 4608 as a circular delay. CloudVerb addresses a 4096-sample
 *   window (CLOUDVERB_MASK) inside that allocation. They never run together:
 *   engaging echo clears CloudVerb.live, and vice versa. Do not split the buffer.
 *
 * Real-time contract:
 *   cloudverb_rd / cloudverb_wr / cloudverb_step / cloudverb_process_block are static
 *   inline always_inline so they compile into the audio translation unit with
 *   the same visibility they had as file-static inlines. Do not move them
 *   into a separate .c without measuring audio-thread CPU.
 *
 * RAM: CloudVerb is ~40 bytes of state plus the shared line (already allocated).
 * CPU: one inlined 12 kHz network step per 4 output frames (64 steps / 256-frame
 * block). Function-call overhead is not introduced while these stay in the header.
 */
#ifndef SP1_DSP_CLOUDVERB_H
#define SP1_DSP_CLOUDVERB_H

#include <stdint.h>
#include <string.h>

enum {
	CLOUDVERB_SAMPLE_RATE = 12000,
	CLOUDVERB_N           = 10,
	CLOUDVERB_MASK        = 4095u,
	CLOUDVERB_KAP         = 160,     /* 0.625 allpass coefficient */
	CLOUDVERB_WINDOW      = 4096u,   /* Clouds FxEngine window; shared line is >= this */
};

/* RV2-680: sum 4088 <= 4096 */
static const uint16_t CLOUDVERB_LEN[CLOUDVERB_N]  = {
	28u, 41u, 60u, 100u, 414u, 510u, 855u, 480u, 418u, 1182u
};
static const uint16_t CLOUDVERB_BASE[CLOUDVERB_N] = {
	0u, 28u, 69u, 129u, 229u, 643u, 1153u, 2008u, 2488u, 2906u
};

typedef struct CloudVerb {
	int16_t *line;
	uint32_t line_len;            /* allocated samples (firmware: 4608) */
	uint32_t w;                   /* write pointer, decrements per step */
	int32_t  lp1, lp2;            /* the two loops' damping states */
	int32_t  pl, pr;              /* last group's wet L/R (interp start) */
	uint8_t  live;                /* line currently holds CloudVerb (cleared on engage) */
	volatile uint32_t *clip;      /* POPS-700 clamp counter; may be NULL */
} CloudVerb;

static inline void cloudverb_bind(CloudVerb *rv, int16_t *line, uint32_t line_len,
			       volatile uint32_t *clip)
{
	rv->line = line;
	rv->line_len = line_len;
	rv->clip = clip;
}

/* Engage edge: wipe the shared line and zero network state. Sets live = 1. */
static inline void cloudverb_reset(CloudVerb *rv)
{
	if (rv->line && rv->line_len)
		memset(rv->line, 0, (size_t)rv->line_len * sizeof(int16_t));
	rv->w = 0u;
	rv->lp1 = 0;
	rv->lp2 = 0;
	rv->pl = 0;
	rv->pr = 0;
	rv->live = 1u;
}

static inline __attribute__((always_inline)) int32_t cloudverb_rd(const CloudVerb *rv, uint32_t k)
{
	return rv->line[(rv->w + CLOUDVERB_BASE[k] + CLOUDVERB_LEN[k] - 1u) & CLOUDVERB_MASK];
}

static inline __attribute__((always_inline)) void cloudverb_wr(CloudVerb *rv, uint32_t k, int32_t v)
{
	if (v > 32767 || v < -32768) {          /* POPS-700: a clip inside the loop recirculates */
		if (rv->clip)
			(*rv->clip)++;
	}
	rv->line[(rv->w + CLOUDVERB_BASE[k]) & CLOUDVERB_MASK] =
		(int16_t)(v > 32767 ? 32767 : (v < -32768 ? -32768 : v));   /* RV2-680: clamp, not the knee */
}

/* One 12 kHz step of the Clouds network. in = mono sum, krt/klp are q8. */
static inline __attribute__((always_inline)) void cloudverb_step(CloudVerb *rv, int32_t in,
							      int32_t krt, int32_t klp,
							      int32_t *oL, int32_t *oR)
{
	const int32_t kap = CLOUDVERB_KAP;
	int32_t acc = in >> 2, t;               /* RV2-680: -12 dB into the network (x4 out) */
#define RV_AP(k, s) \
	t = cloudverb_rd(rv, k); \
	acc += ((s) * kap * t) >> 8; \
	cloudverb_wr(rv, k, acc); \
	acc = (((-(s)) * kap * acc) >> 8) + t
	RV_AP(0u, 1); RV_AP(1u, 1); RV_AP(2u, 1); RV_AP(3u, 1);
	const int32_t apout = acc;
	acc = apout + ((krt * cloudverb_rd(rv, 9u)) >> 8);
	rv->lp1 += ((acc - rv->lp1) * klp) >> 8; acc = rv->lp1;
	RV_AP(4u, -1); RV_AP(5u, 1);
	cloudverb_wr(rv, 6u, acc); *oL = acc;
	acc = apout + ((krt * cloudverb_rd(rv, 6u)) >> 8);
	rv->lp2 += ((acc - rv->lp2) * klp) >> 8; acc = rv->lp2;
	RV_AP(7u, 1); RV_AP(8u, -1);
	cloudverb_wr(rv, 9u, acc); *oR = acc;
	rv->w = (rv->w - 1u) & CLOUDVERB_MASK;     /* RV2-680 */
#undef RV_AP
}

/*
 * 48 kHz stereo block -> 12 kHz Clouds steps. The wet pair is interpolated
 * across 4 frames from the PREVIOUS step (one group of latency, 83 us).
 * nframes must be a multiple of 4 (firmware BLK_FRAMES = 256).
 * wet is q8 mix>>1; the >>6 restores the x4 from the -12 dB network pad.
 */
static inline void cloudverb_process_block(CloudVerb *rv, int32_t *mixL, int32_t *mixR,
					uint32_t nframes, int32_t krt, int32_t klp,
					int32_t wet)
{
	int32_t pL = rv->pl, pR = rv->pr;
	for (uint32_t k = 0u; k < nframes / 4u; k++) {
		const uint32_t f0 = k * 4u;
		int32_t in = (mixL[f0]      + mixR[f0]
			    + mixL[f0 + 1u] + mixR[f0 + 1u]
			    + mixL[f0 + 2u] + mixR[f0 + 2u]
			    + mixL[f0 + 3u] + mixR[f0 + 3u]) >> 3;
		int32_t oL, oR;
		cloudverb_step(rv, in, krt, klp, &oL, &oR);
		oL = (oL * wet) >> 6; oR = (oR * wet) >> 6;   /* RV2-680: x4 back out */
		const int32_t dL = oL - pL, dR = oR - pR;
		mixL[f0]      += pL;                  mixR[f0]      += pR;
		mixL[f0 + 1u] += pL + (dL >> 2);      mixR[f0 + 1u] += pR + (dR >> 2);
		mixL[f0 + 2u] += pL + (dL >> 1);      mixR[f0 + 2u] += pR + (dR >> 1);
		mixL[f0 + 3u] += pL + dL - (dL >> 2); mixR[f0 + 3u] += pR + dR - (dR >> 2);
		pL = oL; pR = oR;
	}
	rv->pl = pL; rv->pr = pR;
}

#endif /* SP1_DSP_CLOUDVERB_H */
