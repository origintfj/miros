#include "util.h"

.globl vectors

.section .vector_table
vectors:
    .align  2
    j       trap_usi_excp
    .align  2
    mret
    .align  2
    mret
    .align  2
    mret
    .align  2
    mret
    .align  2
    mret
    .align  2
    mret
    .align  2
    j       mtip_handler    // mtip
    .align  2
    mret
    .align  2
    mret
    .align  2
    mret
    .align  2
    mret                    // meip

    .align  2

trap_usi_excp:
    csrrw   sp,         mscratch,   sp
    addi    sp,         sp,         -124
    SAVE_CALLER         0
    csrr    t0,         mcause
    addi    t0,         t0,         -EXCP_ECU
    beq     t0,         zero,       trap_ecu    // ecall from u-mode
    addi    t0,         t0,         EXCP_ECU
    blt     t0,         zero,       trap_usi    // user sw interrupt
trap_excp:
    // exception code here
    RESTORE_CALLER      0
    addi    sp,         sp,         124
    csrrw   sp,         mscratch,   sp
    mret
trap_usi:
    // usi code here
    RESTORE_CALLER      0
    addi    sp,         sp,         124
    csrrw   sp,         mscratch,   sp
    mret
trap_ecu:
    // ecall from u-mode here
    csrr    t0,         mepc
    addi    t0,         t0,         4
    csrw    mepc,       t0
    jal     syscall
    RESTORE_CALLER      0
    addi    sp,         sp,         124
    csrrw   sp,         mscratch,   sp
    mret

mtip_handler:
    csrrw   sp,         mscratch,   sp // TODO
    addi    sp,         sp,         -124
    SAVE_CALLER         0
    jal     mtip
    RESTORE_CALLER      0
    addi    sp,         sp,         124
    csrrw   sp,         mscratch,   sp // TODO
    mret
