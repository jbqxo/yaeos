#ifndef _KERNEL_ARCH_I686_PLATFORM_H
#define _KERNEL_ARCH_I686_PLATFORM_H

#define PAGE_SIZE 4096

#ifndef __ASSEMBLER__
#include <multiboot.h>
#include <stdint.h>
#include <stddef.h>

struct arch_info_i686 {
	multiboot_info_t *multiboot;
};

extern struct arch_info_i686 I686_INFO;

#endif // __ASSEMBLER__
#endif // _KERNEL_ARCH_I686_PLATFORM_H
