#ifndef _KERNEL_ARCH_PMM_H
#define _KERNEL_ARCH_PMM_H

#include <stddef.h>
#include <stdint.h>

struct pmm_chunk {
	void *mem;
	size_t length;
#define MEM_TYPE_AVAIL (0x1)
#define MEM_TYPE_RESERVED (0x2)
#define MEM_TYPE_ACPI (0x3)
#define MEM_TYPE_HIBER (0x4)
#define MEM_TYPE_UNAVAIL (0x5)
	uint8_t type;
};

int pmm_arch_available_chunks(void);
void pmm_arch_get_chunks(struct pmm_chunk *chunks);

#endif // _KERNEL_ARCH_PMM_H
