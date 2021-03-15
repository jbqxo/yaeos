#include "kernel/mm/dev.h"

#include "kernel/config.h"
#include "kernel/klog.h"
#include "kernel/mm/kmm.h"
#include "kernel/mm/vm.h"
#include "kernel/mm/vm_area.h"
#include "kernel/resources.h"

#include "lib/cppdefs.h"
#include "lib/ds/slist.h"

#define DEV_AREA_FLAGS (VM_CACHE_OFF | VM_WRITE)

/*
 * It's possible now to register same phys address multiple times.
 * TODO: Store regions in a form of a radix tree with a physical address as the tag.
 * KDev now uses single dev area.
 * TODO: Implement KDev as a multiple areas.
 */

/* The state of an area is described via a linked list of such regions.
 * Single region describes either a region occupied by a resource, or a free region. */

struct region {
        void *page_vaddr;
        size_t length;
        union region_resource {
                enum region_resource_state {
                        REGION_RES_STATE_FREE = 0x0,
                        REGION_RES_STATE_UNKNOWN = 0x1,
                } state;
                struct resource *ptr;
        } resource;

#define region_of(PTR) container_of((PTR), struct region, list)
        struct slist_ref list;
};

static struct kmm_cache REGIONS_CACHE;
static struct vm_area *KDEV_AREA = NULL;

void dev_init(void)
{
        kmm_cache_init(&REGIONS_CACHE, "dev_regions", sizeof(struct region), 0, 0, NULL, NULL);
}

static struct region *new_region(struct vm_area *area, size_t len)
{
        SLIST_FOREACH (it, area->data) {
                struct region *r = region_of(it);

                if (r->resource.state != REGION_RES_STATE_FREE || r->length < len) {
                        continue;
                }

                if (r->length > len) {
                        /* Create a new region that will act as a new shrinked "free" region. */
                        struct region *new = kmm_cache_alloc(&REGIONS_CACHE);
                        if (__unlikely(NULL == new)) {
                                return (NULL);
                        }

                        new->page_vaddr = (void *)((uintptr_t)r->page_vaddr + len);
                        new->length = r->length - len;
                        slist_init(&new->list);
                        slist_insert(&r->list, &new->list);
                        new->resource.state = REGION_RES_STATE_FREE;

                        r->length = len;
                }

                r->resource.state = REGION_RES_STATE_UNKNOWN;

                return (r);
        }

        return (NULL);
}

static void free_region_for(struct vm_area *area, struct resource *res, void **region_base, size_t *region_len)
{
        struct region *last = NULL;
        SLIST_FOREACH (it, area->data) {
                struct region *r = region_of(it);

                /* Three possibilities:
                 * 1. A region is among other occupied regions; simply mark it as free.
                 * 2. Either region before or after the current region is free; coalesce them.
                 * 3. Both regions, before and after, are free; coalesce them all. */
                if (r->resource.ptr == res) {
                        r->resource.state = REGION_RES_STATE_FREE;

                        if (REGION_RES_STATE_FREE == last->resource.state) {
                                last->length += r->length;
                                slist_remove_next(&last->list);
                                kmm_cache_free(&REGIONS_CACHE, r);
                                r = last;
                        }

                        *region_base = r->page_vaddr;
                        *region_len = r->length;

                        struct slist_ref *next_ref = slist_next(&r->list);
                        if (NULL != next_ref) {
                                struct region *next = region_of(next_ref);

                                if (REGION_RES_STATE_FREE == next->resource.state) {
                                        r->length += next->length;
                                        slist_remove_next(&r->list);
                                        kmm_cache_free(&REGIONS_CACHE, r);
                                }
                        }

                        return;
                }

                last = r;
        }

        LOGF_P("Tried to free resource mapping %s:%s, but the page isn't mapped\n", res->device_id,
               res->resource_id);
}

/* Receives struct resource as an argument. */
static void *register_resource(struct vm_area *area, void *arg)
{
        struct resource *res = arg;
        if (__unlikely(RESOURCE_TYPE_DEV_BUFFER != res->type)) {
                LOGF_E("Tried to register a resource of a wrong type in a dev area\n");
                return (NULL);
        }

        uintptr_t const res_start = (uintptr_t)res->data.dev_buffer.base;
        uintptr_t const res_end = res_start + res->data.dev_buffer.len - 1;

        size_t const pages = (align_roundup(res_end, PLATFORM_PAGE_SIZE) -
                              align_rounddown(res_start, PLATFORM_PAGE_SIZE)) /
                             PLATFORM_PAGE_SIZE;

        struct region *new = new_region(area, pages * PLATFORM_PAGE_SIZE);

        new->resource.ptr = res;

        uintptr_t const pbase = align_rounddown(res_start, PLATFORM_PAGE_SIZE);
        uintptr_t const vbase = align_rounddown((uintptr_t) new->page_vaddr, PLATFORM_PAGE_SIZE);
        for (size_t i = 0; i < pages; i++) {
                uintptr_t const phys_addr = pbase + i * PLATFORM_PAGE_SIZE;
                uintptr_t const virt_addr = vbase + i * PLATFORM_PAGE_SIZE;

                vm_arch_pt_map(area->owner->root_dir, (void *)phys_addr, (void *)virt_addr,
                               area->flags);
        }

        return (new->page_vaddr);
}

static void unregister_resource(struct vm_area *area, void *arg)
{
        struct resource *res = arg;

        void *reg_base = NULL;
        size_t reg_len = 0;
        free_region_for(area, res, &reg_base, &reg_len);

        size_t const pages = div_ceil(reg_len, PLATFORM_PAGE_SIZE);
        for (size_t i = 0; i < pages; i++) {
                void *vaddr = (void*)((uintptr_t)reg_base + i * PLATFORM_PAGE_SIZE);

                vm_arch_pt_unmap(area->owner->root_dir, vaddr);
        }
}

static struct vm_area_ops DEV_AREA_OPS = {
        .handle_pg_fault = vm_pgfault_handle_panic,
        .register_map = register_resource,
        .unregister_map = unregister_resource,
};

struct vm_area *dev_area_new(struct vm_space *owner, size_t min_len)
{
        struct vm_area *area = vm_new_area_within_space(owner, min_len, CONF_DEV_MAX_AREA_SIZE);
        if (__unlikely(NULL == area)) {
                return (NULL);
        }

        area->flags |= DEV_AREA_FLAGS;
        area->ops = DEV_AREA_OPS;
        struct region *m = kmm_cache_alloc(&REGIONS_CACHE);
        if (__unlikely(NULL == m)) {
                return (NULL);
        }

        slist_init(&m->list);
        m->page_vaddr = area->base;
        m->length = area->length;
        m->resource.state = REGION_RES_STATE_FREE;

        /* Data contains an address of the first region. */
        area->data = &m->list;

        return (area);
}

void kdev_init(struct vm_space *kernel_space)
{
        kassert(NULL != kernel_space);
        KDEV_AREA = dev_area_new(kernel_space, CONF_DEV_MAX_AREA_SIZE);
        kassert(NULL != KDEV_AREA);
}

void *kdev_map_resource(struct resource *res)
{
        kassert(NULL != res);

        return (vm_area_register_map(KDEV_AREA, res));
}

void kdev_unmap_resource(void *page_vaddr)
{
        return (vm_area_unregister_map(KDEV_AREA, page_vaddr));
}
