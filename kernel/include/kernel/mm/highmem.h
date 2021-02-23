#ifndef _KERNEL_MM_HIGHMEM_H
#define _KERNEL_MM_HIGHMEM_H

#include "lib/cppdefs.h"
#include "lib/cstd/assert.h"

#include <stdbool.h>
#include <stdint.h>

void highmem_early_set_offset(uintptr_t offset);

void highmem_set_offset(uintptr_t offset);
void *highmem_get_offset(void);

void *highmem_to_low(const void *high_addr);
void *highmem_to_high(const void *low_addr);
bool highmem_is_high(const void *addr);

#endif /* _KERNEL_MM_HIGHMEM_H */
