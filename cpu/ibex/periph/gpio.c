#include <stdlib.h>
#include <unistd.h>

#include "bits.h"
#include "irq.h"
#include "cpu.h"
#include "periph_cpu.h"
#include "periph_conf.h"
#include "periph/gpio.h"
#include "plic.h"
#include "vendor/riscv_csr.h"
#include "vendor/platform.h"

/* Num of GPIOs supported */
#define GPIO_NUMOF (32)

typedef struct {
    unsigned input_val;
    unsigned input_en;
    unsigned output_en;
    unsigned output_val;
    unsigned iof_en;
    unsigned iof_sel;
    unsigned out_xor;
} gpio_reg_t;

static volatile gpio_reg_t* const gpio = (gpio_reg_t*) GPIO_CTRL_ADDR;

int gpio_init(gpio_t pin, gpio_mode_t mode)
{
    /* Check for valid pin */
    if (pin >= GPIO_NUMOF) {
        return -1;
    }

    /*  Configure the mode */

    switch (mode) {
    case GPIO_IN:
        gpio->output_en = bit_clr(gpio->output_en, pin);
        gpio->input_en = bit_set(gpio->input_en, pin);
        break;

    case GPIO_IN_PU:
        return -1;

    case GPIO_OUT:
        gpio->output_en = bit_set(gpio->output_en, pin);
        gpio->input_en = bit_clr(gpio->input_en, pin);
        break;

    default:
        return -1;
    }

    return 0;
}

int gpio_read(gpio_t pin)
{
    return bit_get(gpio->input_val, pin);
}

void gpio_set(gpio_t pin)
{
    gpio->output_val = bit_set(gpio->output_val, pin);
}

void gpio_clear(gpio_t pin)
{
    gpio->output_val = bit_clr(gpio->output_val, pin);
}

void gpio_toggle(gpio_t pin)
{
    gpio_write(pin, !bit_get(gpio->output_val, pin));
}

void gpio_write(gpio_t pin, int value)
{
    gpio->output_val = bit_assign(gpio->output_val, pin, value);
}
