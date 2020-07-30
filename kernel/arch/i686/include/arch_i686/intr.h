#ifndef _KERNEL_ARCH_I686_INTR_H
#define _KERNEL_ARCH_I686_INTR_H

#include <stdint.h>

struct intr_ctx {
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
	uint32_t ss;
};

enum intr_cpu {
	INTR_CPU_DIV_ERR      = 0x0,
	INTR_CPU_DEBUG        = 0x1,
	INTR_CPU_NMI          = 0x2,
	INTR_CPU_INT3         = 0x3,
	INTR_CPU_INTO         = 0x4,
	INTR_CPU_BOUND        = 0x5,
	INTR_CPU_INV_OP       = 0x6,
	INTR_CPU_NOMATH       = 0x7,
	INTR_CPU_DFAULT       = 0x8,
	INTR_CPU_INV_TSS      = 0xA,
	INTR_CPU_SEG_NP       = 0xB,
	INTR_CPU_SEG_SS       = 0xC,
	INTR_CPU_GP           = 0xD,
	INTR_CPU_PAGEFAULT    = 0xE,
	INTR_CPU_MATHFP_FAULT = 0x10,
	INTR_CPU_ALIGN_CH     = 0x11,
	INTR_CPU_HW_CH        = 0x12,
	INTR_CPU_SIMD_EXC     = 0x13,
	INTR_CPU_VIRT_EXC     = 0x14,
	INTR_CPU_CTRL_EXC     = 0x15,
 };

typedef void (*intr_handler_fn)(struct intr_ctx *);

void intr_i686_init(void);
void intr_i686_set_default(intr_handler_fn defaulth);
void intr_i686_set_exception_h(enum intr_cpu, intr_handler_fn);

#endif // _KERNEL_ARCH_I686_INTR_H
