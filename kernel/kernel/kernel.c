#include "kernel/kernel.h"

#include "kernel/config.h"
#include "kernel/console.h"
#include "kernel/klog.h"
#include "kernel/mm/addr.h"
#include "kernel/mm/kheap.h"
#include "kernel/mm/kmalloc.h"
#include "kernel/mm/kmm.h"
#include "kernel/mm/mm.h"
#include "kernel/mm/vm.h"
#include "kernel/modules.h"
#include "kernel/resources.h"

#include "lib/align.h"
#include "lib/cppdefs.h"
#include "lib/cstd/string.h"
#include "lib/utils.h"

#include <limits.h>

struct vm_area KERNELBIN_AREAS[KSEGMENT_COUNT] = { 0 };
struct vm_space CURRENT_KERNEL = { 0 };
struct vm_space *CURRENT_USER = NULL;

static void init_kernel_vmspace(void)
{
        vm_space_init(&CURRENT_KERNEL, vm_arch_get_early_pgroot(), addr_get_offset());

        for (size_t i = 0; i < ARRAY_SIZE(KERNELBIN_AREAS); i++) {
                struct vm_area *a = &KERNELBIN_AREAS[i];
                /* TODO: Reimplement init of segments areas. */
                enum kernel_segments const seg = (enum kernel_segments)i;

                uintptr_t start = 0;
                uintptr_t end = 0;
                kernel_arch_get_segment(seg, (void **)&start, (void **)&end);

                /* Every segment must start and end at page boundaries. */
                end = align_roundup(end, PLATFORM_PAGE_SIZE);
                const size_t len = end - start;

                kassert(check_align(start, PLATFORM_PAGE_SIZE));
                kassert(check_align(len, PLATFORM_PAGE_SIZE));

                vm_area_init(a, (void *)start, len, &CURRENT_KERNEL);
                a->ops.handle_pg_fault = vm_pgfault_handle_panic;
                vm_space_append_area(&CURRENT_KERNEL, a);
        }
}

static void register_zone_for_mem(void *base, size_t len)
{
        uintptr_t chunk_base = (uintptr_t)base;
        uintptr_t chunk_end = chunk_base + len;
        chunk_base = align_roundup(chunk_base, PLATFORM_PAGE_SIZE);
        chunk_end = align_rounddown(chunk_end, PLATFORM_PAGE_SIZE);

        const size_t chunk_len = chunk_end - chunk_base;
        mm_zone_create((void *)chunk_base, chunk_len, &CURRENT_KERNEL);
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
        kassert(len <= INT_MAX);
        return ((int)len);
}

__noreturn static void test_allocation(void)
{
        size_t overall = 0;
        for (int i = 0;; i++) {
                void *mem = kmalloc(0x700);
                if (mem != NULL) {
                        overall += 0x700;
                        kmemset(mem, 0x0, 0x700);
                        LOGF_I("Allocation #%d. Overall allocated %lx bytes\n", i, overall);
                } else {
                        LOGF_P("Failed to allocate #%d\n", i);
                }
        }
}

__noreturn void kernel_init(void)
{
        consoles_init();
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
