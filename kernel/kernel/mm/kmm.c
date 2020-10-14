#include "kernel/mm/kmm.h"

#include "kernel/config.h"
#include "kernel/cppdefs.h"
#include "kernel/ds/slist.h"
#include "kernel/kernel.h"
#include "kernel/klog.h"
#include "kernel/mm/pool.h"
#include "kernel/mm/vmm.h"
#include "kernel/platform.h"
#include "kernel/utils.h"

#include "lib/assert.h"
#include "lib/string.h"

#include <stdalign.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

static char STATIC_STORAGE[CONF_STATIC_SLAB_PAGES * PLATFORM_PAGE_SIZE];
static struct mem_pool STATIC_PAGE_POOL;

///
/// Buffer control block for small objects (< PAGE_SIZE / 8).
/// It's stored at the end of the buffer it manages.
///
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

///
/// Returns an address of a memory block that is owned by a small bufctl.
///
static void *bufctl_small_get_mem(struct bufctl_small *ctl, struct kmm_cache *cache)
{
        uintptr_t ctl_addr = ptr2uint(ctl);
        uintptr_t buffer_addr = ctl_addr - cache->size;
        buffer_addr = align_rounddown(buffer_addr, cache->alignment);
        return (uint2ptr(buffer_addr));
}

///
/// Returns an address of a memory block that is owned by a large bufctl.
///
static void *bufctl_large_get_mem(struct bufctl_large *ctl)
{
        return (ctl->memory);
}

///
/// Returns an address of a memory block owned by a bufctl.
///
static void *bufctl_mem(union bufctl *ctl, struct kmm_cache *cache)
{
        if (cache->flags & KMM_CACHE_LARGE) {
                return (bufctl_large_get_mem(&ctl->large));
        }
        return (bufctl_small_get_mem(&ctl->small, cache));
}

///
/// Returns an address of a small bufctl that owns given buffer.
///
static void *get_bufctl_small(uintptr_t buffer_addr, struct kmm_cache *cache)
{
        buffer_addr += cache->size;
        buffer_addr = align_roundup(buffer_addr, sizeof(struct bufctl_small));
        return (uint2ptr(buffer_addr));
}

///
/// The structure of every used page.
/// We keep some information at the beginning and use the rest of the page.
///
struct page {
        struct kmm_slab *owner;
        bool dynamic;  //! Whether the page was allocated from VMM, or from the static memory pool.
        uintptr_t : 0; //! Align address of buffer's data region.
        char data[0];
};

struct kmm_slab {
        SLIST_FIELD(struct kmm_slab) slabs_list;
        SLIST_HEAD(, union bufctl) free_buffers;

        struct page *page;
        unsigned objects_inuse;
};

static struct {
        struct kmm_cache caches;
        struct kmm_cache large_bufctls;
        struct kmm_cache slabs;
} CACHES;

/// Used to find empty memory slabs.
static SLIST_HEAD(, struct kmm_cache) ALLOCATED_CACHES;

static void page_free(struct page *p)
{
        kassert(p);
        if (p->dynamic) {
                vmm_free_pages(&kvm_space, p, 1);
        } else {
                mem_pool_free(&STATIC_PAGE_POOL, p);
        }
}

static void slab_destroy(struct kmm_slab *slab, struct kmm_cache *cache)
{
        kassert(slab);
        kassert(cache);

        if (slab->objects_inuse > 0) {
                LOGF_W("Freeing the slab %p while there are %d objects in use...", slab,
                       slab->objects_inuse);
        }

        if (cache->flags & KMM_CACHE_LARGE) {
                union bufctl *it;
                SLIST_FOREACH (it, &slab->free_buffers, slist) {
                        if (cache->dtor) {
                                void *mem = bufctl_large_get_mem(&it->large);
                                cache->dtor(mem);
                        }
                        kmm_cache_free(&CACHES.large_bufctls, it);
                }
                kmm_cache_free(&CACHES.slabs, slab);
        } else {
                union bufctl *it;
                SLIST_FOREACH (it, &slab->free_buffers, slist) {
                        if (cache->dtor) {
                                void *mem = bufctl_small_get_mem(&it->small, cache);
                                cache->dtor(mem);
                        }
                }
        }

        if (__likely(slab->objects_inuse == 0)) {
                SLIST_REMOVE(&cache->slabs_empty, slab, slabs_list);
        } else if (slab->objects_inuse == cache->slab_capacity) {
                SLIST_REMOVE(&cache->slabs_full, slab, slabs_list);
        } else {
                SLIST_REMOVE(&cache->slabs_partial, slab, slabs_list);
        }

        page_free(slab->page);
}

