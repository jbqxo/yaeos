#include "kernel/mm/kmm.h"

#include "kernel/klog.h"
#include "kernel/platform_consts.h"

#include "lib/align.h"
#include "lib/cppdefs.h"
#include "lib/cstd/assert.h"
#include "lib/cstd/string.h"
#include "lib/ds/slist.h"
#include "lib/utils.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

static alloc_page_fn_t ALLOC_PAGE_FN;
static free_page_fn_t FREE_PAGE_FN;

/**
 *  Buffer control block for small objects (< PAGE_SIZE / 8).
 *  It's stored at the end of the buffer it manages.
 */
struct bufctl_small {
        struct slist_ref slist;
};

struct bufctl_large {
        struct slist_ref slist;
        void *memory;
};

union bufctl {
        struct slist_ref slist;
        struct bufctl_small small;
        struct bufctl_large large;
};

/**
 * Returns an address of a memory block that is owned by a small bufctl.
 */
static void *bufctl_small_get_mem(struct bufctl_small *ctl, struct kmm_cache *cache)
{
        uintptr_t ctl_addr = (uintptr_t)ctl;
        uintptr_t buffer_addr = ctl_addr - cache->size;
        buffer_addr = align_rounddown(buffer_addr, cache->alignment);
        return ((void *)buffer_addr);
}

/**
 * Returns an address of a memory block that is owned by a large bufctl.
 */
static void *bufctl_large_get_mem(struct bufctl_large *ctl)
{
        return (ctl->memory);
}

/**
 * Returns an address of a memory block owned by a bufctl.
 */
static void *bufctl_mem(union bufctl *ctl, struct kmm_cache *cache)
{
        if (cache->flags & KMM_CACHE_LARGE) {
                return (bufctl_large_get_mem(&ctl->large));
        }
        return (bufctl_small_get_mem(&ctl->small, cache));
}

/**
 * Returns an address of a small bufctl that owns given buffer.
 */
static struct bufctl_small *get_bufctl_small(void *buffer, struct kmm_cache *cache)
{
        uintptr_t b = (uintptr_t)buffer;
        b += cache->size;
        b = align_roundup(b, sizeof(struct bufctl_small));

        struct bufctl_small *s = (struct bufctl_small *)b;
        kassert(properly_aligned(s));

        return (s);
}

/**
 * The structure of every used page.
 * We keep some information at the beginning and use the rest of the page.
 */
struct page {
        struct kmm_slab *owner;
        uintptr_t : 0; /**< Align address of buffer's data region. */
        char data[];
};

struct kmm_slab {
        struct slist_ref slabs_list;
        struct slist_ref free_buffers;

        struct page *page;
        size_t objects_inuse;
};

static struct {
        struct kmm_cache caches;
        struct kmm_cache large_bufctls;
        struct kmm_cache slabs;
} CACHES;

/* Used to find empty memory slabs. */
static struct slist_ref ALLOCATED_CACHES_HEAD;

static void page_free(struct page *p)
{
        kassert(p);
        FREE_PAGE_FN(p);
}

/**
 * Allocates a new page from VMM.
 * If the allocation fails, it will try to reclaim some memory from other caches.
 * If the allocation fails again and the cache is capable of using static storage,
 * it will get memory from static memory pool as a last resort.
 */
static struct page *page_alloc(struct kmm_cache *cache __unused)
{
        kassert(cache);

        struct page *page = ALLOC_PAGE_FN();
        if (__unlikely(page == NULL)) {
                LOGF_E("There are no more free pages in the kernel's heap.\n");
        }

        kassert(check_align((uintptr_t)page, PLATFORM_PAGE_SIZE));

        return (page);
}

static void free_large_buffers(struct slist_ref *list_head, struct kmm_cache *cache)
{
        kassert(list_head != NULL);
        kassert(cache != NULL);

        /* Can't use SLIST_FOREACH 'cause it would use a slab after freeing. */
        struct slist_ref *next = NULL;
        struct slist_ref *current = slist_next(list_head);
        while (current != NULL) {
                union bufctl *b = container_of(current, union bufctl, slist);
                next = slist_next(current);

                if (cache->dtor) {
                        void *mem = bufctl_large_get_mem(&b->large);
                        cache->dtor(mem);
                }
                kmm_cache_free(cache, b);

                current = next;
        }
}

