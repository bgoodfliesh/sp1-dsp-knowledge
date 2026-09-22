#include "sp1_api.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TEST_ASSERT(condition) do { \
    checks++; \
    if (!(condition)) { \
        fprintf(stderr, "  FAIL: %s:%d: %s\n", __FILE__, __LINE__, #condition); \
        failures++; \
    } \
} while (0)

#define TEST_ASSERT_EQ(expected, actual) do { \
    long long expected_value = (long long)(expected); \
    long long actual_value = (long long)(actual); \
    checks++; \
    if (expected_value != actual_value) { \
        fprintf(stderr, "  FAIL: %s:%d: expected %lld, got %lld\n", \
                __FILE__, __LINE__, expected_value, actual_value); \
        failures++; \
    } \
} while (0)

#define TEST_ASSERT_NEAR(expected, actual, tolerance) do { \
    double expected_value = (double)(expected); \
    double actual_value = (double)(actual); \
    checks++; \
    if (actual_value < expected_value - (tolerance) || \
        actual_value > expected_value + (tolerance)) { \
        fprintf(stderr, "  FAIL: %s:%d: expected %.6f, got %.6f\n", \
                __FILE__, __LINE__, expected_value, actual_value); \
        failures++; \
    } \
} while (0)

struct sp1_module *sp1_test_modules[32];
size_t sp1_test_module_count;
unsigned checks;
unsigned failures;

void sp1_test_register_module(struct sp1_module *module)
{
    if (sp1_test_module_count < 32)
        sp1_test_modules[sp1_test_module_count++] = module;
}

static float test_faders[SP1_FADER_COUNT];
static bool test_buttons[SP1_BTN_COUNT];

float controls_hw_read_fader(sp1_fader_t f)
{
    return f < SP1_FADER_COUNT ? test_faders[f] : 0.f;
}

bool controls_hw_read_button(sp1_btn_t b)
{
    return b < SP1_BTN_COUNT && test_buttons[b];
}

static void reset_inputs(void)
{
    memset(test_faders, 0, sizeof(test_faders));
    memset(test_buttons, 0, sizeof(test_buttons));
    controls_scan();
}

static void test_controls(void)
{
    reset_inputs();
    test_faders[SP1_FADER_1] = 0.25f;
    test_faders[SP1_FADER_4] = 0.75f;
    controls_scan();
    TEST_ASSERT_NEAR(0.25f, sp1_fader(SP1_FADER_1), 0.0001f);
    TEST_ASSERT_NEAR(0.75f, sp1_fader(SP1_FADER_4), 0.0001f);
    TEST_ASSERT_NEAR(0.f, sp1_fader((sp1_fader_t)-1), 0.0001f);
    TEST_ASSERT_NEAR(0.f, sp1_fader(SP1_FADER_COUNT), 0.0001f);

    test_buttons[SP1_BTN_PLAY] = true;
    controls_scan();
    TEST_ASSERT(sp1_btn_down(SP1_BTN_PLAY));
    TEST_ASSERT(sp1_btn_pressed(SP1_BTN_PLAY));
    controls_scan();
    TEST_ASSERT(!sp1_btn_pressed(SP1_BTN_PLAY));
    test_buttons[SP1_BTN_PLAY] = false;
    controls_scan();
    TEST_ASSERT(sp1_btn_released(SP1_BTN_PLAY));
    controls_scan();
    TEST_ASSERT(!sp1_btn_released(SP1_BTN_PLAY));
}

static void test_audio(void)
{
    sp1_sample_t input[2] = {
        {.l = 1000, .r = -1000},
        {.l = 2000, .r = -2000},
    };
    sp1_sample_t output[3] = {
        {0}, {0}, {.l = 31415, .r = -27182}
    };

    reset_inputs();
    test_faders[SP1_FADER_1] = 0.5f;
    controls_scan();
    audio_process_block(input, output, 2);
    TEST_ASSERT_EQ(500, output[0].l);
    TEST_ASSERT_EQ(-500, output[0].r);
    TEST_ASSERT_EQ(1000, output[1].l);
    TEST_ASSERT_EQ(-1000, output[1].r);
    TEST_ASSERT_EQ(31415, output[2].l);
    TEST_ASSERT_EQ(-27182, output[2].r);

    output[0].l = 1234;
    output[0].r = -1234;
    audio_process_block(NULL, output, 0);
    TEST_ASSERT_EQ(1234, output[0].l);
    TEST_ASSERT_EQ(-1234, output[0].r);
}

static void test_module_registry(void)
{
    bool found = false;
    for (size_t i = 0; i < sp1_test_module_count; i++) {
        TEST_ASSERT(sp1_test_modules[i]->process != NULL);
        TEST_ASSERT(sp1_test_modules[i]->name != NULL);
        if (strcmp(sp1_test_modules[i]->name, "test_gain") == 0)
            found = true;
    }
    TEST_ASSERT(found);
    TEST_ASSERT_EQ(128, sp1_audio_block_size());
    TEST_ASSERT_EQ(48000, sp1_sample_rate());
}

int main(void)
{
    test_controls();
    test_audio();
    test_module_registry();
    printf("SP-1 unit tests: %u checks, %u failures\n", checks, failures);
    return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
