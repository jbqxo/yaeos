#ifndef _KERNEL_MM_H
#define _KERNEL_MM_H

#include <stddef.h>
#include <stdint.h>

/**
 * @brief Somewhat space and speed efficient page frame allocator.
 *
 * Don't quote this comment on the part of space and speed efficiency.
 */
struct buddy_allocator {
	uintptr_t data; /**< Address of allocator's control information. Page aligned. */
	size_t data_size; /**< Length of the allocator's control information. */
	uintptr_t data_limit; /**< Last address that we can use for our purposes. */

	struct chunk *chunks; /**< Pointer at the memory chunks' structures. */
	unsigned chunks_count; /**< The number of memory chunks managed. */
	unsigned levels; /**< The number of buddies' levels. */
/**
 * The memory given to the allocator was taken from the low memory region.
 * It will perform then simple address translation during initialization.
 */
#define BUDDY_LOWMEM (0x1)
	int flags;
};

/**
 * @brief Initialize the buddy allocator.
 * @param a Buddy allocator structure.
 * @param mem_chunks Array of memory chunks to manage.
 * @param sizes Array of sizes of those chunks.
 * @param count Number of the chunks.
 * @return Pointer on the allocator's structure. NULL if there was not enough memory.
 * @warning Some part of the last memory chunk will be used for allocator's purposes.
 * Therefore it would be good idea to put the largest memory chunk at the end.
 */
struct buddy_allocator *buddy_init(void **mem_chunks, const size_t *sizes, unsigned count,
				   int flags);

/**
 * @brief Allocate specified number of pages.
 * @param pages Number of required pages.
 * @note Return pointer is page aligned.
 * @return Pointer at the beginning of the allocated space.
 */
void *buddy_alloc(struct buddy_allocator *allocator, unsigned pages);

/**
 * @brief Free specified memory space.
 * @param mem Pointer to the space to free.
 * @param pages Number of pages that was requested.
 * @note It's important to specify the same number of pages that was received from the allocator.
 * Bad things will happen otherwise.
 */
void buddy_free(struct buddy_allocator *allocator, void *mem, unsigned pages);

struct mem_chunk {
	void *mem;
	size_t length;
#define MEM_TYPE_AVAIL (0x1)
#define MEM_TYPE_RESERVED (0x2)
#define MEM_TYPE_ACPI (0x3)
#define MEM_TYPE_HIBER (0x4)
#define MEM_TYPE_UNAVAIL (0x5)
	uint8_t type;
};

#define PAGE_FLAG_KERNEL (0x1)
#define PAGE_FLAG_USER (0x1)
typedef unsigned pageflag;

struct page {
	uintptr_t virtual;
	uintptr_t physical;
	pageflag flags;
};

void page_allocator_init(struct buddy_allocator *allocator);

/**
 * @brief Allocate specified number of pages.
 * @param pages Number of required pages.
 * @note Return pointer is page aligned.
 * @return Pointer at the beginning of the allocated space.
 */
struct page *alloc_pages(unsigned pages, pageflag flags);

/**
 * @brief Free specified memory space.
 * @param mem Pointer to the space to free.
 * @param pages Number of pages that was requested.
 * @note It's important to specify the same number of pages that was received from the allocator.
 * Bad things will happen otherwise.
 */
void free_pages(struct page *pages, unsigned number);

int mm_arch_available_chunks(void *platform_info);

void mm_arch_get_chunks(void *platform_info, struct mem_chunk *chunks);

#endif // _KERNEL_MM_H
