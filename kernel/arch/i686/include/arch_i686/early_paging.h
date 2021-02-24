#ifndef _KERNEL_ARCH_I686_EARLYPG_H
#define _KERNEL_ARCH_I686_EARLYPG_H

#include "lib/cppdefs.h"

#include <multiboot.h>

/**
 * @brief Setup paging to actualy boot the kernel.
 */
__noinline void setup_boot_paging(void);

/**
 * @brief Patch some multiboot information in order to use it from high memory.
 *
 * @param info Multiboot info block.
 */
void patch_multiboot_info(multiboot_info_t *info);

#endif /* _KERNEL_ARCH_I686_EARLYPG_H */
