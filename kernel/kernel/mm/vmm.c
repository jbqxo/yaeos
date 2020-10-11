#include "kernel/mm/vmm.h"

#include "kernel/cppdefs.h"
#include "kernel/ds/rbtree.h"
#include "kernel/ds/slist.h"
#include "kernel/kernel.h"
#include "kernel/klog.h"
#include "kernel/mm/kmm.h"
#include "kernel/utils.h"

static struct {
	struct kmm_cache *mappings;
	struct kmm_cache *regions;
} CACHES;

struct vm_mapping vm_mapping_new(void *start, size_t length, int flags, struct vmm_region *region,
				 size_t region_offset)
{
	struct vm_mapping mapping = { 0 };
	mapping.start = start;
	mapping.length = length;
	mapping.region = region;
	mapping.region_offset = region_offset;
	mapping.flags = flags;
	return (mapping);
}

int vm_mapping_cmp(void *_x, void *_y)
{
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
	kassert(CACHES.mappings);
	kassert(CACHES.regions);
}

///
/// Checks if there are occupied pages in the given range.
///
static bool space_occupied(struct vm_space *s, void *start_vaddr, void *end_vaddr)
{
	kassert(s);

	uintptr_t left_bound = ptr2uint(start_vaddr);
	uintptr_t right_bound = ptr2uint(end_vaddr);

	struct rbtree_node *infront_node =
		rbtree_search_max(&s->vmappings.tree, uint2ptr(left_bound - 1));
	if (infront_node == NULL) {
		// The tree is empty.
		return (false);
	}

	struct vm_mapping *infront = infront_node->data;
	struct vm_mapping *next = SLIST_NEXT(infront, sorted);

	return (ptr2uint(next->start) <= right_bound);
}

struct vm_mapping *vmm_alloc_pages_at(struct vm_space *s, void *vaddr, size_t count)
{
	// TODO: Return error codes.
	// There is no way for the caller to know why the operation failed.
	// Is it because the system is ran out of memory, or because the desired range is occupied?
	kassert(s);

	size_t mem_length = count * PLATFORM_PAGE_SIZE;

	void *end_vaddr = uint2ptr(ptr2uint(vaddr) + mem_length);
	if (space_occupied(s, vaddr, end_vaddr)) {
		return (NULL);
	}

	struct vm_mapping *map = kmm_cache_alloc(CACHES.mappings);
	map->start = vaddr;
	map->length = mem_length;
	map->region = NULL;
	map->region_offset = 0;

	rbtree_init_node(&map->process_mappings);
	map->process_mappings.data = map;

	struct vm_mapping *predecessor = NULL;
	struct vm_mapping *it;
	SLIST_FOREACH (it, &s->vmappings.sorted_list, sorted) {
		if (ptr2uint(it->start) > ptr2uint(vaddr)) {
			break;
		}
		predecessor = it;
	}

	if (predecessor != NULL) {
		SLIST_INSERT_AFTER(predecessor, map, sorted);
	} else {
		SLIST_INSERT_HEAD(&s->vmappings.sorted_list, map, sorted);
	}
	rbtree_insert(&s->vmappings.tree, &map->process_mappings);

	// TODO: Configure page tree

	return (map);
}

static size_t find_free_space(struct vm_space *s, size_t pg_count, uintptr_t *result)
{
	// TODO: Search for free space before the list, then after, then in the middle.
	// Maybe implement circular doubly linked list?
	kassert(pg_count > 0);

	// TODO: Calculate actual platform's address spaces' limits.
	static const uintptr_t low_platform = 0;
	static const uintptr_t high_platform = -1;

	if (SLIST_FIRST(&s->vmappings.sorted_list) != NULL) {
		struct vm_mapping *first = SLIST_FIRST(&s->vmappings.sorted_list);
		size_t space_before_first = ptr2uint(first->start) - low_platform;
		if (space_before_first >= pg_count * PLATFORM_PAGE_SIZE) {
			*result = low_platform;
			return (space_before_first / PLATFORM_PAGE_SIZE);
		}
	}

	struct vm_mapping *prev = NULL;
	struct vm_mapping *current;
	SLIST_FOREACH (current, &s->vmappings.sorted_list, sorted) {
		if (prev != NULL) {
			uintptr_t current_start = ptr2uint(current->start);
			uintptr_t prev_end = ptr2uint(prev->start) + prev->length;

			kassert(current_start > prev_end);

			uintptr_t low_bound = align_roundup(prev_end, PLATFORM_PAGE_SIZE);
			uintptr_t high_bound = align_rounddown(current_start, PLATFORM_PAGE_SIZE);

			kassert(low_bound <= high_bound);
			size_t free_space = high_bound - low_bound;
			if (free_space >= pg_count * PLATFORM_PAGE_SIZE) {
				*result = low_bound;
				return ((int)free_space / PLATFORM_PAGE_SIZE);
			}
		}
		prev = current;
	}

	if (prev != NULL) {
		size_t space_after_last = high_platform - (ptr2uint(prev->start) + prev->length);
		if (space_after_last >= pg_count * PLATFORM_PAGE_SIZE) {
			*result = ptr2uint(prev->start) + prev->length;
			return (space_after_last / PLATFORM_PAGE_SIZE);
		}
	} else {
		// If prev == NULL, the list is empty, so whole address space is empty.
		*result = 0;
		return ((high_platform - low_platform) / PLATFORM_PAGE_SIZE);
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

void vmm_free_mapping(struct vm_space *s, struct vm_mapping *mapping)
{
	kassert(s);

	SLIST_REMOVE(&s->vmappings.sorted_list, mapping, sorted);
	rbtree_delete(&s->vmappings.tree, &mapping->process_mappings);
	kmm_cache_free(CACHES.mappings, mapping);

	// TODO: Free region if it's reference counter is 0.
}

void vmm_free_pages(struct vm_space *s, void *vaddress, size_t count)
{
	kassert(s);

	// TODO: Make an appropriate mapping search by a virtual address.
	struct rbtree_node dummy_node;
	dummy_node.data = vaddress;

	struct vm_mapping *current = rbtree_search(&s->vmappings.tree, &dummy_node)->data;
	struct vm_mapping *next = NULL;

	for (int i = 0; i < count; i++) {
		if (current == NULL) {
			LOGF_E("Tried to free too many pages");
			break;
		}

		// BUG: Won't work correctly if mapping's length isn't equal to PAGE_SIZE.
		kassert(current->length == PLATFORM_PAGE_SIZE);

		next = SLIST_NEXT(current, sorted);
		vmm_free_mapping(s, current);
		current = next;
	}
}
