#include "kernel/mm/mm.h"

#include "kernel/kernel.h"
#include "kernel/klog.h"
#include "kernel/platform_consts.h"

#include "lib/align.h"
#include "lib/cstd/assert.h"
#include "lib/cstd/string.h"
#include "lib/mm/linear.h"

static struct slist_ref MM_ZONES;

void mm_init(void)
{
        slist_init(&MM_ZONES);
}

static void mm_zone_register(struct mm_zone *zone)
{
        kassert(zone != NULL);
        slist_insert(&MM_ZONES, &zone->sys_zones);
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
        uintptr_t area_start = (uintptr_t)phys_start;

        kassert(kernel_vmspace != NULL);
        kassert(check_align(area_start, PLATFORM_PAGE_SIZE));
        kassert(check_align(length, PLATFORM_PAGE_SIZE));

        /* Place temporary area over the zone until we can allocate a zone object from itself.
         * Then we could copy the area to the zone. */
        struct vm_area tmp_area;

        area_start += kernel_vmspace->offset;

        kassert(check_align(area_start, PLATFORM_PAGE_SIZE));

        vm_area_init(&tmp_area, (void *)area_start, length, kernel_vmspace);
        vm_space_append_area(kernel_vmspace, &tmp_area);

        tmp_area.length = length;
        tmp_area.flags |= VM_WRITE;
        tmp_area.ops.handle_pg_fault = addr_pgfault_handler_maplow;

        struct linear_alloc *alloc = bootstrap_zone_alloc((void *)area_start, length);

        struct mm_zone *zone = linear_alloc_alloc(alloc, sizeof(*zone));
        zone->buddym = linear_alloc_alloc(alloc, sizeof(*zone->buddym));
        zone->alloc = alloc;

        kmemcpy(&zone->info_area, &tmp_area, sizeof(zone->info_area));
        zone->info_area.rb_areas.data = &zone->info_area;
        vm_space_remove_area(kernel_vmspace, &tmp_area);
        vm_space_append_area(kernel_vmspace, &zone->info_area);

        zone->start = phys_start;
        zone->length = length;
        slist_init(&zone->sys_zones);

        size_t free_pages = buddy_init(zone->buddym, length / PLATFORM_PAGE_SIZE, zone->alloc);
        zone->pages = linear_alloc_alloc(zone->alloc, free_pages * sizeof(*zone->pages));

        /* We don't need to allocate more space, and we can ruin zone's content if we do. */
        linear_forbid_further_alloc(zone->alloc);

        /* The area should cover only the information required for maintaining a zone. */
        size_t zone_area_len = linear_alloc_occupied(zone->alloc);
        zone_area_len = align_roundup(zone_area_len, PLATFORM_PAGE_SIZE);
        zone->info_area.length = zone_area_len;

        for (size_t i = 0; i < free_pages; i++) {
                kassert(i * PLATFORM_PAGE_SIZE <= zone->length);
                mm_page_init_free(&zone->pages[i], zone->start + i * PLATFORM_PAGE_SIZE);
        }

        const size_t used_pages = zone->info_area.length / PLATFORM_PAGE_SIZE;

        for (size_t i = 0; i < used_pages; i++) {
                zone->pages[i].state = PAGESTATE_FIXED;
                bool success = buddy_try_alloc(zone->buddym, 0, i);
                if (__unlikely(!success)) {
                        LOGF_P("Couldn't reserve a page!\n");
                }
        }

        zone->pages_count = free_pages - used_pages;
        mm_zone_register(zone);

        return (zone);
}

void mm_page_init_free(struct mm_page *p, void *phys_addr)
{
        kassert(p != NULL);

        p->paddr = phys_addr;
        p->state = PAGESTATE_FREE;
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
        SLIST_FOREACH (it, slist_next(&MM_ZONES)) {
                struct mm_zone *z = container_of(it, struct mm_zone, sys_zones);
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

static struct mm_zone *find_zone(phys_addr_t addr)
{
        uintptr_t paddr = (uintptr_t)addr;
        struct mm_zone *zone = NULL;
        SLIST_FOREACH (it, slist_next(&MM_ZONES)) {
                struct mm_zone *z = container_of(it, struct mm_zone, sys_zones);

                uintptr_t zstart = (uintptr_t)z->start;
                uintptr_t zend = zstart + z->length;
                if (paddr >= zstart && paddr < zend) {
                        zone = z;
                        break;
                }
        }

        return (zone);
}

static size_t get_page_ndx(struct mm_zone *zone, phys_addr_t addr)
{
        uintptr_t p = (uintptr_t)addr;

        p &= align_rounddown(p, PLATFORM_PAGE_SIZE);
        p -= (uintptr_t)zone->start;
        p /= PLATFORM_PAGE_SIZE;

        kassert(p < zone->pages_count);

        return (p);
}

struct mm_page *mm_get_page_by_paddr(void *phys_addr)
{
        struct mm_zone *zone = find_zone(phys_addr);

        if (__unlikely(zone == NULL)) {
                /* Passed a virtual address?
                 * What a silly person. */
                return (NULL);
        }

        size_t const page_ndx = get_page_ndx(zone, phys_addr);

        kassert(({
                struct mm_page *page = &zone->pages[page_ndx];
                page->paddr == (void *)((uintptr_t)phys_addr & -PLATFORM_PAGE_SIZE);
        }));

        return (&zone->pages[page_ndx]);
}

void mm_free_page(phys_addr_t addr)
{
        struct mm_zone *zone = find_zone(addr);
        kassert(zone != NULL);

        size_t const page_ndx = get_page_ndx(zone, addr);

        kassert(!buddy_is_free(zone->buddym, page_ndx));
        kassert(zone->pages[page_ndx].state == PAGESTATE_OCCUPIED);

        buddy_free(zone->buddym, page_ndx, 0);
        zone->pages[page_ndx].state = PAGESTATE_FREE;
}
