ENTRY(_start)

SECTIONS
{
    . = 0x80000;
    __kernel_start = .;

    .text ALIGN(4K) : {
        __kernel_text_start = .;
        *(.boot.text)
        *(.text*)
        __kernel_text_end = .;
    }

    .rodata ALIGN(4K) : {
        __kernel_rodata_start = .;
        *(.rodata*)
        __kernel_rodata_end = .;
    }

    .data ALIGN(4K) : {
        __kernel_data_start = .;
        *(.data*)
        *(SORT_BY_NAME(.elflist_*));
        __kernel_data_end = .;
    }

    .bss ALIGN(4K) : {
        __kernel_bss_start = .;
        *(COMMON)
        *(.bss*)
        . = ALIGN(0x1000);
        *(.bootstack)
        __kernel_bss_end = .;
    }

    __kernel_end = .;
}
