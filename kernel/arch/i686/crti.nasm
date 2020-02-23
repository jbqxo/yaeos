section .init
global _init:function
_init:
    push ebp
    mov esp, ebp

    ; ... here goes content of crtbegin.o's .init section

section .fini
global _fini:function
_fini:
    push ebp
    mov esp, ebp

    ; ... here goes content of crtbegin.o's .fini section
