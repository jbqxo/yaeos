#include "kernel/mm/mm.h"

#include "kernel/mm/linear.h"
#include "kernel/platform.h"

#include "lib/assert.h"
#include "lib/string.h"

static SLIST_HEAD(, struct mm_zone) MM_ZONES;

void mm_init(void)
{
        SLIST_INIT(&MM_ZONES);
}

static struct linear_alloc *bootstrap_zone_alloc(struct mem_chunk *chunk)
{
        struct linear_alloc bootstrap_alloc;
        linear_alloc_init(&bootstrap_alloc, chunk->mem, chunk->length);
        struct linear_alloc *zone_alloc = linear_alloc_alloc(&bootstrap_alloc, sizeof(*zone_alloc));
        kmemcpy(zone_alloc, &bootstrap_alloc, sizeof(*zone_alloc));

        return (zone_alloc);
}

struct mm_zone *mm_zone_create_from(struct mem_chunk *chunk)
{
        struct linear_alloc *alloc = bootstrap_zone_alloc(chunk);
        struct mm_zone *zone = linear_alloc_alloc(alloc, sizeof(*zone));

        zone->start = chunk->mem;
        zone->length = chunk->length;
        zone->priv_alloc = alloc;
        zone->buddym = linear_alloc_alloc(zone->priv_alloc, sizeof(*zone->buddym));
        SLIST_FIELD_INIT(zone, list);

        size_t free_pages = buddy_init(zone->buddym, zone->length, zone->priv_alloc);
        zone->pages = linear_alloc_alloc(zone->priv_alloc, free_pages * sizeof(struct mm_page));

        for (int i = 0; i < free_pages; i++) {
                kassert(i * PLATFORM_PAGE_SIZE <= zone->length);
                mm_page_init_free(&zone->pages[i], zone->start + i * PLATFORM_PAGE_SIZE);
                zone->pages[i].state = PAGESTATE_FREE;
        }
        zone->pages_count = free_pages;

        size_t used_pages = linear_alloc_occupied_mem(zone->priv_alloc) / PLATFORM_PAGE_SIZE;

        for (int i = 0; i < used_pages; i++) {
                zone->pages[i].state = PAGESTATE_SYSTEM_IMPORTANT;
        }

        return (zone);
}

void mm_page_init_free(struct mm_page *p, void *phys_addr)
{
        kassert(p != NULL);
        /* Note, that it's undefined behaviour to touch *0x0 in C. */
        kassert(phys_addr != NULL);

        p->paddr = phys_addr;
        p->state = PAGESTATE_FREE;
        ownership_init(&p->owners);
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
        SLIST_FOREACH(z, &MM_ZONES, list) {
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
        SLIST_FOREACH(i, &MM_ZONES , list) {
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
