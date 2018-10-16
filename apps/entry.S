.globl _entry
.section .init
_entry:
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
1:  j       main
