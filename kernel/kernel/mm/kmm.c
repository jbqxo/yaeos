#include "kernel/mm/kmm.h"

#include "arch/platform.h"

#include "kernel/config.h"
#include "kernel/cppdefs.h"
#include "kernel/ds/slist.h"
#include "kernel/klog.h"
#include "kernel/mm/pool.h"
#include "kernel/mm/vmm.h"
#include "kernel/utils.h"

#include "lib/assert.h"
#include "lib/string.h"

#include <stdalign.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

static char STATIC_STORAGE[CONF_STATIC_SLAB_PAGES * PLATFORM_PAGE_SIZE];
static struct mem_pool STATIC_PAGE_POOL;

struct bufctl_small {
	SLIST_FIELD(struct bufctl_small) slist;
};

struct bufctl_large {
	SLIST_FIELD(struct bufctl_large) slist;
	void *memory;
};

union bufctl {
	SLIST_FIELD(union bufctl) slist;
	struct bufctl_small small;
	struct bufctl_large large;
};

static union uiptr bufctl_small_mem(struct bufctl_small *ctl, struct kmm_cache *cache)
{
	union uiptr ctl_mem = UIPTR((void*)ctl);
	ctl_mem.i -= cache->size;
	return (align_rounddown(ctl_mem, cache->alignment));
}

static union uiptr bufctl_large_mem(struct bufctl_large *ctl)
{
	return (UIPTR(ctl->memory));
}

static union uiptr bufctl_mem(union bufctl *ctl, struct kmm_cache *cache)
{
	if (cache->flags & KMM_CACHE_LARGE) {
		return (bufctl_large_mem(&ctl->large));
	}
	return (bufctl_small_mem(&ctl->small, cache));
}

static union uiptr bufctl_small_from_mem(union uiptr mem, struct kmm_cache *cache)
{
	mem.i += cache->size;
	mem = align_roundup(mem, sizeof(struct bufctl_small));
	return (mem);
}

struct page {
	struct kmm_slab *owner;
	bool dynamic;
	// Align address of buffer's data region.
	uintptr_t : 0;
	char data[0];
};

struct kmm_slab {
	SLIST_FIELD(struct kmm_slab) slabs_list;
	SLIST_HEAD(, union bufctl) free_buffers;

	struct page *page;
	unsigned inuse;
};

static struct {
	struct kmm_cache caches;
	struct kmm_cache large_bufctls;
	struct kmm_cache slabs;
} CACHES;

static SLIST_HEAD(, struct kmm_cache) ALLOCATED_CACHES;

static void page_free(struct page *p)
{
	assert(p);
	if (p->dynamic) {
		vmm_free_pages_at(p);
	} else {
		mem_pool_free(&STATIC_PAGE_POOL, p);
	}
}

static void slab_destroy(struct kmm_slab *slab, struct kmm_cache *cache)
{
	assert(slab);
	assert(cache);

	if (slab->inuse > 0) {
		LOGF_W("Freeing the slab %p while there are %d objects in use...", slab,
		       slab->inuse);
	}

	if (cache->flags & KMM_CACHE_LARGE) {
		union bufctl *it;
		SLIST_FOREACH (it, &slab->free_buffers, slist) {
			if (cache->dtor) {
				union uiptr mem = bufctl_large_mem(&it->large);
				cache->dtor(mem.p);
			}
			kmm_cache_free(&CACHES.large_bufctls, it);
		}
		kmm_cache_free(&CACHES.slabs, slab);
	} else {
		union bufctl *it;
		SLIST_FOREACH (it, &slab->free_buffers, slist) {
			if (cache->dtor) {
				union uiptr mem = bufctl_small_mem(&it->small, cache);
				cache->dtor(mem.p);
			}
		}
	}

	if (__likely(slab->inuse == 0)) {
		SLIST_REMOVE(&cache->slabs_empty, slab, slabs_list);
	} else if (slab->inuse == cache->capacity) {
		SLIST_REMOVE(&cache->slabs_full, slab, slabs_list);
	} else {
		SLIST_REMOVE(&cache->slabs_partial, slab, slabs_list);
	}

	page_free(slab->page);
}

