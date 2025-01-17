#include <stdlib.h>
#include <unistd.h>

#include "cpu.h"
#include "cpu_conf.h"
#include "periph_cpu.h"
#include "periph_conf.h"
#include "periph/timer.h"
#include "vendor/riscv_csr.h"

/**
 * @brief   Save reference to the timer callback
 */
static timer_cb_t isr_cb;

/**
 * @brief   Save argument for the callback
 */
static void *isr_arg;

int timer_init(tim_t dev, uint32_t freq, timer_cb_t cb, void *arg)
{
    /* Using RISC-V built in timer (64bit value) */
    if (dev != 0) {
        return -1;
    }

    (void) freq;
    /* if (freq != RTC_FREQ) { */
    /*     return -1; */
    /* } */

    /* Save timer callback and arg */
    isr_cb = cb;
    isr_arg = arg;

    /* reset timer counter */
    volatile uint64_t *mtime = (uint64_t *)(TIMER_BASE_ADDR + TIMER_MTIME);

    *mtime = 0;

    return 0;
}

/* int timer_set(tim_t dev, int channel, unsigned int timeout) */
/* { */
/*     volatile uint64_t *mtime = (uint64_t *)(TIMER_BASE_ADDR + TIMER_MTIME); */
/*     volatile uint64_t *mtimecmp = */
/*         (uint64_t *)(TIMER_BASE_ADDR + TIMER_MTIMECMP); */
/*  */
/*     #<{(| Compute delta for timer |)}># */
/*     uint64_t now = *mtime; */
/*     uint64_t then = now + (uint64_t)timeout; */
/*  */
/*     if (dev != 0 || channel != 0) { */
/*         return -1; */
/*     } */
/*  */
/*     #<{(| Avoid spurious timer intr |)}># */
/*     clear_csr(mie, MIP_MTIP); */
/*  */
/*     #<{(| New intr time |)}># */
/*     *mtimecmp = then; */
/*  */
/*     #<{(| Re-enalble timer intr |)}># */
/*     set_csr(mie, MIP_MTIP); */
/*     return 0; */
/* } */

int timer_set_absolute(tim_t dev, int channel, unsigned int value)
{

    volatile uint64_t *mtime = (uint64_t *)(TIMER_BASE_ADDR + TIMER_MTIME);
    volatile uint64_t *mtimecmp =
        (uint64_t *)(TIMER_BASE_ADDR + TIMER_MTIMECMP);

    /* Compute absolute for timer */
    uint64_t now = *mtime;
    uint64_t then = (now & 0xFFFFFFFF00000000) + (uint64_t)value;

    if (dev != 0 || channel != 0) {
        return -1;
    }

    /* Avoid spurious timer intr */
    clear_csr(mie, MIP_MTIP);

    /* New intr time (handle 32bit rollover) */
    *mtimecmp = (then > now) ? then : then + 0x100000000;

    /* Re-enable timer intr */
    set_csr(mie, MIP_MTIP);
    return 0;
}

int timer_clear(tim_t dev, int channel)
{
    if (dev != 0 || channel != 0) {
        return -1;
    }

    return 0;
}

unsigned int timer_read(tim_t dev)
{
    uint32_t lo = *(volatile uint32_t *)(TIMER_BASE_ADDR + TIMER_MTIME);

    if (dev != 0) {
        return 0;
    }

    /* Read current timer value */
    return (unsigned int)lo;
}

void timer_start(tim_t dev)
{
    if (dev != 0) {
        return;
    }

    /* Timer is continuous running
     * Enable the timer interrupt */
    set_csr(mie, MIP_MTIP);
}

void timer_stop(tim_t dev)
{
    if (dev != 0) {
        return;
    }

    /* Disable the timer interrupt */
    clear_csr(mie, MIP_MTIP);
}

void timer_isr(void)
{
    volatile uint64_t *mtimecmp =
        (uint64_t *)(TIMER_BASE_ADDR + TIMER_MTIMECMP);

    /* Clear intr */
    clear_csr(mie, MIP_MTIP);

    /* Set mtimecmp to largest value to clear the interrupt */
    *mtimecmp = 0xFFFFFFFFFFFFFFFF;

    /* Call timer callback function */
    if (isr_cb) {
        isr_cb(isr_arg, 0);
    }

    /* Reset interrupt */
    set_csr(mie, MIP_MTIP);
}
