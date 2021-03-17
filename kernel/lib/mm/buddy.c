#include "lib/mm/buddy.h"

#include "lib/align.h"
#include "lib/cppdefs.h"
#include "lib/cstd/assert.h"
#include "lib/utils.h"
#include "lib/cstd/string.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 *
 * @brief Calculate the maximum bit index on the level for the given chunk.
 *
 */
static size_t get_max_index(size_t const pages, size_t const lvl)
{
        kassert((pages >> lvl) > 0);
        return ((pages >> lvl) - 1);
}

size_t buddy_predict_req_space(size_t const pages)
{
        const size_t lvls = log2_floor(pages) + 1;

        const size_t lvls_array = lvls * sizeof(*((struct buddy_manager *)NULL)->lvl_bitmaps);

        size_t bitmaps = 0;
        for (size_t i = 0; i < lvls; i++) {
                const size_t bits_req = get_max_index(pages, i) + 1;
                bitmaps += bitmap_predict_size(bits_req);
        }

        return (bitmaps + lvls_array);
}

size_t buddy_init(struct buddy_manager *bmgr, size_t const pages, struct linear_alloc *alloc)
{
        const size_t alloc_space_before __maybe_unused = linear_alloc_occupied(alloc);

        bmgr->lvls = log2_floor(pages) + 1;
        bmgr->alloc = alloc;

        bmgr->lvl_bitmaps =
                linear_alloc_alloc(bmgr->alloc, bmgr->lvls * sizeof(*bmgr->lvl_bitmaps));

        for (size_t lvl = 0; lvl < bmgr->lvls; lvl++) {
                size_t const bits_req = get_max_index(pages, lvl) + 1;
                void *space = linear_alloc_alloc(bmgr->alloc, bitmap_predict_size(bits_req));
                kassert(NULL != space);
                bitmap_init(&bmgr->lvl_bitmaps[lvl], space, bits_req);
        }

        kassert(linear_alloc_occupied(bmgr->alloc) - alloc_space_before ==
                buddy_predict_req_space(pages));

        return (get_max_index(pages, 0));
}

static void occupy_buddys_descendants(struct buddy_manager *bmgr, size_t lvl, size_t bit)
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

static void occupy_buddy(struct buddy_manager *bmgr, size_t lvl, size_t bit)
{
        occupy_buddys_descendants(bmgr, lvl, bit);

        /* Occupy buddy and it's ancestors. */
        for (size_t i = 0; i < bmgr->lvls; i++) {
                /* Guard against trailing 1 when length is odd. */
                if (bit >> i < bmgr->lvl_bitmaps[i].length) {
                        bitmap_set_true(&bmgr->lvl_bitmaps[i], bit >> i);
                }
        }
}

static void free_buddys_ancestors(struct buddy_manager *bmgr, size_t lvl, size_t bit)
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

static void free_buddys_children(struct buddy_manager *bmgr, size_t lvl, size_t bit)
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

static void free_buddy(struct buddy_manager *bmgr, size_t lvl, size_t bit)
{
        bitmap_set_false(&bmgr->lvl_bitmaps[lvl], bit);
        free_buddys_children(bmgr, lvl, bit);
        free_buddys_ancestors(bmgr, lvl, bit);
}

bool buddy_try_alloc(struct buddy_manager *bmgr, size_t order, size_t page_ndx)
{
        kassert(bmgr != NULL);

        if (__unlikely(bitmap_get(&bmgr->lvl_bitmaps[order], page_ndx))) {
                return (false);
        }
        occupy_buddy(bmgr, order, page_ndx);
        return (true);
}

bool buddy_alloc(struct buddy_manager *bmgr, size_t order, size_t *result)
{
        if (order > bmgr->lvls) {
                return (false);
        }

        size_t ndx = 0;
        if (!bitmap_search_false(&bmgr->lvl_bitmaps[order], &ndx)) {
                return (false);
        }

        occupy_buddy(bmgr, order, ndx);
        *result = ndx;
        return (true);
}

void buddy_free(struct buddy_manager *bmgr, size_t page_ndx, size_t order)
{
        free_buddy(bmgr, order, page_ndx);
}

bool buddy_is_free(struct buddy_manager *bmgr, size_t page_ndx)
{
        kassert(bmgr != NULL);

        return (bitmap_get(&bmgr->lvl_bitmaps[0], page_ndx) == false);
}
