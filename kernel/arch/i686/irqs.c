#include <stdint.h>

#include <kernel/tty.h>

struct registers {
	uint32_t ds;

	uint32_t edi;
	uint32_t esi;
	uint32_t ebp;
	uint32_t esp;
	uint32_t ebx;
	uint32_t edx;
	uint32_t ecx;
	uint32_t eax;

	uint32_t int_n;
	uint32_t err_code;

	uint32_t eip;
	uint32_t cs;
	uint32_t eflags;
	uint32_t preint_esp;
};

struct interrupt_frame {
	uint32_t eip;
	uint32_t cs;
	uint32_t flags;
	uint32_t esp;
	uint32_t ss;
};

void irq_handler(struct registers regs)
{
	return;
}
