#include "kernel/kernel.h"

#include "kernel/config.h"
#include "kernel/console.h"
#include "kernel/klog.h"
#include "kernel/mm/kmm.h"
#include "kernel/mm/mm.h"
#include "kernel/mm/vmm.h"

#include "lib/align.h"
#include "lib/cppdefs.h"
#include "lib/cstd/nonstd.h"

struct vm_area KERNELBIN_AREAS[KSEGMENT_COUNT] = { 0 };
struct vm_area KHEAP_AREA = { 0 };
struct vm_space CURRENT_KERNEL = { 0 };
struct vm_space *CURRENT_USER = NULL;

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

static void vmarea_heap_fault_handler(struct vm_area *area, void *fault_addr)
{
        union uiptr const page_addr =
                uint2uiptr(align_rounddown(ptr2uint(fault_addr), PLATFORM_PAGE_SIZE));
        struct vm_area_heap_data *data = area->data;
        kassert(data != NULL);

        size_t const area_pg_ndx =
                (page_addr.num - ptr2uint(area->base_vaddr)) / PLATFORM_PAGE_SIZE;
        if (buddy_is_free(&data->buddy, area_pg_ndx)) {
                /* The page isn't registered in the heap.
                 * So it's true page fault. */
                vm_pgfault_handle_default(area, fault_addr);
                return;
        }

        /* The page is registered in the heap, but mappings are missing.
         * For now, it means that we haven't allocated the page yet. */
        struct mm_page *page = mm_alloc_page();
        vm_arch_ptree_map(area->owner->root_dir, page->paddr, page_addr.ptr, area->flags);
}

static void *heap_register_page(struct vm_area *area, void *page_addr)
{
        kassert(area != NULL);

        struct vm_area_heap_data *data = area->data;
        const union uiptr area_start = ptr2uiptr(area->base_vaddr);
        const union uiptr area_end = uint2uiptr(area_start.num + area->length - 1);
        const union uiptr desired_addr = ptr2uiptr(page_addr);

        /* Check that address falls in the heap. */
        kassert(desired_addr.ptr == NULL ||
                desired_addr.num >= area_start.num && desired_addr.num < area_end.num);

        bool alloc_failed = false;
        uint32_t page_ndx = (desired_addr.num - area_start.num) / PLATFORM_PAGE_SIZE;
        if (desired_addr.ptr != NULL) {
                alloc_failed = !buddy_try_alloc(&data->buddy, page_ndx);
        } else {
                alloc_failed = !buddy_alloc(&data->buddy, 0, &page_ndx);
        }

        if (alloc_failed) {
                return (NULL);
        }

        const union uiptr result_addr = uint2uiptr(area_start.num + page_ndx * PLATFORM_PAGE_SIZE);
        return (result_addr.ptr);
}

static void init_kernel_heap_area(struct vm_area *heap_area, struct vm_area_heap_data *data)
{
        struct find_largest_data fld = { 0 };
        vm_space_find_gap(&CURRENT_KERNEL, find_largest, &fld);
        /* NOTE: An area describes properties of the *pages*.
         * So it makes 0 sense to not align it at page boundaries. */
        const union uiptr area_start =
                uint2uiptr(align_roundup(fld.max_base.num, PLATFORM_PAGE_SIZE));
        const size_t area_len = fld.max_length - (area_start.num - fld.max_base.num);
        vm_area_init(heap_area, area_start.ptr, area_len, &CURRENT_KERNEL);

        heap_area->ops.handle_pg_fault = vmarea_heap_fault_handler;
        heap_area->ops.register_page = heap_register_page;
        heap_area->data = data;
        heap_area->flags |= VM_WRITE;

        vm_space_append_area(&CURRENT_KERNEL, heap_area);

        const size_t heap_pages = area_len / PLATFORM_PAGE_SIZE;
        /* This is completely random number... */
        const size_t req_space = buddy_predict_req_space(heap_pages);
        const size_t req_pages = div_ceil(req_space, PLATFORM_PAGE_SIZE);

        /* Map first pages by hands to allow kernel heap to start. */
        for (int i = 0; i < req_pages; i++) {
                union uiptr map_addr = ptr2uiptr(heap_area->base_vaddr);
                map_addr.num += i * PLATFORM_PAGE_SIZE;

                struct mm_page *p = mm_alloc_page();
                p->state = PAGESTATE_FIXED;
                vm_arch_ptree_map(heap_area->owner->root_dir, p->paddr, map_addr.ptr,
                                  heap_area->flags);
        }

        linear_alloc_init(&data->buddy_alloc, heap_area->base_vaddr,
                          req_pages * PLATFORM_PAGE_SIZE);
        buddy_init(&data->buddy, heap_pages, &data->buddy_alloc);
        linear_forbid_further_alloc(&data->buddy_alloc);

        for (size_t i = 0; i < req_pages; i++) {
                bool failed = !buddy_try_alloc(&data->buddy, i);
                if (__unlikely(failed)) {
                        LOGF_P("Couldn't reserve a page!\n");
                }
        }
}

