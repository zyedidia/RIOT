/*
 * Copyright (C) 2017, 2019 Ken Rabold, JP Bonn
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     cpu_riscv_common
 * @{
 *
 * @file        cpu.c
 * @brief       Implementation of the CPU IRQ management for RISC-V clint/plic
 *              peripheral
 *
 * @author      Ken Rabold
 * @}
 */

#include <stdio.h>
#include <inttypes.h>
#include <string.h>

#include "macros/xtstr.h"
#include "cpu.h"
#include "context_frame.h"
#include "irq.h"
#include "irq_arch.h"
#include "panic.h"
#include "sched.h"
#include "plic.h"
#include "clic.h"
#include "thread.h"

#include "vendor/riscv_csr.h"

/* Default state of mstatus register */
#define MSTATUS_DEFAULT     (MSTATUS_MPP | MSTATUS_MPIE)

volatile int riscv_in_isr = 0;

/**
 * @brief   ISR trap vector
 */
static void trap_entry(void);

/**
 * @brief   Timer ISR
 */
void timer_isr(void);

extern char _sp;

regs_t trap_regs;

void riscv_irq_init(void)
{
    write_csr(0x346, &trap_regs); // mtrf
    memset(&trap_regs, 0, sizeof(regs_t));

    uint32_t gp;

    __asm__ volatile ("\t mv %0, gp" : "=r"(gp));

    trap_regs.gp = gp;
    trap_regs.sp = (uint32_t) &_sp;

    /* Setup trap handler function */
    if (IS_ACTIVE(MODULE_PERIPH_CLIC)) {
        /* Signal CLIC usage to the core */
        write_csr(mtvec, (uintptr_t)&trap_entry | 0x03);
    }
    else {
        write_csr(mtvec, (uintptr_t)&trap_entry);
    }

    /* Clear all interrupt enables */
    write_csr(mie, 0);

    /* Initial PLIC external interrupt controller */
    if (IS_ACTIVE(MODULE_PERIPH_PLIC)) {
        plic_init();
    }
    if (IS_ACTIVE(MODULE_PERIPH_CLIC)) {
        clic_init();
    }

    /* Enable external interrupts */
    set_csr(mie, MIP_MEIP);

    /*  Set default state of mstatus */
    set_csr(mstatus, MSTATUS_DEFAULT);

    irq_enable();
}

/**
 * @brief Global trap and interrupt handler
 */
__attribute((used))
static void handle_trap(uint32_t mcause)
{
    /*  Tell RIOT to set sched_context_switch_request instead of
     *  calling thread_yield(). */
    riscv_in_isr = 1;

    uint32_t trap = mcause & CPU_CSR_MCAUSE_CAUSE_MSK;

    /* Check for INT or TRAP */
    if ((mcause & MCAUSE_INT) == MCAUSE_INT) {
        /* Cause is an interrupt - determine type */
        switch (mcause & MCAUSE_CAUSE) {

        case IRQ_M_TIMER:
            /* Handle timer interrupt */
            timer_isr();
            break;
        case IRQ_M_EXT:
            /* Handle external interrupt */
            if (IS_ACTIVE(MODULE_PERIPH_PLIC)) {
                plic_isr_handler();
            }
            break;

        default:
            if (IS_ACTIVE(MODULE_PERIPH_CLIC)) {
                clic_isr_handler(trap);
            }
            else {
                /* Unknown interrupt */
                core_panic(PANIC_GENERAL_ERROR, "Unhandled interrupt");
            }
            break;
        }
    }
    else {
        switch (trap) {
        case CAUSE_USER_ECALL:      /* ECALL from user mode */
        case CAUSE_MACHINE_ECALL:   /* ECALL from machine mode */
        {
            /* TODO: get the ecall arguments */
            sched_context_switch_request = 1;
            /* Increment the return program counter past the ecall
             * instruction */
            uint32_t return_pc = read_csr(mepc);
            write_csr(mepc, return_pc + 4);
            break;
        }
        default:
#ifdef DEVELHELP
            printf("Unhandled trap:\n");
            printf("  mcause: 0x%" PRIx32 "\n", trap);
            printf("  mepc:   0x%lx\n", read_csr(mepc));
            printf("  mtval:  0x%lx\n", read_csr(mtval));
#endif
            /* Unknown trap */
            core_panic(PANIC_GENERAL_ERROR, "Unhandled trap");
        }
    }
    /* ISR done - no more changes to thread states */
    riscv_in_isr = 0;
}

static void __attribute__((used)) ctrap_entry(void)
{
    extern volatile thread_t* sched_active_thread;

    handle_trap(read_csr(mcause));

    volatile thread_t* prev_active_thread = sched_active_thread;

    // restore to caller if there is no context switch requested,
    // or a context switch is not required by the scheduler
    if (!sched_context_switch_request || !sched_run()) {
        return;
    }

    // context switch

    if (prev_active_thread) {
        prev_active_thread->pc = read_csr(mepc);
        prev_active_thread->sp = (char*) prev_active_thread->regs.sp;
    }

    // write to mprf
    write_csr(0x347, &sched_active_thread->regs);
    write_csr(mepc, sched_active_thread->pc);
}

/* Marking this as interrupt to ensure an mret at the end, provided by the
 * compiler. Aligned to 64-byte boundary as per RISC-V spec and required by some
 * of the supported platforms (gd32)*/
__attribute__((aligned(64)))
static void __attribute__((interrupt)) trap_entry(void)
{
    __asm__ volatile ("call ctrap_entry\n");
}
