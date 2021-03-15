#ifndef _KERNEL_MM_DEV_H
#define _KERNEL_MM_DEV_H

#include "kernel/mm/vm_area.h"
#include "kernel/mm/vm_space.h"
#include "kernel/resources.h"

#include <stddef.h>

void dev_init(void);

struct vm_area *dev_area_new(struct vm_space *owner, size_t min_len);

void kdev_init(struct vm_space *kernel_space);

void *kdev_map_resource(struct resource *res);

void kdev_unmap_resource(void *page_vaddr);

#endif /* _KERNEL_MM_DEV_H */
