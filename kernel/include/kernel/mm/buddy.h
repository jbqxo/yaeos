#ifndef _KERNEL_MM_BUDDY_H
#define _KERNEL_MM_BUDDY_H

#include "kernel/ds/bitmap.h"

#include <stddef.h>
#include <stdint.h>

struct buddy_mem {
        void *source_zone;
        int page_ndx; /**< Index inside of the */
};

/**
 * @brief Buddy allocator.
 */
struct buddy_manager {
        uintptr_t internp;     /**< Address of allocator's control information. Page aligned. */
        size_t intern_sz;      /**< Length of the allocator's control information. */
        uintptr_t internp_lim; /**< Last address that we can use for our purposes. */

        struct bitmap *lvl_bitmaps; /**< Each buddy level has it's own bitmap. */
        size_t lvls;
};

/**
 * @brief Initialize the buddy allocator.
 * @param alloc Buddy allocator structure.
 * @param sizes Array of sizes of those chunks.
 * @param intern_data Pointer to a block of memory where the allocator's data can be stored.
 * @param intern_len Length of the memory for the allocator's data.
 * @return Number of free pages.
 */
uint32_t buddy_init(struct buddy_manager *bmgr, size_t size, void *intern_data, size_t intern_len);

/**
 * @brief Allocate specified number of pages.
 * @param order 2^(order) of required pages.
 * @param result An index of the allocated page/pages.
 * @return Indicates success of the operation.
 */
bool buddy_alloc(struct buddy_manager *bmgr, unsigned order, uint32_t *result);

/**
 * @brief Free specified memory space.
 * @param page_ndx Index of the allocated page to free.
 * @param order 2^(order) of pages that was requested.
 */
void buddy_free(struct buddy_manager *bmgr, uint32_t page_ndx, unsigned order);

#endif /* _KERNEL_MM_BUDDY_H */
