#ifndef _LIB_NONSTD_H
#define _LIB_NONSTD_H

#include "kernel/cppdefs.h"

#include <stdint.h>

#define __force_inline __attribute__((always_inline))

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

static uint32_t div_ceil(uint32_t x, uint32_t y)
{
        return ((x + y - 1) / y);
}

static uint32_t div_near(uint32_t x, uint32_t y)
{
        return ((x + y / 2) / y);
}

/* Find First Zero bit
 * Beware, couldn't tell the difference between 0xFFFFFFFF and 0x7FFFFFFF */
static uint32_t ffz(uint32_t x)
{
        if (x == 0) {
                return (0);
        }
        uint32_t result = 1;

        if ((x & 0xFFFF) == 0xFFFF) {
                result += 16;
                x >>= 16;
        }

        if ((x & 0xFF) == 0xFF) {
                result += 8;
                x >>= 8;
        }

        if ((x & 0xF) == 0xF) {
                result += 4;
                x >>= 4 ;
        }

        if ((x & 0x3) == 0x3) {
                result += 2;
                x >>= 2;
        }

        if ((x & 0x1) == 0x1) {
                result += 1;
                x >>= 1;
        }

        return (result);
}

#define MAX(x, y) ((x) < (y) ? (y) : (x))
#define MIN(x, y) ((x) > (y) ? (y) : (x))

#endif /* _LIB_NONSTD_H */
