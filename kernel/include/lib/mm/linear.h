#ifndef _LIB_MM_LINEAR_H
#define _LIB_MM_LINEAR_H

#include "lib/cppdefs.h"

#include <stddef.h>
#include <stdint.h>

struct linear_alloc {
        uintptr_t base;
        uintptr_t position;
        uintptr_t limit;
};

void linear_alloc_init(struct linear_alloc *, void *mem, size_t len);

__warn_unused void *linear_alloc_alloc(struct linear_alloc *, size_t len);

__alloc_align(3) __warn_unused
        void *linear_alloc_alloc_aligned(struct linear_alloc *, size_t len, size_t align);

void linear_alloc_free(struct linear_alloc *, size_t len);

size_t linear_alloc_occupied(struct linear_alloc *a);

void linear_forbid_further_alloc(struct linear_alloc *alloc);

void linear_alloc_used_mem_range(struct linear_alloc *alloc, void **start, void **end);

#endif /* _LIB_MM_LINEAR_H */
