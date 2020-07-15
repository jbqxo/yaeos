#include <kernel/mm.h>
#include <kernel/cppdefs.h>
#include <arch/platform.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

struct chunk {
	void *mem;
	size_t size;
	// The value of 1 denotes FREE page
	// The value of 0 denotes OCCUPIED page
	unsigned **bitmaps;
#define BITMAP_SET_SIZE (sizeof((((struct chunk *)NULL)->bitmaps[0][0])))
};

/**
 * @brief Perform division and round the result up.
 *
 */
static unsigned div_roundup(unsigned dividend, unsigned divisor)
{
	return ((dividend + (divisor - 1)) / divisor);
}

/**
 * @brief Take the base 2 logarithm of x and round the result up.
 */
static unsigned log2(unsigned x)
{
	if (x == 1) {
		// As we are rounding results to the upper bound,
		// 1 requires special handling.
		return 1;
	}
	unsigned b = 0;
	unsigned tmp = x;
	while (tmp >>= 1) {
		b++;
	}
	// Round the results to the upper bound.
	return (x == 0x1 << b ? b : b + 1);
}

/**
 * @brief Return the nearest address that is bigger than the address and fit the alignment.
 */
static uintptr_t align_addr(uintptr_t addr, uintptr_t alignment)
{
	return (addr + alignment - 1) & -alignment;
}

static void *alloc_data_mem(struct buddy_allocator *alloc, size_t size)
{
	uintptr_t pos = alloc->data + alloc->data_size;

	// Assert chunk space.
	if (__unlikely(pos + size > alloc->data_limit)) {
		return (NULL);
	}
	alloc->data_size += size;

	if (alloc->flags & BUDDY_LOWMEM) {
		pos += KERNEL_VMA;
	}

	return ((void *)pos);
}

static bool get_bit(struct chunk *ch, unsigned lvl, unsigned pos)
{
	unsigned bitpos_mask = BITMAP_SET_SIZE * 8 - 1;
	unsigned bitmap_idx = (pos & ~bitpos_mask) >> log2(BITMAP_SET_SIZE * 8);
	return ch->bitmaps[lvl][bitmap_idx] & (1 << (pos & bitpos_mask));
}

static void free_bit(struct chunk *ch, unsigned lvl, unsigned pos)
{
	unsigned bitpos_mask = BITMAP_SET_SIZE * 8 - 1;
	unsigned bitmap_idx = (pos & ~bitpos_mask) >> log2(BITMAP_SET_SIZE * 8);
	ch->bitmaps[lvl][bitmap_idx] |= (1 << (pos & bitpos_mask));
}

static void occupy_bit(struct chunk *ch, unsigned lvl, unsigned pos)
{
	unsigned bitpos_mask = BITMAP_SET_SIZE * 8 - 1;
	unsigned bitmap_idx = (pos & ~bitpos_mask) >> log2(BITMAP_SET_SIZE * 8);
	ch->bitmaps[lvl][bitmap_idx] &= ~(1 << (pos & bitpos_mask));
}

/**
 * @brief Calculate the maximum bit index on the level for the given chunk.
 *
 */
static unsigned max_index(size_t chunk_size, unsigned lvl)
{
	return (chunk_size / PLATFORM_PAGE_SIZE) >> lvl;
}

static unsigned max_lvl(const size_t *sizes, size_t regions)
{
	unsigned max = 0;
	for (int i = 0; i < regions; i++) {
		unsigned region_max = log2(sizes[i] / PLATFORM_PAGE_SIZE);
		if (max < region_max) {
			max = region_max;
		}
	}
	return max;
}

