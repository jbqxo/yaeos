#include "kernel/mm/vm.h"

#include "kernel/kernel.h"
#include "kernel/klog.h"
#include "kernel/mm/addr.h"
#include "kernel/mm/kheap.h"
#include "kernel/mm/kmm.h"
#include "kernel/mm/vm_area.h"
#include "kernel/mm/vm_space.h"

#include "lib/align.h"
#include "lib/cppdefs.h"
#include "lib/cstd/string.h"
#include "lib/ds/rbtree.h"
#include "lib/ds/slist.h"

static struct kmm_cache AREAS_CACHE = { 0 };

void vm_pgfault_handle_panic(struct vm_area *area, void *addr)
{
        LOGF_P("Page fault at the address %p inside area %p-%p!\n", addr, area->base,
               (void *)((uintptr_t)area->base + area->length - 1));
}

void vm_init(void)
{
        kmm_cache_init(&AREAS_CACHE, "areas", sizeof(struct vm_area), 0, 0, NULL, NULL);
}

static bool find_atleast(void *base __unused, size_t len, void *data)
{
        size_t const min_len = (size_t)data;
        return (len >= min_len);
}

struct vm_area *vm_new_area_within_space(struct vm_space *space, size_t const min_size,
                                         size_t const max_size)
{
        kassert(space != NULL);
        kassert(min_size <= max_size);

        kassert(sizeof(min_size) == sizeof(void *));
        size_t gap_len = 0;
        virt_addr_t gap_base = vm_space_find_gap(space, &gap_len, find_atleast, (void *)min_size);

        size_t const occupy_len = MIN(max_size, gap_len);

        struct vm_area *new_area = kmm_cache_alloc(&AREAS_CACHE);
        if (__unlikely(new_area == NULL)) {
                return (NULL);
        }

        vm_area_init(new_area, gap_base, occupy_len, space);
        vm_space_append_area(space, new_area);

        return (new_area);
}

void vm_free_area(struct vm_area *area)
{
        kassert(area != NULL);

        vm_space_remove_area(area->owner, area);
        kmm_cache_free(&AREAS_CACHE, area);
}
