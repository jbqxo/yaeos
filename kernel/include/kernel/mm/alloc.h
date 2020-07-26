#ifndef _KERNEL_MM_ALLOC_H
#define _KERNEL_MM_ALLOC_H

#include <stddef.h>
#include <stdint.h>

/**
 * @brief Somewhat space and speed efficient page frame allocator.
 *
 * Don't quote this comment on the part of space and speed efficiency.
 */
struct buddy_allocator {
	uintptr_t internp; /**< Address of allocator's control information. Page aligned. */
	size_t intern_sz; /**< Length of the allocator's control information. */
	uintptr_t internp_lim; /**< Last address that we can use for our purposes. */

	struct chunk *chunks; /**< Pointer at the memory chunks' structures. */
	unsigned chunks_num;
	unsigned max_lvl;
};

/**
 * @brief Initialize the buddy allocator.
 * @param alloc Buddy allocator structure.
 * @param mem_chunks Array of memory chunks to manage.
 * @param sizes Array of sizes of those chunks.
 * @param chnum Number of the chunks.
 * @param intern_data Pointer to a block of memory where the allocator's data can be stored.
 * @param intern_len Length of the memory for the allocator's data.
 */
void buddy_init(struct buddy_allocator *alloc, void **mem_chunks, const size_t *sizes, unsigned chnum,
				   void *intern_data, size_t intern_len);

/**
 * @brief Allocate specified number of pages.
 * @param order 2^(order) of required pages.
 * @note Return pointer is page aligned.
 * @return Pointer at the beginning of the allocated space.
 */
void *buddy_alloc(struct buddy_allocator *allocator, unsigned order);

/**
 * @brief Free specified memory space.
 * @param mem Pointer to the space to free.
 * @param order 2^(order) of pages that was requested.
 * @note It's important to specify the same number of pages that was received from the allocator.
 * Bad things will happen otherwise.
 */
void buddy_free(struct buddy_allocator *allocator, void *mem, unsigned order);


#endif // _KERNEL_MM_ALLOC_H
