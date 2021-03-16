#ifndef _KERNEL_MM_KMM_H
#define _KERNEL_MM_KMM_H

#include "kernel/mm/vm.h"

#include "lib/ds/slist.h"
#include "lib/cppdefs.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef void *(*alloc_page_fn_t)(void);
typedef void (*free_page_fn_t)(void *);

struct kmm_cache {
        size_t size;          /**< Size of objects in the cache. */
        size_t alignment;     /**< Alignment of each object. */
        size_t slab_capacity; /**< Number of objects stuffed in one slab. */
        size_t stride;        /**< Actual space wasted on each object. */

        /* Slab coloring. */
        size_t colour_max;
        size_t colour_off;
        size_t colour_next;

#define KMM_CACHE_LARGE (0x1 << 0)
        unsigned flags;

        const char *name;
        void (*ctor)(void *);
        void (*dtor)(void *);

        struct slist_ref slabs_empty;
        struct slist_ref slabs_partial;
        struct slist_ref slabs_full;

        struct slist_ref sys_caches; /**< List of all created caches. Used to reclaim memory. */
};

/**
 * @brief Initialize the KMM subsystem. It needs to be called only on boot.
 */
void kmm_init(alloc_page_fn_t alloc_page_fn, free_page_fn_t free_page_fn);
void kmm_cache_trim_all(void);

void kmm_cache_init(struct kmm_cache *restrict cache, const char *name, size_t size, size_t align,
                    unsigned flags, void (*ctor)(void *), void (*dtor)(void *));
void kmm_cache_trim(struct kmm_cache *cache);

/**
 * @brief Allocates, inits, and registers new cache object.
 *
 * @param name Displayed name of the cache.
 * @param size Size of the object.
 * @param align Required object alignment.
 * @param cache_flags See KMM_CACHE_ defines.
 * @param ctor Method to call on each object's creation.
 * @param dtor Method to call on each object's deletion.
 * @return Pointer on a newly created cache.
 */
struct kmm_cache *kmm_cache_create(const char *name, size_t size, size_t align,
                                   unsigned cache_flags, void (*ctor)(void *),
                                   void (*dtor)(void *));

/**
 * @brief Destroy given cache.
 *
 * @warning It will call specifiend destructor to delete every object owned by the cache. May take some time.
 */
void kmm_cache_destroy(struct kmm_cache *);

/**
 * @brief Get a free object from a cache.
 *
 * @warning May take some time if there is no free objects.
 */
__malloc void *kmm_cache_alloc(struct kmm_cache *);

/**
 * @brief Free given object.
 */
void kmm_cache_free(struct kmm_cache *, void *mem);

#endif /* _KERNEL_MM_KMM_H */
