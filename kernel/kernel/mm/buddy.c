#include <kernel/mm/alloc.h>
#include <kernel/cppdefs.h>
#include <kernel/klog.h>
#include <arch/platform.h>
#include <lib/string.h>

#include <stddef.h>
#include <stdint.h>
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
 */
static unsigned div_roundup(unsigned dividend, unsigned divisor)
{
	return ((dividend + (divisor - 1)) / divisor);
}

/**
 * @brief Take the base 2 logarithm of x and round the result down.
 */
static unsigned log2_down(unsigned x)
{
	unsigned tmp = 0;
	while (x >>= 1) {
		tmp++;
	}
	return tmp;
}

/**
 * @brief Return the nearest address that is bigger than the address and fit the alignment.
 */
static uintptr_t align_at(uintptr_t addr, uintptr_t alignment)
{
	return (addr + alignment - 1) & -alignment;
}

static void *intern_alloc(struct buddy_allocator *alloc, size_t size)
{
	uintptr_t pos = alloc->internp + alloc->intern_sz;

	// Assert chunk space.
	if (__unlikely(pos + size > alloc->internp_lim)) {
		// TODO: Panic!
		return (NULL);
	}
	alloc->intern_sz += size;

	return ((void *)pos);
}

static bool get_bit(struct chunk *ch, unsigned lvl, unsigned pos)
{
	unsigned bitpos_mask = BITMAP_SET_SIZE * 8 - 1;
	unsigned bitmap_idx = (pos & ~bitpos_mask) >> log2_down(BITMAP_SET_SIZE * 8);
	return ch->bitmaps[lvl][bitmap_idx] & (1 << (pos & bitpos_mask));
}

static void free_bit(struct chunk *ch, unsigned lvl, unsigned pos)
{
	unsigned bitpos_mask = BITMAP_SET_SIZE * 8 - 1;
	unsigned bitmap_idx = (pos & ~bitpos_mask) >> log2_down(BITMAP_SET_SIZE * 8);
	ch->bitmaps[lvl][bitmap_idx] |= (1 << (pos & bitpos_mask));
}

static void occupy_bit(struct chunk *ch, unsigned lvl, unsigned pos)
{
	unsigned bitpos_mask = BITMAP_SET_SIZE * 8 - 1;
	unsigned bitmap_idx = (pos & ~bitpos_mask) >> log2_down(BITMAP_SET_SIZE * 8);
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

static struct chunk *init_chunks(struct buddy_allocator *alloc, void **mem_chunks,
				 const size_t *sizes, unsigned chnum)
{
	struct chunk *chunks = intern_alloc(alloc, chnum * sizeof(*chunks));
	unsigned levels = alloc->max_lvl + 1;

	for (int c = 0; c < chnum; c++) {
		uintptr_t aligned = align_at((uintptr_t)mem_chunks[c], PLATFORM_PAGE_SIZE);
		chunks[c].mem = (void *)aligned;
		chunks[c].size = sizes[c] - (aligned - (uintptr_t)mem_chunks[c]);
		chunks[c].bitmaps = intern_alloc(alloc, sizeof(*chunks[c].bitmaps) * levels);
	}

	// Initialize bitmaps for every chunk.
	for (int lvl = 0; lvl < levels; lvl++) {
		for (int ch = 0; ch < chnum; ch++) {
			// Round the required space to prevent overlapping.
			size_t space = div_roundup(max_index(chunks[ch].size, lvl), 8);
			size_t pad = 0;
			if (space % BITMAP_SET_SIZE) {
				pad = BITMAP_SET_SIZE - (space % BITMAP_SET_SIZE);
			}
			chunks[ch].bitmaps[lvl] = intern_alloc(alloc, space + pad);

			// Mark all pages as FREE.
			memset(chunks[ch].bitmaps[lvl], 0xFF, space);
			// Mark all padding space as OCCUPIED.
			uintptr_t pad_addr = (uintptr_t)chunks[ch].bitmaps[lvl] + space;
			memset((void *)pad_addr, 0x0, pad);
		}
	}

	return (chunks);
}

void buddy_init(struct buddy_allocator *alloc, void **mem_chunks, const size_t *sizes,
		unsigned chnum, void *intern_data, size_t intern_len)
{
	alloc->internp = (uintptr_t)intern_data;
	alloc->intern_sz = 0;
	alloc->internp_lim = (uintptr_t)intern_data + intern_len;
	{
		alloc->max_lvl = 0;
		for (int i = 0; i < chnum; i++) {
			unsigned region_max = log2_down(sizes[i] / PLATFORM_PAGE_SIZE);
			if (alloc->max_lvl < region_max) {
				alloc->max_lvl = region_max;
			}
		}
	}
	alloc->chunks_num = chnum;
	alloc->chunks = init_chunks(alloc, mem_chunks, sizes, chnum);
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
	while (lvl <= max_lvl) {
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

void *buddy_alloc(struct buddy_allocator *a, unsigned order)
{
	if (order > a->max_lvl) {
		return (NULL);
	}

	// Find bit index of a free buddy
	struct chunk *chunk;
	int idx = -1;
	for (unsigned ch = 0; ch < a->chunks_num; ch++) {
		chunk = &a->chunks[ch];
		idx = find_free(chunk, order);
		if (idx != -1) {
			break;
		}
	}

	if (idx == -1) {
		return (NULL);
	}

	occupy_buddy(chunk, a->max_lvl, order, idx);
	return ((void *)((uintptr_t)chunk->mem + (idx << order) * PLATFORM_PAGE_SIZE));
}

void buddy_free(struct buddy_allocator *a, void *mem, unsigned order)
{
	// Find buddy's chunk.
	struct chunk *chunk = NULL;
	for (unsigned c = 0; c < a->chunks_num; c++) {
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
		index = (offset / PLATFORM_PAGE_SIZE) >> order;
	}

	free_buddy(chunk, a->max_lvl, order, index);
}
