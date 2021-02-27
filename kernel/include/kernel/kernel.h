#ifndef _KERNEL_KERNEL_H
#define _KERNEL_KERNEL_H

#include "kernel/config.h"
#include "kernel/mm/vm.h"

#include "lib/cppdefs.h"

enum kernel_segments {
        KSEGMENT_TEXT,
        KSEGMENT_RODATA,
        KSEGMENT_DATA,
        KSEGMENT_BSS,
};
#define KSEGMENT_COUNT (4)

extern struct vm_area KERNELBIN_AREAS[KSEGMENT_COUNT];

void kernel_arch_get_segment(enum kernel_segments seg, void **start, void **end);

/* TODO: This two belong to process context. */
extern struct vm_space CURRENT_KERNEL;
extern struct vm_space *CURRENT_USER;

union vm_arch_page_dir *kernel_arch_get_early_pg_root(void);

void kernel_init(void);

#endif /* _KERNEL_KERNEL_H */
