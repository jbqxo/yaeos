#include "kernel/mm/vmm.h"

#include "kernel/cppdefs.h"
#include "kernel/ds/rbtree.h"
#include "kernel/ds/slist.h"
#include "kernel/kernel.h"
#include "kernel/mm/kmm.h"
#include "kernel/utils.h"

static struct {
	struct kmm_cache *mappings;
	struct kmm_cache *regions;
} CACHES;

struct vm_mapping vm_mapping_new(void *start, size_t length, struct vmm_region *region,
				size_t region_offset)
{
	struct vm_mapping mapping = {0};
	mapping.start = start;
	mapping.length = length;
	mapping.region = region;
	mapping.region_offset = region_offset;
	return (mapping);
}

int vm_mapping_cmp(void *_x, void *_y)
{
	assert(_x);
	assert(_y);

	uintptr_t xstart = ptr2uint(((struct vm_mapping *)_x)->start);
	uintptr_t ystart = ptr2uint(((struct vm_mapping *)_y)->start);

	if (xstart == ystart) {
		return (0);
	} else if (xstart < ystart) {
		return (-1);
	} else {
		return (1);
	}
}

struct vm_space vm_space_new(size_t offset)
{
	struct vm_space new;
	new.offset = offset;
	rbtree_init_tree(&new.vmappings.tree, vm_mapping_cmp);
	SLIST_INIT(&new.vmappings.sorted_list);

	return (new);
}

void vmm_init(void)
{
	CACHES.mappings = kmm_cache_create("vmm_mappings", sizeof(struct vm_mapping), 0,
					   KMM_CACHE_STATIC, NULL, NULL);
	CACHES.regions = kmm_cache_create("vmm_regions", sizeof(struct vmm_region), 0,
					  KMM_CACHE_STATIC, NULL, NULL);
	assert(CACHES.mappings);
	assert(CACHES.regions);
}

struct vm_mapping *vmm_alloc_pages_at(struct vm_space *s, void *vaddr, size_t count)
{
	assert(s);

	bool occupied = false;
}

static size_t find_free_space(struct vm_space *s, size_t pg_count, uintptr_t *result)
{
	assert(s);
	assert(pg_count > 0);

	struct vm_mapping *prev = NULL;
	struct vm_mapping *current;
	SLIST_FOREACH (current, &s->vmappings.sorted_list, sorted) {
		if (prev) {
			uintptr_t current_start = ptr2uint(current->start);
			uintptr_t prev_end = ptr2uint(prev->start) + prev->length;

			assert(current_start > prev_end);

			uintptr_t low_bound = align_roundup(prev_end, PLATFORM_PAGE_SIZE);
			uintptr_t high_bound = align_rounddown(current_start, PLATFORM_PAGE_SIZE);

			assert(low_bound <= high_bound);
			size_t free_space = high_bound - low_bound;
			if (free_space >= pg_count * PLATFORM_PAGE_SIZE) {
				*result = low_bound;
				return ((int)free_space / PLATFORM_PAGE_SIZE);
			}
		}
		prev = current;
	}

	// Couldn't find required space.
	*result = 0;
	return (0);
}

struct vm_mapping *vmm_alloc_pages(struct vm_space *s, size_t count)
{
	uintptr_t location;
	if (find_free_space(s, count, &location) < count) {
		return (NULL);
	}

	return (vmm_alloc_pages_at(s, uint2ptr(location), count));
}
