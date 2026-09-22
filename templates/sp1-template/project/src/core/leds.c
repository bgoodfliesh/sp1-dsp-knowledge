#include "sp1_api.h"

/*
 * Logical LED seam. Replace led_hw_write() with the board's GPIO/PWM driver.
 * Keeping clamping here gives plugins consistent behavior on every target.
 */
__attribute__((weak)) void led_hw_write(uint8_t led, uint8_t brightness)
{
    (void)led;
    (void)brightness;
}

void sp1_led_set(uint8_t led, uint8_t brightness)
{
    led_hw_write(led, brightness);
}

void sp1_led_track(uint8_t track, bool on)
{
    led_hw_write(track, on ? 255u : 0u);
}

void sp1_led_playback(uint8_t idx, bool on)
{
    led_hw_write(idx, on ? 255u : 0u);
}

void sp1_led_all_off(void)
{
    for (uint8_t led = 0; led < 4u; led++)
        led_hw_write(led, 0u);
}
