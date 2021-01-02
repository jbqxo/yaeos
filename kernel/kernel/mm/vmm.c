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

void vmm_mapping_new(struct vmm_mapping *mapping, void *start, size_t length, int flags,
                     struct vmm_region *region, size_t region_offset)
{
        // TODO: Implement eager mappings.
        kassert(!(flags & VMMM_FLAGS_EAGER));
        kassert(check_align(length, PLATFORM_PAGE_SIZE));
        kassert(region_offset + length <= region->length);

        mapping->start = start;
        mapping->length = length;
        mapping->region = region;
        mapping->region_offset = region_offset;
        mapping->flags = flags;

        SLIST_FIELD_INIT(mapping, sorted_list);

        rbtree_init_node(&mapping->process_mappings);
        mapping->process_mappings.data = mapping;
}

int vmm_mapping_cmp(void *_x, void *_y)
{
        uintptr_t xstart = ptr2uint(((struct vmm_mapping *)_x)->start);
        uintptr_t ystart = ptr2uint(((struct vmm_mapping *)_y)->start);

        if (xstart == ystart) {
                return (0);
        } else if (xstart < ystart) {
                return (-1);
        } else {
                return (1);
        }
}

struct vmm_space vmm_space_new(size_t offset)
{
        struct vmm_space new;
        new.offset = offset;
        rbtree_init_tree(&new.vmappings.tree, vmm_mapping_cmp);
        SLIST_INIT(&new.vmappings.sorted_list);

        return (new);
}

void vmm_init(void)
{
        CACHES.mappings = kmm_cache_create("vmm_mappings", sizeof(struct vmm_mapping), 0,
                                           KMM_CACHE_STATIC, NULL, NULL);
        CACHES.regions = kmm_cache_create("vmm_regions", sizeof(struct vmm_region), 0,
                                          KMM_CACHE_STATIC, NULL, NULL);

        kassert(CACHES.mappings);
        kassert(CACHES.regions);
}

///
/// Checks if there are occupied pages in the given range.
///
static bool space_occupied(struct vmm_space *s, void *start_vaddr, void *end_vaddr)
{
        kassert(s);

        uintptr_t left_edge = ptr2uint(start_vaddr);
        uintptr_t right_edge = ptr2uint(end_vaddr);
        kassert(right_edge - left_edge > 0);

        struct rbtree_node *prior_node =
                rbtree_search_max(&s->vmappings.tree, uint2ptr(left_edge - 1));
        if (prior_node == NULL) {
                // The tree is empty.
                return (false);
        }

        struct vmm_mapping *prior = prior_node->data;

        if (ptr2uint(prior->start) + prior->length >= left_edge) {
                return (true);
        }

        // Next mapping can't start before left_edge. See \rbtree_search_max.
        struct vmm_mapping *next = SLIST_NEXT(prior, sorted_list);

        return (ptr2uint(next->start) <= right_edge);
}

struct vmm_mapping *vmm_alloc_pages_at(struct vmm_space *s, void *vaddr, size_t count)
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

        struct vmm_mapping *map = kmm_cache_alloc(CACHES.mappings);
        if (map == NULL) {
                return (NULL);
        }
        vmm_mapping_new(map, vaddr, mem_length, 0, NULL, 0);

        struct rbtree_node *rbt_prior_node = rbtree_search_max(&s->vmappings.tree, vaddr);

        if (rbt_prior_node != NULL) {
                struct vmm_mapping *prior_mapping = rbt_prior_node->data;
                kassert(prior_mapping);

                SLIST_INSERT_AFTER(prior_mapping, map, sorted_list);
        } else {
                SLIST_INSERT_HEAD(&s->vmappings.sorted_list, map, sorted_list);
        }

        rbtree_insert(&s->vmappings.tree, &map->process_mappings);

        // TODO: Configure page tree

        return (map);
}

