#include "kernel/mm/kheap.h"

#include "kernel/klog.h"
#include "kernel/mm/kmm.h"
#include "kernel/mm/mm.h"
#include "kernel/mm/vm.h"
#include "kernel/platform_consts.h"

#include "lib/align.h"
#include "lib/cppdefs.h"
#include "lib/mm/buddy.h"
#include "lib/mm/linear.h"
#include "lib/utils.h"

#include <stdbool.h>
#include <stddef.h>

static struct vm_area KHEAP_AREA = { 0 };

struct find_largest_data {
        virt_addr_t max_base;
        size_t max_length;
};

static bool find_largest(void *base, size_t len, void *data)
{
        struct find_largest_data *d = data;

        if (len > d->max_length) {
                d->max_base = base;
                d->max_length = len;
        }

        return (false);
}

struct vm_area_heap_data {
        struct buddy_manager buddy;
        struct linear_alloc buddy_alloc;
} KHEAP_DATA;

static bool area_page_ndx(struct vm_area *area, void *addr, size_t *result)
{
        uintptr_t const wanted = (uintptr_t)addr;

        uintptr_t const area_start = (uintptr_t)area->base;
        uintptr_t const area_end = area_start + area->length - 1;

        if (wanted < area_start || wanted > area_end) {
                return (false);
        }

        *result = (wanted - area_start) / PLATFORM_PAGE_SIZE;
        return (true);
}

static bool is_registered_page(struct vm_area *area, virt_addr_t page)
{
        kassert(area != NULL);

        struct vm_area_heap_data *data = area->data;
        kassert(data != NULL);

        size_t pg_ndx = 0;
        if (!area_page_ndx(area, page, &pg_ndx)) {
                return (false);
        }
        return (buddy_is_free(&data->buddy, pg_ndx));
}

static void vmarea_heap_fault_handler(struct vm_area *area, void *fault_addr)
{
        void *const page_addr =
                (void *)(align_rounddown((uintptr_t)fault_addr, PLATFORM_PAGE_SIZE));

        if (is_registered_page(area, page_addr)) {
                /* The page isn't registered in the heap.
                 * So it's true page fault. */
                vm_pgfault_handle_default(area, fault_addr);
                return;
        }

        /* The page is registered in the heap, but mappings are missing.
         * For now, it means that we haven't allocated the page yet. */
        struct mm_page *page = mm_alloc_page();
        if (__unlikely(page == NULL)) {
                LOGF_W("Not enough physicall space. Trying to trim some caches.\n");
                kmm_cache_trim_all();
                page = mm_alloc_page();
                if (__unlikely(page == NULL)) {
                        LOGF_P("Couldn't trim enough space. Bye.\n");
                }
        }
        vm_arch_pt_map(area->owner->root_dir, page->paddr, page_addr, area->flags);
}

static void *heap_register_page(struct vm_area *area, void *page_addr)
{
        kassert(area != NULL);

        struct vm_area_heap_data *data = area->data;
        uintptr_t const area_start = (uintptr_t)area->base;

        size_t page_ndx = 0;
        if (page_addr != NULL && !area_page_ndx(area, page_addr, &page_ndx)) {
                return (NULL);
        }

        bool alloc_failed = false;
        if (page_addr != NULL) {
                alloc_failed = !buddy_try_alloc(&data->buddy, 0, page_ndx);
        } else {
                alloc_failed = !buddy_alloc(&data->buddy, 0, &page_ndx);
        }

        if (alloc_failed) {
                return (NULL);
        }

        void *const result_addr = (void *)(area_start + page_ndx * PLATFORM_PAGE_SIZE);
        return (result_addr);
}

static void heap_unregister_page(struct vm_area *area, void *page_addr)
{
        kassert(area != NULL);

        struct vm_area_heap_data *data = area->data;

        size_t page_ndx = 0;
        if (!area_page_ndx(area, page_addr, &page_ndx)) {
                LOGF_P("Tried unregister page from an area that is not belong to it.\n");
        }

        buddy_free(&data->buddy, page_ndx, 0);
}

