/*
 * Host regression for the extracted Clouds cloudverb.
 *
 * Compares effects/cloudverb/implementation/cloudverb.h against the frozen 3.0 oracle
 * on deterministic vectors: zero, impulse, constant, full-scale ±,
 * and a short noise-like ramp. Any algorithm drift fails the test.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "../../effects/cloudverb/implementation/cloudverb.h"
#include "cloudverb_oracle.h"

#define BLK 256u
#define NBLK 8u
#define NSAMP (BLK * NBLK)

static int g_fail;

static void fail(const char *name, uint32_t i, int32_t a, int32_t b)
{
	fprintf(stderr, "FAIL %s sample %u: extracted=%d oracle=%d\n", name, i, a, b);
	g_fail = 1;
}

static void run_case(const char *name, void (*fill)(int32_t *, int32_t *, uint32_t),
		     int32_t krt, int32_t klp, int32_t wet)
{
	int16_t line[4608];
	volatile uint32_t clip = 0;
	CloudVerb rv;
	memset(&rv, 0, sizeof(rv));
	memset(line, 0, sizeof(line));
	cloudverb_bind(&rv, line, 4608u, &clip);
	cloudverb_reset(&rv);
	or_cv_reset();

	int32_t eL[NSAMP], eR[NSAMP], oL[NSAMP], oR[NSAMP];
	fill(eL, eR, NSAMP);
	memcpy(oL, eL, sizeof(eL));
	memcpy(oR, eR, sizeof(eR));

	for (uint32_t b = 0; b < NBLK; b++) {
		cloudverb_process_block(&rv, eL + b * BLK, eR + b * BLK, BLK, krt, klp, wet);
		or_cv_process_block(oL + b * BLK, oR + b * BLK, BLK, krt, klp, wet);
	}

	for (uint32_t i = 0; i < NSAMP; i++) {
		if (eL[i] != oL[i]) { fail(name, i, eL[i], oL[i]); return; }
		if (eR[i] != oR[i]) { fail(name, i, eR[i], oR[i]); return; }
	}
	if ((uint32_t)clip != or_cv_clip) {
		fprintf(stderr, "FAIL %s clip: extracted=%u oracle=%u\n",
			name, (unsigned)clip, (unsigned)or_cv_clip);
		g_fail = 1;
		return;
	}
	if (rv.w != or_cv_w || rv.lp1 != or_cv_lp1 || rv.lp2 != or_cv_lp2 ||
	    rv.pl != or_cv_pl || rv.pr != or_cv_pr) {
		fprintf(stderr, "FAIL %s state w/lp/p mismatch\n", name);
		g_fail = 1;
		return;
	}
	if (memcmp(line, or_cv_line, sizeof(line)) != 0) {
		fprintf(stderr, "FAIL %s delay-line contents differ\n", name);
		g_fail = 1;
		return;
	}
	printf("  PASS  %s  (krt=%d klp=%d wet=%d clip=%u)\n",
	       name, krt, klp, wet, (unsigned)clip);
}

static void fill_zero(int32_t *L, int32_t *R, uint32_t n)
{
	memset(L, 0, n * sizeof(int32_t));
	memset(R, 0, n * sizeof(int32_t));
}
static void fill_impulse(int32_t *L, int32_t *R, uint32_t n)
{
	fill_zero(L, R, n);
	L[0] = 32767; R[0] = 32767;
}
static void fill_const(int32_t *L, int32_t *R, uint32_t n)
{
	for (uint32_t i = 0; i < n; i++) { L[i] = 1000; R[i] = -700; }
}
static void fill_maxpos(int32_t *L, int32_t *R, uint32_t n)
{
	for (uint32_t i = 0; i < n; i++) { L[i] = 32767; R[i] = 32767; }
}
static void fill_maxneg(int32_t *L, int32_t *R, uint32_t n)
{
	for (uint32_t i = 0; i < n; i++) { L[i] = -32768; R[i] = -32768; }
}
static void fill_ramp(int32_t *L, int32_t *R, uint32_t n)
{
	for (uint32_t i = 0; i < n; i++) {
		L[i] = (int32_t)((i * 37u) & 0x7fffu) - 16000;
		R[i] = (int32_t)((i * 91u) & 0x7fffu) - 8000;
	}
}

static void test_coefficients(void)
{
	static const uint16_t want_len[10]  = { 28u, 41u, 60u, 100u, 414u, 510u, 855u, 480u, 418u, 1182u };
	static const uint16_t want_base[10] = { 0u, 28u, 69u, 129u, 229u, 643u, 1153u, 2008u, 2488u, 2906u };
	uint32_t sum = 0;
	for (int i = 0; i < 10; i++) {
		if (CLOUDVERB_LEN[i] != want_len[i] || CLOUDVERB_BASE[i] != want_base[i]) {
			fprintf(stderr, "FAIL coefficient table at %d\n", i);
			g_fail = 1;
			return;
		}
		sum += CLOUDVERB_LEN[i];
		if (CLOUDVERB_BASE[i] + CLOUDVERB_LEN[i] > 4096u) {
			fprintf(stderr, "FAIL sub-line %d overflows window\n", i);
			g_fail = 1;
			return;
		}
	}
	if (sum != 4088u) {
		fprintf(stderr, "FAIL length sum %u != 4088\n", sum);
		g_fail = 1;
		return;
	}
	if (CLOUDVERB_MASK != 4095u || CLOUDVERB_KAP != 160) {
		fprintf(stderr, "FAIL mask/kap\n");
		g_fail = 1;
		return;
	}
	printf("  PASS  coefficients (sum=4088, mask=4095, kap=160)\n");
}

int main(void)
{
	printf("test_cloudverb — extracted vs 3.0 oracle\n");
	test_coefficients();
	/* default firmware room: mix=128 -> krt=128+61=189, klp=179, wet=64 */
	run_case("zero",        fill_zero,     189, 179, 64);
	run_case("impulse",     fill_impulse,  189, 179, 64);
	run_case("constant",    fill_const,    189, 179, 64);
	run_case("max+",        fill_maxpos,   189, 179, 64);
	run_case("max-",        fill_maxneg,   189, 179, 64);
	run_case("ramp",        fill_ramp,     189, 179, 64);
	run_case("hall krt=250", fill_impulse, 250, 250, 127);
	run_case("room krt=128", fill_impulse, 128,  32,  1);
	if (g_fail) {
		fprintf(stderr, "test_cloudverb FAILED\n");
		return 1;
	}
	printf("test_cloudverb: all passed\n");
	return 0;
}
