#include "kernel/mm/linear.h"
#include "kernel/utils.h"

#include "lib/assert.h"


void linear_alloc_init(struct linear_alloc *a, void *mem, size_t len)
{
        kassert(a != NULL);
        kassert(mem != NULL);
        kassert(len > 0);

        union uiptr p = ptr2uiptr(mem);
        a->base = p.num;
        a->limit = a->base + len;
        a->position = a->base;

        kmemset(p.ptr, 0x00, len);
}

void *linear_alloc_alloc(struct linear_alloc *a, size_t len)
{
        kassert(a != NULL);

        union uiptr pos = num2uiptr(a->position);

        pos.num += len;
        kassert(pos.num < a->limit);

        a->position = pos.num;

        return (pos.ptr);
}

void linear_alloc_free(struct linear_alloc *a, size_t len)
{
        kassert(a != NULL);

        uintptr_t new_pos = a->position - len;
        kassert(new_pos >= a->base);

        a->position = new_pos;
}

size_t linear_alloc_occupied_mem(struct linear_alloc *a)
{
        kassert(a != NULL);

        return (a->position - a->base);
}