static void slab_destroy(struct kmm_slab *slab, struct kmm_cache *cache)
{
        kassert(slab);
        kassert(cache);

        if (slab->objects_inuse > 0) {
                LOGF_W("Freeing the slab %p while there are %zu objects in use...\n", slab,
                       slab->objects_inuse);
        }

        if (cache->flags & KMM_CACHE_LARGE) {
                free_large_buffers(&slab->free_buffers, cache);
                kmm_cache_free(&CACHES.slabs, slab);
        } else {
                if (cache->dtor) {
                        SLIST_FOREACH (it, slist_next(&slab->free_buffers)) {
                                union bufctl *b = container_of(it, union bufctl, slist);
                                void *mem = bufctl_small_get_mem(&b->small, cache);
                                cache->dtor(mem);
                        }
                }
        }

        if (__likely(slab->objects_inuse == 0)) {
                slist_remove(&cache->slabs_empty, &slab->slabs_list);
        } else if (slab->objects_inuse == cache->slab_capacity) {
                slist_remove(&cache->slabs_full, &slab->slabs_list);
        } else {
                slist_remove(&cache->slabs_partial, &slab->slabs_list);
        }

        page_free(slab->page);
}

static void free_slabs_list(struct slist_ref *list_head, struct kmm_cache *from_cache)
{
        kassert(list_head != NULL);
        kassert(from_cache != NULL);

        /* Can't use SLIST_FOREACH 'cause it would use a slab after freeing. */
        struct slist_ref *next = NULL;
        struct slist_ref *current = slist_next(list_head);
        while (current != NULL) {
                struct kmm_slab *slab = container_of(current, struct kmm_slab, slabs_list);
                next = slist_next(current);
                slab_destroy(slab, from_cache);
                current = next;
        }
}

void kmm_cache_trim(struct kmm_cache *cache)
{
        kassert(cache != NULL);
        free_slabs_list(&cache->slabs_empty, cache);
}

void kmm_cache_trim_all(void)
{
        SLIST_FOREACH (it, slist_next(&ALLOCATED_CACHES_HEAD)) {
                struct kmm_cache *c = container_of(it, struct kmm_cache, sys_caches);
                kmm_cache_trim(c);
        }
}

/**
 * Returns page structure that contains an address.
 */
static struct page *page_get_by_addr(void *addr)
{
        uintptr_t p = (uintptr_t)addr;
        p &= -PLATFORM_PAGE_SIZE;

        struct page *page = (struct page *)p;
        kassert(properly_aligned(page));

        return (page);
}

/**
 * Returns slab that owns an address.
 */
static struct kmm_slab *slab_get_by_addr(void *addr)
{
        return (page_get_by_addr(addr)->owner);
}

static struct kmm_slab *slab_create_small(struct kmm_cache *cache, size_t const colour)
{
        kassert(cache);

        struct page *page = page_alloc(cache);
        if (__unlikely(page == NULL)) {
                return (NULL);
        }

        uintptr_t cursor = (uintptr_t)page->data;

        struct kmm_slab *slab = (void *)cursor;
        kassert(properly_aligned(slab));

        cursor += sizeof(*slab);
        cursor += colour;
        cursor = align_roundup(cursor, cache->alignment);

        page->owner = slab;

        slab->objects_inuse = 0;
        slab->page = page;
        slist_init(&slab->slabs_list);
        slist_init(&slab->free_buffers);

        kassert(({
                uintptr_t const page_end = (uintptr_t)page + PLATFORM_PAGE_SIZE;
                uintptr_t const slab_elems_end = cursor + cache->slab_capacity * cache->stride;
                /* Check that leftover space is less than a full stride. */
                (page_end - slab_elems_end < cache->stride);
        }));
        for (size_t i = 0; i < cache->slab_capacity; i++, cursor += cache->stride) {
                void *buffer = (void *)cursor;
                struct bufctl_small *ctl = get_bufctl_small(buffer, cache);
                kassert((uintptr_t)ctl < cursor + cache->stride);

                slist_init(&ctl->slist);
                slist_insert(&slab->free_buffers, &ctl->slist);

                void *mem = bufctl_small_get_mem(ctl, cache);
                if (cache->ctor) {
                        cache->ctor(mem);
                }
        }

        return (slab);
}

static struct kmm_slab *slab_create_large(struct kmm_cache *cache, size_t const colour)
{
        kassert(cache);

        struct kmm_slab *slab = kmm_cache_alloc(&CACHES.slabs);
        if (__unlikely(slab == NULL)) {
                return (NULL);
        }

        struct page *page = page_alloc(cache);
        if (__unlikely(page == NULL)) {
                goto clean_slab;
        }
        page->owner = slab;

