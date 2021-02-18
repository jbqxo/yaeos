#include "kernel/mm/linear.h"

#include "kernel/klog.h"
#include "kernel/utils.h"
#include "kernel/cppdefs.h"

#include "lib/cstd/assert.h"

void linear_alloc_init(struct linear_alloc *a, void *mem, size_t len)
{
        kassert(a != NULL);
        kassert(mem != NULL);
        kassert(len > 0);

        union uiptr p = ptr2uiptr(mem);
        a->base = p.num;
        a->limit = a->base + len;
        a->position = a->base;
}

void *linear_alloc_alloc(struct linear_alloc *a, size_t len)
{
        kassert(a != NULL);

        union uiptr pos = uint2uiptr(a->position);

        if (__unlikely(pos.num + len >= a->limit)) {
                LOGF_W("Allocation from a linear allocator failed; not enough memory.\n");
                return (NULL);
        }

        a->position = pos.num + len;

        return (pos.ptr);
}

void linear_alloc_free(struct linear_alloc *a, size_t len)
{
        kassert(a != NULL);

        uintptr_t new_pos = a->position - len;
        kassert(new_pos >= a->base);

        a->position = new_pos;
}

size_t linear_alloc_occupied(struct linear_alloc *a)
{
        kassert(a != NULL);

        return (a->position - a->base);
}

void linear_forbid_further_alloc(struct linear_alloc *alloc)
{
        kassert(alloc != NULL);

        alloc->limit = alloc->position;
}

void linear_alloc_used_mem_range(struct linear_alloc *alloc, void **start, void **end)
{
        kassert(alloc != NULL);
        kassert(start != NULL);
        kassert(end != NULL);

        const union uiptr mem_start = uint2uiptr(alloc->base);
        const union uiptr mem_end = uint2uiptr(alloc->position);

        *start = mem_start.ptr;
        *end = mem_end.ptr;
}
