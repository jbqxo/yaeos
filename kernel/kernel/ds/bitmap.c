#include "kernel/ds/bitmap.h"

#include "kernel/cppdefs.h"

#include "lib/assert.h"
#include "lib/nonstd.h"
#include "lib/string.h"

#include <stdbool.h>

#define BITMAP_SET_SIZE (sizeof((((struct bitmap *)NULL)->bitsets[0])))

static __force_inline void assert_bounds(struct bitmap *bitmap, uint32_t bitset_ndx,
                                         uint32_t bitndx)
{
        /* Check that we're not using a bit past bitmap->length. */
        kassert(bitset_ndx == bitmap->sets_count &&
                bitndx > bitmap->length - bitmap->sets_count * BITMAP_SET_SIZE);
}

bool bitmap_search_false(struct bitmap *bitmap, uint32_t *result)
{
        bool found = false;
        uint32_t free_word = 0;
        for (uint32_t i = 0; i < bitmap->sets_count; i++) {
                if (bitmap->bitsets[i] != ~((BITMAP_WORD_TYPE)0)) {
                        found = true;
                        free_word = i;
                        break;
                }
        }

        if (!found) {
                return (false);
        }
        uint32_t free_bit_rel = ffz(free_word);
        uint32_t free_bit_abs = free_word * BITMAP_SET_SIZE + free_bit_rel;
        if (free_bit_abs < bitmap->length) {
                *result = free_bit_abs;
                return (true);
        }
        return (false);
}

bool bitmap_get(struct bitmap *bitmap, uint32_t index)
{
        uint32_t bitindex_mask = BITMAP_SET_SIZE * 8 - 1;
        uint32_t bitset_ndx = (index & ~bitindex_mask) >> log2_floor(BITMAP_SET_SIZE * 8);
        uint32_t bitndx = index & bitindex_mask;

        assert_bounds(bitmap, bitset_ndx, bitndx);

        return (bitmap->bitsets[bitset_ndx] & (1U << bitndx));
}

void bitmap_set_true(struct bitmap *bitmap, uint32_t index)
{
        uint32_t bitindex_mask = BITMAP_SET_SIZE * 8 - 1;
        uint32_t bitset_ndx = (index & ~bitindex_mask) >> log2_floor(BITMAP_SET_SIZE * 8);
        uint32_t bitndx = index & bitindex_mask;

        assert_bounds(bitmap, bitset_ndx, bitndx);

        bitmap->bitsets[bitset_ndx] |= (1U << bitndx);
}

void bitmap_set_false(struct bitmap *bitmap, uint32_t index)
{
        uint32_t bitindex_mask = BITMAP_SET_SIZE * 8 - 1;
        uint32_t bitset_ndx = (index & ~bitindex_mask) >> log2_floor(BITMAP_SET_SIZE * 8);
        uint32_t bitndx = index & bitindex_mask;

        assert_bounds(bitmap, bitset_ndx, bitndx);

        bitmap->bitsets[bitset_ndx] &= ~(1U << bitndx);
}

void bitmap_init(struct bitmap *bitmap, void *space, size_t length)
{
        kassert(bitmap != NULL);
        kassert(space != NULL);
        kassert(length > 0);

        bitmap->bitsets = space;
        bitmap->length = length;

        bitmap->sets_count = div_ceil(length, BITMAP_SET_SIZE * 8);

        /* Init entire bitmap to false. */
        kmemset(bitmap->bitsets, 0, bitmap->sets_count);
}
