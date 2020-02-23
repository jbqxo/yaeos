ENTRY(_start)

SECTIONS
{
    _kernel_vma = 0xC0000000;
    . = 0xC0100000;

    _kernel_start = .;

    _kernel_text_start = .;
    .text ALIGN(4K): AT (ADDR(.text) - 0xC0000000)
    {
        *(.multiboot)
        *(.text)
    }
    _kernel_text_end = .;

    _kernel_rodata_start = .;
    .rodata ALIGN(4K): AT (ADDR(.rodata) - 0xC0000000)
    {
        *(.rodata)
    }
    _kernel_rodata_end = .;

    _kernel_data_start = .;
    .data ALIGN(4K): AT (ADDR(.data) - 0xC0000000)
    {
        *(.data)
    }
    _kernel_data_end = .;

    _kernel_bss_start = .;
    .bss ALIGN(4K): AT (ADDR(.bss) - 0xC0000000)
    {
        *(COMMON)
        *(.bss)
        *(.bootstack)
    }
    _kernel_bss_end = .;

    _kernel_end = .;
}
