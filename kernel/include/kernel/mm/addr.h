#ifndef _KERNEL_MM_ADDR_H
#define _KERNEL_MM_ADDR_H

#include <stdbool.h>
#include <stdint.h>

typedef void *phys_addr_t;
typedef void *virt_addr_t;

void addr_set_offset(uintptr_t offset);
uintptr_t addr_get_offset(void);

void *addr_to_low(const void *high_addr);
void *addr_to_high(const void *low_addr);
bool addr_is_high(const void *addr);

struct vm_area;
void addr_pgfault_handler_maplow(struct vm_area *area, virt_addr_t addr);
#endif /* _KERNEL_MM_ADDR_H */