        uintptr_t obj_addr = (uintptr_t)page->data;
        obj_addr += colour;
        obj_addr = align_roundup(obj_addr, cache->alignment);

        slab->objects_inuse = 0;
        slab->page = page;
        slist_init(&slab->slabs_list);
        slist_init(&slab->free_buffers);

        for (size_t i = 0; i < cache->slab_capacity; i++, obj_addr += cache->stride) {
                struct bufctl_large *ctl = kmm_cache_alloc(&CACHES.large_bufctls);
                slist_init(&ctl->slist);
                slist_insert(&slab->free_buffers, &ctl->slist);

                void *obj = (void *)obj_addr;
                ctl->memory = obj;
                if (cache->ctor) {
                        cache->ctor(obj);
                }
        }

        return (slab);

clean_slab:
        kmm_cache_free(&CACHES.slabs, slab);
        return (NULL);
}

static void object_free(void *mem, struct kmm_slab *slab, struct kmm_cache *cache)
{
        kassert(mem);
        kassert(slab);

        slab->objects_inuse--;
        union bufctl *ctl;
        if (cache->flags & KMM_CACHE_LARGE) {
                ctl = kmm_cache_alloc(&CACHES.large_bufctls);
                ctl->large.memory = mem;
        } else {
                ctl = (union bufctl *)get_bufctl_small(mem, cache);
        }
        slist_insert(&slab->free_buffers, &ctl->slist);
}

static void *object_alloc(struct kmm_slab *slab, struct kmm_cache *cache)
{
        kassert(slab);
        kassert(!slist_is_empty(&slab->free_buffers));

        union bufctl *free_ctl = container_of(slist_next(&slab->free_buffers), union bufctl, slist);
        slist_remove(&slab->free_buffers, &free_ctl->slist);
        void *mem = bufctl_mem(free_ctl, cache);
        if (cache->flags & KMM_CACHE_LARGE) {
                kmm_cache_free(&CACHES.large_bufctls, free_ctl);
        }

        slab->objects_inuse++;

        return (mem);
}

static size_t cache_get_avail_space(struct kmm_cache *cache)
{
        kassert(cache);
        kassert(cache->stride > 0);

        /* struct page is always stored at the beginning of the page. */
        size_t avail_space = PLATFORM_PAGE_SIZE - sizeof(struct page);
        if (!(cache->flags & KMM_CACHE_LARGE)) {
                avail_space -= sizeof(struct kmm_slab);
        }

        return (avail_space);
}

static size_t cache_get_capacity(struct kmm_cache *cache)
{
        kassert(cache);
        kassert(cache->stride > 0);

        return (cache_get_avail_space(cache) / cache->stride);
}

static size_t cache_get_wasted(struct kmm_cache *cache)
{
        kassert(cache);
        kassert(cache->slab_capacity > 0);
        kassert(cache->stride > 0);

        return (cache_get_avail_space(cache) - cache->slab_capacity * cache->stride);
}

void kmm_cache_init(struct kmm_cache *restrict cache, const char *name, size_t size, size_t align,
                    unsigned flags, void (*ctor)(void *), void (*dtor)(void *))
{
        kassert(cache);

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

        if (cache->alignment != 0) {
                cache->stride = align_roundup(obj_space, cache->alignment);
        } else {
                cache->stride = obj_space;
        }
        cache->slab_capacity = cache_get_capacity(cache);

        cache->colour_max = cache_get_wasted(cache);
        cache->colour_off = cache->alignment;
        cache->colour_next = 0;

        cache->ctor = ctor;
        cache->dtor = dtor;
        slist_init(&cache->slabs_empty);
        slist_init(&cache->slabs_partial);
        slist_init(&cache->slabs_full);
}

static void kmm_cache_register(struct kmm_cache *cache)
{
        slist_insert(&ALLOCATED_CACHES_HEAD, &cache->sys_caches);
}

void kmm_init(alloc_page_fn_t alloc_page_fn, free_page_fn_t free_page_fn)
{
        kassert(alloc_page_fn != NULL);
        kassert(free_page_fn != NULL);

        ALLOC_PAGE_FN = alloc_page_fn;
        FREE_PAGE_FN = free_page_fn;

        slist_init(&ALLOCATED_CACHES_HEAD);

        kmm_cache_register(&CACHES.caches);
        kmm_cache_register(&CACHES.slabs);
        kmm_cache_register(&CACHES.large_bufctls);

        kmm_cache_init(&CACHES.caches, "slab_alloc_caches", sizeof(struct kmm_cache), 0, 0, NULL,
                       NULL);
        kmm_cache_init(&CACHES.slabs, "slab_alloc_slabs", sizeof(struct kmm_slab), 0, 0, NULL,
                       NULL);
        kmm_cache_init(&CACHES.large_bufctls, "slab_alloc_bufctls", sizeof(struct bufctl_large), 0,
                       0, NULL, NULL);
}

