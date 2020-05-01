KERNEL_DS_SELECTOR equ 0x10

;; Macros to generate IRQ for exceptions/interrupts without error codes.
%macro GEN_IRQ_NOERR 1
global irq%1:function (irq%1.end - irq%1)
irq%1:
	cli
	push dword 0
	push dword %1
	jmp common_handler
.end:
%endmacro

;; Macros to generate IRQ for exceptions/interrupts with error codes.
%macro GEN_IRQ_ERR 1
global irq%1:function (irq%1.end - irq%1)
irq%1:
	cli
	push dword %1
	jmp common_handler
.end:
%endmacro

section .text


extern irq_handler
;; The routine prepares environment for interrupt handler.
common_handler:
	;; Save GP registers of the interrupted task.
	pusha

;; TODO: Save all segment selectors.
	;; Save Data segment selector.
	mov ax, ds
	push eax
	
	mov ax, KERNEL_DS_SELECTOR
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	
	call irq_handler
	
	pop eax
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	
	popa
	;; Erase error code and Vector number
	add esp, 8
	sti
	iret

GEN_IRQ_NOERR 0
GEN_IRQ_NOERR 1
GEN_IRQ_NOERR 2
GEN_IRQ_NOERR 3
GEN_IRQ_NOERR 4
GEN_IRQ_NOERR 5
GEN_IRQ_NOERR 6
GEN_IRQ_NOERR 7
GEN_IRQ_ERR 8
GEN_IRQ_NOERR 9
GEN_IRQ_ERR 10
GEN_IRQ_ERR 11
GEN_IRQ_ERR 12
GEN_IRQ_ERR 13
GEN_IRQ_ERR 14
GEN_IRQ_NOERR 15
GEN_IRQ_NOERR 16
GEN_IRQ_ERR 17
GEN_IRQ_NOERR 18
GEN_IRQ_NOERR 19
GEN_IRQ_NOERR 20
GEN_IRQ_ERR 21
GEN_IRQ_NOERR 22
GEN_IRQ_NOERR 23
GEN_IRQ_NOERR 24
GEN_IRQ_NOERR 25
GEN_IRQ_NOERR 26
GEN_IRQ_NOERR 27
GEN_IRQ_NOERR 28
GEN_IRQ_NOERR 29
GEN_IRQ_NOERR 30
GEN_IRQ_NOERR 31
