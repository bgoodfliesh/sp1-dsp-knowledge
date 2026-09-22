#include "sp1_api.h"
#include <zephyr/kernel.h>

int main(void)
{
    printk("{{ project_name }} starting\n");

    while (1) {
        controls_scan();
        system_feed_watchdog();
        k_msleep(8);
    }
    return 0;
}