struct kmm_cache *kmm_cache_create(const char *name, size_t size, size_t align,
                                   unsigned cache_flags, void (*ctor)(void *), void (*dtor)(void *))
{
        struct kmm_cache *cache = kmm_cache_alloc(&CACHES.caches);
        if (!cache) {
                return (NULL);
        }
        kmm_cache_init(cache, name, size, align, cache_flags, ctor, dtor);

        kmm_cache_register(cache);

        return (cache);
}

void kmm_cache_destroy(struct kmm_cache *cache)
{
        kassert(cache);

        if (!slist_is_empty(&cache->slabs_full)) {
                LOGF_W("Freeing slab with objects in use from %s cache\n", cache->name);
                free_slabs_list(&cache->slabs_full, cache);
        }
        if (!slist_is_empty(&cache->slabs_partial)) {
                LOGF_W("Freeing slab with objects in use from %s cache\n", cache->name);
                free_slabs_list(&cache->slabs_partial, cache);
        }
        free_slabs_list(&cache->slabs_empty, cache);

        slist_remove(&ALLOCATED_CACHES_HEAD, &cache->sys_caches);

        kmm_cache_free(&CACHES.caches, cache);
}

static struct kmm_slab *find_existing_slab(struct kmm_cache *cache)
{
        kassert(cache != NULL);

        struct kmm_slab *slab = NULL;
        if (!slist_is_empty(&cache->slabs_partial)) {
                slab = container_of(slist_next(&cache->slabs_partial), struct kmm_slab, slabs_list);

                bool becomes_full = slab->objects_inuse + 1 == cache->slab_capacity;
                if (becomes_full) {
                        slist_remove(&cache->slabs_partial, &slab->slabs_list);
                        slist_insert(&cache->slabs_full, &slab->slabs_list);
                }
        } else if (!slist_is_empty(&cache->slabs_empty)) {
                slab = container_of(slist_next(&cache->slabs_empty), struct kmm_slab, slabs_list);

                /* Becomes partial. */
                slist_remove(&cache->slabs_empty, &slab->slabs_list);
                slist_insert(&cache->slabs_partial, &slab->slabs_list);
        }

        return (slab);
}

static struct kmm_slab *get_new_slab(struct kmm_cache *cache)
{
        kassert(cache != NULL);

        struct kmm_slab *slab = NULL;
        if (cache->flags & KMM_CACHE_LARGE) {
                slab = slab_create_large(cache, cache->colour_next);
        } else {
                slab = slab_create_small(cache, cache->colour_next);
        }
        cache->colour_next += cache->colour_off;
        if (cache->colour_next >= cache->colour_max) {
                cache->colour_next = 0;
        }

        if (__unlikely(slab == NULL)) {
                return (NULL);
        }

        if (cache->slab_capacity == 1) {
                slist_insert(&cache->slabs_full, &slab->slabs_list);
        } else {
                slist_insert(&cache->slabs_partial, &slab->slabs_list);
        }

        return (slab);
}

void *kmm_cache_alloc(struct kmm_cache *cache)
{
        kassert(cache != NULL);

        struct kmm_slab *slab = find_existing_slab(cache);
        if (slab == NULL) {
                slab = get_new_slab(cache);
                if (__unlikely(slab == NULL)) {
                        return (NULL);
                }
        }

        void *obj = object_alloc(slab, cache);
        kassert(obj);

        return (obj);
}

void kmm_cache_free(struct kmm_cache *cache, void *mem)
{
        kassert(cache);
        kassert(mem);

        struct kmm_slab *slab = slab_get_by_addr(mem);
        kassert(slab);

        bool was_full = slab->objects_inuse == cache->slab_capacity;
        bool becomes_empty = slab->objects_inuse == 1;

        if (was_full) {
                slist_remove(&cache->slabs_full, &slab->slabs_list);
        } else if (becomes_empty) {
                kassert(cache->slab_capacity > 1);
                slist_remove(&cache->slabs_partial, &slab->slabs_list);
        }

        if (becomes_empty) {
                slist_insert(&cache->slabs_empty, &slab->slabs_list);
        } else if (was_full) {
                slist_insert(&cache->slabs_partial, &slab->slabs_list);
        }

        object_free(mem, slab, cache);
}
