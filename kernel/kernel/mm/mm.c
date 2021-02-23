#include "kernel/mm/mm.h"

#include "kernel/kernel.h"
#include "kernel/klog.h"
#include "kernel/mm/kmm.h"
#include "kernel/platform_consts.h"

#include "lib/align.h"
#include "lib/cstd/assert.h"
#include "lib/cstd/string.h"
#include "lib/mm/linear.h"

static struct kmm_cache ZONES_CACHE;

static SLIST_HEAD(, struct mm_zone) MM_ZONES;

void mm_init(void)
{
        SLIST_INIT(&MM_ZONES);
        kmm_cache_init(&ZONES_CACHE, "mm_zones", sizeof(struct mm_zone), 0, 0, NULL, NULL);
        kmm_cache_register(&ZONES_CACHE);
}

struct mm_zone *mm_zone_new(void)
{
        return (kmm_cache_alloc(&ZONES_CACHE));
}

static struct linear_alloc *bootstrap_zone_alloc(void *virt_start, const size_t length)
{
        struct linear_alloc bootstrap_alloc;
        linear_alloc_init(&bootstrap_alloc, virt_start, length);

        struct linear_alloc *zone_alloc = linear_alloc_alloc(&bootstrap_alloc, sizeof(*zone_alloc));
        kmemcpy(zone_alloc, &bootstrap_alloc, sizeof(*zone_alloc));

        return (zone_alloc);
}

struct mm_zone *mm_zone_create(void *phys_start, size_t length, struct vm_space *kernel_vmspace)
{
        kassert(kernel_vmspace != NULL);
        kassert(check_align(ptr2uint(phys_start), PLATFORM_PAGE_SIZE));
        kassert(check_align(length, PLATFORM_PAGE_SIZE));

        /* Place temporary area over the zone until we can allocate a zone object from itself.
         * Then we could copy the area to the zone. */
        struct vm_area tmp_area;

        union uiptr area_start = ptr2uiptr(phys_start);
        area_start.num += kernel_vmspace->offset.num;

        kassert(check_align(area_start.num, PLATFORM_PAGE_SIZE));

        vm_area_init(&tmp_area, area_start.ptr, length, kernel_vmspace);
        vm_space_append_area(kernel_vmspace, &tmp_area);

        tmp_area.length = length;
        tmp_area.flags |= VM_WRITE;
        tmp_area.ops.handle_pg_fault = vm_pgfault_handle_direct;

        struct linear_alloc *alloc = bootstrap_zone_alloc(area_start.ptr, length);

        struct mm_zone *zone = linear_alloc_alloc(alloc, sizeof(*zone));
        zone->buddym = linear_alloc_alloc(alloc, sizeof(*zone->buddym));
        zone->alloc = alloc;

        kmemcpy(&zone->info_area, &tmp_area, sizeof(zone->info_area));
        zone->info_area.rb_areas.data = &zone->info_area;
        vm_space_remove_area(kernel_vmspace, &tmp_area);
        vm_space_append_area(kernel_vmspace, &zone->info_area);

        zone->start = phys_start;
        zone->length = length;
        SLIST_FIELD_INIT(zone, list);

        size_t free_pages = buddy_init(zone->buddym, length / PLATFORM_PAGE_SIZE, zone->alloc);
        zone->pages = linear_alloc_alloc(zone->alloc, free_pages * sizeof(*zone->pages));

        /* We don't need to allocate more space, and we can ruin zone's content if we do. */
        linear_forbid_further_alloc(zone->alloc);

        /* The area should cover only the information required for maintaining a zone. */
        zone->info_area.length =
                align_roundup(linear_alloc_occupied(zone->alloc), PLATFORM_PAGE_SIZE);

        for (int i = 0; i < free_pages; i++) {
                kassert(i * PLATFORM_PAGE_SIZE <= zone->length);
                mm_page_init_free(&zone->pages[i], zone->start + i * PLATFORM_PAGE_SIZE);
        }

        const size_t used_pages = zone->info_area.length / PLATFORM_PAGE_SIZE;

        for (size_t i = 0; i < used_pages; i++) {
                zone->pages[i].state = PAGESTATE_FIXED;
                bool success = buddy_try_alloc(zone->buddym, i);
                if (__unlikely(!success)) {
                        LOGF_P("Couldn't reserve a page!\n");
                }
        }

        zone->pages_count = free_pages - used_pages;
}

void mm_page_init_free(struct mm_page *p, void *phys_addr)
{
        kassert(p != NULL);

        p->paddr = phys_addr;
        p->state = PAGESTATE_FREE;
}

void mm_zone_register(struct mm_zone *zone)
{
        kassert(zone != NULL);
        SLIST_INSERT_HEAD(&MM_ZONES, zone, list);
}

struct mm_page *mm_alloc_page_from(struct mm_zone *zone)
{
        kassert(zone != NULL);

        uint32_t page_ndx = 0;
        if (!buddy_alloc(zone->buddym, 0, &page_ndx)) {
                return (NULL);
        }

        return (&zone->pages[page_ndx]);
}

struct mm_page *mm_alloc_page(void)
{
        /* For now, just get a page from any zone. */
        struct mm_page *p = NULL;
        struct mm_zone *z = NULL;
        SLIST_FOREACH (z, &MM_ZONES, list) {
                p = mm_alloc_page_from(z);
                if (p != NULL) {
                        break;
                }
        }

        if (__unlikely(p == NULL)) {
                /* Out of memory? */
                return (NULL);
        }

        p->state = PAGESTATE_OCCUPIED;
        return (p);
}

struct mm_page *mm_get_page_by_paddr(void *phys_addr)
{
        union uiptr paddr = ptr2uiptr(phys_addr);
        struct mm_zone *zone = NULL;
        struct mm_zone *i = NULL;
        SLIST_FOREACH (i, &MM_ZONES, list) {
                uintptr_t zstart = ptr2uint(i->start);
                uintptr_t zend = zstart + i->length;
                if (paddr.num >= zstart && paddr.num < zend) {
                        zone = i;
                        break;
                }
        }

        if (__unlikely(zone == NULL)) {
                /* Passed a virtual address?
                 * What a silly person. */
                return (NULL);
        }

        paddr.num &= -PLATFORM_PAGE_SIZE;
        paddr.num -= ptr2uint(zone->start);
        paddr.num /= PLATFORM_PAGE_SIZE;
        size_t page_ndx = paddr.num;

        kassert(page_ndx < zone->pages_count);
        struct mm_page *page = &zone->pages[page_ndx];
        kassert(ptr2uint(page->paddr) == (ptr2uint(phys_addr) & -PLATFORM_PAGE_SIZE));

        return (&zone->pages[page_ndx]);
}
