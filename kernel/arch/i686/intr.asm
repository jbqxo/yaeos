#define KERNEL_DS_SELECTOR $0x10

#define GEN_ISR_NOERR(number)                   \
        .global isr ## number                  ;\
        .type isr ## number, @function         ;\
        isr ## number:                         ;\
                cli                            ;\
                pushl $0                       ;\
                pushl $ ## number              ;\
                jmp isr_handler                ;\
        .size isr ## number, . - isr ## number

// Macros to generate ISR for exceptions/interrupts without error codes.
#define GEN_ISR_ERR(number)                     \
        .global isr ## number                  ;\
        .type isr ## number, @function         ;\
        isr ## number:                         ;\
                cli                            ;\
                pushl $ ## number              ;\
                jmp stub_isr_handler           ;\
        .size isr ## number, . - isr ## number

#define GEN_IRQ(number)                                \
        .global irq ## number                         ;\
        .type irq ## number, @function                ;\
        irq ## number:                                ;\
                cli                                   ;\
                pushl $0                              ;\
                pushl $ ## number                     ;\
                jmp stub_irq_handler                  ;\
        .size irq ## number, . - irq ## number

.section .text
// The routine prepares environment for interrupt handler.
stub_isr_handler:
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

        call isr_handler

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

// The routine prepares environment for interrupt handler.
stub_irq_handler:
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

GEN_IRQ(0)
GEN_IRQ(1)
// IRQ2 is never raised.
GEN_IRQ(3)
GEN_IRQ(4)
GEN_IRQ(5)
GEN_IRQ(6)
GEN_IRQ(7)

GEN_IRQ(8)
GEN_IRQ(9)
GEN_IRQ(10)
GEN_IRQ(11)
GEN_IRQ(12)
GEN_IRQ(13)
GEN_IRQ(14)
GEN_IRQ(15)
