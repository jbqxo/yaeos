#include "kernel/kernel.h"

#include "kernel/config.h"
#include "kernel/console.h"
#include "kernel/klog.h"
#include "kernel/mm/discover.h"
#include "kernel/mm/highmem.h"
#include "kernel/mm/kheap.h"
#include "kernel/mm/kmalloc.h"
#include "kernel/mm/kmm.h"
#include "kernel/mm/mm.h"
#include "kernel/mm/vm.h"

#include "lib/align.h"
#include "lib/cppdefs.h"
#include "lib/cstd/nonstd.h"
#include "lib/cstd/string.h"

struct vm_area KERNELBIN_AREAS[KSEGMENT_COUNT] = { 0 };
struct vm_space CURRENT_KERNEL = { 0 };
struct vm_space *CURRENT_USER = NULL;

static void init_kernel_vmspace(void)
{
        vm_space_init(&CURRENT_KERNEL, vm_arch_get_early_pgroot(),
                      ptr2uiptr(highmem_get_offset()));

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
        int chunks_count = mm_discover_chunks_len();

        struct mem_chunk *chunks = __builtin_alloca(chunks_count * sizeof(*chunks));
        mm_discover_get_chunks(chunks);

        for (int i = 0; i < chunks_count; i++) {
                struct mem_chunk *chunk = &chunks[i];
                /* All invalid chunks shoudl've been excluded by mm_arch_get_chunks. */
                kassert(chunk->type == MEM_TYPE_AVAIL);
                create_zone_from_chunk(chunk);
        }
}

static int conwrite(const char *msg, size_t len)
{
        console_write(msg, len);
        return (len);
}


void kernel_init()
{
        console_init();
        assertion_init(conwrite);
        LOGF_I("Platform Layer is... Up and running\n");

        init_kernel_vmspace();
        mm_init();
        create_mem_zones();
        kheap_init(&CURRENT_KERNEL);

        kmm_init(kheap_alloc_page, kheap_free_page);
        kmalloc_init(CONF_MALLOC_MIN_POW, CONF_MALLOC_MAX_POW);
        LOGF_I("Kernel Memory Manager is... Up and running\n");

        LOGF_I("Trying to allocate a bit of memory...\n");
        for (int i = 0;; i++) {
                void *mem = kmalloc(0x700);
                if (mem != NULL) {
                        kmemset(mem, 0x0, 0x700);
                        LOGF_I("Success! Allocated %p at P%p\n", mem, vm_arch_get_phys_page(mem));
                } else {
                        LOGF_P("Failed to allocate #%d\n", i);
                }
        }
}