static unsigned caches_try_reclaim(unsigned reclaim)
{
	unsigned reclaimed = 0;
	struct kmm_cache *itc;
	SLIST_FOREACH (itc, &ALLOCATED_CACHES, caches) {
		// Can't use SLIST_FOREACH 'cause it would use a slab after freeing.
		struct kmm_slab *next;
		struct kmm_slab *current = SLIST_FIRST(&itc->slabs_empty);
		while (current) {
			if (reclaimed >= reclaim) {
				return (reclaimed);
			}
			next = SLIST_NEXT(current, slabs_list);
			slab_destroy(current, itc);
			current = next;
			reclaimed++;
		}

		if (reclaimed >= reclaim) {
			return (reclaimed);
		}
	}
}

static struct page *page_alloc(struct kmm_cache *cache)
{
	assert(cache);

	const size_t to_allocate = 1;
	struct page *page = vmm_alloc_pages(to_allocate, VMM_ALLOC_KERNEL);
	if (!page) {
		LOGF_W("System is running out of memory. Trying to reclaim some...");
		if (caches_try_reclaim(to_allocate) >= to_allocate) {
			page = vmm_alloc_pages(to_allocate, VMM_ALLOC_KERNEL);
		} else {
			LOGF_W("Failed to reclaim required amount of memory.");
		}
	}

	if (__unlikely(!page)) {
		if (cache->flags & KMM_CACHE_STATIC) {
			LOGF_W("Couldn't allocate a page from dynamic memory for %s cache. Trying "
			       "static...",
			       cache->name);

			page = mem_pool_alloc(&STATIC_PAGE_POOL);
			if (__unlikely(!page)) {
				LOGF_E("Couldn't allocate a page from static memory for %s cache.",
				       cache->name);
				return (NULL);
			}
			LOGF_I("A page has been allocated from static memory for %s cache.",
			       cache->name);
			page->dynamic = false;
		} else {
			LOGF_E("Couldn't allocate a page for %s cache.", cache->name);
			return (NULL);
		}
	} else {
		page->dynamic = true;
	}

	{
		union uiptr page_addr = UIPTR((void *)page);
		assert(page_addr.p == align_roundup(page_addr, PLATFORM_PAGE_SIZE).p);
	}

	return (page);
}

static struct page *page_get_by_addr(void *addr)
{
	union uiptr p = UIPTR(addr);
	p.i &= -PLATFORM_PAGE_SIZE;
	return (p.p);
}

static struct kmm_slab *slab_get_by_addr(void *addr)
{
	return (page_get_by_addr(addr)->owner);
}

static struct kmm_slab *slab_create_small(struct kmm_cache *cache, unsigned colour)
{
	assert(cache);

	struct page *page = page_alloc(cache);
	if (__unlikely(!page)) {
		return (NULL);
	}

	union uiptr cursor = UIPTR((void *)page->data);

	struct kmm_slab *slab = cursor.p;
	cursor.i += sizeof(*slab);
	cursor.i += colour;
	cursor = align_roundup(cursor, cache->alignment);

	page->owner = slab;

	slab->inuse = 0;
	slab->page = page;
	SLIST_FIELD_INIT(slab, slabs_list);
	SLIST_INIT(&slab->free_buffers);

	// Check that leftover space is less than a full stride.
	assert((UIPTR((void *)page).i + PLATFORM_PAGE_SIZE) -
		       (cursor.i + cache->capacity * cache->stride) <
	       cache->stride);
	for (int i = 0; i < cache->capacity; i++, cursor.i += cache->stride) {
		union uiptr ctl_mem = bufctl_small_from_mem(cursor, cache);
		assert(ctl_mem.i < cursor.i + cache->stride);

		struct bufctl_small *ctl = ctl_mem.p;
		SLIST_FIELD_INIT(ctl, slist);
		SLIST_INSERT_HEAD(&slab->free_buffers, (union bufctl *)ctl, slist);

		union uiptr mem = bufctl_small_mem(ctl, cache);
		if (cache->ctor) {
			cache->ctor(mem.p);
		}
	}

	return (slab);
}

static struct kmm_slab *slab_create_large(struct kmm_cache *cache, unsigned colour)
{
	assert(cache);

	struct kmm_slab *slab = kmm_cache_alloc(&CACHES.slabs);
	if (__unlikely(!slab)) {
		return (NULL);
	}

	struct page *page = page_alloc(cache);
	if (__unlikely(!page)) {
		goto clean_slab;
	}
	page->owner = slab;

	union uiptr obj = UIPTR((void *)page->data);
	obj.i += colour;
	obj = align_roundup(obj, cache->alignment);

	slab->inuse = 0;
	slab->page = page;
	SLIST_FIELD_INIT(slab, slabs_list);
	SLIST_INIT(&slab->free_buffers);

