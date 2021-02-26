#include "lib/mm/pool.h"

#include "lib/align.h"
#include "lib/cppdefs.h"
#include "lib/cstd/assert.h"
#include "lib/cstd/nonstd.h"
#include "lib/ds/slist.h"

#include <stddef.h>

union node {
        struct slist_ref nodes;
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

        slist_init(&pool->nodes);
#ifndef NDEBUG
        pool->mem_start = ptr2uiptr(pool);
        pool->mem_end = uint2uiptr(limit);
#endif

        kassert(mblock + elem_size <= limit);
        union node *node = uint2ptr(mblock);
        slist_insert(&pool->nodes, &node->nodes);

        while (mblock + stride + elem_size <= limit) {
                union node *current = uint2ptr(mblock);
                union node *next = uint2uiptr(mblock + stride).ptr;
                mblock = ptr2uint(next);

                slist_insert(&current->nodes, &next->nodes);
        }
}

void *mem_pool_alloc(struct mem_pool *pool)
{
        kassert(pool);

        union node *n = container_of(slist_next(&pool->nodes), union node, nodes);
        if (!n) {
                return (NULL);
        }

        slist_remove(&pool->nodes, &n->nodes);
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

        slist_insert(&pool->nodes, &((union node*)m.ptr)->nodes);
}
