#ifndef _KERNEL_VM_AREA_H
#define _KERNEL_VM_AREA_H

#include "kernel/mm/addr.h"

#include "lib/ds/rbtree.h"
#include "lib/ds/slist.h"

#include <stddef.h>

enum vm_flags {
        VM_WRITE = 0x1 << 0,
        VM_USER = 0x1 << 1,
        VM_CACHE_OFF = 0x1 << 2,
};

/**
 * Describes properties of those *virtual* pages that are located in the area's space.
 */
struct vm_area {
        virt_addr_t base; /**< Beginning of the area. */
        size_t length;
        enum vm_flags flags;

        struct slist_ref sorted_areas; /**< Sorted list of areas in an address space. */
        struct rbtree_node rb_areas;   /**< RBT of areas in an address space. */
        struct vm_space *owner;

        struct vm_area_ops {
                void (*handle_pg_fault)(struct vm_area *area, void *addr);
                void *(*register_page)(struct vm_area *area, void *page_addr);
                void (*unregister_page)(struct vm_area *area, void *page_addr);
        } ops;
        void *data;
};

void vm_area_init(struct vm_area *area, virt_addr_t base, size_t length, struct vm_space *owner);

/**
 * @brief Used as a comparison function to search addresses inside of Red-Black Tree.
 *
 * On return value:
 * If the address lays before the area, retval < 0.
 * If the address lays after the area, retval > 0.
 * If the address lays inside the area, retval == 0.
 * @return Comparison result.
 */
int vm_area_rbtcmpfn_area_to_addr(const void *area, const void *vaddr);

int vm_area_rbtcmpfn(const void *area_x, const void *area_y);

/**
 * @brief Registers the page within the area.
 * @page_base Desired page location. Could be NULL; in this case it'll be chosen automatically.
 * @return A pointer to the allocated page or NULL on fail.
 * */
void *vm_area_register_page(struct vm_area *area, virt_addr_t *page_base);

/**
 * @brief Unregisters the page within the area.
 * @page_base Page to free.
 * */
void vm_area_unregister_page(struct vm_area *area, virt_addr_t *page_base);

#endif /* _KERNEL_VM_AREA_H */