	for (int i = 0; i < cache->capacity; i++, obj.i += cache->stride) {
		struct bufctl_large *ctl = kmm_cache_alloc(&CACHES.large_bufctls);
		SLIST_FIELD_INIT(ctl, slist);
		SLIST_INSERT_HEAD(&slab->free_buffers, (union bufctl *)ctl, slist);

		ctl->memory = obj.p;
		if (cache->ctor) {
			cache->ctor(obj.p);
		}
	}

	return (slab);

clean_slab:
	kmm_cache_free(&CACHES.slabs, slab);
	return (NULL);
}

static void object_free(void *mem, struct kmm_slab *slab, struct kmm_cache *cache)
{
	assert(mem);
	assert(slab);

	slab->inuse--;
	union bufctl *ctl;
	if (cache->flags & KMM_CACHE_LARGE) {
		ctl = kmm_cache_alloc(&CACHES.large_bufctls);
		ctl->large.memory = mem;
	} else {
		ctl = bufctl_small_from_mem(UIPTR(mem), cache).p;
	}
	SLIST_INSERT_HEAD(&slab->free_buffers, ctl, slist);
}

static void *object_alloc(struct kmm_slab *slab, struct kmm_cache *cache)
{
	assert(slab);
	assert(!SLIST_EMPTY(&slab->free_buffers));

	union bufctl *free_ctl = SLIST_FIRST(&slab->free_buffers);
	SLIST_REMOVE(&slab->free_buffers, free_ctl, slist);
	union uiptr mem = bufctl_mem(free_ctl, cache);
	if (cache->flags & KMM_CACHE_LARGE) {
		kmm_cache_free(&CACHES.large_bufctls, free_ctl);
	}

	slab->inuse++;

	return (mem.p);
}

static size_t cache_get_capacity(struct kmm_cache *cache)
{
	assert(cache);
	assert(cache->stride > 0);

	size_t avail_space = PLATFORM_PAGE_SIZE - sizeof(struct page);
	if (!(cache->flags & KMM_CACHE_LARGE)) {
		avail_space -= sizeof(struct kmm_slab);
	}

	return (avail_space / cache->stride);
}

static size_t cache_get_wasted(struct kmm_cache *cache)
{
	assert(cache);
	assert(cache->capacity > 0);
	assert(cache->stride > 0);

	size_t avail_space = PLATFORM_PAGE_SIZE - sizeof(struct page);
	if (!(cache->flags & KMM_CACHE_LARGE)) {
		avail_space -= sizeof(struct kmm_slab);
	}

	return (avail_space - cache->capacity * cache->stride);
}

static void cache_init(struct kmm_cache *restrict cache, const char *name, size_t size,
		       size_t align, unsigned flags, void (*ctor)(void *), void (*dtor)(void *))
{
	assert(cache);

	cache->name = name;
	cache->size = size;
	cache->alignment = align;
	cache->flags = flags;

	bool large = cache->size >= (PLATFORM_PAGE_SIZE / 8);
	cache->flags |= large ? KMM_CACHE_LARGE : 0;

	size_t obj_space = cache->size;
	if (!large) {
		obj_space += sizeof(struct bufctl_small);
		cache->alignment = MAX(sizeof(struct bufctl_small), cache->alignment);
	}

	cache->stride = align_roundup(UIPTR(obj_space), cache->alignment).i;
	cache->capacity = cache_get_capacity(cache);

	cache->colour_max = cache_get_wasted(cache) & (sizeof(void *) * -1);
	cache->colour_off = sizeof(void *);
	cache->colour_next = 0;

	cache->ctor = ctor;
	cache->dtor = dtor;
	SLIST_INIT(&cache->slabs_empty);
	SLIST_INIT(&cache->slabs_partial);
	SLIST_INIT(&cache->slabs_full);
}

void kmm_init(void)
{
	mem_pool_init(&STATIC_PAGE_POOL, STATIC_STORAGE, sizeof(STATIC_STORAGE), PLATFORM_PAGE_SIZE,
		      PLATFORM_PAGE_SIZE);

	SLIST_INIT(&ALLOCATED_CACHES);
	SLIST_INSERT_HEAD(&ALLOCATED_CACHES, &CACHES.caches, caches);
	SLIST_INSERT_HEAD(&ALLOCATED_CACHES, &CACHES.slabs, caches);
	SLIST_INSERT_HEAD(&ALLOCATED_CACHES, &CACHES.large_bufctls, caches);

	cache_init(&CACHES.caches, "slab_alloc_caches", sizeof(struct kmm_cache), 0,
		   KMM_CACHE_STATIC, NULL, NULL);
	cache_init(&CACHES.slabs, "slab_alloc_slabs", sizeof(struct kmm_slab), 0, KMM_CACHE_STATIC,
		   NULL, NULL);
	cache_init(&CACHES.large_bufctls, "slab_alloc_bufctls", sizeof(struct bufctl_large), 0,
		   KMM_CACHE_STATIC, NULL, NULL);
}

