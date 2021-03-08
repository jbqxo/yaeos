#ifndef _KERNEL_MM_H
#define _KERNEL_MM_H

#include "kernel/mm/addr.h"
#include "kernel/mm/vm.h"

#include "lib/cppdefs.h"
#include "lib/ds/slist.h"
#include "lib/mm/buddy.h"

#include <stdint.h>

struct mm_page {
        phys_addr_t paddr;

        enum page_state {
                PAGESTATE_FREE,
                PAGESTATE_OCCUPIED,
                PAGESTATE_FIXED, /**< A page must remain in memory. */
        } state;
};

void mm_page_init_free(struct mm_page *, void *phys_addr);

struct mm_zone {
        phys_addr_t start;
        size_t length;

        struct mm_page *pages;
        size_t pages_count;

        struct linear_alloc *alloc;
        struct buddy_manager *buddym;
        /* There is a vm area for every zone that covers linear allocator space.
         * Basically, it maps a virtual address to it's physical counterpart. */
        struct vm_area info_area;

        struct slist_ref sys_zones;
};

struct mm_zone *mm_zone_create(phys_addr_t start, size_t length, struct vm_space *kernel_vmspace);

struct mm_page *mm_alloc_page_from(struct mm_zone *zone);

struct mm_page *mm_alloc_page(void);

struct mm_page *mm_get_page_by_paddr(phys_addr_t addr);

void mm_init(void);

#endif /* _KERNEL_MM_H_ */
