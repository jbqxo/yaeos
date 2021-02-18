#ifndef _KERNEL_MM_BUDDY_H
#define _KERNEL_MM_BUDDY_H

#include "kernel/mm/linear.h"

#include "lib/ds/bitmap.h"

#include <stddef.h>
#include <stdint.h>

/**
 * @brief Buddy allocator.
 */
struct buddy_manager {
        struct linear_alloc *alloc;
        struct bitmap *lvl_bitmaps; /**< Each buddy level has it's own bitmap. */
        size_t lvls;
};

/**
 * @brief Initialize the buddy allocator.
 * @param pages Number of pages to manage.
 * @param alloc Allocator to use for internal data.
 * @return Number of free pages.
 */
uint32_t buddy_init(struct buddy_manager *bmgr, size_t pages, struct linear_alloc *alloc);

/**
 * @brief Allocate specified number of pages.
 * @param order 2^(order) of required pages.
 * @param result An index of the allocated page/pages.
 * @return Indicates success of the operation.
 */
bool buddy_alloc(struct buddy_manager *bmgr, unsigned order, uint32_t *result);

/**
 * @brief Try to allocate specified page.
 * @param page_ndx Index of the page to allocate.
 * @return Indicates success of the operation. */
bool buddy_try_alloc(struct buddy_manager *bmgr, uint32_t page_ndx);

/**
 * @brief Free specified memory space.
 * @param page_ndx Index of the allocated page to free.
 * @param order 2^(order) of pages that was requested.
 */
void buddy_free(struct buddy_manager *bmgr, uint32_t page_ndx, unsigned order);

bool buddy_is_free(struct buddy_manager *bmgr, uint32_t page_ndx);

/**
 * @brief Predict space required by a buddy manager for a specified number of pages.
 */
size_t buddy_predict_req_space(size_t pages);

#endif /* _KERNEL_MM_BUDDY_H */
