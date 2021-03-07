#ifndef _KERNEL_ARCH_I686_INTR_H
#define _KERNEL_ARCH_I686_INTR_H

#include <stdint.h>

#define IRQ_MASTER_OFFSET (32)
#define IRQ_SLAVE_OFFSET  (IRQ_MASTER_OFFSET + 8)

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

#define INTR_CPU_DIV_ERR      (0x0)
#define INTR_CPU_DEBUG        (0x1)
#define INTR_CPU_NMI          (0x2)
#define INTR_CPU_INT3         (0x3)
#define INTR_CPU_INTO         (0x4)
#define INTR_CPU_BOUND        (0x5)
#define INTR_CPU_INV_OP       (0x6)
#define INTR_CPU_NOMATH       (0x7)
#define INTR_CPU_DFAULT       (0x8)
#define INTR_CPU_INV_TSS      (0xA)
#define INTR_CPU_SEG_NP       (0xB)
#define INTR_CPU_SEG_SS       (0xC)
#define INTR_CPU_GP           (0xD)
#define INTR_CPU_PAGEFAULT    (0xE)
#define INTR_CPU_MATHFP_FAULT (0x10)
#define INTR_CPU_ALIGN_CH     (0x11)
#define INTR_CPU_HW_CH        (0x12)
#define INTR_CPU_SIMD_EXC     (0x13)
#define INTR_CPU_VIRT_EXC     (0x14)
#define INTR_CPU_CTRL_EXC     (0x15)

typedef void (*intr_handler_fn)(struct intr_ctx *);

void intr_init(void);
void intr_handler_cpu_default(intr_handler_fn handler_fn);
void intr_handler_cpu(uint8_t int_no, intr_handler_fn);
void intr_handler_pic(uint8_t int_no, intr_handler_fn);

void irq_handler(struct intr_ctx ctx);
void isr_handler(struct intr_ctx ctx);

#endif /* _KERNEL_ARCH_I686_INTR_H */
