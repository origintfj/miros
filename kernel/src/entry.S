#include "util.h"

.equ RAM_START_ADDR,    0x40000000
.equ RAM_END_ADDR,      0x40040000

.globl _entry
.section .init
_entry:
    # load an initial stack pointer
    li      sp,         RAM_END_ADDR
    addi    sp,         sp,     -4
    # set the m-mode trap vector
1:	auipc   t0,         %pcrel_hi(vectors)
	addi    t0,         t0,     %pcrel_lo(1b)
    ori     t0,         t0,     1
    csrw    mtvec,      t0
    # initalise the GOT
1:	auipc   a0,         %pcrel_hi(_got_start)
  	addi    a0,         a0,     %pcrel_lo(1b)
1:	auipc   a1,         %pcrel_hi(_got_end)
  	addi    a1,         a1,     %pcrel_lo(1b)
1:	auipc   a2,         %pcrel_hi(_entry)
  	addi    a2,         a2,     %pcrel_lo(1b)
1:  bgeu    a0,         a1,     1f
    lw      t0,         0(a0)
    add     t0,         t0,     a2
    sw      t0,         0(a0)
    addi    a0,         a0,     4
    j       1b
    # initalise the .bss section to zero
1:	auipc   a0,         %pcrel_hi(_bss_start)
  	addi    a0,         a0,     %pcrel_lo(1b)
1:	auipc   a1,         %pcrel_hi(_bss_end)
  	addi    a1,         a1,     %pcrel_lo(1b)
1:  bgeu    a0,         a1,     1f
    sw      zero,       0(a0)
    addi    a0,         a0,     4
    j       1b
1:  j       boot

mtip_handler:
    addi    sp,         sp,         -64
    SAVE_CALLER     0
    jal mtip
    RESTORE_CALLER  0
    addi    sp,         sp,         64
    mret
.section .vector_table
vectors:
    .align  2
    j       1f
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
    j mtip_handler  // mtip
    .align  2
    mret
    .align  2
    mret
    .align  2
    mret
    .align  2
    mret            // meip

    .align  2
1:  addi    sp,     sp,         -8
/*
    sw      ra,     0(sp)
    sw      a1,     4(sp)
    csrr    a1,     mcause
    bltz    a1,     1f
    jal     trap_excp
    j       2f
1:  jal     trap_usip
2:  lw      ra,     0(sp)
    lw      a1,     4(sp)
    addi    sp,     sp,         8
    mret
*/
