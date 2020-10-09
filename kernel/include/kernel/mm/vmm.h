#ifndef _KERNEL_MM_VMM_H
#define _KERNEL_MM_VMM_H

#include "kernel/ds/rbtree.h"
#include "kernel/ds/slist.h"

#include <stddef.h>

struct vmm_region {
	void *paddr;
	size_t length;
};

struct vm_mapping {
	void *start;
	size_t length;

#define VMMM_FLAGS_WRITE     (0x1 << 0)
#define VMMM_FLAGS_USERSPACE (0x1 << 1)
	int flags;

	struct vmm_region *region;
	size_t region_offset;

	/* Store mappings in both, a RBT and a linked list.
	 * Generally, we use RBT to create, find a mapping.
	 * But in cases when order is important (to allocate or free continuous space, for example),
	 * we will be able to quickly find consequent pages with a linked list. */
	struct rbtree_node process_mappings;
	SLIST_FIELD(struct vm_mapping) sorted;
};
struct vm_mapping vm_mapping_new(void *start, size_t length, int flags, struct vmm_region *region,
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
#define VM_SPACE_MAPPINGS_FOREACH(_vmspace, _it_mapping) \
	SLIST_FOREACH ((_it_mapping), &(_vmspace)->vmappings.sorted_list, sorted)

void vmm_init(void);
struct vm_mapping *vmm_alloc_pages(struct vm_space *, size_t count);
struct vm_mapping *vmm_alloc_pages_at(struct vm_space *, void *vaddr, size_t count);
void vmm_free_mapping(struct vm_space *, struct vm_mapping *);
void vmm_free_pages(struct vm_space *, void *vaddress, size_t count);

#endif // _KERNEL_MM_VMM_H
