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

void riscv_irq_init(void)
{
    rf_regs(RF_CTX_EXC)->sp = (uint32_t) &_sp;
    rf_regs(RF_CTX_IRQ)->sp = (uint32_t) &_sp;
    rf_regs(RF_CTX_ECALL)->sp = (uint32_t) &_sp;

    uint32_t gp;

    __asm__ volatile ("\t mv %0, gp" : "=r"(gp));

    rf_regs(RF_CTX_EXC)->gp = gp;
    rf_regs(RF_CTX_IRQ)->gp = gp;
    rf_regs(RF_CTX_ECALL)->gp = gp;

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
    handle_trap(read_csr(mcause));

    extern volatile thread_t* sched_active_thread;

    // restore to caller if there is no context switch requested,
    // or a context switch is not required by the scheduler
    if (!sched_context_switch_request) {
        return;
    }

    volatile thread_t* prev_active_thread = sched_active_thread;

    if (!sched_run()) {
        return;
    }

    volatile regs_t* regs = rf_regs(RF_CTX_NORMAL);

    if (prev_active_thread) {
        // save all registers
        regs->sp -= sizeof(struct context_switch_frame);
        struct context_switch_frame* prev_thread_sf = (struct context_switch_frame*) regs->sp;

        prev_thread_sf->s0 = regs->s0;
        prev_thread_sf->s1 = regs->s1;
        prev_thread_sf->s2 = regs->s2;
        prev_thread_sf->s3 = regs->s3;
        prev_thread_sf->s4 = regs->s4;
        prev_thread_sf->s5 = regs->s5;
        prev_thread_sf->s6 = regs->s6;
        prev_thread_sf->s7 = regs->s7;
        prev_thread_sf->s8 = regs->s8;
        prev_thread_sf->s9 = regs->s9;
        prev_thread_sf->s10 = regs->s10;
        prev_thread_sf->s11 = regs->s11;
        prev_thread_sf->ra = regs->ra;
        prev_thread_sf->t0 = regs->t0;
        prev_thread_sf->t1 = regs->t1;
        prev_thread_sf->t2 = regs->t2;
        prev_thread_sf->t3 = regs->t3;
        prev_thread_sf->t4 = regs->t4;
        prev_thread_sf->t5 = regs->t5;
        prev_thread_sf->t6 = regs->t6;
        prev_thread_sf->a0 = regs->a0;
        prev_thread_sf->a1 = regs->a1;
        prev_thread_sf->a2 = regs->a2;
        prev_thread_sf->a3 = regs->a3;
        prev_thread_sf->a4 = regs->a4;
        prev_thread_sf->a5 = regs->a5;
        prev_thread_sf->a6 = regs->a6;
        prev_thread_sf->a7 = regs->a7;

        prev_thread_sf->pc = read_csr(mepc);

        prev_active_thread->sp = (char*) regs->sp;
    }

    struct context_switch_frame* thread_sf = (struct context_switch_frame*) (void*) sched_active_thread->sp;

    regs->s0 = thread_sf->s0;
    regs->s1 = thread_sf->s1;
    regs->s2 = thread_sf->s2;
    regs->s3 = thread_sf->s3;
    regs->s4 = thread_sf->s4;
    regs->s5 = thread_sf->s5;
    regs->s6 = thread_sf->s6;
    regs->s7 = thread_sf->s7;
    regs->s8 = thread_sf->s8;
    regs->s9 = thread_sf->s9;
    regs->s10 = thread_sf->s10;
    regs->s11 = thread_sf->s11;
    regs->ra = thread_sf->ra;
    regs->t0 = thread_sf->t0;
    regs->t1 = thread_sf->t1;
    regs->t2 = thread_sf->t2;
    regs->t3 = thread_sf->t3;
    regs->t4 = thread_sf->t4;
    regs->t5 = thread_sf->t5;
    regs->t6 = thread_sf->t6;
    regs->a0 = thread_sf->a0;
    regs->a1 = thread_sf->a1;
    regs->a2 = thread_sf->a2;
    regs->a3 = thread_sf->a3;
    regs->a4 = thread_sf->a4;
    regs->a5 = thread_sf->a5;
    regs->a6 = thread_sf->a6;
    regs->a7 = thread_sf->a7;

    write_csr(mepc, thread_sf->pc);
    regs->sp = (uint32_t) thread_sf + sizeof(struct context_switch_frame);
}

/* Marking this as interrupt to ensure an mret at the end, provided by the
 * compiler. Aligned to 64-byte boundary as per RISC-V spec and required by some
 * of the supported platforms (gd32)*/
__attribute((aligned(64)))
static void __attribute__((interrupt)) trap_entry(void)
{
    __asm__ volatile (
        "call ctrap_entry\n"
        :
        :
        :
        );
}
