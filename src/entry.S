.globl _entry
.section .init
_entry:
    li      sp,         0x4003fffc
    j       boot
