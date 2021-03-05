#ifndef _LIB_MM_POOL_H
#define _LIB_MM_POOL_H

#include "lib/cppdefs.h"
#include "lib/ds/slist.h"

#include <stddef.h>

struct mem_pool {
        struct slist_ref nodes;

#ifndef NDEBUG
        void *mem_start;
        void *mem_end;
#endif
};

void mem_pool_init(struct mem_pool *, void *mem, size_t mem_size, size_t elem_size,
                   size_t elem_align);
void *mem_pool_alloc(struct mem_pool *);
void mem_pool_free(struct mem_pool *, void *);

#endif /* _LIB_MM_POOL_H */
