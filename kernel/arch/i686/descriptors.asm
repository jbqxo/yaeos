.section .text

.global gdt_set_table
.type gdt_set_table, @function
gdt_set_table:
	// Load GDT
	movl 0x4(%esp), %eax
	lgdt (%eax)

	// Set data segment
	movw 0x8(%esp), %ax
	movw %ax, %ds
	movw %ax, %ss
	movw %ax, %es
	movw %ax, %fs
	movw %ax, %gs

	// Set code segment
	// Use Far Call return routine to override CS value.
	pushl 0xC(%esp)
	pushl $1f
	retf
	1:

	ret
.size gdt_set_table, . - gdt_set_table


.global idt_set_table
.type idt_set_table, @function
idt_set_table:
	movl 4(%esp), %eax
	lidt (%eax)
	ret
.size idt_set_table, . - idt_set_table
