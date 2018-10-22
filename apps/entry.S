.globl _entry
.section .init
_entry:
    j       1f
    .align  2
    .word   _got_start
    .word   _got_end
    .word   _bss_start
    .word   _bss_end
1:  j       main
