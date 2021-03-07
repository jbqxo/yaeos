#ifndef _LIB_UTILS_H
#define _LIB_UTILS_H

#include "lib/align.h"
#include "lib/cppdefs.h"
#include "lib/cstd/assert.h"

#include <stdint.h>

#define TO_SSTR(X)       #X
#define TO_SSTR_MACRO(X) TO_SSTR(X)

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#ifndef NDEBUG
#define __container_of_check_alignment(result) kassert(properly_aligned(result))
#else
#define __container_of_check_alignment(result)
#endif

#define container_of(ptr, type, field_name)                                                \
        ({                                                                                 \
                void *__fptr = (void *)(ptr);                                              \
                type *__result = (type *)((uintptr_t)__fptr - offsetof(type, field_name)); \
                __container_of_check_alignment(__result);                                  \
                __result;                                                                  \
        })

static inline uint8_t log2_floor(uint32_t x)
{
        kassert(x != 0);
        return (8 * sizeof(x) - 1 - (uint8_t)__builtin_clzl(x));
}

static inline uint8_t log2_ceil(uint32_t x)
{
        /* TODO: Implement branch-less log2. */
        if (x <= 1) {
                return (0);
        }
        return (log2_floor(x - 1) + 1);
}

__const static inline uint32_t div_ceil(uint32_t x, uint32_t y)
{
        return ((x + y - 1) / y);
}

__const static inline uint32_t div_near(uint32_t x, uint32_t y)
{
        return ((x + y / 2) / y);
}

/* Find First One bit */
__const static inline unsigned find_first_one(unsigned x)
{
        unsigned ndx = 0;

        /* Turn all of the trailing 0s to 1s. Zero the rest of a word. */
        x = (x ^ (x - 1)) >> 1;

        /* Count leading 1s. */
        for (ndx = 0; x != 0; ndx++) {
                x >>= 1;
        }
        return (ndx);
}

__const static inline unsigned find_first_zero(unsigned x)
{
        return (find_first_one(~x));
}

#define MAX(x, y)                                          \
        ({                                                 \
                typeof(x) __v1 = (x);                      \
                typeof(y) __v2 = (y);                      \
                (typeof(__v1))(__v1 < __v2 ? __v2 : __v1); \
        })
#define MIN(x, y)                                          \
        ({                                                 \
                typeof(x) __v1 = (x);                      \
                typeof(y) __v2 = (y);                      \
                (typeof(__v1))(__v1 > __v2 ? __v2 : __v1); \
        })

#endif /* _LIB_UTILS_H */
