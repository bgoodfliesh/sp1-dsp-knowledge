/*
 * Frozen snapshot of the 3.0 CloudVerb kernel as it lived in main.c
 * (CLOUDVERB-676 / RV2-680). Used only by host tests as the bit-exact
 * reference. Do not "improve" this file — if the firmware algorithm
 * is intentionally changed, update this oracle in the same commit
 * with a recorded reason.
 */
#ifndef SP1_CLOUDVERB_ORACLE_H
#define SP1_CLOUDVERB_ORACLE_H

#include <stdint.h>
#include <string.h>

#define OR_CV_N 10u
static const uint16_t or_cv_len[OR_CV_N]  = { 28u, 41u, 60u, 100u, 414u, 510u, 855u, 480u, 418u, 1182u };
static const uint16_t or_cv_base[OR_CV_N] = { 0u, 28u, 69u, 129u, 229u, 643u, 1153u, 2008u, 2488u, 2906u };
#define OR_CV_MASK 4095u

static int16_t  or_cv_line[4608];
static uint32_t or_cv_w;
static int32_t  or_cv_lp1, or_cv_lp2;
static int32_t  or_cv_pl, or_cv_pr;
static uint32_t or_cv_clip;

static inline int32_t or_cv_rd(uint32_t k)
{
	return or_cv_line[(or_cv_w + or_cv_base[k] + or_cv_len[k] - 1u) & OR_CV_MASK];
}
static inline void or_cv_wr(uint32_t k, int32_t v)
{
	if (v > 32767 || v < -32768) or_cv_clip++;
	or_cv_line[(or_cv_w + or_cv_base[k]) & OR_CV_MASK] =
		(int16_t)(v > 32767 ? 32767 : (v < -32768 ? -32768 : v));
}
static inline void or_cv_step(int32_t in, int32_t krt, int32_t klp, int32_t *oL, int32_t *oR)
{
	const int32_t kap = 160;
	int32_t acc = in >> 2, t;
#define CV_AP(k, s) t = or_cv_rd(k); acc += ((s) * kap * t) >> 8; or_cv_wr(k, acc); acc = (((-(s)) * kap * acc) >> 8) + t;
	CV_AP(0u, 1) CV_AP(1u, 1) CV_AP(2u, 1) CV_AP(3u, 1)
	const int32_t apout = acc;
	acc = apout + ((krt * or_cv_rd(9u)) >> 8);
	or_cv_lp1 += ((acc - or_cv_lp1) * klp) >> 8; acc = or_cv_lp1;
	CV_AP(4u, -1) CV_AP(5u, 1)
	or_cv_wr(6u, acc); *oL = acc;
	acc = apout + ((krt * or_cv_rd(6u)) >> 8);
	or_cv_lp2 += ((acc - or_cv_lp2) * klp) >> 8; acc = or_cv_lp2;
	CV_AP(7u, 1) CV_AP(8u, -1)
	or_cv_wr(9u, acc); *oR = acc;
	or_cv_w = (or_cv_w - 1u) & OR_CV_MASK;
#undef CV_AP
}
static void or_cv_reset(void)
{
	memset(or_cv_line, 0, sizeof(or_cv_line));
	or_cv_w = 0u; or_cv_lp1 = 0; or_cv_lp2 = 0; or_cv_pl = 0; or_cv_pr = 0; or_cv_clip = 0;
}
static void or_cv_process_block(int32_t *mixL, int32_t *mixR, uint32_t nframes,
			     int32_t krt, int32_t klp, int32_t wet)
{
	int32_t pL = or_cv_pl, pR = or_cv_pr;
	for (uint32_t k = 0u; k < nframes / 4u; k++) {
		const uint32_t f0 = k * 4u;
		int32_t in = (mixL[f0]      + mixR[f0]
			    + mixL[f0 + 1u] + mixR[f0 + 1u]
			    + mixL[f0 + 2u] + mixR[f0 + 2u]
			    + mixL[f0 + 3u] + mixR[f0 + 3u]) >> 3;
		int32_t oL, oR;
		or_cv_step(in, krt, klp, &oL, &oR);
		oL = (oL * wet) >> 6; oR = (oR * wet) >> 6;
		const int32_t dL = oL - pL, dR = oR - pR;
		mixL[f0]      += pL;                  mixR[f0]      += pR;
		mixL[f0 + 1u] += pL + (dL >> 2);      mixR[f0 + 1u] += pR + (dR >> 2);
		mixL[f0 + 2u] += pL + (dL >> 1);      mixR[f0 + 2u] += pR + (dR >> 1);
		mixL[f0 + 3u] += pL + dL - (dL >> 2); mixR[f0 + 3u] += pR + dR - (dR >> 2);
		pL = oL; pR = oR;
	}
	or_cv_pl = pL; or_cv_pr = pR;
}

#endif
