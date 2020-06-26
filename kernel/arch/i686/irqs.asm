#define KERNEL_DS_SELECTOR $0x10

#define GEN_IRQ_NOERR(number)           \
	.global irq ## number          ;\
	.type irq ## number, @function ;\
	irq ## number:                 ;\
		cli                    ;\
		pushl $0                ;\
		pushl number           ;\
		jmp common_handler      ;\
	.size irq ## number, . - irq ## number

// Macros to generate IRQ for exceptions/interrupts without error codes.
#define GEN_IRQ_ERR(number)             \
	.global irq ## number          ;\
	.type irq ## number, @function ;\
	irq ## number:                 ;\
		cli                    ;\
		pushl number           ;\
		jmp common_handler     ;\
	.size irq ## number, . - irq ## number

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
	
	call irq_handler
	
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

GEN_IRQ_NOERR(0)
GEN_IRQ_NOERR(1)
GEN_IRQ_NOERR(2)
GEN_IRQ_NOERR(3)
GEN_IRQ_NOERR(4)
GEN_IRQ_NOERR(5)
GEN_IRQ_NOERR(6)
GEN_IRQ_NOERR(7)
GEN_IRQ_ERR(8)
GEN_IRQ_NOERR(9)
GEN_IRQ_ERR(10)
GEN_IRQ_ERR(11)
GEN_IRQ_ERR(12)
GEN_IRQ_ERR(13)
GEN_IRQ_ERR(14)
GEN_IRQ_NOERR(15)
GEN_IRQ_NOERR(16)
GEN_IRQ_ERR(17)
GEN_IRQ_NOERR(18)
GEN_IRQ_NOERR(19)
GEN_IRQ_NOERR(20)
GEN_IRQ_ERR(21)
GEN_IRQ_NOERR(22)
GEN_IRQ_NOERR(23)
GEN_IRQ_NOERR(24)
GEN_IRQ_NOERR(25)
GEN_IRQ_NOERR(26)
GEN_IRQ_NOERR(27)
GEN_IRQ_NOERR(28)
GEN_IRQ_NOERR(29)
GEN_IRQ_NOERR(30)
GEN_IRQ_NOERR(31)
