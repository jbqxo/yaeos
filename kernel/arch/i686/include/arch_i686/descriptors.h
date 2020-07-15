#ifndef _KERNEL_ARCH_I686_DESCRIPTORS_H
#define _KERNEL_ARCH_I686_DESCRIPTORS_H

#include <stdint.h>
#include <stdbool.h>

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

/**
 * set_gdt() - set a gdt table.
 * @table - pointer to the gdt table.
 * @data_offset - offset in the gdt table for data selector.
 * @code_offset - offset in the gdt table for code selector.
 */
void gdt_set_table(struct gdt_ptr *table, uint16_t data_offset, uint16_t code_offset);
void boot_setup_gdt(void);

enum gate_type {
	GATE_TYPE_INTERRUPT_16 = 0x6,
	GATE_TYPE_TRAP_16 = 0x7,
	GATE_TYPE_INTERRUPT_32 = 0xE,
	GATE_TYPE_TRAP_32 = 0xF
};

enum idt_flag {
	IDT_FLAG_PRESENT = 0x1 << 3,
	IDT_FLAG_RING_0 = 0x0 << 1,
	IDT_FLAG_RING_1 = 0x1 << 1,
	IDT_FLAG_RING_2 = 0x2 << 1,
	IDT_FLAG_RING_3 = 0x3 << 1,
	IDT_FLAG_STORAGE = 0x1 << 0
};

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

void idt_set_table(struct idt_ptr *table);
void boot_setup_idt(void);

void divide_err_exception(void *frame);
// TODO: Is there better way to declare these?
extern void irq0(void);
extern void irq1(void);
extern void irq2(void);
extern void irq3(void);
extern void irq4(void);
extern void irq5(void);
extern void irq6(void);
extern void irq7(void);
extern void irq8(void);
extern void irq9(void);
extern void irq10(void);
extern void irq11(void);
extern void irq12(void);
extern void irq13(void);
extern void irq14(void);
extern void irq15(void);
extern void irq16(void);
extern void irq17(void);
extern void irq18(void);
extern void irq19(void);
extern void irq20(void);
extern void irq21(void);
extern void irq22(void);
extern void irq23(void);
extern void irq24(void);
extern void irq25(void);
extern void irq26(void);
extern void irq27(void);
extern void irq28(void);
extern void irq29(void);
extern void irq30(void);

#endif // _KERNEL_ARCH_I686_DESCRIPTORS_H
