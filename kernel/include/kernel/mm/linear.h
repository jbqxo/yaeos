#ifndef _KERNEL_MM_LINEAR_H
#define _KERNEL_MM_LINEAR_H

#include <stdint.h>
#include <stddef.h>

struct linear_alloc {
        uintptr_t base;
        uintptr_t position;
        uintptr_t limit;
};

void linear_alloc_init(struct linear_alloc *, void *mem, size_t len);

void *linear_alloc_alloc(struct linear_alloc *, size_t len);

void linear_alloc_free(struct linear_alloc *, size_t len);

size_t linear_alloc_occupied_mem(struct linear_alloc *a);

#endif /* _KERNEL_MM_LINEAR_H */
