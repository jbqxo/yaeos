#define KERNEL_DS_SELECTOR $0x10

#define GEN_ISR_NOERR(number)           \
	.global isr ## number              ;\
	.type isr ## number, @function     ;\
	isr ## number:                     ;\
		cli                            ;\
		pushl $0                       ;\
		pushl $ ## number              ;\
		jmp common_handler             ;\
	.size isr ## number, . - isr ## number

// Macros to generate ISR for exceptions/interrupts without error codes.
#define GEN_ISR_ERR(number)             \
	.global isr ## number              ;\
	.type isr ## number, @function     ;\
	isr ## number:                     ;\
		cli                            ;\
		pushl $ ## number              ;\
		jmp common_handler             ;\
	.size isr ## number, . - isr ## number

.section .text
// The routine prepares environment for interrupt handler.
common_handler:
	// Save GP registers of the interrupted task.
	pusha

	// TODO: Save all segment selectors.
	// Save Data segment selector.
	movw %ds, %ax
	pushl %eax

	movw KERNEL_DS_SELECTOR, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
	movw %ax, %gs

	call switch_isr_handler

	popl %eax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
	movw %ax, %gs

	popa
	// Erase error code and Vector number
	addl $8, %esp
	sti
	iret

GEN_ISR_NOERR(0)
GEN_ISR_NOERR(1)
GEN_ISR_NOERR(2)
GEN_ISR_NOERR(3)
GEN_ISR_NOERR(4)
GEN_ISR_NOERR(5)
GEN_ISR_NOERR(6)
GEN_ISR_NOERR(7)
GEN_ISR_ERR(8)
GEN_ISR_NOERR(9)
GEN_ISR_ERR(10)
GEN_ISR_ERR(11)
GEN_ISR_ERR(12)
GEN_ISR_ERR(13)
GEN_ISR_ERR(14)
GEN_ISR_NOERR(15)
GEN_ISR_NOERR(16)
GEN_ISR_ERR(17)
GEN_ISR_NOERR(18)
GEN_ISR_NOERR(19)
GEN_ISR_NOERR(20)
GEN_ISR_ERR(21)
