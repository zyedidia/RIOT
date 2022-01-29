#ifndef PERIPH_CONF_H
#define PERIPH_CONF_H

#include "kernel_defines.h"
#include "macros/units.h"
#include "periph_cpu.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name    Timer configuration
 *
 * @{
 */
// #define TIMER_NUMOF                 (1)
/** @} */

/**
 * @name   UART configuration
 * @{
 */
// static const uart_conf_t uart_config[] = {
//     {
//         .addr       = UART0_CTRL_ADDR,
//         .rx         = GPIO_PIN(0, 16),
//         .tx         = GPIO_PIN(0, 17),
//         .isr_num    = INT_UART0_BASE,
//     },
//     {
//         .addr       = UART1_CTRL_ADDR,
//         .rx         = GPIO_PIN(0, 23),
//         .tx         = GPIO_PIN(0, 18),
//         .isr_num    = INT_UART1_BASE,
//     },
// };
//
// #define UART_NUMOF                  ARRAY_SIZE(uart_config)
/** @} */


#ifdef __cplusplus
}
#endif

#endif /* PERIPH_CONF_H */
/** @} */

