#include "kernel/mm/vm_area.h"

#include "kernel/klog.h"
#include "kernel/platform_consts.h"

#include "lib/align.h"
#include "lib/cstd/assert.h"
#include "lib/cstd/string.h"

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
        slist_init(&area->sorted_areas);
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