static void register_invalid_pages(void const *addr, size_t len, void *data __unused)
{
        uintptr_t const inv_start = (uintptr_t)addr;
        uintptr_t const inv_end = inv_start + len - 1;

        uintptr_t const area_start = (uintptr_t)KHEAP_AREA.base;
        uintptr_t const area_end = area_start + KHEAP_AREA.length - 1;

        if ((inv_end < area_start) || (inv_start > area_end)) {
                /* There is no overlap. */
                return;
        }

        uintptr_t const overlap_start = (MAX(area_start, inv_start));
        uintptr_t const overlap_end = MIN(area_end, inv_end);
        size_t const overlap_len = overlap_end - overlap_start + 1;

        size_t page_ndx = 0;
        if (__unlikely(!area_page_ndx(&KHEAP_AREA, (void *)overlap_start, &page_ndx))) {
                LOGF_P("There is a bug somewhere. We can't possibly be here.");
        }

        size_t const pages = div_ceil(overlap_len, PLATFORM_PAGE_SIZE);

        for (size_t i = 0; i < pages; i++) {
                if (!buddy_try_alloc(&KHEAP_DATA.buddy, 0, page_ndx + i)) {
                        LOGF_P("Couldn't reserve invalid pages!\n");
                }
        }
}

void kheap_init(struct vm_space *space)
{
        struct find_largest_data fld = { 0 };
        vm_space_find_gap(space, find_largest, &fld);
        /* NOTE: An area describes properties of the *pages*.
         * So it makes 0 sense to not align it at page boundaries. */
        char *const start = (void *)align_roundup((uintptr_t)fld.max_base, PLATFORM_PAGE_SIZE);
        const size_t len = fld.max_length - ((uintptr_t)start - (uintptr_t)fld.max_base);
        vm_area_init(&KHEAP_AREA, start, len, space);

        KHEAP_AREA.ops.handle_pg_fault = vmarea_heap_fault_handler;
        KHEAP_AREA.ops.register_page = heap_register_page;
        KHEAP_AREA.ops.unregister_page = heap_unregister_page;
        KHEAP_AREA.data = &KHEAP_DATA;
        KHEAP_AREA.flags |= VM_WRITE;

        vm_space_append_area(space, &KHEAP_AREA);

        const size_t heap_pages = len / PLATFORM_PAGE_SIZE;
        const size_t req_space = buddy_predict_req_space(heap_pages);
        const size_t req_pages = div_ceil(req_space, PLATFORM_PAGE_SIZE);

        /* Map first pages by hands to allow kernel heap to start. */
        for (size_t i = 0; i < req_pages; i++) {
                uintptr_t map_addr = (uintptr_t)KHEAP_AREA.base;
                map_addr += i * PLATFORM_PAGE_SIZE;

                struct mm_page *p = mm_alloc_page();
                kassert(p != NULL);

                p->state = PAGESTATE_FIXED;
                vm_arch_pt_map(KHEAP_AREA.owner->root_dir, p->paddr, (void *)map_addr,
                               KHEAP_AREA.flags);
        }

        linear_alloc_init(&KHEAP_DATA.buddy_alloc, KHEAP_AREA.base, req_pages * PLATFORM_PAGE_SIZE);
        buddy_init(&KHEAP_DATA.buddy, heap_pages, &KHEAP_DATA.buddy_alloc);
        linear_forbid_further_alloc(&KHEAP_DATA.buddy_alloc);

        for (size_t i = 0; i < req_pages; i++) {
                bool failed = !buddy_try_alloc(&KHEAP_DATA.buddy, 0, i);
                if (__unlikely(failed)) {
                        LOGF_P("Couldn't reserve a page!\n");
                }
        }

        /* The heap area usually takes really big space for itself.
         * Because of this, there is a big chance that this space will contain regions
         * that we can't use. So we mark them as "allocated" beforehand. */
        vm_arch_iter_reserved_vaddresses(register_invalid_pages, NULL);
}

void *kheap_alloc_page(void)
{
        return (vm_area_register_page(&KHEAP_AREA, NULL));
}

void kheap_free_page(void *page)
{
        vm_area_unregister_page(&KHEAP_AREA, page);
}
