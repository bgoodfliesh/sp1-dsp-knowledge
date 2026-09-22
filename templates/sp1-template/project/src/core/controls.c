#include "sp1_api.h"
#ifndef SP1_HOST_TEST
#include <zephyr/kernel.h>
#endif
#include <string.h>

static float fader_val[SP1_FADER_COUNT];
static bool  btn_state[SP1_BTN_COUNT];
static bool  btn_prev[SP1_BTN_COUNT];

__attribute__((weak)) float controls_hw_read_fader(sp1_fader_t f)
{
    (void)f;
    return 0.f;
}

__attribute__((weak)) bool controls_hw_read_button(sp1_btn_t b)
{
    (void)b;
    return false;
}

float sp1_fader(sp1_fader_t f)
{
    if (f >= SP1_FADER_COUNT) return 0.f;
    return fader_val[f];
}

bool sp1_btn_down(sp1_btn_t b)     { return b < SP1_BTN_COUNT && btn_state[b]; }
bool sp1_btn_pressed(sp1_btn_t b)  { return b < SP1_BTN_COUNT && btn_state[b] && !btn_prev[b]; }
bool sp1_btn_released(sp1_btn_t b) { return b < SP1_BTN_COUNT && !btn_state[b] && btn_prev[b]; }

void controls_scan(void)
{
    memcpy(btn_prev, btn_state, sizeof(btn_state));
    for (int i = 0; i < SP1_FADER_COUNT; i++)
        fader_val[i] = controls_hw_read_fader((sp1_fader_t)i);
    for (int i = 0; i < SP1_BTN_COUNT; i++)
        btn_state[i] = controls_hw_read_button((sp1_btn_t)i);
}

#ifndef SP1_HOST_TEST
static int controls_init(void)
{
    memset(fader_val, 0, sizeof(fader_val));
    memset(btn_state, 0, sizeof(btn_state));
    memset(btn_prev, 0, sizeof(btn_prev));
    return 0;
}

SYS_INIT(controls_init, APPLICATION, 40);
#endif
