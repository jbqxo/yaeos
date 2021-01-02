#ifndef _KERNEL_UTILS_H
#define _KERNEL_UTILS_H

#include "kernel/cppdefs.h"

#include <stdbool.h>

/**
 * @brief Return the nearest address that is bigger than the address and fit the alignment.
 */
static inline uintptr_t align_roundup(uintptr_t from, uintptr_t alignment)
{
        if (alignment == 0) {
                return (from);
        }
        from += alignment - 1;
        from &= -alignment;
        return (from);
}

/**
 * @brief Return the nearest address that is smaller than the address and fit the alignment.
 */
static inline uintptr_t align_rounddown(uintptr_t from, uintptr_t alignment)
{
        from &= -alignment;
        return (from);
}

static inline bool check_align(uintptr_t value, uintptr_t alignment)
{
        return (value == align_rounddown(value, alignment));
}

#define MAX(x, y) ((x) < (y) ? (y) : (x))
#define MIN(x, y) ((x) > (y) ? (y) : (x))
#define LOG2(x)                                     \
        (8 * sizeof(x) - 1 -                        \
         _Generic((x), unsigned int                 \
                  : __builtin_clz(x), unsigned long \
                  : __builtin_clzl(x)))

#endif // _KERNEL_UTILS_H
