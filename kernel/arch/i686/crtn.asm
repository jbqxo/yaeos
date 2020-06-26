.section .init
	// ... here goes content of crtend.o's .init section
	popl %ebp
	ret 

.section .fini
	// ... here goes content of crtend.o's .fini section
	popl %ebp
	ret
