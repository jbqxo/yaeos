#ifndef _KERNEL_KMALLOC_H
#define _KERNEL_KMALLOC_H

#include <stddef.h>
#include <stdint.h>

void kmalloc_init(uint8_t lowest_pow2_size, uint8_t max_pow2_size);

void *kmalloc(size_t size);

void kfree(void *);

#endif /* _KERNEL_KMALLOC_H */
