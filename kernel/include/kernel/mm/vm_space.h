#ifndef _KERNEL_VM_SPACE_H
#define _KERNEL_VM_SPACE_H

#include "kernel/mm/vm_area.h"

#include "lib/cppdefs.h"
#include "lib/ds/rbtree.h"
#include "lib/ds/slist.h"

#include <stdbool.h>

/**
 * Describes an address space of a user process or the kernel.
 */
struct vm_space {
        phys_addr_t root_dir; /**< Top level directory of every *userspace* vm_space is always present.*/
        uintptr_t offset; /**< Offset of all allocations inside of a space. */

        struct rbtree rb_areas;
        struct slist_ref sorted_areas;
};

void vm_space_init(struct vm_space *space, phys_addr_t root_pdir, uintptr_t offset);

void vm_space_append_area(struct vm_space *space, struct vm_area *area);

void vm_space_remove_area(struct vm_space *space, struct vm_area *area);

void *vm_space_find_gap(struct vm_space *space,
                        bool (*predicate)(void *base, size_t len, void *data), void *pred_data);

#endif /* _KERNEL_VM_SPACE_H */