// Return the number of free pages at a resulted address.
// We can't just return an address because there would be no way to indicate failure.
// Any value from 0 to UINTPTR_MAX may be valid.
static size_t find_free_space(struct vmm_space *s, size_t pg_count, uintptr_t *result)
{
        // Search for free space before the list, then after, then in the middle

        // TODO: Implement circular doubly linked list?
        kassert(pg_count > 0);

        // TODO: Calculate actual platform's address spaces' limits.
        // Skip the first page to not occupy 0x0.
        static const uintptr_t platform_lowest = PLATFORM_PAGE_SIZE;
        static const uintptr_t platform_highest = UINTPTR_MAX;
        const size_t required = pg_count * PLATFORM_PAGE_SIZE;

        if (SLIST_FIRST(&s->vmappings.sorted_list) != NULL) {
                struct vmm_mapping *first = SLIST_FIRST(&s->vmappings.sorted_list);
                size_t space_before_first = ptr2uint(first->start) - platform_lowest;
                kassert(ptr2uint(first->start) - platform_lowest > 0);

                if (space_before_first >= required) {
                        *result = ptr2uint(first->start) - required;
                        const size_t pages_available = space_before_first / PLATFORM_PAGE_SIZE;
                        return (pages_available);
                }
        }

        struct vmm_mapping *prev = NULL;
        struct vmm_mapping *current;
        SLIST_FOREACH (current, &s->vmappings.sorted_list, sorted_list) {
                if (prev != NULL) {
                        uintptr_t current_start = ptr2uint(current->start);
                        uintptr_t prev_end = ptr2uint(prev->start) + prev->length;

                        kassert(current_start > prev_end);
                        kassert(check_align(prev_end, PLATFORM_PAGE_SIZE));
                        kassert(check_align(current_start, PLATFORM_PAGE_SIZE));

                        const size_t free_space = current_start - prev_end;
                        if (free_space >= required) {
                                *result = prev_end;
                                const size_t pages_available = free_space / PLATFORM_PAGE_SIZE;
                                return (pages_available);
                        }
                }
                prev = current;
        }

        if (prev != NULL) {
                size_t space_after_last = platform_highest - (ptr2uint(prev->start) + prev->length);
                if (space_after_last >= required) {
                        *result = ptr2uint(prev->start) + prev->length;
                        const size_t pages_available = space_after_last / PLATFORM_PAGE_SIZE;
                        return (pages_available);
                }
        } else {
                // If prev == NULL, the list is empty, so whole address space is empty.
                *result = 0;
                const size_t platform_pages =
                        (platform_highest - platform_lowest) / PLATFORM_PAGE_SIZE;
                return (platform_pages);
        }

        *result = 0;
        return (0);
}

struct vmm_mapping *vmm_alloc_pages(struct vmm_space *s, size_t count)
{
        uintptr_t location;
        if (find_free_space(s, count, &location) < count) {
                return (NULL);
        }

        return (vmm_alloc_pages_at(s, uint2ptr(location), count));
}

void vmm_free_mapping(struct vmm_space *s, struct vmm_mapping *mapping)
{
        kassert(s);

        SLIST_REMOVE(&s->vmappings.sorted_list, mapping, sorted_list);
        rbtree_delete(&s->vmappings.tree, &mapping->process_mappings);
        kmm_cache_free(CACHES.mappings, mapping);

        // TODO: Free region if it's reference counter is 0.
}

void vmm_free_pages(struct vmm_space *s, void *vaddress, size_t count)
{
        kassert(s);

        // TODO: Make an appropriate mapping search by a virtual address.
        struct rbtree_node dummy_node;
        dummy_node.data = vaddress;

        struct vmm_mapping *current = rbtree_search(&s->vmappings.tree, &dummy_node)->data;
        struct vmm_mapping *next = NULL;

        for (int i = 0; i < count; i++) {
                if (current == NULL) {
                        LOGF_E("Tried to free too many pages\n");
                        break;
                }

                // BUG: Won't work correctly if mapping's length isn't equal to PAGE_SIZE.
                kassert(current->length == PLATFORM_PAGE_SIZE);

                next = SLIST_NEXT(current, sorted_list);
                vmm_free_mapping(s, current);
                current = next;
        }
}
