/*
 * Copyright (C) 2017, 2019 JP Bonn, Ken Rabold
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     cpu_riscv_common
 * @{
 *
 * @file
 * @brief       Thread context frame stored on stack.
 *
 * @author      JP Bonn
 */

#ifndef CONTEXT_FRAME_H
#define CONTEXT_FRAME_H

#if !defined(__ASSEMBLER__)
#include <stdint.h>
#endif /* __ASSEMBLER__ */

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(__ASSEMBLER__)

/**
 * @brief   Stores the registers and PC for a context switch.
 *
 * This also defines context_switch_frame offsets for assembly language.  The
 * structure is sized to maintain 16 byte stack alignment per the ABI.
 * https://github.com/riscv/riscv-elf-psabi-doc
 *
 */
struct context_switch_frame {
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
};

#endif /* __ASSEMBLER__ */

/**
 * @name Register offsets
 * @{
 */
/* These values are checked for correctness in context_frame.c */
#define s0_OFFSET     0
#define s1_OFFSET     4
#define s2_OFFSET     8
#define s3_OFFSET     12
#define s4_OFFSET     16
#define s5_OFFSET     20
#define s6_OFFSET     24
#define s7_OFFSET     28
#define s8_OFFSET     32
#define s9_OFFSET     36
#define s10_OFFSET    40
#define s11_OFFSET    44
#define ra_OFFSET     48
#define t0_OFFSET     52
#define t1_OFFSET     56
#define t2_OFFSET     60
#define t3_OFFSET     64
#define t4_OFFSET     68
#define t5_OFFSET     72
#define t6_OFFSET     76
#define a0_OFFSET     80
#define a1_OFFSET     84
#define a2_OFFSET     88
#define a3_OFFSET     92
#define a4_OFFSET     96
#define a5_OFFSET     100
#define a6_OFFSET     104
#define a7_OFFSET     108
#define pc_OFFSET     112
#define pad_OFFSET    116
/** @} */

/**
 * @brief Size of context switch frame
 */
#define CONTEXT_FRAME_SIZE (pad_OFFSET + 12)

/**
 * @brief Offset of stack pointer in struct _thread
 */
#define SP_OFFSET_IN_THREAD 0

#ifdef __cplusplus
}
#endif

#endif /* CONTEXT_FRAME_H */
/** @} */
