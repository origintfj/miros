.equ RAM_START_ADDR,    0x40000000
.equ RAM_END_ADDR,      0x40040000

.globl _entry
.section .init
_entry:
    # load an initial stack pointer
    li      sp,         RAM_END_ADDR
    addi    sp,         sp,     -4
    # set the m-mode trap vector
1:	auipc   t0,         %pcrel_hi(vector_table)
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
    # jump to the boot function in m-mode with interrupts enabled globaly
1:  j       boot

1:	auipc   t0,         %pcrel_hi(boot)
  	addi    t0,         t0,     %pcrel_lo(1b)
    csrw    mepc,       t0
    li      t0,         0x1880
    csrw    mstatus,    t0
    csrw    mie,        0
    mret

.section .text
vector_table:
    j       trap_usip_excp
    j       trap_ssip
    mret
    j       trap_msip
    j       trap_utip
    j       trap_stip
    mret
    j       vthread32_mtrap_handler
    j       trap_ueip
    j       trap_seip
    mret
    j       trap_meip
