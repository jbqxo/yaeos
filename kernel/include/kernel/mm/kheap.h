#ifndef _KERNEL_MM_KHEAP_H
#define _KERNEL_MM_KHEAP_H

#include "kernel/mm/vm_area.h"

void init_kernel_heap(struct vm_area *heap_area, struct vm_space *space);

#endif /* _KERNEL_MM_KHEAP_H */
