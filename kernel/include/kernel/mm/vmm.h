#ifndef _KERNEL_MM_VMM_H
#define _KERNEL_MM_VMM_H

#include "kernel/cppdefs.h"
#include "kernel/ds/rbtree.h"
#include "kernel/ds/slist.h"

#include "lib/cstd/assert.h"

#include <stdbool.h>
#include <stddef.h>

enum vm_flags {
        VM_WRITE = 0x1 << 0,
        VM_USER = 0x1 << 1,
};

/**
 * Describes an address space of a user process or the kernel.
 */
struct vm_space {
        union vm_arch_page_dir *
                root_dir; /**< Top level directory of every *userspace* vm_space is always present.*/
        union uiptr offset; /**< I think it will be used only by the kernel space. */

        struct rbtree rb_areas;
        SLIST_HEAD(, struct vm_area) sorted_areas;
};

/**
 * Should be used for initialization of the kernel space only.
 * It does the same, except that it won't allocate top-level pagedir by itself.
 */
void vm_space_init(struct vm_space *space, union vm_arch_page_dir *root_pdir, union uiptr offset);

void vm_space_append_area(struct vm_space *space, struct vm_area *area);

void vm_space_remove_area(struct vm_space *space, struct vm_area *area);

void *vm_space_find_gap(struct vm_space *space,
                        bool (*predicate)(void *base, size_t len, void *data), void *pred_data);

/**
 * Similar to the same entity in the Linux kernel.
 * Describes properties of those *virtual* pages that are located in the area's space.
 */
struct vm_area {
        void *base_vaddr; /**< Beginning of the area. */
        size_t length;
        enum vm_flags flags;

        SLIST_FIELD(struct vm_area) sorted_areas; /**< Sorted list of areas in an address space. */
        struct rbtree_node rb_areas;              /**< RBT of areas in an address space. */
        const struct vm_space *owner;

        struct vm_area_ops {
                void (*handle_pg_fault)(struct vm_area *area, void *addr);
                void *(*register_page)(struct vm_area *area, void *page_addr);
                void (*unregister_page)(struct vm_area *area, void *page_addr);
        } ops;
        void *data;
};

void vm_area_init(struct vm_area *area, void *vaddr, size_t length,
                  const struct vm_space *owner);

void vm_pgfault_handle_default(struct vm_area *area, void *addr);
void vm_pgfault_handle_direct(struct vm_area *area, void *addr);

/**
 * @brief Used as a comparison function to search addresses inside of Red-Black Tree.
 *
 * On return value:
 * If the address lays before the area, retval < 0.
 * If the address lays after the area, retval > 0.
 * If the address lays inside the area, retval == 0.
 * @return Comparison result.
 */
int vm_area_rbtcmpfn_area_to_addr(const void *area, const void *addr);

int vm_area_rbtcmpfn(const void *area_x, const void *area_y);

/**
 * @brief Registers a page within the area.
 * @page_addr Desired page location. Could be NULL; in this case it'll be chosen automatically.
 * @return A pointer to the allocated page or NULL on fail.
 * */
void *vm_area_register_page(struct vm_area *area, void *page_addr);

/**
 * @brief Unregisters the page within the area.
 * @page_addr Page to free.
 * */
void vm_area_unregister_page(struct vm_area *area, void *page_addr);

void vm_arch_load_spaces(const struct vm_space *user, const struct vm_space *kernel);

void vm_arch_ptree_map(union vm_arch_page_dir *tree_root, const void *phys_addr,
                       const void *at_virt_addr, enum vm_flags flags);

void *vm_heap_alloc_page(void);
void *vm_heap_free_page(void);

#endif // _KERNEL_MM_VMM_H
