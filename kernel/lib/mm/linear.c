#include "lib/mm/linear.h"

#include "lib/align.h"
#include "lib/cppdefs.h"
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
                return (NULL);
        }

        a->position = pos.num + len;

        return (pos.ptr);
}

void *linear_alloc_alloc_aligned(struct linear_alloc *a, size_t const len, size_t const align)
{
        kassert(a != NULL);
        /* Just use linear_alloc_alloc(). */
        kassert(align > 0);

        union uiptr pos = uint2uiptr(a->position);
        pos.num = align_roundup(pos.num, align);

        if (__unlikely(pos.num + len >= a->limit)) {
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
