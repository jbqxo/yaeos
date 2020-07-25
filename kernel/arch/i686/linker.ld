ENTRY(_start)

SECTIONS
{
	__kernel_vma = 0xC0000000;
	. = 0xC0100000;

	__kernel_start = .;

	.text ALIGN(4K): AT (ADDR(.text) - 0xC0000000)
	{
		__kernel_text_start = .;
		*(.multiboot)
		*(.text)
		__kernel_text_end = .;
	}

	.rodata ALIGN(4K): AT (ADDR(.rodata) - 0xC0000000)
	{
		__kernel_rodata_start = .;
		*(.rodata)
		__kernel_rodata_end = .;
	}

	.data ALIGN(4K): AT (ADDR(.data) - 0xC0000000)
	{
		__kernel_data_start = .;
		*(.data)
		__kernel_data_end = .;

		*(SORT_BY_NAME(.elflist_*));
	}

	.bss ALIGN(4K): AT (ADDR(.bss) - 0xC0000000)
	{
		__kernel_bss_start = .;
		*(COMMON)
		*(.bss)
		*(.bootstack)
		__kernel_bss_end = .;
	}

	__kernel_end = .;
}
