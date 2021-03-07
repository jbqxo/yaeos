#include "lib/mm/pool.h"

#include "lib/align.h"
#include "lib/cppdefs.h"
#include "lib/cstd/assert.h"
#include "lib/ds/slist.h"
#include "lib/utils.h"

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
        uintptr_t mblock = align_roundup((uintptr_t)mem, elem_align);
        size_t stride = align_roundup(elem_size, elem_align);
        size_t limit = (uintptr_t)mem + mem_size;

        slist_init(&pool->nodes);
#ifndef NDEBUG
        pool->mem_start = pool;
        pool->mem_end = (void *)limit;
#endif

        kassert(mblock + elem_size <= limit);
        union node *node = (void *)mblock;
        kassert(properly_aligned(node));

        slist_insert(&pool->nodes, &node->nodes);

        while (mblock + stride + elem_size <= limit) {
                union node *current = (void *)mblock;
                union node *next = (void *)(mblock + stride);
                kassert(properly_aligned(current));
                kassert(properly_aligned(next));

                mblock = (uintptr_t)(next);

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

#ifndef NDEBUG
        kassert(mem > pool->mem_start && mem < pool->mem_end);
#endif

        union node *node = mem;
        kassert(properly_aligned(node));
        slist_insert(&pool->nodes, &node->nodes);
}
