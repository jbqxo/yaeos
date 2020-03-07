section .text

global set_gdt:function (set_gdt.end - set_gdt)
set_gdt:
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
