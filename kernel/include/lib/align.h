#ifndef _LIB_ALIGN_H
#define _LIB_ALIGN_H

#include "lib/cppdefs.h"
#include "lib/cstd/assert.h"

#include <stdalign.h>
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
        if (alignment == 0) {
                return (from);
        }

        from &= -alignment;
        return (from);
}

static inline bool check_align(uintptr_t value, uintptr_t alignment)
{
        return (value == align_rounddown(value, alignment));
}

#define properly_aligned(EXP) (check_align((uintptr_t)(EXP), alignof(__typeof(*(EXP)))))

#endif /* _LIB_ALIGN_H */