struct kmm_cache *kmm_cache_create(const char *name, size_t size, size_t align,
				   unsigned cache_flags, void (*ctor)(void *), void (*dtor)(void *))
{
	struct kmm_cache *cache = kmm_cache_alloc(&CACHES.caches);
	if (!cache) {
		return (NULL);
	}
	cache_init(cache, name, size, align, cache_flags, ctor, dtor);

	SLIST_INSERT_HEAD(&ALLOCATED_CACHES, cache, caches);

	return (cache);
}

void kmm_cache_destroy(struct kmm_cache *cache)
{
	assert(cache);

	// Can't use SLIST_FOREACH 'cause it would use a slab after freeing.
	struct kmm_slab *next;
	struct kmm_slab *current = SLIST_FIRST(&cache->slabs_full);
	while (current) {
		LOGF_W("Freeing slab with objects in use from %s cache\n", cache->name);
		next = SLIST_NEXT(current, slabs_list);
		slab_destroy(current, cache);
		current = next;
	}

	current = SLIST_FIRST(&cache->slabs_partial);
	while (current) {
		LOGF_W("Freeing slab with objects in use from %s cache\n", cache->name);
		next = SLIST_NEXT(current, slabs_list);
		slab_destroy(current, cache);
		current = next;
	}

	current = SLIST_FIRST(&cache->slabs_empty);
	while (current) {
		next = SLIST_NEXT(current, slabs_list);
		slab_destroy(current, cache);
		current = next;
	}

	SLIST_REMOVE(&ALLOCATED_CACHES, cache, caches);

	kmm_cache_free(&CACHES.caches, cache);
}

void *kmm_cache_alloc(struct kmm_cache *cache)
{
	assert(cache);

	struct kmm_slab *slab = NULL;
	if (!SLIST_EMPTY(&cache->slabs_partial)) {
		slab = SLIST_FIRST(&cache->slabs_partial);

		bool becomes_full = slab->inuse + 1 == cache->capacity;
		if (becomes_full) {
			SLIST_REMOVE(&cache->slabs_partial, slab, slabs_list);
			SLIST_INSERT_HEAD(&cache->slabs_full, slab, slabs_list);
		}
	} else if (!SLIST_EMPTY(&cache->slabs_empty)) {
		slab = SLIST_FIRST(&cache->slabs_empty);

		// Becomes partial.
		SLIST_REMOVE(&cache->slabs_empty, slab, slabs_list);
		SLIST_INSERT_HEAD(&cache->slabs_partial, slab, slabs_list);
	} else {
		if (cache->flags & KMM_CACHE_LARGE) {
			slab = slab_create_large(cache, cache->colour_next);
		} else {
			slab = slab_create_small(cache, cache->colour_next);
		}
		if (cache->colour_max > 0) {
			cache->colour_next =
				(cache->colour_next + cache->colour_off) % cache->colour_max;
		}

		if (__unlikely(!slab)) {
			return (NULL);
		}

		SLIST_INSERT_HEAD(&cache->slabs_partial, slab, slabs_list);
	}

	void *obj = object_alloc(slab, cache);
	assert(obj);

	return (obj);
}

void kmm_cache_free(struct kmm_cache *cache, void *mem)
{
	assert(cache);
	assert(mem);

	struct kmm_slab *slab = slab_get_by_addr(mem);
	assert(slab);

	bool was_full = slab->inuse == cache->capacity;
	bool becomes_empty = slab->inuse == 1;

	if (was_full) {
		SLIST_REMOVE(&cache->slabs_full, slab, slabs_list);
	} else if (becomes_empty) {
		SLIST_REMOVE(&cache->slabs_partial, slab, slabs_list);
	}

	if (becomes_empty) {
		SLIST_INSERT_HEAD(&cache->slabs_empty, slab, slabs_list);
	} else if (was_full) {
		SLIST_INSERT_HEAD(&cache->slabs_partial, slab, slabs_list);
	}

	object_free(mem, slab, cache);
}
