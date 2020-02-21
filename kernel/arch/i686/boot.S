; TODO: Move these constants in a separate header
; Constants
%define KERNEL_VIRT_OFFSET 0xC0000000
%define STACK_SIZE         16384 ; 16 KiB
%define VGA_BUFFER         0xB8000

%define PE_FLAGS_PRESENT (0x1 << 0)
%define PE_FLAGS_RW      (0x1 << 1)
%define PE_DEFAULT_FLAGS (PE_FLAGS_PRESENT | PE_FLAGS_RW)

; Constants for the multiboot header
%define FLAG_ALIGN   (0x1 << 0)                  ; align loaded modules on page boundaries
%define FLAG_MEMINFO (0x1 << 1)                  ; provide memory map
%define FLAGS        (FLAG_ALIGN | FLAG_MEMINFO) ; multiboot flags
%define MAGIC        0x1BADB002                  ; set "magic number"
%define CHECKSUM     -(MAGIC + FLAGS)            ; set multiboot checksum

; Multiboot header
section .multiboot align=4
    dd MAGIC
    dd FLAGS
    dd CHECKSUM

; Preallocate space used for boot-time stack.
section .bootstrap_stack write nobits align=16
stack_bottom: 
    resb STACK_SIZE
stack_top:

; Preallocate space used for boot-time paging.
section .bss write nobits align=4096
boot_paging_pd:
    resd 1024
boot_paging_pt:
    resd 1024

; "Interesting places"
extern _kernel_start
extern _kernel_end
%define PADDR_KERNEL_START (_kernel_start - KERNEL_VIRT_OFFSET)
%define PADDR_KERNEL_END (_kernel_end - KERNEL_VIRT_OFFSET)
%define PADDR_STACKTOP     (stack_top - KERNEL_VIRT_OFFSET)
%define PADDR_PDIR         (boot_paging_pd - KERNEL_VIRT_OFFSET)
%define PADDR_PTABLE       (boot_paging_pt - KERNEL_VIRT_OFFSET)

;(PADDR_KERNEL_START & 0x3FF000) >> 12
; 0x3FF000 is all 1's from bit 12 up to bit 21.
; See section 4.3 in Intel's Software Dev Manual V3.
%macro ptable_index 2
    mov %1, %2
    and %1, 0x3FF000
    shr %1, 12
%endmacro

%macro pdir_index 2
    mov %1, %2
    and %1, 0xFFC00000
    shr %1, 22
%endmacro

section .text

; Maps kernel addresses.
map_kernel:
    push ebp
    mov ebp, esp
    push edi
    push esi

    ; Start index in the page table.
    ptable_index eax, PADDR_KERNEL_START

    ; Physical Address of the entry in the page table.
    lea edi, [PADDR_PTABLE + eax * 4]
    ; Loop counter. Do not write more PTE than available.
    mov ecx, eax
    ; Physical Address to be mapped.
    mov esi, PADDR_KERNEL_START

.map:
    ; Loop condition: Only map the kernel's addresses.
    cmp esi, PADDR_KERNEL_END
    jge .done

    ; Write new page table entry
    mov edx, esi
    or edx, PE_DEFAULT_FLAGS
    mov [edi], edx

    add esi, 4096
    add edi, 4                  ; 4 is the size of a single entry

    ; Loop condition: Don't map more pages then required
    loop .map
    ; TODO: Add runtime warning about the kernel's size. There is a chance that it doesn't fit into one page table.

    ; At this point we have mapped the whole kernel, so exit.
.done:
    pop esi
    pop edi
    pop ebp
    ret

; Maps platform specific areas (VGA buffer, for example)
map_platform:
    push ebp
    mov ebp, esp
    push edi

    ; Map VGA buffer
    ptable_index eax, VGA_BUFFER
    lea edi, [PADDR_PTABLE + eax * 4]
    mov dword [edi], VGA_BUFFER | PE_DEFAULT_FLAGS

    pop edi
    pop ebp
    ret

; Enables paging.
enable_paging:
    ; Identity Map page table.
    mov edi, PADDR_PTABLE
    or edi, PE_DEFAULT_FLAGS
    mov [PADDR_PDIR], edi

    pdir_index eax, KERNEL_VIRT_OFFSET
    mov [PADDR_PDIR + eax * 4], edi

    ; Enable paging
    mov eax, PADDR_PDIR
    mov cr3, eax
    ; TODO: Check that protected mode was enabled.
    ; Enable paging
    mov eax, cr0
    or eax, 0x1 << 31
    mov cr0, eax

    ; Update stack pointers
    add esp, KERNEL_VIRT_OFFSET
    add ebp, KERNEL_VIRT_OFFSET

    ; Fix return address on the stack
    add dword [esp], KERNEL_VIRT_OFFSET

    ; Continue execution of the function in the higher-half
    lea eax, [.virtual_eip]
    jmp eax

.virtual_eip:
    ; Remove PT identity mapping
    mov dword [PADDR_PDIR], 0
    invlpg [PADDR_PTABLE]
    ret


; The kernel's entry point
global _start:function (_start.end - _start)
_start:
    mov esp, PADDR_STACKTOP
    mov ebp, PADDR_STACKTOP

    call map_kernel
    call map_platform
    call enable_paging

    extern i686_init
    call i686_init

    cli
.hang:
    hlt
    jmp .hang
.end:
