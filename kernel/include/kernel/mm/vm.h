#ifndef _KERNEL_MM_VM_H
#define _KERNEL_MM_VM_H

#include "kernel/mm/vm_area.h"
#include "kernel/mm/vm_space.h"

#include "lib/cppdefs.h"

#include <stdbool.h>
#include <stddef.h>

/**
 * @brief Initialize VM management.
 */
void vm_init(void);

/**
 * @brief Find free place in the space, allocate new area to manage it.
 */
struct vm_area *vm_new_area_within_space(struct vm_space *space, size_t min_size, size_t max_size);

/**
 * @brief Unregister the area from it's space, and free resources.
 */
void vm_free_area(struct vm_area *area);

/**
 * @brief Page Fault function that panics on page faults.
 */
__noreturn void vm_pgfault_handle_panic(struct vm_area *area, virt_addr_t addr);

/**
 * @brief Iterate over all virtual addresses that are, for some reason, not available for use.
 */
void vm_arch_iter_reserved_vaddresses(void (*fn)(void const *addr, size_t len, void *data),
                                      void *data);

/**
 * @brief Check that given virtual address range is available for use.
 */
__const bool vm_arch_is_range_valid(void const *base, size_t len);

/**
 * @brief Get the root of the early Page Directory.
 */
__const void *vm_arch_get_early_pgroot(void);

/**
 * @brief Resolve the virtual address to it's physicall address *from it's vmspace*.
 */
void *vm_arch_resolve_phys_page(void const *virt_page);

/**
 * @brief Map the virtual address to the physical address for the given page tree.
 */
void vm_arch_pt_map(void *tree_root, const void *phys_addr, const void *at_virt_addr,
                    enum vm_flags flags);

#endif /* _KERNEL_MM_VM_H */
