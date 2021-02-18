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

size_t linear_alloc_occupied(struct linear_alloc *a);

void linear_forbid_further_alloc(struct linear_alloc *alloc);

void linear_alloc_used_mem_range(struct linear_alloc *alloc, void **start, void **end);

#endif /* _KERNEL_MM_LINEAR_H */
