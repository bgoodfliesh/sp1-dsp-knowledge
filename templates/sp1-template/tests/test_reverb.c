#include "test.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "../dsp/reverb/sp1_dsp_reverb.h"

#define LINE_LENGTH 4608u
#define BLOCK_FRAMES 256u
#define TOTAL_FRAMES 48000u

static void init_reverb(Reverb *rv, int16_t *line, volatile uint32_t *clips)
{
    memset(line, 0x5a, LINE_LENGTH * sizeof(*line));
    reverb_bind(rv, line, LINE_LENGTH, clips);
    reverb_reset(rv);
}

static void test_reset_clears_state_and_line(void)
{
    int16_t line[LINE_LENGTH];
    Reverb rv;
    volatile uint32_t clips = 0;

    init_reverb(&rv, line, &clips);
    rv.w = 123;
    rv.lp1 = 456;
    rv.lp2 = -789;
    rv.pl = 12;
    rv.pr = -34;
    reverb_reset(&rv);

    TEST_ASSERT_EQ_INT(1, rv.live);
    TEST_ASSERT_EQ_INT(0, rv.w);
    TEST_ASSERT_EQ_INT(0, rv.lp1);
    TEST_ASSERT_EQ_INT(0, rv.lp2);
    TEST_ASSERT_EQ_INT(0, rv.pl);
    TEST_ASSERT_EQ_INT(0, rv.pr);
    for (size_t i = 0; i < LINE_LENGTH; i++)
        TEST_ASSERT_EQ_INT(0, line[i]);
}

static void test_silence_stays_silent(void)
{
    int16_t line[LINE_LENGTH];
    Reverb rv;
    int32_t left[BLOCK_FRAMES] = {0};
    int32_t right[BLOCK_FRAMES] = {0};

    init_reverb(&rv, line, NULL);
    reverb_process_block(&rv, left, right, BLOCK_FRAMES, 180, 200, 160);

    for (size_t i = 0; i < BLOCK_FRAMES; i++) {
        TEST_ASSERT_EQ_INT(0, left[i]);
        TEST_ASSERT_EQ_INT(0, right[i]);
    }
}

static void test_impulse_produces_stereo_tail(void)
{
    int16_t line[LINE_LENGTH];
    Reverb rv;
    uint64_t energy = 0;

    init_reverb(&rv, line, NULL);
    for (size_t block = 0; block < 8; block++) {
        int32_t left[BLOCK_FRAMES] = {0};
        int32_t right[BLOCK_FRAMES] = {0};
        if (block == 0) {
            left[0] = 20000;
            right[0] = 20000;
        }
        reverb_process_block(&rv, left, right, BLOCK_FRAMES, 180, 200, 160);
        for (size_t i = 0; i < BLOCK_FRAMES; i++)
            energy += (uint64_t)llabs(left[i]) + (uint64_t)llabs(right[i]);
    }
    TEST_ASSERT(energy > 0);
    TEST_ASSERT(rv.w != 0);
    TEST_ASSERT(rv.pl != 0 || rv.pr != 0);
}

static void test_block_processing_preserves_remainder(void)
{
    int16_t line[LINE_LENGTH];
    Reverb rv;
    int32_t left[BLOCK_FRAMES + 3];
    int32_t right[BLOCK_FRAMES + 3];

    init_reverb(&rv, line, NULL);
    for (size_t i = 0; i < BLOCK_FRAMES + 3; i++) {
        left[i] = 17;
        right[i] = -23;
    }
    reverb_process_block(&rv, left, right, BLOCK_FRAMES + 3, 180, 200, 160);

    TEST_ASSERT_EQ_INT(17, left[BLOCK_FRAMES]);
    TEST_ASSERT_EQ_INT(-23, right[BLOCK_FRAMES]);
    TEST_ASSERT_EQ_INT(17, left[BLOCK_FRAMES + 1]);
    TEST_ASSERT_EQ_INT(-23, right[BLOCK_FRAMES + 1]);
    TEST_ASSERT_EQ_INT(17, left[BLOCK_FRAMES + 2]);
    TEST_ASSERT_EQ_INT(-23, right[BLOCK_FRAMES + 2]);
}

static void test_clipping_is_counted_and_clamped(void)
{
    int16_t line[LINE_LENGTH];
    Reverb rv;
    volatile uint32_t clips = 0;

    init_reverb(&rv, line, &clips);
    reverb_wr(&rv, 0, 40000);
    TEST_ASSERT_EQ_INT(1, clips);
    TEST_ASSERT_EQ_INT(32767, line[0]);

    reverb_wr(&rv, 0, -40000);
    TEST_ASSERT_EQ_INT(2, clips);
    TEST_ASSERT_EQ_INT(-32768, line[0]);
}

