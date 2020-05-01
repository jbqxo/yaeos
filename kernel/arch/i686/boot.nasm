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

; TODO: Move these constants in a separate header
; Constants
%define STACK_SIZE         16384 ; 16 KiB

; Preallocate space used for boot-time stack.
section .bootstack write nobits align=16
global bootstack_bottom:data, bootstack_top:data
bootstack_bottom:
    resb STACK_SIZE
bootstack_top:

; Preallocate space used for boot-time paging.
section .bss write nobits align=4096
global boot_paging_pd:data, boot_paging_pt:data
boot_paging_pd:
    resd 1024
boot_paging_pt:
    resd 1024

section .text

extern _kernel_vma
; The kernel's entry point
global _start:function (_start.end - _start)
_start:
    mov esp, bootstack_top
    sub esp, _kernel_vma
    mov ebp, esp

    extern i686_init
    push eax
    push ebx
    call i686_init

    cli
.hang:
    hlt
    jmp .hang
.end:
