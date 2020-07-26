#ifndef _KERNEL_MM_PMM_H
#define _KERNEL_MM_PMM_H

#include <stdint.h>

#define PMM_PAGES_ADDR(pages) ((pages).paddr)

typedef struct {
	void *paddr;
	unsigned order;
} pmm_pages_t;

void pmm_init(void);
pmm_pages_t pmm_alloc(unsigned order);
void pmm_free(pmm_pages_t);

#endif // _KERNEL_MM_PMM_H
