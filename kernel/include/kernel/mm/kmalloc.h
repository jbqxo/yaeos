#ifndef _KERNEL_KMALLOC_H
#define _KERNEL_KMALLOC_H

#include <stddef.h>

void kmalloc_init(unsigned lowest_pow2_size, unsigned max_pow2_size);

void *kmalloc(size_t size);

void kfree(void *);

#endif /* _KERNEL_KMALLOC_H */
