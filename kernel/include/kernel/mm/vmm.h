#ifndef _KERNEL_MM_VMM_H
#define _KERNEL_MM_VMM_H

#include <stddef.h>

struct vmm_addr_space {
	void *page_tree_root;
};

#define VMM_ALLOC_KERNEL (0x1 << 0)
#define VMM_ALLOC_HUGE (0x1 << 0)

void vmm_init(void);
void *vmm_alloc_pages(size_t count, int flags);
void *vmm_alloc_pages_at(void *address, size_t count, int flags);
void vmm_free_pages_at(void *address);

#endif // _KERNEL_MM_VMM_H
