#ifndef _KERNEL_KERNEL_H
#define _KERNEL_KERNEL_H

#include "kernel/config.h"
#include "kernel/ds/kvstore.h"

struct kernel_panic_info {
	char *description;
	char *location;

	KVSTATIC_DECLARE(const char*, size_t, PLATFORM_REGISTERS_COUNT, strcmp) regs;
};

void kernel_panic(struct kernel_panic_info *);
void kernel_init(void);

#endif // _KERNEL_KERNEL_H
