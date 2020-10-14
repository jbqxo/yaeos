#ifndef _KERNEL_MM_POOL_H
#define _KERNEL_MM_POOL_H

#include "kernel/cppdefs.h"
#include "kernel/ds/slist.h"

#include <stddef.h>

struct mem_pool {
        SLIST_HEAD(, union node) list;

#ifndef NDEBUG
        union uiptr mem_start;
        union uiptr mem_end;
#endif
};

void mem_pool_init(struct mem_pool *, void *mem, size_t mem_size, size_t elem_size,
                   size_t elem_align);
void *mem_pool_alloc(struct mem_pool *);
void mem_pool_free(struct mem_pool *, void *);

#endif // _KERNEL_MM_POOL_H
