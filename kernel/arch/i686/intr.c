#include <stdint.h>
#include <kernel/klog.h>
#include <arch_i686/intr.h>
#include <arch_i686/descriptors.h>

static intr_handler_fn HANDLERS[22] = { NULL };
static intr_handler_fn DEFAULT_HANDLER = NULL;

void switch_isr_handler(struct intr_ctx ctx)
{
	int intn = ctx.int_n;

	if (HANDLERS[intn]) {
		HANDLERS[intn](&ctx);
	} else if (DEFAULT_HANDLER) {
		DEFAULT_HANDLER(&ctx);
	} else {
		LOGF_E("Received an interrupt #%d, but there are no registered handlers...\n",
		       intn);
	}
}

void intr_i686_set_default(intr_handler_fn f)
{
	DEFAULT_HANDLER = f;
}

void intr_i686_set_exception_h(enum intr_cpu i, intr_handler_fn f)
{
	HANDLERS[i] = f;
}

void intr_i686_init(void)
{
	enum idt_flag flags = IDT_FLAG_PRESENT | IDT_FLAG_RING_0;
#define SET_ISR(num, type)                                                                         \
	extern void isr##num(void);                                                                \
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
#undef SET_ISR
}
