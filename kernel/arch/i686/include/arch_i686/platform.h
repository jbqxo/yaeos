#ifndef _KERNEL_ARCH_I686_PLATFORM_H
#define _KERNEL_ARCH_I686_PLATFORM_H

#define PLATFORM_PAGE_SIZE       (4096)
#define PLATFORM_REGISTERS_COUNT (20)
#define PLATFORM_PAGEDIR_SIZE    PLATFORM_PAGE_SIZE
#define PLATFORM_PAGEDIR_COUNT   (2)

#ifndef __ASSEMBLER__
#include "kernel/cppdefs.h"

#include <multiboot.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct arch_info_i686 {
	multiboot_info_t *multiboot;
};

extern struct arch_info_i686 I686_INFO;

static inline void irq_enable(void)
{
	asm volatile("sti");
}

static inline void irq_disable(void)
{
	asm volatile("cli");
}

__noreturn static inline void halt(bool serve_irq)
{
	if (serve_irq) {
		irq_enable();
	} else {
		irq_disable();
	}
	while (1) {
		asm volatile("hlt");
	}
}

#endif // __ASSEMBLER__
#endif // _KERNEL_ARCH_I686_PLATFORM_H
