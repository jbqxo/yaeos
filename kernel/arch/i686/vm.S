section .text

global vm_paging_set:function (vm_paging_set.end - vm_paging_set)
vm_paging_set:
    mov edi, [esp + 4]
    mov cr3, edi
    ret
.end:

global vm_paging_enable:function (vm_paging_enable.end - vm_paging_enable)
vm_paging_enable:
    ; Offset at which kernel will be loaded
    mov edi, [esp + 4]

    ; Enable paging
    mov eax, cr0
    or eax, 0x1 << 31
    mov cr0, eax

    ; Update stack pointers
    add esp, edi
    add ebp, edi

    ; Fix return address
    add dword[esp], edi

    ; Jump to the current half addresses.
    lea eax, [.virtual_eip]
    jmp eax
.virtual_eip:
    ret
    
.end:
