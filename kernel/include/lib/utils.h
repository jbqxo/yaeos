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

/* TODO: Implement branch-less log2. */
#define log2_floor(X)                                                                 \
        ({                                                                            \
                __typeof(X) __l2fl_xvar = (X);                                        \
                kassert(0 != __l2fl_xvar);                                            \
                (8 * sizeof(__l2fl_xvar) - 1 - (uint8_t)__builtin_clzl(__l2fl_xvar)); \
        })

#define log2_ceil(X)                                                          \
        ({                                                                    \
                __typeof(X) __l2cl_xvar = (X);                                \
                (__l2cl_xvar <= 1) ? (0) : (log2_floor(__l2cl_xvar - 1) + 1); \
        })

#define div_ceil(X, Y)                                         \
        ({                                                     \
                __typeof(X) __divc_xvar = (X);                 \
                __typeof(Y) __divc_yvar = (Y);                 \
                (__divc_xvar + __divc_yvar - 1) / __divc_yvar; \
        })

#define div_near(X, Y)                                         \
        ({                                                     \
                __typeof(X) __divn_xvar = (X);                 \
                __typeof(Y) __divn_yvar = (Y);                 \
                (__divn_xvar + __divn_yvar / 2) / __divn_yvar; \
        })

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
