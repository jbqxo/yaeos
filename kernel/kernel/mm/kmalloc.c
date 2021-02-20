#include "kernel/mm/kmalloc.h"

#include "kernel/config.h"
#include "kernel/mm/kmm.h"

#include "lib/align.h"
#include "lib/cppdefs.h"
#include "lib/cstd/assert.h"
#include "lib/cstd/nonstd.h"
#include "lib/klog.h"

#include <stdalign.h>

static struct kmm_cache *KMALLOC_CACHES[CONF_MALLOC_MAX_POW - CONF_MALLOC_MIN_POW + 1];

/* We store index of cache level along with every allocated piece of memory. */
struct kmalloc_ident_info {
        uint8_t cache_index;
};
kstatic_assert(sizeof(struct kmalloc_ident_info) <= alignof(max_align_t),
               "kmalloc_ident_info is biggen than alignof(max_align_t)");
kstatic_assert((uint8_t)(~0) >= ARRAY_SIZE(KMALLOC_CACHES),
               "KMALLOC_CACHES length exceeds maximum length");

void kmalloc_init(void)
{
        const char *const heap_names[13] = {
                "kmalloc_1",   "kmalloc_2",   "kmalloc_4",    "kmalloc_8",
                "kmalloc_16",  "kmalloc_32",  "kmalloc_64",   "kmalloc_128",
                "kmalloc_256", "kmalloc_512", "kmalloc_1024", "kmalloc_2048",
        };

        kstatic_assert(ARRAY_SIZE(heap_names) >= ARRAY_SIZE(KMALLOC_CACHES),
                       "Some kmalloc caches don't have names");
        kstatic_assert(ARRAY_SIZE(heap_names) >= CONF_MALLOC_MAX_POW, "Missing heap names");

        for (int i = 0; i < ARRAY_SIZE(KMALLOC_CACHES); i++) {
                const char *heap_name = heap_names[i + CONF_MALLOC_MIN_POW];

                size_t obj_size = sizeof(struct kmalloc_ident_info);
                obj_size = align_roundup(obj_size, alignof(max_align_t));
                obj_size += (1 << (i + CONF_MALLOC_MIN_POW));

                KMALLOC_CACHES[i] =
                        kmm_cache_create(heap_name, obj_size, alignof(max_align_t), 0, NULL, NULL);
        }
}

void *kmalloc(size_t size)
{
        size_t const req_space = size + sizeof(struct kmalloc_ident_info);
        int pow = log2_ceil(req_space);
        const int cache_index = pow - (CONF_MALLOC_MIN_POW + 1);

        if (cache_index >= ARRAY_SIZE(KMALLOC_CACHES)) {
                const size_t max_available = 1 << CONF_MALLOC_MAX_POW;
                LOGF_E("Requested too much memory from kmalloc. Requested: %zu Max available: %zu",
                       req_space, max_available);
                return (NULL);
        }
        kassert(cache_index >= 0);

        struct kmm_cache *c = KMALLOC_CACHES[cache_index];
        kassert(c != NULL);

        struct kmalloc_ident_info *info = kmm_cache_alloc(c);
        kassert(check_align(ptr2uint(info), alignof(max_align_t)));
        info->cache_index = cache_index;

        void *mem = uint2ptr(align_roundup(ptr2uint(info) + sizeof(*info), alignof(max_align_t)));

        return (mem);
}

void kfree(void *mem)
{
        union uiptr const m = ptr2uiptr(mem);

        kassert(m.ptr != NULL);
        kassert(check_align(m.num, alignof(max_align_t)));

        struct kmalloc_ident_info *info =
                uint2ptr(align_rounddown(m.num - 1, alignof(max_align_t)));
        uint8_t cache_index = info->cache_index;
        kassert(cache_index < ARRAY_SIZE(KMALLOC_CACHES));

        kmm_cache_free(KMALLOC_CACHES[cache_index], info);
}