static void init_kernel_vmspace(void)
{
        vm_space_init(&CURRENT_KERNEL, kernel_arch_get_early_pg_root(),
                      ptr2uiptr(kernel_arch_vm_offset()));

        for (int i = 0; i < ARRAY_SIZE(KERNELBIN_AREAS); i++) {
                struct vm_area *a = &KERNELBIN_AREAS[i];

                union uiptr start = ptr2uiptr(NULL);
                union uiptr end = ptr2uiptr(NULL);
                kernel_arch_get_segment(i, &start.ptr, &end.ptr);

                /* Every segment must start and end at page boundaries. */
                end.num = align_roundup(end.num, PLATFORM_PAGE_SIZE);
                const size_t len = end.num - start.num;

                kassert(check_align(start.num, PLATFORM_PAGE_SIZE));
                kassert(check_align(len, PLATFORM_PAGE_SIZE));

                vm_area_init(a, start.ptr, len, &CURRENT_KERNEL);
                a->ops.handle_pg_fault = vm_pgfault_handle_default;
                vm_space_append_area(&CURRENT_KERNEL, a);
        }
}

static struct mm_zone *create_zone_from_chunk(struct mem_chunk *chunk)
{
        union uiptr chunk_base = ptr2uiptr(chunk->mem);
        union uiptr chunk_end = uint2uiptr(chunk_base.num + chunk->length);
        chunk_base.num = align_roundup(chunk_base.num, PLATFORM_PAGE_SIZE);
        chunk_end.num = align_rounddown(chunk_end.num, PLATFORM_PAGE_SIZE);

        const size_t chunk_len = chunk_end.num - chunk_base.num;
        struct mm_zone *zone = mm_zone_create(chunk_base.ptr, chunk_len, &CURRENT_KERNEL);
        mm_zone_register(zone);
        return (zone);
}

static void create_mem_zones(void)
{
        int chunks_count = mm_arch_chunks_len();

        struct mem_chunk *chunks = __builtin_alloca(chunks_count * sizeof(*chunks));
        mm_arch_get_chunks(chunks);

        for (int i = 0; i < chunks_count; i++) {
                struct mem_chunk *chunk = &chunks[i];
                /* All invalid chunks shoudl've been excluded by mm_arch_get_chunks. */
                kassert(chunk->type == MEM_TYPE_AVAIL);
                create_zone_from_chunk(chunk);
        }
}

void kernel_init()
{
        console_init();
        LOGF_I("Platform Layer is... Up and running\n");

        init_kernel_vmspace();
        mm_init();
        create_mem_zones();
        init_kernel_heap_area(&KHEAP_AREA, &KHEAP_DATA);

        kmm_init();
        kmm_init_kmalloc();
        LOGF_I("Kernel Memory Manager is... Up and running\n");

        LOGF_I("Trying to allocate a bit of memory...\n");
        void *mem = kmalloc(0x100);
        if (mem != NULL) {
                kmemset(mem, 0x0, 0x100);
                LOGF_I("Success! We now own writable memory at %p\n", mem);
        } else {
                LOGF_I("Sad :(\n");
        }
}
