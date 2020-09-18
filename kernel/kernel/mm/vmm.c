#include "kernel/mm/vmm.h"

#include "kernel/cppdefs.h"
#include "kernel/ds/rbtree.h"
#include "kernel/ds/slist.h"
#include "kernel/kernel.h"
#include "kernel/mm/kmm.h"
#include "kernel/task.h"
#include "kernel/utils.h"

static struct {
	struct kmm_cache *mappings;
	struct kmm_cache *regions;
} CACHES;

struct vm_mapping vm_mapping_new(union uiptr start, size_t length, struct vmm_region *region,
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

	union uiptr xstart = ((struct vm_mapping *)_x)->start;
	union uiptr ystart = ((struct vm_mapping *)_y)->start;

	if (xstart.num == ystart.num) {
		return (0);
	} else if (xstart.num < ystart.num) {
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

struct vm_mapping *vmm_alloc_pages_at(struct vm_space *s, void *vaddr, size_t count, int flags)
{
	assert(s);

	bool occupied = false;
}

static size_t find_free_space(struct vm_space *s, size_t pg_count, union uiptr *result)
{
	assert(s);
	assert(pg_count > 0);

	struct vm_mapping *prev = NULL;
	struct vm_mapping *current;
	SLIST_FOREACH (current, &s->vmappings.sorted_list, sorted) {
		if (prev) {
			union uiptr current_start = current->start;
			union uiptr prev_end = num2uiptr(prev->start.num + prev->length);

			assert(current_start.num > prev_end.num);

			union uiptr low_bound = align_roundup(prev_end, PLATFORM_PAGE_SIZE);
			union uiptr high_bound = align_rounddown(current_start, PLATFORM_PAGE_SIZE);

			assert(low_bound.num <= high_bound.num);
			size_t free_space = high_bound.num - low_bound.num;
			if (free_space >= pg_count * PLATFORM_PAGE_SIZE) {
				*result = low_bound;
				return ((int)free_space / PLATFORM_PAGE_SIZE);
			}
		}
		prev = current;
	}

	// Couldn't find required space.
	*result = num2uiptr(0);
	return (0);
}

struct vm_mapping *vmm_alloc_pages(struct vm_space *s, size_t count, int flags)
{
	// Find required free space.
	// Call alloc_at.
	//     Assert that there is enough space.
	//     Create mapping object.
	//     Create region object.
	//     init region
	//         Set paddr to 0, as it won't be backed by the physical memory by default.
	//         Set length.
	//         Set flags.
	//     init mapping
	//         Set start: Can be in higher or lower memory.
	//         Set length.
	//         Connect with region.
	//         *Calculate* and set region_offset.
	//         Wire it up with RBT and linked list.
	// Result: Inited mapping for user- or kernel- space.
	union uiptr location;
	if (find_free_space(s, count, &location) < count) {
		return (NULL);
	}

	return (vmm_alloc_pages_at(s, location.ptr, count, flags));
}
