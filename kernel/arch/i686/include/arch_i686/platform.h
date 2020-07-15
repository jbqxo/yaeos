#ifndef _KERNEL_ARCH_I686_PLATFORM_H
#define _KERNEL_ARCH_I686_PLATFORM_H

#include <arch_i686/vm.h>

#include <multiboot.h>

#include <stdint.h>

#define CONF_STACK_SIZE 16384
#define PLATFORM_PAGE_SIZE 4096

struct arch_info_i686 {
	multiboot_info_t *info;
};

#endif // _KERNEL_ARCH_I686_PLATFORM_H
