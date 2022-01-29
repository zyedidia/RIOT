#include <stdint.h>
#include "periph/pm.h"
#include "vendor/platform.h"

void pm_set_lowest(void)
{
    __asm__ volatile ("wfi");
}

void pm_reboot(void)
{
    while (1) {}
}

