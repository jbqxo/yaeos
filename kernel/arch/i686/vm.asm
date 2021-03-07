.section .text

.global i686_vm_set_pt
.type   i686_vm_set_pt, @function

i686_vm_set_pt:
        movl 4(%esp), %edi
        movl %edi, %cr3
        ret
.size i686_vm_set_pt, . - i686_vm_set_pt

.global i686_vm_paging_enable
.type   i686_vm_paging_enable, @function

i686_vm_paging_enable:
        /* Offset at which kernel will be loaded */
        movl 4(%esp), %edi

        /* Enable paging */
        movl %cr0, %eax
        orl  $(0x1 << 31), %eax
        movl %eax, %cr0

        /* Update stack pointers */
        addl %edi, %esp
        addl %edi, %ebp

        /* Fix return address */
        addl %edi, (%esp)

        /* Jump to the current half addresses. */
        leal 1f, %eax
        jmp  *%eax

1:
        ret
.size i686_vm_paging_enable, . - i686_vm_paging_enable
