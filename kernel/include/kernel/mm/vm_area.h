#ifndef _KERNEL_VM_AREA_H
#define _KERNEL_VM_AREA_H

#include "kernel/mm/addr.h"

#include "lib/cppdefs.h"
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
                void *(*register_map)(struct vm_area *area, void *arg);
                void (*unregister_map)(struct vm_area *area, void *arg);
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
 * @brief Registers a mapping within the area.
 * @return A pointer to the allocated page or NULL on fail.
 * */
void *vm_area_register_map(struct vm_area *area, void *data);

/**
 * @brief Unregisters a mapping within the area.
 * */
void vm_area_unregister_map(struct vm_area *area, void *data);

#endif /* _KERNEL_VM_AREA_H */
