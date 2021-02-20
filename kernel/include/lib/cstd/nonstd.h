#ifndef _LIB_NONSTD_H
#define _LIB_NONSTD_H

#include "lib/cppdefs.h"

#include <stdint.h>

static __force_inline uint32_t log2_floor(uint32_t x)
{
        return (8 * sizeof(x) - 1 - __builtin_clzl(x));
}

static __force_inline uint32_t log2_ceil(uint32_t x)
{
        // TODO: Implement branch-less log2.
        if (x <= 1) {
                return (0);
        }
        return (log2_floor(x - 1) + 1);
}

static __force_inline uint32_t div_ceil(uint32_t x, uint32_t y)
{
        return ((x + y - 1) / y);
}

static __force_inline uint32_t div_near(uint32_t x, uint32_t y)
{
        return ((x + y / 2) / y);
}

/* Find First One bit */
static __force_inline int find_first_one(unsigned x)
{
        int ndx = 0;

        /* Turn all of the trailing 0s to 1s. Zero the rest of a word. */
        x = (x ^ (x - 1)) >> 1;

        /* Count leading 1s. */
        for (ndx = 0; x != 0; ndx++) {
                x >>= 1;
        }
        return (ndx);
}

static __force_inline int find_first_zero(unsigned x)
{
        return (find_first_one(~x));
}

#define MAX(x, y) ((x) < (y) ? (y) : (x))
#define MIN(x, y) ((x) > (y) ? (y) : (x))

#endif /* _LIB_NONSTD_H */