struct buddy_allocator *buddy_init(void **mem_chunks, const size_t *sizes, unsigned count,
				   int flags)
{
	unsigned data_idx = count - 1;

	// Initialize struct buddy_allocator enough to be able to allocate memory further.
	uintptr_t data_start_addr = align_addr((uintptr_t)mem_chunks[data_idx], PLATFORM_PAGE_SIZE);
	struct buddy_allocator *alloc =
		(void *)((uintptr_t)data_start_addr + ((flags & BUDDY_LOWMEM) ? KERNEL_VMA : 0));
	alloc->data = data_start_addr;
	// We will reallocate this space again, so there is no need to increate data_size.
	alloc->data_size = 0;
	alloc->data_limit = (uintptr_t)mem_chunks[data_idx] + sizes[data_idx];
	alloc->flags = flags;

	// Reallocate the same space again.
	// There may be additional checks and such in the allocation function.
	alloc = alloc_data_mem(alloc, sizeof(*alloc));

	alloc->levels = max_lvl(sizes, count);
	alloc->chunks_count = count;

	// Preallocate space for chunks bookkeeping.
	struct chunk *chunks = alloc_data_mem(alloc, count * sizeof(*chunks));
	alloc->chunks = chunks;

	for (int c = 0; c < count; c++) {
		uintptr_t aligned_mem = align_addr((uintptr_t)mem_chunks[c], PLATFORM_PAGE_SIZE);
		chunks[c].mem = (void *)aligned_mem;
		chunks[c].size = sizes[c] - (aligned_mem - (uintptr_t)mem_chunks[c]);
		chunks[c].bitmaps =
			alloc_data_mem(alloc, sizeof(*chunks[c].bitmaps) * alloc->levels);
	}

	// Initialize bitmaps for every chunk.
	for (int lvl = 0; lvl < alloc->levels; lvl++) {
		for (int ch = 0; ch < count; ch++) {
			size_t space = div_roundup(max_index(chunks[ch].size, lvl), 8);
			// Round the required space to prevent overlapping.
			size_t pad = 0;
			if (space % BITMAP_SET_SIZE) {
				pad = BITMAP_SET_SIZE - (space % BITMAP_SET_SIZE);
			}
			chunks[ch].bitmaps[lvl] = alloc_data_mem(alloc, space + pad);

			// Mark all pages as FREE.
			memset(chunks[ch].bitmaps[lvl], 0xFF, space);
			// Mark all padding space as OCCUPIED.
			uintptr_t pad_addr = (uintptr_t)chunks[ch].bitmaps[lvl] + space;
			memset((void *)pad_addr, 0x0, pad);
		}
	}

	// Adjust the last block.
	/* Actually, right now we may(!) lose a bit of memory,
	 * because we have initialized chunk's bitmap with a larger size in mind. It's not critical,
	 * but it would be nice to fix it.
	 * TODO: Fix the error with data allocation.
	 */
	chunks[data_idx].mem = (void *)align_addr(alloc->data, PLATFORM_PAGE_SIZE);
	chunks[data_idx].size = sizes[data_idx] - alloc->data_size;

	// We may freely use any space remaining in front of data_idx's new address.
	alloc->data_limit = (uintptr_t)chunks[data_idx].mem - 1;

	return (alloc);
#undef ASSERT_CHUNK_SPACE
}

/**
 * @brief Find the first free bit among the level in the chunk.
 *
 * @return An index of the bit, or -1 if there are no free bits.
 */
static int find_free(struct chunk *ch, unsigned lvl)
{
	unsigned *bitmap = ch->bitmaps[lvl];
	// The last index of the element that contains the last bit.
	unsigned max_bitmap_idx = max_index(ch->size, lvl) / BITMAP_SET_SIZE;
	int free_idx = -1;
	for (unsigned i = 0; i <= max_bitmap_idx; i++) {
		// If at least 1 bit in the whole set isn't 0,
		// then there is a free page here.
		if (bitmap[i] != 0) {
			free_idx = (int)(i * BITMAP_SET_SIZE);
			break;
		}
	}

	if (free_idx == -1) {
		return (-1);
	}

	for (int i = free_idx; i < max_index(ch->size, lvl); i++) {
		if (get_bit(ch, lvl, i) != 0) {
			return i;
		}
	}

	return (-1);
}

