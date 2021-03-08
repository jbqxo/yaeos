#include "kernel/mm/vm_space.h"

#include "kernel/mm/vm.h"

#include "lib/cstd/assert.h"
#include "lib/ds/slist.h"
#include "lib/utils.h"

#include <stddef.h>

void *vm_space_find_gap(struct vm_space *space, size_t *result_len,
                        bool (*predicate)(void *base, size_t len, void *data), void *pred_data)
{
        kassert(space != NULL);
        kassert(predicate != NULL);

        uintptr_t next_after_last_area_end = space->offset;

        SLIST_FOREACH (it, slist_next(&space->sorted_areas)) {
                struct vm_area *a = container_of(it, struct vm_area, sorted_areas);

                uintptr_t current_base = next_after_last_area_end;
                size_t current_length = (uintptr_t)a->base - current_base;

                kassert(check_align(current_base, PLATFORM_PAGE_SIZE));
                kassert(check_align(current_length, PLATFORM_PAGE_SIZE));

                bool hit = predicate((void *)current_base, current_length, pred_data);
                if (hit) {
                        *result_len = current_length;
                        return ((void *)current_base);
                }

                next_after_last_area_end = (uintptr_t)a->base + a->length;
        }

        const uintptr_t LAST_AVAILABLE_ADDR = ~0UL;
        const size_t length_til_space_end = LAST_AVAILABLE_ADDR - next_after_last_area_end + 1;

        bool hit = predicate((void *)next_after_last_area_end, length_til_space_end, pred_data);
        if (hit) {
                *result_len = length_til_space_end;
                return ((void *)next_after_last_area_end);
        }

        *result_len = 0;
        return (NULL);
}

void vm_space_init(struct vm_space *space, phys_addr_t root_pdir, uintptr_t offset)
{
        kassert(space != NULL);
        kassert(root_pdir != NULL);

        slist_init(&space->sorted_areas);
        rbtree_init_tree(&space->rb_areas);
        space->root_dir = root_pdir;
        space->offset = offset;
}

void vm_space_append_area(struct vm_space *space, struct vm_area *area)
{
        kassert(space != NULL);

        kassert(space->offset <= (uintptr_t)area->base);

        struct rbtree_node *left_neigh_node =
                rbtree_search_max(&space->rb_areas, area, vm_area_rbtcmpfn);
        struct vm_area *left_neigh = left_neigh_node != NULL ? left_neigh_node->data : NULL;

        rbtree_insert(&space->rb_areas, &area->rb_areas, vm_area_rbtcmpfn);

        if (left_neigh == NULL) {
                slist_insert(&space->sorted_areas, &area->sorted_areas);
        } else {
                slist_insert(&left_neigh->sorted_areas, &area->sorted_areas);
        }
}

void vm_space_remove_area(struct vm_space *space, struct vm_area *area)
{
        kassert(space != NULL);
        kassert(area != NULL);

        rbtree_delete(&space->rb_areas, &area->rb_areas);
        slist_remove(&space->sorted_areas, &area->sorted_areas);
}
