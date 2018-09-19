OUTPUT_ARCH ("riscv")

SECTIONS
{
    . = 0x00000000;
    .init                    : { *(.init) }
    . = ALIGN(4);
    _got_start = .;
    .got                     : { *(.got) }
    .got.plt        ALIGN(4) : { *(.got.plt) }
    . = ALIGN(4);
    _got_end = .;
    .text          ALIGN(64) : { *(.vector_table) }
    .text           ALIGN(4) : { *(.text*) }
    .data           ALIGN(4) : { *(.data) }
    .rodata         ALIGN(4) : { *(.rodata*) }
    /*.rodata         ALIGN(4) : { *(.rodata.str1.4) }*/
    . = ALIGN(4);
    _bss_start = .;
    .bss                     : { *(.bss) }
    .bss            ALIGN(4) : { *(COMMON) }
    . = ALIGN(4);
    _bss_end = .;
    . = ALIGN(4);
    _mpool_start = .;
    /DISCARD/                : {*(*)}
    /*
    */
}

ENTRY(_entry)
