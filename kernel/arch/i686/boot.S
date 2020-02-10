#include <arch/platform.h>

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
    .skip CONF_STACK_SIZE
stack_top:
#define STACKTOP_PHYS (stack_top - CONF_KERNEL_RESIDE_ADDR)

.section .bss, "aw", @nobits
    .align PLATFORM_PAGE_SIZE
paging_pd:
    .skip PLATFORM_PAGE_SIZE
paging_pt:
    .skip PLATFORM_PAGE_SIZE

#define PT_PHYS (paging_pt - CONF_KERNEL_RESIDE_ADDR)
#define PD_PHYS (paging_pd - CONF_KERNEL_RESIDE_ADDR)

.section .text

# The function maps kernel addresses.
# @param pt_pos current position in a page table.
# @param len remaining length of the table.
# @return number of placed entries.
# unsigned map_kernel(void *pt_pos, unsigned len)
map_kernel:
    pushl %ebp
    movl %esp, %ebp
    pushl %edi
    pushl %esi

    # PA of the entry in PT.
    movl 8(%ebp), %edi
    # Loop counter. Do not write more PTE than available.
    movl 12(%ebp), %ecx
    # PA of the current address to be mapped.
    movl $(_kernel_start - CONF_KERNEL_RESIDE_ADDR), %esi

1:
    # Loop condition: Only map the kernel's addresses.
    cmpl $(_kernel_end - CONF_KERNEL_RESIDE_ADDR), %esi
    jge 2f

    # Write a PTE
    movl %esi, %edx
    orl $(PLATFORM_PTE_WRITE | PLATFORM_PTE_PRESENT), %edx
    movl %edx, (%edi)

    addl $(PLATFORM_PAGE_SIZE), %esi
    # 4 is the size of one PT entry.
    addl $4, %edi

    # Loop condition: Don't map more pages that we need.
    loop 1b
    # TODO: Add runtime warning about the kernel's size. There is a chance that it doesn't fit into one page table.

# At this point we have mapped the whole kernel, so exit.
2:
    # Calculate the number of placed entries.
    movl 8(%ebp), %eax
    subl %ecx, %eax

    popl %esi
    popl %edi
    popl %ebp
    ret

# The function maps platform specific areas (VGA buffer, for example).
# @param pt_pos current position in a page table
# @param len remaining length of the table
# @return number of placed entries. 0 if an error occurred.
# unsigned map_platform(void *pt_pos, unsigned len)
map_platform:
    pushl %ebp
    movl %esp, %ebp
    pushl %edi
    pushl %esi

    movl 8(%esp), %edi
    movl 12(%esp), %ecx

    # Map VGA buffer
    testl %ecx, %ecx
    jz 1f
    movl $(PLATFORM_VGA_BUFFER + PLATFORM_PTE_PRESENT + PLATFORM_PTE_WRITE), (%edi)
    decl %ecx

    # Calculate the number of placed entries.
    movl 8(%esp), %eax
    subl %ecx, %eax

    popl %esi
    popl %edi
    popl %ebp
    ret

1:
    # An error occurred.
    movl $0, %eax

    popl %esi
    popl %edi
    popl %ebp
    ret

# The function enables paging.
# void enable_paging(void)
enable_paging:
    #define PTE_FLAGS (PLATFORM_PTE_WRITE + PLATFORM_PTE_PRESENT)

    # Map the page table itself. 
    movl $(PT_PHYS + PTE_FLAGS), PD_PHYS
    movl $(PT_PHYS + PTE_FLAGS), PD_PHYS + (CONF_KERNEL_RESIDE_ADDR >> 22)

    # Enable paging
    movl $PD_PHYS, %eax
    movl %eax, %cr3
    movl %cr0, %eax
    # From Intel manuals:
    # "Setting the PG flag when the PE flag is clear causes a general-protection exception."
    orl $(1 << 31 | 1), %eax
    movl %eax, %cr0

    lea 2f, %eax
    jmp *%eax

2:
    # Remove PT identity mapping
    movl $0, PD_PHYS
    invlpg PT_PHYS
    ret


# The kernel's entry point
.global _start
.type _start, @function
_start:
    movl $STACKTOP_PHYS, %esp
    movl $STACKTOP_PHYS, %ebp

    # Registers esi, edi contains number of free page table entries and the current page table position, respectevely.
    movl $1023, %esi
    movl $PT_PHYS, %edi

    pushl %esi
    pushl %edi
    call map_kernel
    subl $8, %esp

    test %eax, %eax
    jz 1f

    # Update esi, edi registers according to result.
    subl %eax, %esi
    movl $4, %edx
    mull %edx
    addl %eax, %edi

    pushl %esi
    pushl %edi
    call map_platform
    subl $8, %esp

    test %eax, %eax
    jz 1f

    call enable_paging

    # Update stack pointers
    addl $CONF_KERNEL_RESIDE_ADDR, %esp
    addl $CONF_KERNEL_RESIDE_ADDR, %ebp

    call _init
    call i686_init

    # If platform code return control by some reason
    cli
1:  hlt
    jmp 1b
.size _start, . - _start
