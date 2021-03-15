#include "kernel/mm/kheap.h"

#include "kernel/config.h"
#include "kernel/klog.h"
#include "kernel/mm/kmalloc.h"
#include "kernel/mm/kmm.h"
#include "kernel/mm/mm.h"
#include "kernel/mm/vm.h"
#include "kernel/platform_consts.h"

#include "lib/align.h"
#include "lib/cppdefs.h"
#include "lib/cstd/string.h"
#include "lib/mm/buddy.h"
#include "lib/mm/linear.h"
#include "lib/utils.h"

#include <stdbool.h>
#include <stddef.h>

static struct kmm_cache CHUNK_DATA_CACHE;

static struct vm_space *VMSPACE = NULL;

static struct vm_area CHUNK_FIRST = { 0 };
struct chunk_data {
        struct slist_ref list;
        struct vm_area *owner;
        size_t free_space;

        struct buddy_manager buddy;
        struct linear_alloc buddy_alloc;
} FIRST_CHUNK_DATA;

struct {
        struct slist_ref head_list;
        size_t heap_free_space;
} GLOBAL_DATA;

#define HEAP_VM_FLAGS (VM_WRITE)
/* 1 page should be always available in case if we will need to allocate a new area,
 * but the cache doesn't contain free slabs. */
#define CHUNK_AREA_MIN_SPACE (2 * PLATFORM_PAGE_SIZE)

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

        struct chunk_data *data = area->data;
        kassert(data != NULL);

        size_t pg_ndx = 0;
        if (!area_page_ndx(area, page, &pg_ndx)) {
                return (false);
        }
        return (buddy_is_free(&data->buddy, pg_ndx));
}

static void chunk_pgfault_handler(struct vm_area *area, void *fault_addr)
{
        void *const page_addr = align_rounddownptr(fault_addr, PLATFORM_PAGE_SIZE);

        if (is_registered_page(area, page_addr)) {
                /* The page isn't registered in the chunk.
                 * So it's true page fault. */
                vm_pgfault_handle_panic(area, fault_addr);
                return;
        }

        /* The page is registered in the chunk, but mappings are missing.
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

static void *chunk_register_page(struct vm_area *chunk, void *page_addr)
{
        kassert(chunk != NULL);

        struct chunk_data *data = chunk->data;
        uintptr_t const area_start = (uintptr_t)chunk->base;

        size_t page_ndx = 0;
        if (page_addr != NULL && !area_page_ndx(chunk, page_addr, &page_ndx)) {
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

        data->free_space -= PLATFORM_PAGE_SIZE;

        void *const result_addr = (void *)(area_start + page_ndx * PLATFORM_PAGE_SIZE);
        return (result_addr);
}

static void chunk_unregister_page(struct vm_area *chunk, void *page_addr)
{
        kassert(chunk != NULL);

        struct chunk_data *data = chunk->data;

        size_t page_ndx = 0;
        if (!area_page_ndx(chunk, page_addr, &page_ndx)) {
                LOGF_P("Tried unregister page from an area that is not belong to it.\n");
        }
        data->free_space += PLATFORM_PAGE_SIZE;

        buddy_free(&data->buddy, page_ndx, 0);
        /* Return the page to MM. */
        void *phys_addr = vm_arch_resolve_phys_page(chunk->owner->root_dir, page_addr);
        mm_free_page(phys_addr);
}

static struct vm_area_ops HEAP_OPS = {
        .handle_pg_fault = chunk_pgfault_handler,
        .register_map = chunk_register_page,
        .unregister_map = chunk_unregister_page,
};

static void preregister_invalid_pages(void const *addr, size_t len, void *data)
{
        struct vm_area *area = data;
        struct chunk_data *area_data = area->data;

        uintptr_t const inv_start = (uintptr_t)addr;
        uintptr_t const inv_end = inv_start + len - 1;

        uintptr_t const area_start = (uintptr_t)area->base;
        uintptr_t const area_end = area_start + area->length - 1;

        if ((inv_end < area_start) || (inv_start > area_end)) {
                /* There is no overlap. */
                return;
        }

        uintptr_t const overlap_start = (MAX(area_start, inv_start));
        uintptr_t const overlap_end = MIN(area_end, inv_end);
        size_t const overlap_len = overlap_end - overlap_start + 1;

        size_t page_ndx = 0;
        if (__unlikely(!area_page_ndx(area, (void *)overlap_start, &page_ndx))) {
                LOGF_P("There is a bug somewhere. We can't possibly be here.");
        }

        size_t const pages = div_ceil(overlap_len, PLATFORM_PAGE_SIZE);

        for (size_t i = 0; i < pages; i++) {
                if (!buddy_try_alloc(&area_data->buddy, 0, page_ndx + i)) {
                        LOGF_P("Couldn't reserve invalid pages!\n");
                }
        }
}

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

static struct vm_area *init_first_chunk(struct vm_space *space)
{
        struct vm_area *chunk = &CHUNK_FIRST;
        struct chunk_data *data = &FIRST_CHUNK_DATA;

        struct find_largest_data fld = { 0 };
        size_t ignore __unused;
        vm_space_find_gap(space, &ignore, find_largest, &fld);

        char *const start = fld.max_base;
        const size_t len = MIN(fld.max_length, CONF_HEAP_MAX_CHUNK_SIZE);
        vm_area_init(chunk, start, len, space);

        chunk->ops = HEAP_OPS;
        chunk->flags |= HEAP_VM_FLAGS;
        chunk->data = data;

        vm_space_insert_area(space, chunk);

        size_t const chunk_pages = len / PLATFORM_PAGE_SIZE;
        size_t const req_space = buddy_predict_req_space(chunk_pages);
        size_t const req_pages = div_ceil(req_space, PLATFORM_PAGE_SIZE);

