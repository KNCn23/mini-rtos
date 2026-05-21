ENTRY(_start)

SECTIONS
{
    . = 0x40080000;

    .text : ALIGN(8) {
        KEEP(*(.text.boot))
        *(.text*)
    }
    .rodata : ALIGN(8) { *(.rodata*) }
    .data   : ALIGN(8) { *(.data*)   }

    .bss : ALIGN(16) {
        __bss_start = .;
        *(.bss*) *(COMMON)
        __bss_end = .;
    }

    . = ALIGN(16);
    . = . + 0x10000;
    _stack_top = .;
}
