#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <inttypes.h>

#include "bits.h"
#include "clk.h"
#include "irq.h"
#include "cpu.h"
#include "periph/uart.h"
#include "plic.h"
#include "vendor/riscv_csr.h"
#include "vendor/platform.h"

#define UART_NUMOF 1

typedef struct {
    unsigned tx_data;
    unsigned rx_data;
    unsigned dvsr;
    unsigned clear;
} uart_reg_t;

#define BIT_EMPTY 8
#define BIT_FULL 9
#define BIT_DATA_LOW 0
#define BIT_DATA_HIGH 7

static volatile uart_reg_t* const uart = (uart_reg_t*) UART0_CTRL_ADDR;

static void uart_set_baud(int baud) {
    uint32_t dvsr = coreclk() / 16 / baud - 1;
    uart->dvsr = dvsr;
}

static int uart_tx_full(void) {
    return bit_get(uart->rx_data, BIT_FULL);
}

static void uart_tx(uint8_t byte) {
    while (uart_tx_full()) {}
    uart->tx_data = (uint32_t) byte;
}

int uart_init(uart_t dev, uint32_t baudrate, uart_rx_cb_t rx_cb, void *arg)
{
    (void) rx_cb;
    (void) arg;

    assert(dev < UART_NUMOF);
    uart_poweron(dev);
    uart_set_baud(baudrate);
    return UART_OK;
}

void uart_write(uart_t dev, const uint8_t *data, size_t len)
{
    assert(dev < UART_NUMOF);

    for (size_t i = 0; i < len; i++) {
        uart_tx(data[i]);
    }
}

void uart_poweron(uart_t dev)
{
    (void) dev;
}

void uart_poweroff(uart_t dev)
{
    (void) dev;
}

