#ifndef _KERNEL_UTILS_H
#define _KERNEL_UTILS_H

#include "kernel/cppdefs.h"

#include "lib/assert.h"

#include <stdbool.h>

/**
 * @brief Return the nearest address that is bigger than the address and fit the alignment.
 */
static inline uintptr_t align_roundup(uintptr_t from, uintptr_t alignment)
{
        kassert(alignment > 0);

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

#endif // _KERNEL_UTILS_H