///
/// Reclaims required number of free pages from all system caches.
///
/// @return The number of reclaimed pages. May be less or more than required.
static unsigned caches_try_reclaim(unsigned reclaim)
{
        unsigned reclaimed = 0;
        struct kmm_cache *itc;
        SLIST_FOREACH (itc, &ALLOCATED_CACHES, caches) {
                // Can't use SLIST_FOREACH because it would use a slab after freeing.
                struct kmm_slab *next;
                struct kmm_slab *current = SLIST_FIRST(&itc->slabs_empty);
                while (current != NULL) {
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

        return (reclaimed);
}

///
/// Allocates a new page from VMM.
/// If the allocation fails, it will try to reclaim some memory from other caches.
/// If the allocation fails again and the cache is capable of using static storage,
/// it will get memory from static memory pool as a last resort.
///
static struct page *page_alloc(struct kmm_cache *cache)
{
        kassert(cache);

        const size_t to_allocate = 1;
        struct vm_mapping *mapping = vmm_alloc_pages(&kvm_space, to_allocate);
        if (mapping == NULL) {
                LOGF_W("System is running out of memory. Trying to reclaim some...");
                if (caches_try_reclaim(to_allocate) >= to_allocate) {
                        mapping = vmm_alloc_pages(&kvm_space, to_allocate);
                        if (mapping == NULL) {
                                LOGF_E("Can't allocate new page, but we've successfully reclaimed some.");
                        }
                } else {
                        LOGF_W("Failed to reclaim required amount of memory.");
                }
        }

        struct page *page = mapping != NULL ? mapping->start : NULL;

        if (__unlikely(page == NULL)) {
                if (cache->flags & KMM_CACHE_STATIC) {
                        LOGF_W("Couldn't allocate a page from dynamic memory for %s cache. Trying "
                               "static...",
                               cache->name);

                        page = mem_pool_alloc(&STATIC_PAGE_POOL);
                        if (__unlikely(page == NULL)) {
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

        kassert(ptr2uint(page) == align_roundup(ptr2uint(page), PLATFORM_PAGE_SIZE));

        return (page);
}

///
/// Returns page structure that contains an address.
///
static struct page *page_get_by_addr(void *addr)
{
        uintptr_t p = ptr2uint(addr);
        p &= -PLATFORM_PAGE_SIZE;
        return (uint2ptr(p));
}

///
/// Returns slab that owns an address.
///
static struct kmm_slab *slab_get_by_addr(void *addr)
{
        return (page_get_by_addr(addr)->owner);
}

static struct kmm_slab *slab_create_small(struct kmm_cache *cache, unsigned colour)
{
        kassert(cache);

        struct page *page = page_alloc(cache);
        if (__unlikely(page == NULL)) {
                return (NULL);
        }

        union uiptr cursor = ptr2uiptr(page->data);

        struct kmm_slab *slab = cursor.ptr;
        cursor.num += sizeof(*slab);
        cursor.num += colour;
        cursor.num = align_roundup(cursor.num, cache->alignment);

        page->owner = slab;

        slab->objects_inuse = 0;
        slab->page = page;
        SLIST_FIELD_INIT(slab, slabs_list);
        SLIST_INIT(&slab->free_buffers);

        // Check that leftover space is less than a full stride.
        kassert((ptr2uint(page) + PLATFORM_PAGE_SIZE) -
                        (cursor.num + cache->slab_capacity * cache->stride) <
                cache->stride);
        for (int i = 0; i < cache->slab_capacity; i++, cursor.num += cache->stride) {
                union uiptr ctl_mem = ptr2uiptr(get_bufctl_small(cursor.num, cache));
                kassert(ctl_mem.num < cursor.num + cache->stride);

                struct bufctl_small *ctl = ctl_mem.ptr;
                SLIST_FIELD_INIT(ctl, slist);
                SLIST_INSERT_HEAD(&slab->free_buffers, (union bufctl *)ctl, slist);

                void *mem = bufctl_small_get_mem(ctl, cache);
                if (cache->ctor) {
                        cache->ctor(mem);
                }
        }

        return (slab);
}

static struct kmm_slab *slab_create_large(struct kmm_cache *cache, unsigned colour)
{
        kassert(cache);

        struct kmm_slab *slab = kmm_cache_alloc(&CACHES.slabs);
        if (__unlikely(!slab)) {
                return (NULL);
        }

        struct page *page = page_alloc(cache);
        if (__unlikely(!page)) {
                goto clean_slab;
        }
        page->owner = slab;

        uintptr_t obj_addr = ptr2uint(page->data);
        obj_addr += colour;
        obj_addr = align_roundup(obj_addr, cache->alignment);

        slab->objects_inuse = 0;
        slab->page = page;
        SLIST_FIELD_INIT(slab, slabs_list);
        SLIST_INIT(&slab->free_buffers);

        for (int i = 0; i < cache->slab_capacity; i++, obj_addr += cache->stride) {
                struct bufctl_large *ctl = kmm_cache_alloc(&CACHES.large_bufctls);
                SLIST_FIELD_INIT(ctl, slist);
                SLIST_INSERT_HEAD(&slab->free_buffers, (union bufctl *)ctl, slist);

                void *obj = uint2ptr(obj_addr);
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
                ctl = get_bufctl_small(ptr2uint(mem), cache);
        }
        SLIST_INSERT_HEAD(&slab->free_buffers, ctl, slist);
}

static void *object_alloc(struct kmm_slab *slab, struct kmm_cache *cache)
{
        kassert(slab);
        kassert(!SLIST_EMPTY(&slab->free_buffers));

        union bufctl *free_ctl = SLIST_FIRST(&slab->free_buffers);
        SLIST_REMOVE(&slab->free_buffers, free_ctl, slist);
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

        // struct page is always stored at the beginning of the page.
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

static void cache_init(struct kmm_cache *restrict cache, const char *name, size_t size,
                       size_t align, unsigned flags, void (*ctor)(void *), void (*dtor)(void *))
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

        cache->stride = align_roundup(obj_space, cache->alignment);
        cache->slab_capacity = cache_get_capacity(cache);

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
        kassert(cache);

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
        kassert(cache);

        struct kmm_slab *slab = NULL;
        if (!SLIST_EMPTY(&cache->slabs_partial)) {
                slab = SLIST_FIRST(&cache->slabs_partial);

                bool becomes_full = slab->objects_inuse + 1 == cache->slab_capacity;
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
