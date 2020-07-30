#ifndef _KERNEL_ARCH_I686_DESCRIPTORS_H
#define _KERNEL_ARCH_I686_DESCRIPTORS_H

#include <stdint.h>
#include <stdbool.h>


enum gate_type {
	GATE_TYPE_INTER_16 = 0x6,
	GATE_TYPE_TRAP_16 = 0x7,
	GATE_TYPE_INTER_32 = 0xE,
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

void boot_setup_gdt(void);
void boot_setup_idt(void);
void idt_set_gatedesc(uint8_t gate_num, void *offset, enum idt_flag flags, enum gate_type gt);


#endif // _KERNEL_ARCH_I686_DESCRIPTORS_H
