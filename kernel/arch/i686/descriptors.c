#include <arch/descriptors.h>

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

static struct gdt_structure {
	struct gdt_entry null_descriptor;
	struct gdt_entry code_descriptor;
	struct gdt_entry data_descriptor;
} __attribute__((packed, aligned(8))) GDT;

static struct idt_structure {
	struct idt_entry cpu_gates[32];
	struct idt_entry system_gates[256 - 32];
} __attribute__((packed, aligned(8))) IDT;

/**
 * boot_setup_gdt() - setup system's gdt with flat memory model.
 */
void boot_setup_gdt(void)
{
	static const size_t LIMIT = 0xFFFFFFFF;

	struct gdt_entry *null_d = &GDT.null_descriptor;
	struct gdt_entry *code_d = &GDT.code_descriptor;
	struct gdt_entry *data_d = &GDT.data_descriptor;

	memset(null_d, 0, sizeof(*null_d));

	code_d->limit_low = LIMIT & 0xFFFF;
	code_d->limit_high = (LIMIT >> 0x10) & 0x0F;
	code_d->base_low = 0;
	code_d->base_high = 0;
	code_d->accessed = false;
	code_d->writable = false;
	code_d->direction_conforming = false;
	code_d->code = true;
	code_d->code_or_data = true;
	code_d->privelege = 0;
	code_d->present = true;
	code_d->must_be_false = false;
	code_d->size = true;
	code_d->granularity = true;

	data_d->limit_low = LIMIT & 0xFFFF;
	data_d->limit_high = (LIMIT >> 0x10) & 0x0F;
	data_d->base_low = 0;
	data_d->base_high = 0;
	data_d->accessed = false;
	data_d->writable = true;
	data_d->direction_conforming = false;
	data_d->code = false;
	data_d->code_or_data = true;
	data_d->privelege = 0;
	data_d->present = true;
	data_d->must_be_false = false;
	data_d->size = true;
	data_d->granularity = true;

	struct gdt_ptr p = { .limit = sizeof(GDT) - 1, .base = (uint32_t)&GDT };
	uint16_t code_offset = offsetof(struct gdt_structure, code_descriptor);
	uint16_t data_offset = offsetof(struct gdt_structure, data_descriptor);
	gdt_set_table(&p, data_offset, code_offset);
}

static void idt_set_gate(struct idt_entry *e, void *offset, uint16_t selector, enum idt_flag flags)
{
	e->offset_low = (uintptr_t)offset & 0xFFFF;
	e->offset_high = ((uintptr_t)offset >> 0x10) & 0xFFFF;

	e->seg_selector = selector;
	e->must_be_0 = 0;
	e->flags = flags;
	e->type = GATE_TYPE_INTERRUPT_32;
}

void boot_setup_idt(void)
{
	memset(&IDT, 0, sizeof(IDT));
	enum idt_flag flags = IDT_FLAG_PRESENT | IDT_FLAG_RING_0;
	uint16_t selector = offsetof(struct gdt_structure, code_descriptor);

	idt_set_gate(&IDT.cpu_gates[0], (void *)irq0, selector, flags);
	idt_set_gate(&IDT.cpu_gates[1], (void *)irq1, selector, flags);
	idt_set_gate(&IDT.cpu_gates[2], (void *)irq2, selector, flags);
	idt_set_gate(&IDT.cpu_gates[3], (void *)irq3, selector, flags);
	idt_set_gate(&IDT.cpu_gates[4], (void *)irq4, selector, flags);
	idt_set_gate(&IDT.cpu_gates[5], (void *)irq5, selector, flags);
	idt_set_gate(&IDT.cpu_gates[6], (void *)irq6, selector, flags);
	idt_set_gate(&IDT.cpu_gates[7], (void *)irq7, selector, flags);
	idt_set_gate(&IDT.cpu_gates[8], (void *)irq8, selector, flags);
	idt_set_gate(&IDT.cpu_gates[9], (void *)irq9, selector, flags);
	idt_set_gate(&IDT.cpu_gates[10], (void *)irq10, selector, flags);
	idt_set_gate(&IDT.cpu_gates[11], (void *)irq11, selector, flags);
	idt_set_gate(&IDT.cpu_gates[12], (void *)irq12, selector, flags);
	idt_set_gate(&IDT.cpu_gates[13], (void *)irq13, selector, flags);
	idt_set_gate(&IDT.cpu_gates[14], (void *)irq14, selector, flags);
	idt_set_gate(&IDT.cpu_gates[15], (void *)irq15, selector, flags);
	idt_set_gate(&IDT.cpu_gates[16], (void *)irq16, selector, flags);
	idt_set_gate(&IDT.cpu_gates[17], (void *)irq17, selector, flags);
	idt_set_gate(&IDT.cpu_gates[18], (void *)irq18, selector, flags);
	idt_set_gate(&IDT.cpu_gates[19], (void *)irq19, selector, flags);
	idt_set_gate(&IDT.cpu_gates[20], (void *)irq20, selector, flags);
	idt_set_gate(&IDT.cpu_gates[21], (void *)irq21, selector, flags);
	idt_set_gate(&IDT.cpu_gates[22], (void *)irq22, selector, flags);
	idt_set_gate(&IDT.cpu_gates[23], (void *)irq23, selector, flags);
	idt_set_gate(&IDT.cpu_gates[24], (void *)irq24, selector, flags);
	idt_set_gate(&IDT.cpu_gates[25], (void *)irq25, selector, flags);
	idt_set_gate(&IDT.cpu_gates[26], (void *)irq26, selector, flags);
	idt_set_gate(&IDT.cpu_gates[27], (void *)irq27, selector, flags);
	idt_set_gate(&IDT.cpu_gates[28], (void *)irq28, selector, flags);
	idt_set_gate(&IDT.cpu_gates[29], (void *)irq29, selector, flags);
	idt_set_gate(&IDT.cpu_gates[30], (void *)irq30, selector, flags);

	struct idt_ptr p = { .limit = sizeof(IDT) - 1, .base = (uint32_t)&IDT };
	idt_set_table(&p);
}
