#include "clk.h"
#include "cpu.h"
#include "periph/init.h"
#include "periph_conf.h"

#include "vendor/riscv_csr.h"

#include "stdio_uart.h"

/**
 * @brief Initialize the CPU, set IRQ priorities, clocks, peripheral
 */
void cpu_init(void)
{
    /* Common RISC-V initialization */
    riscv_init();

    /* Initialize stdio */
    /* stdio_init(); */

    /* Initialize static peripheral */
    periph_init();
}

