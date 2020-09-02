#include "kernel/mm/pool.h"

#include "kernel/cppdefs.h"
#include "kernel/ds/slist.h"
#include "kernel/utils.h"

#include "lib/assert.h"

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
	union uiptr mblock = align_roundup(ptr2uiptr(mem), elem_align);
	size_t stride = align_roundup(num2uiptr(elem_size), elem_align).num;
	size_t limit = ptr2uiptr(mem).num + mem_size;

	SLIST_INIT(&pool->list);
#ifndef NDEBUG
	pool->mem_start = ptr2uiptr(pool);
	pool->mem_end = num2uiptr(limit);
#endif

	assert(mblock.num + elem_size <= limit);
	SLIST_INSERT_HEAD(&pool->list, (union node *)mblock.ptr, list);

	while (mblock.num + stride + elem_size <= limit) {
		union node *current = mblock.ptr;
		union node *next = num2uiptr(mblock.num + stride).ptr;
		mblock.ptr = next;

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

	union uiptr m = ptr2uiptr(mem);
#ifndef NDEBUG
	assert(m.num > pool->mem_start.num && m.num < pool->mem_end.num);
#endif

	SLIST_INSERT_HEAD(&pool->list, (union node *)m.ptr, list);
}
