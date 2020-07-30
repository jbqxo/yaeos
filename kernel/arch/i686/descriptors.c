#include <arch_i686/descriptors.h>
#include <lib/string.h>
#include <kernel/cppdefs.h>

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * struct gdt_entry - the structure contains description of one segment descriptor.
 * @limit_low: the lowest part of the segment limit.
 * @base_low: the lowest part of the segment base address.
 * @accessed: the cpu sets the field to true when the segment is accessed.
 * @writable: whether to allow write access.
 * @direction_conforming: direction when code=false, conforming bit otherwise.
 * @code: whether the segment contains code or data.
 * @code_or_data: whether the segment descriptor is for a system management.
 * @privelege: contains the ring level.
 * @present: whether the segment is present in memory.
 * @limit_high: the highest part of the segment limit.
 * @available: you can store your vast arrays of information here.
 * @must_be_false: reserved for IA-32e. On IA-32 must be always 0.
 * @size: represents different size flags. See Intel's Vol. 3A: 3-11
 * @granularity: determines granularity of the limit field. If set to 1 the limit is in 4KiB blocks.
 * @base_high: the highest part of the segment base address.
 */
struct gdt_entry {
	uint16_t limit_low : 16;
	uint32_t base_low : 24;
	bool accessed : 1;
	bool writable : 1;
	bool direction_conforming : 1;
	bool code : 1;
	bool code_or_data : 1;
	uint8_t privelege : 2;
	bool present : 1;
	uint16_t limit_high : 4;
	bool available : 1;
	bool must_be_false : 1;
	bool size : 1;
	bool granularity : 1;
	uint8_t base_high : 8;
} __attribute__((packed));

/**
 * struct gdt_ptr - the structure specifies the address and the size of the gdt.
 * @limit: limit of the table.
 * @base: the address of the table.
 */
struct gdt_ptr {
	uint16_t limit;
	uint32_t base;
} __attribute__((packed));

static struct gdt_structure {
	struct gdt_entry null_descriptor;
	struct gdt_entry code_descriptor;
	struct gdt_entry data_descriptor;
} __attribute__((packed, aligned(8))) GDT;

struct idt_entry {
	uint16_t offset_low;
	uint16_t seg_selector;
	uint8_t must_be_0;
	enum gate_type type : 4;
	enum idt_flag flags : 4;
	uint16_t offset_high;
} __attribute__((packed));

struct idt_ptr {
	uint16_t limit;
	uint32_t base;
} __attribute__((packed));

static struct idt_structure {
	struct idt_entry gates[256];
} __attribute__((packed, aligned(8))) IDT;

static void gdt_set_table(struct gdt_ptr *table, uint16_t data_offset, uint16_t code_offset)
{
	// Load GDT
	asm volatile("lgdt (%[table])" : : [table] "r"(table));

	// Set Data segment
	asm volatile("movw %[data_sel], %%ds \n\t\
		      movw %[data_sel], %%ss \n\t\
		      movw %[data_sel], %%es \n\t\
		      movw %[data_sel], %%fs \n\t\
		      movw %[data_sel], %%gs \n\t"
		     :
		     : [data_sel] "r"(data_offset));

	// Set Code segment
	asm volatile goto("pushl %[code_sel] \n\t\
			   pushl $%l1 \n\t\
			   lret"
			  :
			  : [code_sel] "g"(code_offset)
			  :
			  : end);
end:;
}

static void idt_set_table(struct idt_ptr *table)
{
	asm volatile("lidt (%[table])" : : [table] "r"(table));
}

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

void idt_set_gatedesc(uint8_t gate_num, void *offset, enum idt_flag flags, enum gate_type gt)
{
	// Selector is not going to change, as we use flat memory model.
	uint16_t selector = offsetof(struct gdt_structure, code_descriptor);
	struct idt_entry *e = &IDT.gates[gate_num];
	e->offset_low = (uintptr_t)offset & 0xFFFF;
	e->offset_high = ((uintptr_t)offset >> 0x10) & 0xFFFF;

	e->seg_selector = selector;
	e->must_be_0 = 0;
	e->flags = flags;
	e->type = gt;
}

void boot_setup_idt(void)
{
	memset(&IDT, 0, sizeof(IDT));
	struct idt_ptr p = { .limit = sizeof(IDT) - 1, .base = (uint32_t)&IDT };
	idt_set_table(&p);
}
