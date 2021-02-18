#ifndef _KERNEL_CONFIG_H
#define _KERNEL_CONFIG_H

#include "kernel/platform.h"

#define CONF_STACK_SIZE         (16 << 10)
#define CONF_TIMER_QUEUE_LENGTH (100)
#define CONF_STATIC_SLAB_PAGES  (4)
#define CONF_MALLOC_MIN_POW     (5)
#define CONF_MALLOC_MAX_POW     (11)

#define CONF_VM_FIRST_PAGE (0x1) /**< Use only pages onward from specified.*/
#define CONF_VM_LAST_PAGE (PLATFORM_PAGEDIR_PAGES - 1 - 2) /**< Use last page for error codes. */

#define CONF_VM_RECURSIVE_PAGE (PLATFORM_PAGEDIR_PAGES - 1 - 1)
#define CONF_VM_ERRORS_PAGE (PLATFORM_PAGEDIR_PAGES - 1)

#endif // _KERNEL_CONFIG_H
