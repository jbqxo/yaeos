#ifndef _KERNEL_PANIC_H
#define _KERNEL_PANIC_H

#include "kernel/ds/kvstore.h"
#include "kernel/platform.h"

struct kernel_panic_info {
	char *description;
	char *location;

	KVSTATIC_DECLARE(const char *, size_t, PLATFORM_REGISTERS_COUNT, strcmp) regs;
};

void kernel_panic(struct kernel_panic_info *);

#endif // _KERNEL_PANIC_H
