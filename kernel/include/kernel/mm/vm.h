#ifndef _KERNEL_MM_VM_H
#define _KERNEL_MM_VM_H

#include "kernel/mm/vm_area.h"
#include "kernel/mm/vm_space.h"

#include <stdbool.h>
#include <stddef.h>

void vm_pgfault_handle_default(struct vm_area *area, void *addr);
void vm_pgfault_handle_direct(struct vm_area *area, void *addr);

void *vm_arch_get_phys_page(void const *virt_page);

void vm_arch_load_spaces(const struct vm_space *user, const struct vm_space *kernel);

void vm_arch_ptree_map(union vm_arch_page_dir *tree_root, const void *phys_addr,
                       const void *at_virt_addr, enum vm_flags flags);

#endif /* _KERNEL_MM_VM_H */
