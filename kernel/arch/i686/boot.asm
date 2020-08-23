#include "arch_i686/platform.h"
#include "kernel/config.h"

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

.section .bootstack, "aw", @nobits
.align PLATFORM_PAGE_SIZE
.global bootstack_bottom
bootstack_bottom:
    .skip CONF_STACK_SIZE
.global bootstack_top
bootstack_top:

// Preallocate space used for boot-time paging.
.section .bss, "aw", @nobits
.align PLATFORM_PAGE_SIZE
.global boot_paging_pd
boot_paging_pd:
    .skip PLATFORM_PAGE_SIZE
.global boot_paging_pt
boot_paging_pt:
    .skip PLATFORM_PAGE_SIZE

.section .text
// The kernel's entry point
.global _start
.type _start, @function
_start:
    movl $bootstack_top, %esp
    subl $__kernel_vma, %esp
    // Set return address to 0 to be able to stop here while gathering backtrace.
    pushl $0x0
    movl %esp, %ebp

    pushl %eax
    pushl %ebx
    call i686_init

    cli
1:
    hlt
    jmp 1b
.size _start, . - _start
