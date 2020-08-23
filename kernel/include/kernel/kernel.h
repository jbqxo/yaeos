#ifndef _KERNEL_KERNEL_H
#define _KERNEL_KERNEL_H

#include "arch/platform.h"

#include "kernel/config.h"
#include "kernel/ds/kvstore.h"

struct kernel_panic_info {
	char *description;
	char *location;

	KVSTATIC_DECLARE(size_t, PLATFORM_REGISTERS_COUNT) regs;
};

void kernel_panic(struct kernel_panic_info *);
void kernel_init(void);

#endif // _KERNEL_KERNEL_H
