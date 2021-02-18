#include "kernel/mm/pool.h"

#include "kernel/cppdefs.h"
#include "kernel/ds/slist.h"
#include "kernel/utils.h"

#include "lib/cstd/assert.h"
#include "lib/cstd/nonstd.h"

#include <stddef.h>

union node {
        SLIST_FIELD(union node) list;
        char data[0];
};

void mem_pool_init(struct mem_pool *pool, void *mem, size_t mem_size, size_t elem_size,
                   size_t elem_align)
{
        kassert(pool);
        kassert(mem);
        kassert(mem_size >= elem_size);

        elem_size = MAX(elem_size, sizeof(union node));
        uintptr_t mblock = align_roundup(ptr2uint(mem), elem_align);
        size_t stride = align_roundup(elem_size, elem_align);
        size_t limit = ptr2uiptr(mem).num + mem_size;

        SLIST_INIT(&pool->list);
#ifndef NDEBUG
        pool->mem_start = ptr2uiptr(pool);
        pool->mem_end = uint2uiptr(limit);
#endif

        kassert(mblock + elem_size <= limit);
        SLIST_INSERT_HEAD(&pool->list, (union node *)uint2ptr(mblock), list);

        while (mblock + stride + elem_size <= limit) {
                union node *current = uint2ptr(mblock);
                union node *next = uint2uiptr(mblock + stride).ptr;
                mblock = ptr2uint(next);

                SLIST_INSERT_AFTER(current, next, list);
        }
}

void *mem_pool_alloc(struct mem_pool *pool)
{
        kassert(pool);

        union node *n = SLIST_FIRST(&pool->list);
        if (!n) {
                return (NULL);
        }

        SLIST_REMOVE(&pool->list, n, list);
        return (n);
}

void mem_pool_free(struct mem_pool *pool, void *mem)
{
        kassert(pool);
        kassert(mem);

        union uiptr m = ptr2uiptr(mem);
#ifndef NDEBUG
        kassert(m.num > pool->mem_start.num && m.num < pool->mem_end.num);
#endif

        SLIST_INSERT_HEAD(&pool->list, (union node *)m.ptr, list);
}
