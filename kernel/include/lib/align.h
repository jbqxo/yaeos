#ifndef _LIB_ALIGN_H
#define _LIB_ALIGN_H

#include "lib/cppdefs.h"
#include "lib/cstd/assert.h"

#include <stdalign.h>
#include <stdbool.h>

/**
 * @brief Return the nearest address that is bigger than the address and fit the alignment.
 */
static inline uintptr_t align_roundup(uintptr_t from, size_t alignment)
{
        if (alignment == 0) {
                return (from);
        }

        from += alignment - 1;
        from &= -alignment;
        return (from);
}

#define align_roundupptr(FROM, ALIGN) (void *)align_roundup((uintptr_t)(FROM), (ALIGN))

/**
 * @brief Return the nearest address that is smaller than the address and fit the alignment.
 */
static inline uintptr_t align_rounddown(uintptr_t from, size_t alignment)
{
        if (alignment == 0) {
                return (from);
        }

        from &= -alignment;
        return (from);
}

#define align_rounddownptr(FROM, ALIGN) (void *)align_rounddown((uintptr_t)(FROM), (ALIGN))

static inline bool check_align(uintptr_t value, uintptr_t alignment)
{
        return (value == align_rounddown(value, alignment));
}

#define properly_aligned(EXP) (check_align((uintptr_t)(EXP), alignof(__typeof(*(EXP)))))

#endif /* _LIB_ALIGN_H */
