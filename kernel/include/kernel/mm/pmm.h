#ifndef _KERNEL_MM_PMM_H
#define _KERNEL_MM_PMM_H

#include "lib/ds/slist.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

///
/// Holds a result of physical memory allocation by some physical allocator.
/// @note For internal usage between PMM and physical allocators.
///
struct pmm_alloc_result {
        uintptr_t paddr;
        bool success;
};

///
/// Holds a result of physical memory allocation by PMM.
///
struct pmm_page {
        uintptr_t paddr;
        struct pmm_allocator *alloc; //! Allocator that owns allocated memory.
};

///
/// Initializes physical memory allocator with the given physical memory allocators.
///
void pmm_init(struct pmm_allocator *allocators, size_t alloc_length);

#define PMM_FLAG_DMA (0x1 << 0)
struct pmm_page *pmm_alloc_page(int flags);

void pmm_free(struct pmm_page *);

#endif // _KERNEL_MM_PMM_H
