#ifndef _KERNEL_DS_BITMAP_H
#define _KERNEL_DS_BITMAP_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define BITMAP_WORD_TYPE uint32_t

struct bitmap {
        BITMAP_WORD_TYPE *bitsets;
        size_t sets_count; /**< This one specifies the number of BITMAP_WORD_TYPE objects. */
        size_t length;     /**< This one specifies the number of bits in the bitmap. */
};

void bitmap_init(struct bitmap *bitmap, void *space, size_t length);

bool bitmap_get(struct bitmap *bitmap, uint32_t index);

void bitmap_set_false(struct bitmap *bitmap, uint32_t index);

void bitmap_set_true(struct bitmap *bitmap, uint32_t index);

bool bitmap_search_false(struct bitmap *bitmap, uint32_t *result);

#endif /* _KERNEL_DS_BITMAP_H */
