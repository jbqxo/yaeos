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
        phys_addr_t root_dir;
        uintptr_t offset; /**< Offset of all allocations inside of a space. */

        struct rbtree rb_areas;
        struct slist_ref sorted_areas;
};

/**
 * @brief Initialize the vmspace.
 *
 * @param root_pdir Root of the Page Dir for the vmspace.
 * @param offset Allocate all areas only starting from the specified offset. It's for the kernel's vmspace.
 */
void vm_space_init(struct vm_space *space, phys_addr_t root_pdir, uintptr_t offset);

void vm_space_insert_area(struct vm_space *space, struct vm_area *area);

void vm_space_remove_area(struct vm_space *space, struct vm_area *area);

/**
 * @brief Find a gap in the vmspace that is chosen by the given predicate.
 */
void *vm_space_find_gap(struct vm_space *space, size_t *result_len,
                        bool (*predicate)(void *base, size_t len, void *data), void *pred_data);

#endif /* _KERNEL_VM_SPACE_H */
