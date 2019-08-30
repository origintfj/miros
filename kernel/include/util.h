/*
 * Exception cause codes
 */

.equ EXCP_IAM,  0
.equ EXCP_IAF,  1
.equ EXCP_II,   2
.equ EXCP_BP,   3
.equ EXCP_LAM,  4
.equ EXCP_LAF,  5
.equ EXCP_SAM,  6
.equ EXCP_SAF,  7
.equ EXCP_ECU,  8
.equ EXCP_ECS,  9
.equ EXCP_ECM,  11
.equ EXCP_IPF,  12
.equ EXCP_LPF,  13
.equ EXCP_SPF,  15

/*
 * Stack save/restore macros
 */

.macro SAVE_CALLER offset
    sw      ra,         (\offset + 0)(sp)
    sw      t0,         (\offset + 4)(sp)
    sw      t1,         (\offset + 8)(sp)
    sw      t2,         (\offset + 12)(sp)
    sw      a0,         (\offset + 16)(sp)
    sw      a1,         (\offset + 20)(sp)
    sw      a2,         (\offset + 24)(sp)
    sw      a3,         (\offset + 28)(sp)
    sw      a4,         (\offset + 32)(sp)
    sw      a5,         (\offset + 36)(sp)
    sw      a6,         (\offset + 40)(sp)
    sw      a7,         (\offset + 44)(sp)
    sw      t3,         (\offset + 48)(sp)
    sw      t4,         (\offset + 52)(sp)
    sw      t5,         (\offset + 56)(sp)
    sw      t6,         (\offset + 60)(sp)
.endm
.macro RESTORE_CALLER offset
    lw      ra,         (\offset + 0)(sp)
    lw      t0,         (\offset + 4)(sp)
    lw      t1,         (\offset + 8)(sp)
    lw      t2,         (\offset + 12)(sp)
    lw      a0,         (\offset + 16)(sp)
    lw      a1,         (\offset + 20)(sp)
    lw      a2,         (\offset + 24)(sp)
    lw      a3,         (\offset + 28)(sp)
    lw      a4,         (\offset + 32)(sp)
    lw      a5,         (\offset + 36)(sp)
    lw      a6,         (\offset + 40)(sp)
    lw      a7,         (\offset + 44)(sp)
    lw      t3,         (\offset + 48)(sp)
    lw      t4,         (\offset + 52)(sp)
    lw      t5,         (\offset + 56)(sp)
    lw      t6,         (\offset + 60)(sp)
.endm


.macro SAVE_CALLEE offset
    sw      sp,         (\offset + 0)(sp)
    sw      s0,         (\offset + 4)(sp)
    sw      s1,         (\offset + 8)(sp)
    sw      s2,         (\offset + 12)(sp)
    sw      s3,         (\offset + 16)(sp)
    sw      s4,         (\offset + 20)(sp)
    sw      s5,         (\offset + 24)(sp)
    sw      s6,         (\offset + 28)(sp)
    sw      s7,         (\offset + 32)(sp)
    sw      s8,         (\offset + 36)(sp)
    sw      s9,         (\offset + 40)(sp)
    sw      s10,        (\offset + 44)(sp)
    sw      s11,        (\offset + 48)(sp)
.endm
.macro RESTORE_CALLEE offset
    lw      sp,         (\offset + 0)(sp)
    lw      s0,         (\offset + 4)(sp)
    lw      s1,         (\offset + 8)(sp)
    lw      s2,         (\offset + 12)(sp)
    lw      s3,         (\offset + 16)(sp)
    lw      s4,         (\offset + 20)(sp)
    lw      s5,         (\offset + 24)(sp)
    lw      s6,         (\offset + 28)(sp)
    lw      s7,         (\offset + 32)(sp)
    lw      s8,         (\offset + 36)(sp)
    lw      s9,         (\offset + 40)(sp)
    lw      s10,        (\offset + 44)(sp)
    lw      s11,        (\offset + 48)(sp)
.endm


.macro SAVE_UNSAVED offset
    sw      gp,         (\offset + 0)(sp)
    sw      tp,         (\offset + 4)(sp)
.endm
.macro RESTORE_UNSAVED offset
    lw      gp,         (\offset + 0)(sp)
    lw      tp,         (\offset + 4)(sp)
.endm
