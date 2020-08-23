#ifndef _KERNEL_MM_PMM_H
#define _KERNEL_MM_PMM_H

#include <kernel/ds/slist.h>

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef uintptr_t phy_address_t;

struct pmm_alloc_resutl {
	phy_address_t paddr;
	bool success;
};

struct pmm_page {
	phy_address_t paddr;
	struct pmm_allocator *alloc;
};

struct pmm_allocator {
	const char *name;
#define PMM_RESTRICT_DMA (0x1 << 0)
#define PMM_RESTRICT_SINGLE_ONLY (0x1 << 1)
	int restrict_flags;
	void *data;

	struct pmm_alloc_resutl (*page_alloc)(void *data);
	void (*page_free)(void *data, phy_address_t);

	SLIST_FIELD(struct pmm_allocator) allocators;
};


void pmm_init(struct pmm_allocator *allocators, size_t alloc_length);

#define PMM_FLAG_DMA (0x1 << 0)
struct pmm_page *pmm_alloc_page(int flags);
void pmm_free(struct pmm_page*);

#endif // _KERNEL_MM_PMM_H