static void test_wet_zero_preserves_dry_signal(void)
{
    int16_t line[LINE_LENGTH];
    Reverb rv;
    int32_t left[BLOCK_FRAMES];
    int32_t right[BLOCK_FRAMES];

    init_reverb(&rv, line, NULL);
    for (size_t i = 0; i < BLOCK_FRAMES; i++) {
        left[i] = (int32_t)i * 31 - 4000;
        right[i] = 5000 - (int32_t)i * 17;
    }
    reverb_process_block(&rv, left, right, BLOCK_FRAMES, 255, 255, 0);

    for (size_t i = 0; i < BLOCK_FRAMES; i++) {
        TEST_ASSERT_EQ_INT((int32_t)i * 31 - 4000, left[i]);
        TEST_ASSERT_EQ_INT(5000 - (int32_t)i * 17, right[i]);
    }
}

static void test_reset_is_deterministic(void)
{
    int16_t line_a[LINE_LENGTH], line_b[LINE_LENGTH];
    Reverb rv_a, rv_b;
    int32_t left_a[BLOCK_FRAMES], right_a[BLOCK_FRAMES];
    int32_t left_b[BLOCK_FRAMES], right_b[BLOCK_FRAMES];

    init_reverb(&rv_a, line_a, NULL);
    init_reverb(&rv_b, line_b, NULL);
    for (size_t i = 0; i < BLOCK_FRAMES; i++) {
        left_a[i] = left_b[i] = (int32_t)i * 101 - 12000;
        right_a[i] = right_b[i] = 9000 - (int32_t)i * 43;
    }
    reverb_process_block(&rv_a, left_a, right_a, BLOCK_FRAMES, 180, 200, 160);
    reverb_process_block(&rv_b, left_b, right_b, BLOCK_FRAMES, 180, 200, 160);
    for (size_t i = 0; i < BLOCK_FRAMES; i++) {
        TEST_ASSERT_EQ_INT(left_a[i], left_b[i]);
        TEST_ASSERT_EQ_INT(right_a[i], right_b[i]);
    }
    TEST_ASSERT_EQ_INT(rv_a.w, rv_b.w);
    TEST_ASSERT_EQ_INT(rv_a.lp1, rv_b.lp1);
    TEST_ASSERT_EQ_INT(rv_a.lp2, rv_b.lp2);
    TEST_ASSERT_EQ_INT(rv_a.pl, rv_b.pl);
    TEST_ASSERT_EQ_INT(rv_a.pr, rv_b.pr);
}

static void test_parameter_extremes_remain_bounded(void)
{
    int16_t line[LINE_LENGTH];
    Reverb rv;
    int32_t left[BLOCK_FRAMES];
    int32_t right[BLOCK_FRAMES];
    volatile uint32_t clips = 0;

    init_reverb(&rv, line, &clips);
    for (size_t i = 0; i < BLOCK_FRAMES; i++) {
        left[i] = right[i] = (i == 0) ? 30000 : 0;
    }
    reverb_process_block(&rv, left, right, BLOCK_FRAMES, 0, 0, 256);
    for (size_t i = 0; i < BLOCK_FRAMES; i++) {
        TEST_ASSERT(left[i] <= INT32_MAX && left[i] >= INT32_MIN);
        TEST_ASSERT(right[i] <= INT32_MAX && right[i] >= INT32_MIN);
    }
    TEST_ASSERT(clips < 1000);
}

const struct test_case reverb_tests[] = {
    {"reset clears state and line", test_reset_clears_state_and_line},
    {"silence stays silent", test_silence_stays_silent},
    {"impulse produces stereo tail", test_impulse_produces_stereo_tail},
    {"block processing preserves remainder", test_block_processing_preserves_remainder},
    {"clipping is counted and clamped", test_clipping_is_counted_and_clamped},
    {"wet zero preserves dry signal", test_wet_zero_preserves_dry_signal},
    {"reset is deterministic", test_reset_is_deterministic},
    {"parameter extremes remain bounded", test_parameter_extremes_remain_bounded},
};

const size_t reverb_test_count = sizeof(reverb_tests) / sizeof(reverb_tests[0]);

int render_reverb_wav(const char *path)
{
    int16_t line[LINE_LENGTH];
    int16_t *out_left = calloc(TOTAL_FRAMES, sizeof(*out_left));
    int16_t *out_right = calloc(TOTAL_FRAMES, sizeof(*out_right));
    if (!out_left || !out_right) {
        free(out_left);
        free(out_right);
        return 1;
    }

    Reverb rv;
    init_reverb(&rv, line, NULL);
    for (size_t pos = 0; pos < TOTAL_FRAMES; pos += BLOCK_FRAMES) {
        int32_t left[BLOCK_FRAMES] = {0};
        int32_t right[BLOCK_FRAMES] = {0};
        if (pos == 0)
            left[0] = right[0] = 20000;
        reverb_process_block(&rv, left, right, BLOCK_FRAMES, 180, 200, 160);
        for (size_t i = 0; i < BLOCK_FRAMES; i++) {
            out_left[pos + i] = (int16_t)(left[i] > 32767 ? 32767 :
                                          left[i] < -32768 ? -32768 : left[i]);
            out_right[pos + i] = (int16_t)(right[i] > 32767 ? 32767 :
                                           right[i] < -32768 ? -32768 : right[i]);
        }
    }

    int result = write_wav_s16(path, out_left, out_right, TOTAL_FRAMES);
    free(out_left);
    free(out_right);
    return result;
}
