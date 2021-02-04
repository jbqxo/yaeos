#include "kernel/mm/buddy.h"

#include "kernel/cppdefs.h"
#include "kernel/klog.h"
#include "kernel/platform.h"
#include "kernel/utils.h"

#include "lib/assert.h"
#include "lib/nonstd.h"
#include "lib/string.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 *
 * @brief Calculate the maximum bit index on the level for the given chunk.
 *
 */
static unsigned max_index(size_t chunk_size, unsigned lvl)
{
        return (chunk_size / PLATFORM_PAGE_SIZE) >> lvl;
}

uint32_t buddy_init(struct buddy_manager *bmgr, const size_t size, struct linear_alloc *alloc)
{
        bmgr->lvls = log2_floor(size / PLATFORM_PAGE_SIZE);
        bmgr->alloc = alloc;

        bmgr->lvl_bitmaps = linear_alloc_alloc(bmgr->alloc, bmgr->lvls * sizeof(*bmgr->lvl_bitmaps));

        for (int lvl = 0; lvl < bmgr->lvls; lvl++) {
                size_t max_ndx = max_index(size, lvl);
                void *space = linear_alloc_alloc(bmgr->alloc, div_ceil(max_ndx, 8));
                bitmap_init(&bmgr->lvl_bitmaps[lvl], space, max_ndx);
        }

        return (max_index(size, 0));
}

static void occupy_buddys_descendants(struct buddy_manager *bmgr, unsigned lvl, unsigned bit)
{
        if (lvl == 0) {
                return;
        }
        bit <<= 1;
        lvl--;

        bitmap_set_true(&bmgr->lvl_bitmaps[lvl], bit);
        bitmap_set_true(&bmgr->lvl_bitmaps[lvl], bit + 1);
        occupy_buddys_descendants(bmgr, lvl, bit);
        occupy_buddys_descendants(bmgr, lvl, bit + 1);
}

static void occupy_buddy(struct buddy_manager *bmgr, unsigned lvl, unsigned bit)
{
        /* Occupy buddy. */
        bitmap_set_true(&bmgr->lvl_bitmaps[lvl], bit);
        occupy_buddys_descendants(bmgr, lvl, bit);

        /* Occupy buddy's ancestors. */
        while (lvl < bmgr->lvls) {
                lvl++;
                bit >>= 1;
                bitmap_set_true(&bmgr->lvl_bitmaps[lvl], bit);
        }
}

static void free_buddys_ancestors(struct buddy_manager *bmgr, unsigned lvl, unsigned bit)
{
        while (lvl <= bmgr->lvls) {
                lvl++;
                bit >>= 1;

                bool left_child_free = bitmap_get(&bmgr->lvl_bitmaps[lvl - 1], bit << 1);
                bool right_child_free = bitmap_get(&bmgr->lvl_bitmaps[lvl - 1], (bit << 1) + 1);
                if (!left_child_free || !right_child_free) {
                        return;
                }

                bitmap_set_false(&bmgr->lvl_bitmaps[lvl], bit);
        }
}

static void free_buddys_children(struct buddy_manager *bmgr, unsigned lvl, unsigned bit)
{
        if (lvl == 0) {
                return;
        }
        bit <<= 1;
        lvl--;

        bitmap_set_false(&bmgr->lvl_bitmaps[lvl], bit);
        bitmap_set_false(&bmgr->lvl_bitmaps[lvl], bit + 1);
        free_buddys_children(bmgr, lvl, bit);
        free_buddys_children(bmgr, lvl, bit + 1);
}

static void free_buddy(struct buddy_manager *bmgr, unsigned lvl, unsigned bit)
{
        bitmap_set_false(&bmgr->lvl_bitmaps[lvl], bit);
        free_buddys_children(bmgr, lvl, bit);
        free_buddys_ancestors(bmgr, lvl, bit);
}

bool buddy_alloc(struct buddy_manager *bmgr, unsigned order, uint32_t *result)
{
        if (order > bmgr->lvls) {
                return (false);
        }

        /* Find bit index of a free buddy */
        struct bitmap *bitmap = NULL;
        uint32_t ndx = 0;
        bool found = false;

        for (unsigned lvl = order; lvl < bmgr->lvls; lvl++) {
                bitmap = &bmgr->lvl_bitmaps[lvl];

                if (bitmap_search_false(bitmap, &ndx)) {
                        found = true;
                        break;
                }
        }
        if (!found) {
                return (false);
        }

        occupy_buddy(bmgr, order, ndx);
        *result = ndx;
        return (true);
}

void buddy_free(struct buddy_manager *bmgr, uint32_t page_ndx, unsigned order)
{
        free_buddy(bmgr, order, page_ndx);
}

uint32_t buddy_reduce_size(struct buddy_manager *bmgr, size_t new_size)
{
        kassert(bmgr != NULL);
        kassert(new_size < bmgr->lvl_bitmaps->length);

        bmgr->lvls = log2_floor(new_size / PLATFORM_PAGE_SIZE);
        for (int lvl = 0; lvl < bmgr->lvls; lvl++) {
                size_t max_ndx = max_index(new_size, lvl);
                bitmap_resize(&bmgr->lvl_bitmaps[lvl], max_ndx);
        }
}
