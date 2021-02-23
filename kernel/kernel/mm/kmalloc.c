#include "kernel/mm/kmalloc.h"

#include "kernel/klog.h"
#include "kernel/mm/kmm.h"

#include "lib/align.h"
#include "lib/cppdefs.h"
#include "lib/cstd/assert.h"
#include "lib/cstd/nonstd.h"

#include <stdalign.h>

#define KMALLOC_MAX_CACHES (7)

static struct kmm_cache *KMALLOC_CACHES[KMALLOC_MAX_CACHES];

static uint32_t LOW_POW = 0;
static uint32_t MAX_POW = 0;
#define POWERS (MAX_POW - LOW_POW + 1)

/* We store index of cache level along with every allocated piece of memory. */
struct kmalloc_ident_info {
        uint8_t cache_index;
};
kstatic_assert(sizeof(struct kmalloc_ident_info) <= alignof(max_align_t),
               "kmalloc_ident_info is biggen than alignof(max_align_t)");
kstatic_assert((uint8_t)(~0) >= ARRAY_SIZE(KMALLOC_CACHES),
               "KMALLOC_CACHES length exceeds maximum length");

void kmalloc_init(unsigned lowest_pow2_size, unsigned max_pow2_size)
{
        LOW_POW = lowest_pow2_size;
        MAX_POW = max_pow2_size;

        kassert(max_pow2_size > lowest_pow2_size);
        kassert(POWERS <= KMALLOC_MAX_CACHES);

        const char *const heap_names[] = {
                "kmalloc_1",   "kmalloc_2",   "kmalloc_4",    "kmalloc_8",
                "kmalloc_16",  "kmalloc_32",  "kmalloc_64",   "kmalloc_128",
                "kmalloc_256", "kmalloc_512", "kmalloc_1024", "kmalloc_2048",
        };
        kassert(ARRAY_SIZE(heap_names) >= max_pow2_size);

        for (int i = 0; i < POWERS; i++) {
                const char *heap_name = heap_names[i + lowest_pow2_size];

                size_t obj_size = sizeof(struct kmalloc_ident_info);
                obj_size = align_roundup(obj_size, alignof(max_align_t));
                obj_size += (1 << (i + lowest_pow2_size));

                KMALLOC_CACHES[i] =
                        kmm_cache_create(heap_name, obj_size, alignof(max_align_t), 0, NULL, NULL);
        }
}

void *kmalloc(size_t size)
{
        size_t const req_space = size + sizeof(struct kmalloc_ident_info);
        int pow = log2_ceil(req_space);
        unsigned const cache_index = pow - (LOW_POW + 1);

        if (cache_index >= POWERS) {
                const size_t max_available = 1 << MAX_POW;
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
        kassert(cache_index < POWERS);

        kmm_cache_free(KMALLOC_CACHES[cache_index], info);
}
