#include "kernel/mm/kmm.h"

#include <assert.h>
#include <stdlib.h>

struct kmm_cache *kmm_cache_create(const char *name, size_t size, size_t align,
                                   unsigned cache_flags, void (*ctor)(void *), void (*dtor)(void *))
{
        struct kmm_cache *cache = calloc(1, sizeof(*cache));
        assert(cache);

        cache->size = size;
        cache->alignment = align;
        cache->ctor = ctor;
        cache->dtor = dtor;

        return (cache);
}

void kmm_cache_destroy(struct kmm_cache *cache)
{
        free(cache);
}

void *kmm_cache_alloc(struct kmm_cache *cache)
{
        void *obj = aligned_alloc(cache->alignment, cache->size);
        if (cache->ctor != NULL) {
                cache->ctor(obj);
        }
        return (obj);
}

void kmm_cache_free(struct kmm_cache *cache, void *mem)
{
        if (cache->dtor != NULL) {
                cache->dtor(mem);
        }
        free(mem);
}
