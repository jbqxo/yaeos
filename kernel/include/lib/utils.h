#ifndef _LIB_UTILS_H
#define _LIB_UTILS_H

#include "lib/cstd/assert.h"
#include "lib/align.h"

#define TO_SSTR(X)       #X
#define TO_SSTR_MACRO(X) TO_SSTR(X)

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#ifndef NDEBUG
#define __container_of_check_alignment(result) kassert(properly_aligned(result))
#else
#define __container_of_check_alignment(result)
#endif

/* Shamelessly took the idea from the Linux. It's so cool actually...  */
#define container_of(ptr, type, field_name)                                                \
        ({                                                                                 \
                void *__fptr = (void *)(ptr);                                              \
                type *__result = (type *)((uintptr_t)__fptr - offsetof(type, field_name)); \
                __container_of_check_alignment(__result);                                  \
                __result;                                                                  \
        })

#endif /* _LIB_UTILS_H */
