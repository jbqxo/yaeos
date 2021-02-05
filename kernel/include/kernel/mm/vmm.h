#ifndef _KERNEL_MM_VMM_H
#define _KERNEL_MM_VMM_H

#include "kernel/mm/mm.h"

#include <stdbool.h>
#include <stddef.h>

enum vm_flags {
        VM_WRITE = 0x1 << 0,
        VM_USER = 0x1 << 1,
};

struct vm_arch_page_entry;

struct vm_space {
        struct vm_arch_page_entry *space_dir;
        void *space_dir_paddr;
};

struct vm_region {
        struct vm_arch_page_entry *region_dir;
        void *region_dir_paddr;
};

bool vm_arch_space_map_region(struct vm_region *, struct vm_space *, void *map_point,
                              enum vm_flags);

bool vm_arch_region_map_page(struct mm_page *, struct vm_region *, void *map_point, enum vm_flags);

/* This one is rediculously expensive. (I think) */
void *vm_space_get_paddr(struct vm_space *, void *vaddr);

void vm_arch_change_space(struct vm_space *);

struct vm_space *vm_arch_dir_get_space(struct vm_arch_page_entry *dir);

struct vm_region *vm_arch_dir_get_region(struct vm_arch_page_entry *dir);

#endif // _KERNEL_MM_VMM_H
