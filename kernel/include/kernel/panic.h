#ifndef _KERNEL_PANIC_H
#define _KERNEL_PANIC_H

#include "kernel/platform.h"

#include "lib/ds/kvstore.h"

struct kernel_panic_info {
        char *description;
        char *location;

        KVSTATIC_DECLARE(const char *, size_t, PLATFORM_REGISTERS_COUNT, kstrcmp) regs;
};

void kernel_panic(struct kernel_panic_info *);

#endif // _KERNEL_PANIC_H
