#include "kernel/mm/kheap.h"

#include "kernel/klog.h"
#include "kernel/mm/kmm.h"
#include "kernel/mm/mm.h"
#include "kernel/mm/vm.h"
#include "kernel/platform_consts.h"

#include "lib/align.h"
#include "lib/cppdefs.h"
#include "lib/cstd/nonstd.h"
#include "lib/mm/buddy.h"
#include "lib/mm/linear.h"

#include <stdbool.h>
#include <stddef.h>

static struct vm_area KHEAP_AREA = { 0 };

struct find_largest_data {
        union uiptr max_base;
        size_t max_length;
};

static bool find_largest(void *base, size_t len, void *data)
{
        struct find_largest_data *d = data;

        if (len > d->max_length) {
                d->max_base.ptr = base;
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
        const union uiptr wanted = ptr2uiptr(addr);

        const union uiptr area_start = ptr2uiptr(area->base_vaddr);
        const union uiptr area_end = uint2uiptr(area_start.num + area->length - 1);

        if (wanted.num < area_start.num || wanted.num > area_end.num) {
                return (false);
        }

        *result = (ptr2uint(addr) - ptr2uint(area->base_vaddr)) / PLATFORM_PAGE_SIZE;
        return (true);
}

static bool is_registered_page(struct vm_area *area, union uiptr page_addr)
{
        kassert(area != NULL);

        struct vm_area_heap_data *data = area->data;
        kassert(data != NULL);

        size_t pg_ndx = 0;
        if (!area_page_ndx(area, page_addr.ptr, &pg_ndx)) {
                return (false);
        }
        return (buddy_is_free(&data->buddy, pg_ndx));
}

static void vmarea_heap_fault_handler(struct vm_area *area, void *fault_addr)
{
        union uiptr const page_addr =
                uint2uiptr(align_rounddown(ptr2uint(fault_addr), PLATFORM_PAGE_SIZE));

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
        vm_arch_pt_map(area->owner->root_dir, page->paddr, page_addr.ptr, area->flags);
}

static void *heap_register_page(struct vm_area *area, void *page_addr)
{
        kassert(area != NULL);

        struct vm_area_heap_data *data = area->data;
        const union uiptr area_start = ptr2uiptr(area->base_vaddr);
        const union uiptr desired_addr = ptr2uiptr(page_addr);

        size_t page_ndx = 0;
        if (page_addr != NULL && !area_page_ndx(area, page_addr, &page_ndx)) {
                return (NULL);
        }

        bool alloc_failed = false;
        if (desired_addr.ptr != NULL) {
                alloc_failed = !buddy_try_alloc(&data->buddy, 0, page_ndx);
        } else {
                alloc_failed = !buddy_alloc(&data->buddy, 0, &page_ndx);
        }

        if (alloc_failed) {
                return (NULL);
        }

        const union uiptr result_addr = uint2uiptr(area_start.num + page_ndx * PLATFORM_PAGE_SIZE);
        return (result_addr.ptr);
}

static void register_invalid_pages(void const *addr, size_t len, void *data __unused)
{
        union uiptr const inv_start = ptr2uiptr(addr);
        uintptr_t const inv_end = inv_start.num + len - 1;

        union uiptr const area_start = ptr2uiptr(KHEAP_AREA.base_vaddr);
        uintptr_t const area_end = area_start.num + KHEAP_AREA.length - 1;

        if ((inv_end < area_start.num) || (inv_start.num > area_end)) {
                /* There is no overlap. */
                return;
        }

        union uiptr const overlap_start = uint2uiptr(MAX(area_start.num, inv_start.num));
        uintptr_t const overlap_end = MIN(area_end, inv_end);
        size_t const overlap_len = overlap_end - overlap_start.num + 1;

        size_t page_ndx = 0;
        if (__unlikely(!area_page_ndx(&KHEAP_AREA, overlap_start.ptr, &page_ndx))) {
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
        const union uiptr start = uint2uiptr(align_roundup(fld.max_base.num, PLATFORM_PAGE_SIZE));
        const size_t len = fld.max_length - (start.num - fld.max_base.num);
        vm_area_init(&KHEAP_AREA, start.ptr, len, space);

        KHEAP_AREA.ops.handle_pg_fault = vmarea_heap_fault_handler;
        KHEAP_AREA.ops.register_page = heap_register_page;
        KHEAP_AREA.data = &KHEAP_DATA;
        KHEAP_AREA.flags |= VM_WRITE;

        vm_space_append_area(space, &KHEAP_AREA);

        const size_t heap_pages = len / PLATFORM_PAGE_SIZE;
        const size_t req_space = buddy_predict_req_space(heap_pages);
        const size_t req_pages = div_ceil(req_space, PLATFORM_PAGE_SIZE);

        /* Map first pages by hands to allow kernel heap to start. */
        for (int i = 0; i < req_pages; i++) {
                union uiptr map_addr = ptr2uiptr(KHEAP_AREA.base_vaddr);
                map_addr.num += i * PLATFORM_PAGE_SIZE;

                struct mm_page *p = mm_alloc_page();
                p->state = PAGESTATE_FIXED;
                vm_arch_pt_map(KHEAP_AREA.owner->root_dir, p->paddr, map_addr.ptr,
                               KHEAP_AREA.flags);
        }

        linear_alloc_init(&KHEAP_DATA.buddy_alloc, KHEAP_AREA.base_vaddr,
                          req_pages * PLATFORM_PAGE_SIZE);
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
        vm_area_register_page(&KHEAP_AREA, NULL);
        vm_area_unregister_page(&KHEAP_AREA, page);
}
