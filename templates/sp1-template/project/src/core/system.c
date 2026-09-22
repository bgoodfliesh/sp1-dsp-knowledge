#include "sp1_api.h"
#include <zephyr/kernel.h>
#include <zephyr/drivers/watchdog.h>
#include <zephyr/sys/reboot.h>

static const struct device *wdt = DEVICE_DT_GET_OR_NULL(DT_ALIAS(watchdog0));
static int wdt_channel = -1;

void sp1_request_reboot_to_bootloader(void)
{
    sys_reboot(SYS_REBOOT_COLD);
}

void system_feed_watchdog(void)
{
    if (wdt && wdt_channel >= 0)
        wdt_feed(wdt, wdt_channel);
}

uint32_t sp1_millis(void)
{
    return k_uptime_get_32();
}

static int system_init(void)
{
    if (!wdt || !device_is_ready(wdt))
        return 0;

    struct wdt_timeout_cfg cfg = {
        .window = { .min = 0, .max = 5000 },
        .flags  = WDT_FLAG_RESET_SOC,
    };
    wdt_channel = wdt_install_timeout(wdt, &cfg);
    if (wdt_channel >= 0)
        wdt_setup(wdt, 0);
    return 0;
}
SYS_INIT(system_init, PRE_KERNEL_1, 0);
