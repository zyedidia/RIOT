#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <inttypes.h>

#include "clk.h"
#include "irq.h"
#include "cpu.h"
#include "periph/uart.h"
#include "plic.h"
#include "vendor/riscv_csr.h"
#include "vendor/platform.h"

/* static void _drain(uart_t dev) */
/* { */
/*     (void) dev; */
/* } */

int uart_init(uart_t dev, uint32_t baudrate, uart_rx_cb_t rx_cb, void *arg)
{
    (void) dev;
    (void) baudrate;
    (void) rx_cb;
    (void) arg;
    return UART_OK;
}

void uart_write(uart_t dev, const uint8_t *data, size_t len)
{
    (void) dev;
    (void) data;
    (void) len;
}

void uart_poweron(uart_t dev)
{
    (void)dev;
}

void uart_poweroff(uart_t dev)
{
    (void)dev;
}

