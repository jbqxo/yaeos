#include "kernel/kernel.h"

#include "kernel/config.h"
#include "kernel/console.h"
#include "kernel/klog.h"
#include "kernel/mm/highmem.h"
#include "kernel/mm/kheap.h"
#include "kernel/mm/kmalloc.h"
#include "kernel/mm/kmm.h"
#include "kernel/mm/mm.h"
#include "kernel/mm/vm.h"
#include "kernel/modules.h"
#include "kernel/resources.h"

#include "lib/align.h"
#include "lib/cppdefs.h"
#include "lib/cstd/nonstd.h"
#include "lib/cstd/string.h"

struct vm_area KERNELBIN_AREAS[KSEGMENT_COUNT] = { 0 };
struct vm_space CURRENT_KERNEL = { 0 };
struct vm_space *CURRENT_USER = NULL;

static void init_kernel_vmspace(void)
{
        vm_space_init(&CURRENT_KERNEL, vm_arch_get_early_pgroot(), ptr2uiptr(highmem_get_offset()));

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

static void register_zone_for_mem(void *base, size_t len)
{
        union uiptr chunk_base = ptr2uiptr(base);
        union uiptr chunk_end = uint2uiptr(chunk_base.num + len);
        chunk_base.num = align_roundup(chunk_base.num, PLATFORM_PAGE_SIZE);
        chunk_end.num = align_rounddown(chunk_end.num, PLATFORM_PAGE_SIZE);

        const size_t chunk_len = chunk_end.num - chunk_base.num;
        struct mm_zone *zone = mm_zone_create(chunk_base.ptr, chunk_len, &CURRENT_KERNEL);
        mm_zone_register(zone);
}

static void register_mem_zones(struct resource *r)
{
        if (r->type != RESOURCE_TYPE_MEMORY) {
                return;
        }

        register_zone_for_mem(r->data.mem_reg.base, r->data.mem_reg.len);
}

static int conwrite(const char *msg, size_t len)
{
        console_write(msg, len);
        return (len);
}

static void test_allocation(void)
{
        size_t overall = 0;
        for (int i = 0;; i++) {
                void *mem = kmalloc(0x700);
                if (mem != NULL) {
                        overall += 0x700;
                        kmemset(mem, 0x0, 0x700);
                        LOGF_I("Allocation #%d. Overall allocated %x bytes\n", i, overall);
                } else {
                        LOGF_P("Failed to allocate #%d\n", i);
                }
        }
}

void kernel_init(void)
{
        console_init();
        assertion_init(conwrite);
        LOGF_I("Platform Layer is... Up and running\n");

        init_kernel_vmspace();
        mm_init();
        resources_iter(register_mem_zones);
        kheap_init(&CURRENT_KERNEL);

        kmm_init(kheap_alloc_page, kheap_free_page);
        kmalloc_init(CONF_MALLOC_MIN_POW, CONF_MALLOC_MAX_POW);
        LOGF_I("Kernel Memory Manager is... Up and running\n");

        modules_init();
        modules_load_available();

        test_allocation();
}
