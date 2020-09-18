#ifndef _KERNEL_MM_VMM_H
#define _KERNEL_MM_VMM_H

#include "kernel/ds/rbtree.h"
#include "kernel/ds/slist.h"

#include <stddef.h>

struct vmm_region {
	void *paddr;
	size_t length;
#define VMMR_FLAGS_WRITE   (0x1 << 0)
#define VMMR_FLAGS_EXECUTE (0x1 << 1)
	int flags;
};

struct vm_mapping {
	void *start;
	size_t length;

	struct vmm_region *region;
	size_t region_offset;

	/* Store mappings in both, a RBT and a linked list.
	 * Generally, we use RBT to create, find a mapping.
	 * But in cases when order is important (to allocate or free continuous space, for example),
	 * we will be able to quickly find consequent pages with a linked list. */
	struct rbtree_node process_mappings;
	SLIST_FIELD(struct vm_mapping) sorted;
};
struct vm_mapping vm_mapping_new(union uiptr start, size_t length, struct vmm_region *region,
				 size_t region_offset);
int vm_mapping_cmp(void *_x, void *_y);

struct vm_space {
	size_t offset;
	struct {
		struct rbtree tree;
		SLIST_HEAD(, struct vm_mapping) sorted_list;
	} vmappings;
};
struct vm_space vm_space_new(size_t offset);

void vmm_init(void);
struct vm_mapping *vmm_alloc_pages(struct vm_space *, size_t count);
struct vm_mapping *vmm_alloc_pages_at(struct vm_space *, void *vaddr, size_t count);
void vmm_free_mapping(struct vm_space *, struct vm_mapping *);

#endif // _KERNEL_MM_VMM_H
