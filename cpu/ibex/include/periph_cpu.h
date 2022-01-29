#ifndef PERIPH_CPU_H
#define PERIPH_CPU_H

#include <inttypes.h>

#include "periph_cpu_common.h"
#include "cpu.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef DOXYGEN
/**
 * @brief   Overwrite the default gpio_t type definition
 */
#define HAVE_GPIO_T
typedef uint8_t gpio_t;
#endif

/**
 * @brief   Structure for UART configuration data
 */
typedef struct {
    uint32_t addr;              /**< UART control register address */
    gpio_t rx;                  /**< RX pin */
    gpio_t tx;                  /**< TX pin */
    // irqn_t isr_num;             /**< ISR source number */
} uart_conf_t;

#ifdef __cplusplus
}
#endif

#endif /* PERIPH_CPU_H */