        slist_init(&data->list);
        data->free_space = (chunk_pages - req_pages) * PLATFORM_PAGE_SIZE;
        data->owner = chunk;

        /* Map first pages by hands to allow kernel heap to start. */
        for (size_t i = 0; i < req_pages; i++) {
                uintptr_t map_addr = (uintptr_t)chunk->base;
                map_addr += i * PLATFORM_PAGE_SIZE;

                struct mm_page *p = mm_alloc_page();
                kassert(p != NULL);

                p->state = PAGESTATE_FIXED;
                vm_arch_pt_map(chunk->owner->root_dir, p->paddr, (void *)map_addr, chunk->flags);
        }

        linear_alloc_init(&data->buddy_alloc, chunk->base, req_pages * PLATFORM_PAGE_SIZE);
        buddy_init(&data->buddy, chunk_pages, &data->buddy_alloc);
        linear_forbid_further_alloc(&data->buddy_alloc);


        for (size_t i = 0; i < req_pages; i++) {
                bool failed = !buddy_try_alloc(&data->buddy, 0, i);
                if (__unlikely(failed)) {
                        LOGF_P("Couldn't reserve a page!\n");
                }
        }

        /* The heap area usually takes really big space for itself.
         * Because of this, there is a big chance that this space will contain regions
         * that we can't use. So we mark them as "allocated" beforehand. */
        vm_arch_iter_reserved_vaddresses(preregister_invalid_pages, chunk);

        return (chunk);
}

static struct vm_area *create_new_chunk(struct vm_space *space)
{
        size_t const chunk_minlen = CHUNK_AREA_MIN_SPACE;
        size_t const chunk_maxlen = CONF_HEAP_MAX_CHUNK_SIZE;
        struct vm_area *chunk = vm_new_area_within_space(space, chunk_minlen, chunk_maxlen);
        if (__unlikely(chunk == NULL)) {
                goto fail_return;
        }

        struct chunk_data *data = kmm_cache_alloc(&CHUNK_DATA_CACHE);
        if (__unlikely(data == NULL)) {
                goto free_area;
        }

        size_t const chunk_pages = chunk->length / PLATFORM_PAGE_SIZE;
        size_t const buddy_space = buddy_predict_req_space(chunk_pages);

        void *buddy_mem = kmalloc(buddy_space);
        if (__unlikely(buddy_mem == NULL)) {
                goto free_data;
        }

        kmemset(data, 0x0, sizeof(*data));

        linear_alloc_init(&data->buddy_alloc, buddy_mem, buddy_space);
        buddy_init(&data->buddy, chunk_pages, &data->buddy_alloc);
        linear_forbid_further_alloc(&data->buddy_alloc);

        data->free_space = chunk->length;
        data->owner = chunk;
        slist_init(&data->list);

        chunk->data = data;
        chunk->ops = HEAP_OPS;
        chunk->flags |= HEAP_VM_FLAGS;

        vm_arch_iter_reserved_vaddresses(preregister_invalid_pages, chunk);

        return (chunk);

free_data:
        kmm_cache_free(&CHUNK_DATA_CACHE, data);
free_area:
        vm_free_area(chunk);
fail_return:
        return (NULL);
}

static void append_new_chunk(struct vm_area *chunk)
{
        struct slist_ref *current = &GLOBAL_DATA.head_list;
        while (slist_next(current) != NULL) {
                struct slist_ref *next = slist_next(current);

                struct chunk_data *data = container_of(next, struct chunk_data, list);
                struct vm_area *next_ch = data->owner;

                if (chunk->base < next_ch->base) {
                        break;
                }
                current = next;
        }

        struct chunk_data *data = chunk->data;
        slist_insert(current, &data->list);
        GLOBAL_DATA.heap_free_space += data->free_space;
}

void kheap_init(struct vm_space *space)
{
        slist_init(&GLOBAL_DATA.head_list);
        kmm_cache_init(&CHUNK_DATA_CACHE, "heap_chunk_data", sizeof(struct chunk_data), 0, 0, NULL,
                       NULL);
        VMSPACE = space;

        struct vm_area *first = init_first_chunk(space);
        append_new_chunk(first);
}

void *kheap_alloc_page(void)
{
        struct vm_area *chunk = NULL;
        if (__unlikely(GLOBAL_DATA.heap_free_space == PLATFORM_PAGE_SIZE)) {
                chunk = create_new_chunk(VMSPACE);
                if (__unlikely(chunk == NULL)) {
                        LOGF_P("Couldn't create new heap chunk for some weird reason.\n");
                }
                append_new_chunk(chunk);
        } else {
                kassert(GLOBAL_DATA.heap_free_space > PLATFORM_PAGE_SIZE);

                SLIST_FOREACH(it, slist_next(&GLOBAL_DATA.head_list)) {
                        struct chunk_data *d = container_of(it, struct chunk_data, list);
                        if (d->free_space > 0) {
                                chunk = d->owner;
                                break;
                        }
                }
        }

        kassert(chunk != NULL);

        return (vm_area_register_map(chunk, NULL));
}

void kheap_free_page(void *page)
{
        struct vm_area *origin = NULL;
        SLIST_FOREACH(it, slist_next(&GLOBAL_DATA.head_list)) {
                struct chunk_data *d = container_of(it, struct chunk_data, list);

                if (d->owner->base <= page) {
                        origin = d->owner;
                        break;
                }
        }

        if (__unlikely(origin == NULL)) {
                LOGF_P("Coudln't find origin chunk for the page %p.\n", page);
        }

        /* Ensure that the page is related to the chunk. */
        kassert((uintptr_t)origin->base + origin->length - 1 > (uintptr_t)page);
        vm_area_unregister_map(origin, page);
}
