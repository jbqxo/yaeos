.section .text

.global vm_paging_set
.type   vm_paging_set, @function

vm_paging_set:
        movl 4(%esp), %edi
        movl %edi, %cr3
        ret
.size vm_paging_set, . - vm_paging_set

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
