#include <kernel/mm/pool.h>
#include <kernel/ds/slist.h>
#include <kernel/utils.h>
#include <kernel/cppdefs.h>
#include <lib/assert.h>

#include <stddef.h>

union node {
	SLIST_FIELD(union node) list;
	char data[0];
};

void mem_pool_init(struct mem_pool *pool, void *mem, size_t mem_size, size_t elem_size,
		   size_t elem_align)
{
	assert(pool);
	assert(mem);
	assert(mem_size >= elem_size);

	elem_size = MAX(elem_size, sizeof(union node));
	union uiptr mblock = align_roundup(UIPTR(mem), elem_align);
	size_t stride = align_roundup(UIPTR(elem_size), elem_align).i;
	size_t limit = UIPTR(mem).i + mem_size;

	SLIST_INIT(&pool->list);
#ifndef NDEBUG
	pool->mem_start = UIPTR((void *)pool);
	pool->mem_end = UIPTR(limit);
#endif

	assert(mblock.i + elem_size <= limit);
	SLIST_INSERT_HEAD(&pool->list, (union node *)mblock.p, list);

	while (mblock.i + stride + elem_size <= limit) {
		union node *current = mblock.p;
		union node *next = UIPTR(mblock.i + stride).p;
		mblock.p = next;

		SLIST_INSERT_AFTER(current, next, list);
	}
}

void *mem_pool_alloc(struct mem_pool *pool)
{
	assert(pool);

	union node *n = SLIST_FIRST(&pool->list);
	if (!n) {
		return (NULL);
	}

	SLIST_REMOVE(&pool->list, n, list);
	return (n);
}

void mem_pool_free(struct mem_pool *pool, void *mem)
{
	assert(pool);
	assert(mem);

	union uiptr m = UIPTR(mem);
#ifndef NDEBUG
	assert(m.i > pool->mem_start.i && m.i < pool->mem_end.i);
#endif

	SLIST_INSERT_HEAD(&pool->list, (union node *)m.p, list);
}
