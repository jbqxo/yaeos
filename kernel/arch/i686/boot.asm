// Constants for the multiboot header
#define FLAG_ALIGN   (0x1 << 0)                  // align loaded modules on page boundaries
#define FLAG_MEMINFO (0x1 << 1)                  // provide memory map
#define FLAGS        (FLAG_ALIGN | FLAG_MEMINFO) // multiboot flags
#define MAGIC        0x1BADB002
#define CHECKSUM     -(MAGIC + FLAGS)

// Multiboot header
.section .multiboot
.align 4
    .long MAGIC
    .long FLAGS
    .long CHECKSUM

//TODO: Move these constants in a separate header
#define STACK_SIZE         16384 // 16 KiB

.section .bootstack, "aw", @nobits
.align 4096
.global bootstack_bottom
bootstack_bottom:
    .skip STACK_SIZE
.global bootstack_top
bootstack_top:

// Preallocate space used for boot-time paging.
.section .bss, "aw", @nobits
.align 4096
.global boot_paging_pd
boot_paging_pd:
    .skip 4096
.global boot_paging_pt
boot_paging_pt:
    .skip 4096

.section .text
// The kernel's entry point
.global _start
.type _start, @function
_start:
    movl $bootstack_top, %esp
    subl $_kernel_vma, %esp
    movl %esp, %ebp

    pushl %eax
    pushl %ebx
    call i686_init

    cli
1:
    hlt
    jmp 1b
.size _start, . - _start
