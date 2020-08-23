#ifndef _KERNEL_MM_KMM_H
#define _KERNEL_MM_KMM_H

#include <stddef.h>
#include <stdint.h>
#include <kernel/ds/slist.h>



struct kmm_cache {
	size_t size;
	size_t alignment;
	size_t capacity;
	size_t stride;

	unsigned colour_max;
	unsigned colour_off;
	unsigned colour_next;

#define KMM_CACHE_STATIC (0x1 << 0)
#define KMM_CACHE_LARGE (0x1 << 1)
	unsigned flags;

	const char *name;
	void (*ctor)(void *);
	void (*dtor)(void *);

	SLIST_HEAD(, struct kmm_slab) slabs_empty;
	SLIST_HEAD(, struct kmm_slab) slabs_partial;
	SLIST_HEAD(, struct kmm_slab) slabs_full;

	SLIST_FIELD(struct kmm_cache) caches;
};

void kmm_init(void);

struct kmm_cache *kmm_cache_create(const char *name, size_t size, size_t align, unsigned cache_flags,
				   void (*ctor)(void *), void (*dtor)(void *));
void kmm_cache_destroy(struct kmm_cache *);

void *kmm_cache_alloc(struct kmm_cache *);
void kmm_cache_free(struct kmm_cache *, void *mem);

#endif // _KERNEL_MM_KMM_H
