#include "kernel/mm/vmm.h"

#include "kernel/kernel.h"
#include "kernel/klog.h"

#include "lib/align.h"
#include "lib/cppdefs.h"
#include "lib/ds/rbtree.h"
#include "lib/ds/slist.h"

void vm_pgfault_handle_default(struct vm_area *area, void *addr)
{
        LOGF_P("Page fault at the address %p inside area %p-%p!\n", addr, area->base_vaddr,
               uint2ptr(ptr2uint(area->base_vaddr) + area->length - 1));
}

void vm_pgfault_handle_direct(struct vm_area *area, void *addr)
{
        kassert(area != NULL);

        const union uiptr fault_addr = ptr2uiptr(addr);

        const void *virt_page_addr = uint2ptr(align_rounddown(fault_addr.num, PLATFORM_PAGE_SIZE));
        const void *phys_page_addr = kernel_arch_to_low(virt_page_addr);

        vm_arch_ptree_map(area->owner->root_dir, phys_page_addr, virt_page_addr, area->flags);
}

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

        const uintptr_t LAST_AVAILABLE_ADDR =
                UINTPTR_MAX - (PLATFORM_PAGEDIR_PAGES - CONF_VM_LAST_PAGE - 1) * PLATFORM_PAGE_SIZE;
        const size_t length_til_space_end = LAST_AVAILABLE_ADDR - next_after_last_area_end.num + 1;

        bool hit = predicate(next_after_last_area_end.ptr, length_til_space_end, pred_data);
        if (hit) {
                return (next_after_last_area_end.ptr);
        }

        return (NULL);
}

int vm_area_rbtcmpfn_area_to_addr(const void *area, const void *addr)
{
        kassert(area != NULL);

        const struct vm_area *a = area;
        const uintptr_t address = ptr2uint(addr);
        const uintptr_t area_base = ptr2uint(a->base_vaddr);
        const uintptr_t area_end = area_base + a->length - 1;

        if (address < area_base) {
                return (1);
        }

        if (address <= area_end) {
                return (0);
        }

        return (-1);
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

int vm_area_rbtcmpfn(const void *area_x, const void *area_y)
{
        kassert(area_x != NULL);
        kassert(area_y != NULL);

        const struct vm_area *x = area_x;
        const uintptr_t x_start = ptr2uint(x->base_vaddr);
        const uintptr_t x_end = x_start + x->length - 1;

        const struct vm_area *y = area_y;
        const uintptr_t y_start = ptr2uint(y->base_vaddr);
        const uintptr_t y_end = y_start + y->length - 1;

        if (x_start < y_start) {
                kassert(x_end < y_start);
                return (-1);
        } else if (x_start > y_start) {
                kassert(y_end < x_start);
                return (1);
        } else {
                return (0);
        }
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

void vm_area_init(struct vm_area *area, void *vaddr, size_t length, const struct vm_space *owner)
{
        kassert(area != NULL);
        kassert(check_align(ptr2uint(vaddr), PLATFORM_PAGE_SIZE));
        kassert(check_align(length, PLATFORM_PAGE_SIZE));

        kmemset(area, 0x0, sizeof(*area));

        area->base_vaddr = vaddr;
        area->length = length;
        area->owner = owner;

        rbtree_init_node(&area->rb_areas);
        area->rb_areas.data = area;
        SLIST_FIELD_INIT(area, sorted_areas);
}

void *vm_area_register_page(struct vm_area *area, void *page_addr)
{
        kassert(area != NULL);

        if (__unlikely(area->ops.register_page == NULL)) {
                LOGF_P("Tried to register page at address %p but register function is undefined!\n",
                       page_addr);
                /* Although, the kernel should panic at this point. */
                return (NULL);
        }
        return (area->ops.register_page(area, page_addr));
}

void vm_area_unregister_page(struct vm_area *area, void *page_addr)
{
        kassert(area != NULL);
        kassert(area->ops.unregister_page != NULL);

        if (__unlikely(area->ops.unregister_page == NULL)) {
                LOGF_P("Tried to register page at address %p but register function is undefined!\n",
                       page_addr);
                /* Although, the kernel should panic at this point. */
        }
        return (area->ops.unregister_page(area, page_addr));
}
