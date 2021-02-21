#include "kernel/mm/vm_space.h"

#include "kernel/mm/vm.h"

#include "lib/cstd/assert.h"
#include "lib/ds/slist.h"

#include <stddef.h>

void *vm_space_find_gap(struct vm_space *space,
                        bool (*predicate)(void *base, size_t len, void *data), void *pred_data)
{
        kassert(space != NULL);
        kassert(predicate != NULL);

        union uiptr next_after_last_area_end = space->offset;
        struct vm_area *it = NULL;

        SLIST_FOREACH (it, &space->sorted_areas, sorted_areas) {
                union uiptr current_base = next_after_last_area_end;
                size_t current_length = ptr2uint(it->base_vaddr) - current_base.num;

                bool hit = predicate(current_base.ptr, current_length, pred_data);
                if (hit) {
                        return (current_base.ptr);
                }

                next_after_last_area_end.num = ptr2uint(it->base_vaddr) + it->length;
        }

        const uintptr_t LAST_AVAILABLE_ADDR = ~0UL;
        const size_t length_til_space_end = LAST_AVAILABLE_ADDR - next_after_last_area_end.num + 1;

        bool hit = predicate(next_after_last_area_end.ptr, length_til_space_end, pred_data);
        if (hit) {
                return (next_after_last_area_end.ptr);
        }

        return (NULL);
}

void vm_space_init(struct vm_space *space, union vm_arch_page_dir *root_pdir, union uiptr offset)
{
        kassert(space != NULL);
        kassert(root_pdir != NULL);

        SLIST_INIT(&space->sorted_areas);
        rbtree_init_tree(&space->rb_areas);
        space->root_dir = root_pdir;
        space->offset = offset;
}

void vm_space_append_area(struct vm_space *space, struct vm_area *area)
{
        kassert(space != NULL);

        struct rbtree_node *left_neigh_node =
                rbtree_search_max(&space->rb_areas, area, vm_area_rbtcmpfn);
        struct vm_area *left_neigh = left_neigh_node != NULL ? left_neigh_node->data : NULL;

        rbtree_insert(&space->rb_areas, &area->rb_areas, vm_area_rbtcmpfn);

        if (left_neigh == NULL) {
                SLIST_INSERT_HEAD(&space->sorted_areas, area, sorted_areas);
        } else {
                SLIST_INSERT_AFTER(left_neigh, area, sorted_areas);
        }
}

void vm_space_remove_area(struct vm_space *space, struct vm_area *area)
{
        kassert(space != NULL);
        kassert(area != NULL);

        rbtree_delete(&space->rb_areas, &area->rb_areas);
        SLIST_REMOVE(&space->sorted_areas, area, sorted_areas);
}
