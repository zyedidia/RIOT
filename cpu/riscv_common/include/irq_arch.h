/*
 * Copyright (C) 2017 Ken Rabold
 *               2020 Inria
 *               2020 Otto-von-Guericke-Universität Magdeburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for more
 * details.
 */

/**
 * @ingroup         cpu_riscv_common
 * @{
 *
 * @file
 * @brief           Implementation of the kernels irq interface
 *
 * @author          Ken Rabold
 * @author          Alexandre Abadie <alexandre.abadie@inria.fr>
 * @author          Marian Buschsieweke <marian.buschsieweke@ovgu.de>
 */

#ifndef IRQ_ARCH_H
#define IRQ_ARCH_H

#include <stdint.h>

#include "irq.h"
#include "vendor/riscv_csr.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    RF_CTX_NORMAL = 0,
    RF_CTX_EXC = 1,
    RF_CTX_IRQ = 2,
    RF_CTX_ECALL = 3,
} rf_ctx_t;

typedef struct {
    uint32_t pc;
    uint32_t ra;
    uint32_t sp;
    uint32_t gp;
    uint32_t tp;
    uint32_t t0;
    uint32_t t1;
    uint32_t t2;
    uint32_t s0;
    uint32_t s1;
    uint32_t a0;
    uint32_t a1;
    uint32_t a2;
    uint32_t a3;
    uint32_t a4;
    uint32_t a5;
    uint32_t a6;
    uint32_t a7;
    uint32_t s2;
    uint32_t s3;
    uint32_t s4;
    uint32_t s5;
    uint32_t s6;
    uint32_t s7;
    uint32_t s8;
    uint32_t s9;
    uint32_t s10;
    uint32_t s11;
    uint32_t t3;
    uint32_t t4;
    uint32_t t5;
    uint32_t t6;
} regs_t;

static volatile regs_t* const rf = (volatile regs_t*) 0x10000;

static inline volatile regs_t* rf_regs(rf_ctx_t ctx) {
    return &rf[ctx];
}

static inline __attribute__((always_inline)) void set_trap_ret(uint32_t addr) {
    write_csr(mepc, addr);
    register uint32_t ra __asm__ ("ra") = addr;
    (void) ra;
}

/**
 * @brief   Bit mask for the MCAUSE register
 */
#define CPU_CSR_MCAUSE_CAUSE_MSK        (0x0fffu)

extern volatile int riscv_in_isr;

/**
 * @brief Enable all maskable interrupts
 */
static inline __attribute__((always_inline)) unsigned int irq_enable(void)
{
    /* Enable all interrupts */
    unsigned state;

    __asm__ volatile (
        "csrrs %[dest], mstatus, %[mask]"
        :[dest]    "=r" (state)
        :[mask]    "i" (MSTATUS_MIE)
        : "memory"
        );
    return state;
}

/**
 * @brief Disable all maskable interrupts
 */
static inline __attribute__((always_inline)) unsigned int irq_disable(void)
{

    unsigned int state;

    __asm__ volatile (
        "csrrc %[dest], mstatus, %[mask]"
        :[dest]    "=r" (state)
        :[mask]    "i" (MSTATUS_MIE)
        : "memory"
        );

    return state;
}

/**
 * @brief Restore the state of the IRQ flags
 */
static inline __attribute__((always_inline)) void irq_restore(
    unsigned int state)
{
    /* Restore all interrupts to given state */
    __asm__ volatile (
        "csrw mstatus, %[state]"
        : /* no outputs */
        :[state]   "r" (state)
        : "memory"
        );
}

/**
 * @brief See if the current context is inside an ISR
 */
static inline __attribute__((always_inline)) bool irq_is_in(void)
{
    return riscv_in_isr;
}

static inline __attribute__((always_inline)) bool irq_is_enabled(void)
{
    unsigned state;

    __asm__ volatile (
        "csrr %[dest], mstatus"
        :[dest]    "=r" (state)
        : /* no inputs */
        : "memory"
        );
    return (state & MSTATUS_MIE);
}

#ifdef __cplusplus
}
#endif

#endif /* IRQ_ARCH_H */
/** @} */
