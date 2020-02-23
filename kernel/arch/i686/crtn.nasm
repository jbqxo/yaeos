 section .init
    ; ... here goes content of crtend.o's .init section
    pop ebp
    ret 

section .fini
    ; ... here goes content of crtend.o's .fini section
    pop ebp
    ret
