section .text

global gdt_set_table:function (gdt_set_table.end - gdt_set_table)
gdt_set_table:
	; Load GDT
	mov eax, dword [esp + 0x4]
	lgdt [eax]

	; Set data segment
	mov ax, word [esp + 0x8]
	mov ds, ax
	mov ss, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	; Set code segment
	; Use Far Call return routine to override CS value.
	push dword [esp + 0xC]
	push .continue
	retf
	.continue:

	ret
.end:


global idt_set_table:function (idt_set_table.end - idt_set_table)
idt_set_table:
	mov eax, [esp + 4]
	lidt [eax]
	ret
.end:
