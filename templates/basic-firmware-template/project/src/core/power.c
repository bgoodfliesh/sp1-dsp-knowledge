/*
 * Power-management seam. The BQ24232 adapter belongs behind these functions;
 * keep charging policy out of plugins until board measurements are available.
 */
#include "sp1_api.h"

__attribute__((weak)) bool power_hw_is_charging(void)
{
    return false;
}

__attribute__((weak)) uint8_t power_hw_battery_percent(void)
{
    return 0u;
}
