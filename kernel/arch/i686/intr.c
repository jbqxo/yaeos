#include "arch_i686/intr.h"

#include "arch_i686/descriptors.h"
#include "arch_i686/io.h"

#include "kernel/klog.h"

#include "lib/cstd/assert.h"
#include "lib/cstd/inttypes.h"

#include <stdint.h>

static intr_handler_fn HANDLERS[0x100] = { NULL };
static intr_handler_fn DEFAULT_HANDLER = NULL;

// TODO: PIC. Mask unused interrupt lines.

// See: https://pdos.csail.mit.edu/6.828/2010/readings/hardware/8259A.pdf

#define MASTER_PIC  (uint8_t)(0x20)
#define MASTER_CMD  (uint8_t)(MASTER_PIC)
#define MASTER_DATA (uint8_t)(MASTER_CMD + 1)

#define SLAVE_PIC  (uint8_t)(0xA0)
#define SLAVE_CMD  (uint8_t)(SLAVE_PIC)
#define SLAVE_DATA (uint8_t)(SLAVE_PIC + 1)

#define ICW1_INIT (uint8_t)(0x1 << 4)
#define ICW1_LTIM (uint8_t)(0x1 << 3)
#define ICW1_ADI  (uint8_t)(0x1 << 2)
#define ICW1_SNGL (uint8_t)(0x1 << 1)
#define ICW1_IC4  (uint8_t)(0x1 << 0)

#define ICW4_8086 (uint8_t)(0x1)

#define OCW2_EOI (uint8_t)(0x1 << 5)

static void pic_remap(uint8_t master_offset, uint8_t slave_offset)
{
        uint8_t master_mask;
        uint8_t slave_mask;
        ioread(MASTER_DATA, master_mask);
        ioread(SLAVE_DATA, slave_mask);

        // ICW1
        uint8_t icw1 = ICW1_INIT | ICW1_IC4;
        iowrite(MASTER_CMD, icw1);
        iowrite(SLAVE_CMD, icw1);

        // ICW2
        iowrite(MASTER_DATA, master_offset);
        iowrite(SLAVE_DATA, slave_offset);

        // ICW3
        uint8_t slave_to_master_irq = 0x1 << 2;
        uint8_t slave_id = 0x2;
        iowrite(MASTER_DATA, slave_to_master_irq);
        iowrite(SLAVE_DATA, slave_id);

        // ICW4
        uint8_t icw4 = ICW4_8086;
        iowrite(MASTER_DATA, icw4);
        iowrite(SLAVE_DATA, icw4);

        iowrite(MASTER_DATA, master_mask);
        iowrite(SLAVE_DATA, slave_mask);
}

static void pic_write_eoi(uint8_t irq)
{
        if (irq > 0x7) {
                iowrite(SLAVE_CMD, OCW2_EOI);
        }
        iowrite(MASTER_CMD, OCW2_EOI);
}

static void call_handler(struct intr_ctx *ctx)
{
        uint32_t intn = ctx->int_n;

        if (HANDLERS[intn]) {
                HANDLERS[intn](ctx);
        } else if (DEFAULT_HANDLER) {
                DEFAULT_HANDLER(ctx);
        } else {
                LOGF_E("Received an interrupt #%" PRIu32
                       ", but there are no registered handlers...\n",
                       intn);
        }
}

void irq_handler(struct intr_ctx ctx)
{
        if (ctx.int_n < 0x8) {
                ctx.int_n += IRQ_MASTER_OFFSET;
        } else {
                ctx.int_n += IRQ_SLAVE_OFFSET;
        }
        call_handler(&ctx);

        pic_write_eoi((uint8_t)ctx.int_n);
}

void isr_handler(struct intr_ctx ctx)
{
        call_handler(&ctx);
}

void intr_handler_cpu_default(intr_handler_fn f)
{
        kassert(DEFAULT_HANDLER == NULL);
        DEFAULT_HANDLER = f;
}

void intr_handler_cpu(uint8_t int_no, intr_handler_fn f)
{
        kassert(int_no < 0x16);
        kassert(HANDLERS[int_no] == NULL);
        HANDLERS[int_no] = f;
}

void intr_handler_pic(uint8_t int_no, intr_handler_fn f)
{
        kassert(int_no < 0x10);
        // Cascade IRQ. It's never raised.
        kassert(int_no != 0x2);

        uint8_t isr_no = IRQ_MASTER_OFFSET + int_no;
        if (int_no > 0x7) {
                isr_no = IRQ_SLAVE_OFFSET + int_no - 0x8;
        }
        kassert(HANDLERS[isr_no] == NULL);
        HANDLERS[isr_no] = f;
}

void intr_init(void)
{
        enum idt_flag flags = IDT_FLAG_PRESENT | IDT_FLAG_RING_0;
#define SET_ISR(num, type)          \
        extern void isr##num(void); \
        idt_set_gatedesc((num), isr##num, flags, GATE_TYPE_##type##_32)

        SET_ISR(0, TRAP);
        SET_ISR(1, TRAP);
        SET_ISR(2, INTER);
        SET_ISR(3, TRAP);
        SET_ISR(4, TRAP);
        SET_ISR(5, TRAP);
        SET_ISR(6, TRAP);
        SET_ISR(7, TRAP);
        SET_ISR(8, TRAP);
        SET_ISR(10, TRAP);
        SET_ISR(11, TRAP);
        SET_ISR(12, TRAP);
        SET_ISR(13, TRAP);
        SET_ISR(14, TRAP);
        SET_ISR(15, TRAP);
        SET_ISR(16, TRAP);
        SET_ISR(17, TRAP);
        SET_ISR(18, TRAP);
        SET_ISR(19, TRAP);
        SET_ISR(20, TRAP);
        SET_ISR(21, TRAP);

#define SET_IRQ(num)                \
        extern void irq##num(void); \
        idt_set_gatedesc(IRQ_MASTER_OFFSET + (num), irq##num, flags, GATE_TYPE_INTER_32)

        SET_IRQ(0);
        SET_IRQ(1);
        // IRQ2 is never raised.
        SET_IRQ(3);
        SET_IRQ(4);
        SET_IRQ(5);
        SET_IRQ(6);
        SET_IRQ(7);

        SET_IRQ(8);
        SET_IRQ(9);
        SET_IRQ(10);
        SET_IRQ(11);
        SET_IRQ(12);
        SET_IRQ(13);
        SET_IRQ(14);
        SET_IRQ(15);

        pic_remap(IRQ_MASTER_OFFSET, IRQ_SLAVE_OFFSET);
}
