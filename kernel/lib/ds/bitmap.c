#include "lib/ds/bitmap.h"

#include "lib/align.h"
#include "lib/cppdefs.h"
#include "lib/cstd/assert.h"
#include "lib/cstd/string.h"
#include "lib/utils.h"

#include <stdbool.h>

#define BITS_IN_SET (sizeof((((struct bitmap *)NULL)->bitsets[0])) * 8)

static inline void assert_bounds(struct bitmap *bitmap __maybe_unused,
                                 uint32_t bitset_ndx __maybe_unused, uint32_t bitndx __maybe_unused)
{
        /* Check that we're not using a bit past bitmap->length. */
        kassert(bitset_ndx * BITS_IN_SET + bitndx < bitmap->length);
}

bool bitmap_search_false(struct bitmap *bitmap, uint32_t *result)
{
        bool found = false;
        uint32_t word_ndx = 0;
        for (uint32_t i = 0; i < bitmap->sets_count; i++) {
                if (bitmap->bitsets[i] != ~((BITMAP_WORD_TYPE)0)) {
                        found = true;
                        word_ndx = i;
                        break;
                }
        }

        if (!found) {
                return (false);
        }
        uint32_t bit_rel_ndx = find_first_zero(bitmap->bitsets[word_ndx]);
        uint32_t bit_abs_ndx = word_ndx * BITS_IN_SET + bit_rel_ndx;
        if (bit_abs_ndx < bitmap->length) {
                *result = bit_abs_ndx;
                return (true);
        }
        return (false);
}

static void get_indices(uint32_t index, uint32_t *set_ndx, uint32_t *bit_ndx)
{
        *set_ndx = index / BITS_IN_SET;
        *bit_ndx = index % BITS_IN_SET;
}

bool bitmap_get(struct bitmap *bitmap, uint32_t index)
{
        uint32_t bitset_ndx = 0;
        uint32_t bitndx = 0;
        get_indices(index, &bitset_ndx, &bitndx);

        assert_bounds(bitmap, bitset_ndx, bitndx);

        return (bitmap->bitsets[bitset_ndx] & (1U << bitndx));
}

void bitmap_set_true(struct bitmap *bitmap, uint32_t index)
{
        uint32_t bitset_ndx = 0;
        uint32_t bitndx = 0;
        get_indices(index, &bitset_ndx, &bitndx);

        assert_bounds(bitmap, bitset_ndx, bitndx);

        bitmap->bitsets[bitset_ndx] |= (1U << bitndx);
}

void bitmap_set_false(struct bitmap *bitmap, uint32_t index)
{
        uint32_t bitset_ndx = 0;
        uint32_t bitndx = 0;
        get_indices(index, &bitset_ndx, &bitndx);

        assert_bounds(bitmap, bitset_ndx, bitndx);

        bitmap->bitsets[bitset_ndx] &= ~(1U << bitndx);
}

size_t bitmap_predict_size(uint32_t length_bits)
{
        kassert(length_bits > 0);
        return (align_roundup(length_bits, BITS_IN_SET) / 8);
}

void bitmap_init(struct bitmap *bitmap, void *space, size_t length_bits)
{
        kassert(bitmap != NULL);
        kassert(space != NULL);
        kassert(length_bits > 0);

        bitmap->bitsets = space;
        bitmap->length = length_bits;

        bitmap->sets_count = div_ceil(length_bits, BITS_IN_SET);

        /* Init entire bitmap to false. */
        kmemset(bitmap->bitsets, 0, bitmap->sets_count * sizeof(*bitmap->bitsets));
}

void bitmap_resize(struct bitmap *bitmap, size_t new_length_bits)
{
        kassert(bitmap != NULL);
        kassert(new_length_bits > 0);

        size_t old_sets_count = bitmap->sets_count;

        bitmap->length = new_length_bits;
        bitmap->sets_count = div_ceil(new_length_bits, BITS_IN_SET);

        size_t sets_count_diff = bitmap->sets_count - old_sets_count;
        if (sets_count_diff > 0) {
                kmemset(bitmap->bitsets + old_sets_count, 0x00, sets_count_diff);
        }
}
