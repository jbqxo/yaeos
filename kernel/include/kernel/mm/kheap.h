#ifndef _KERNEL_MM_KHEAP_H
#define _KERNEL_MM_KHEAP_H

#include "kernel/mm/vm_area.h"

void kheap_init(struct vm_space *space);

void *kheap_alloc_page(void);

void kheap_free_page(void *page);

#endif /* _KERNEL_MM_KHEAP_H */
