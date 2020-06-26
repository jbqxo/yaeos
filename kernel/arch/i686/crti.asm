.section .init
.global _init
.type _init, @function
_init:
	pushl %ebp
	movl %esp, %ebp
	// ... here goes content of crtbegin.o's .init section

.section .fini
.global _fini
.type _fini, @function
_fini:
	pushl %ebp
	movl %esp, %ebp
	// ... here goes content of crtbegin.o's .fini section
