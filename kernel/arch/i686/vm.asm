.section .text

.global vm_set_active_pt
.type   vm_set_active_pt, @function

vm_set_active_pt:
        movl 4(%esp), %edi
        movl %edi, %cr3
        ret
.size vm_set_active_pt, . - vm_set_active_pt

.global vm_paging_enable
.type   vm_paging_enable, @function

vm_paging_enable:
        // Offset at which kernel will be loaded
        movl 4(%esp), %edi

        // Enable paging
        movl %cr0, %eax
        orl  $(0x1 << 31), %eax
        movl %eax, %cr0

        // Update stack pointers
        addl %edi, %esp
        addl %edi, %ebp

        // Fix return address
        addl %edi, (%esp)

        // Jump to the current half addresses.
        leal 1f, %eax
        jmp  *%eax

1:
        ret
.size vm_paging_enable, . - vm_paging_enable