static void occupy_buddys_children(struct chunk *ch, unsigned lvl, unsigned bit)
{
	if (lvl == 0) {
		return;
	}
	bit <<= 1;
	lvl--;

	occupy_bit(ch, lvl, bit);
	occupy_bit(ch, lvl, bit + 1);
	occupy_buddys_children(ch, lvl, bit);
	occupy_buddys_children(ch, lvl, bit + 1);
}

static void occupy_buddy(struct chunk *ch, unsigned max_lvl, unsigned lvl, unsigned bit)
{
	// Occupy buddy.
	occupy_bit(ch, lvl, bit);
	occupy_buddys_children(ch, lvl, bit);

	// Occupy buddy's ancestors.
	while (lvl < max_lvl) {
		lvl++;
		bit >>= 1;
		occupy_bit(ch, lvl, bit);
	}
}

static void free_buddys_ancestors(struct chunk *ch, unsigned max_lvl, unsigned lvl, unsigned bit)
{
	while (lvl < max_lvl) {
		lvl++;
		bit >>= 1;
		bool children_free =
			get_bit(ch, lvl - 1, bit << 1) && get_bit(ch, lvl - 1, (bit << 1) + 1);
		if (!children_free) {
			return;
		}

		free_bit(ch, lvl, bit);
	}
}

static void free_buddys_children(struct chunk *ch, unsigned lvl, unsigned bit)
{
	if (lvl == 0) {
		return;
	}
	bit <<= 1;
	lvl--;

	free_bit(ch, lvl, bit);
	free_bit(ch, lvl, bit + 1);
	free_buddys_children(ch, lvl, bit);
	free_buddys_children(ch, lvl, bit + 1);
}

static void free_buddy(struct chunk *ch, unsigned max_lvl, unsigned lvl, unsigned bit)
{
	free_bit(ch, lvl, bit);
	free_buddys_children(ch, lvl, bit);
	free_buddys_ancestors(ch, max_lvl, lvl, bit);
}

void *buddy_alloc(void *allocator, unsigned pages)
{
	struct buddy_allocator *a = allocator;
	unsigned lvl = log2(pages) - 1;
	if (lvl >= a->levels) {
		return (NULL);
	}

	// Find bit index of a free buddy
	struct chunk *chunk;
	int idx = -1;
	for (unsigned ch = 0; ch < a->chunks_count; ch++) {
		chunk = &a->chunks[ch];
		idx = find_free(chunk, lvl);
		if (idx != -1) {
			break;
		}
	}

	if (idx == -1) {
		return (NULL);
	}

	occupy_buddy(chunk, a->levels - 1, lvl, idx);
	return ((void *)((uintptr_t)chunk->mem + (idx << lvl) * PLATFORM_PAGE_SIZE));
}

void buddy_free(void *allocator, void *mem, unsigned pages)
{
	struct buddy_allocator *a = allocator;
	unsigned lvl;
	if (pages == 1) {
		// Edge case.
		lvl = 0;
	} else {
		lvl = log2(pages);
	}

	// Find buddy's chunk.
	struct chunk *chunk = NULL;
	for (unsigned c = 0; c < a->chunks_count; c++) {
		struct chunk *ch = &a->chunks[c];
		void *block_start = ch->mem;
		void *block_end = (void *)((uintptr_t)ch->mem + ch->size);
		if (mem > block_start && mem < block_end) {
			chunk = ch;
			break;
		}
	}

	if (!chunk) {
		return;
	}

	// Calculate buddys index.
	unsigned index;
	{
		uintptr_t offset = (uintptr_t)mem - (uintptr_t)chunk->mem;
		index = (offset / PLATFORM_PAGE_SIZE) >> lvl;
	}

	free_buddy(chunk, a->levels, lvl, index);
}
