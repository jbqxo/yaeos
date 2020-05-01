#pragma once

#include <arch/vm.h>

#include <multiboot.h>

#include <stdint.h>

#define CONF_STACK_SIZE 16384
#define PLATFORM_PAGE_SIZE 4096

struct arch_info_i686 {
	multiboot_info_t *info;
};
