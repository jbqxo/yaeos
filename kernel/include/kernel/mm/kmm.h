#ifndef _KERNEL_MM_KMM_H
#define _KERNEL_MM_KMM_H

#include "kernel/ds/slist.h"
#include "kernel/mm/vmm.h"

#include <stddef.h>
#include <stdint.h>

struct kmm_cache {
	size_t size;          //! Size of objects in the cache.
	size_t alignment;     //! Alignment of each object.
	size_t slab_capacity; //! Number of objects stuffed in one slab.
	size_t stride;        //! Actual space wasted on each object.

	// Slab coloring.
	unsigned colour_max;
	unsigned colour_off;
	unsigned colour_next;

#define KMM_CACHE_STATIC (0x1 << 0) //! Can use reserved static storage.
#define KMM_CACHE_LARGE  (0x1 << 1)
	unsigned flags;

	const char *name;
	void (*ctor)(void *);
	void (*dtor)(void *);

	SLIST_HEAD(, struct kmm_slab) slabs_empty;
	SLIST_HEAD(, struct kmm_slab) slabs_partial;
	SLIST_HEAD(, struct kmm_slab) slabs_full;

	SLIST_FIELD(struct kmm_cache) caches; //! List of all creating caches. Used to reclaim memory.
};

///
/// Initialize the KMM subsystem. It needs to be called only on boot.
///
void kmm_init(void);

///
/// Create a new cache.
///
/// @param name Displayed name of the cache.
/// @param size Size of the object.
/// @param align Required object alignment.
/// @param cache_flags See KMM_CACHE_ defines.
/// @param ctor Method to call on each object's creation.
/// @param dtor Method to call on each object's deletion.
/// @return Pointer on a newly created cache.
///
struct kmm_cache *kmm_cache_create(const char *name, size_t size, size_t align,
				   unsigned cache_flags, void (*ctor)(void *),
				   void (*dtor)(void *));

///
/// Destroy given cache.
///
/// @warning It will call specifiend destructor to delete every object owned by the cache. May take some time.
///
void kmm_cache_destroy(struct kmm_cache *);

///
/// Get a free object from a cache.
///
/// @warning May take some time if there is no free objects.
void *kmm_cache_alloc(struct kmm_cache *);

///
/// Free given object.
///
void kmm_cache_free(struct kmm_cache *, void *mem);

#endif // _KERNEL_MM_KMM_H
