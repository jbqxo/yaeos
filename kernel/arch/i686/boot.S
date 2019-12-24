# Constants for the multiboot header
.set ALIGN,    1<<0             # align loaded modules on page boundaries
.set MEMINFO,  1<<1             # provide memory map
.set FLAGS,    ALIGN | MEMINFO  # multiboot flags
.set MAGIC,    0x1BADB002       # set "magic number"
.set CHECKSUM, -(MAGIC + FLAGS) # set multiboot checksum

# Set the multiboot header
.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

# Reserve space for the kernel stack
.section .bootstrap_stack, "aw", @nobits
stack_bottom: 
.skip 16384 # 16 KiB
stack_top:

.section .bss

# The kernel's entry point
.section .text
.global _start
.type _start, @function
_start:
    movl $stack_top, %esp
    movl $stack_top, %ebp

    call _init
    call i686_init

    # If platform code return control by some reason
    cli
1:  hlt
    jmp 1b
.size _start, . - _start
