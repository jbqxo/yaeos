#ifndef _KERNEL_PANIC_H
#define _KERNEL_PANIC_H

#include "kernel/platform_consts.h"

#include "lib/ds/kvstore.h"

struct kernel_panic_info {
        char *description;
        char *location;

        struct kvstore *regs;
};

void kernel_panic(struct kernel_panic_info *);

#endif /* _KERNEL_PANIC_H */
