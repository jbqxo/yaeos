#ifndef _KERNEL_CONFIG_H
#define _KERNEL_CONFIG_H

#define CONF_STACK_SIZE         (16 << 10)
#define CONF_BUDDY_BITMAP_SIZE  (2 << 20)
#define CONF_TIMER_QUEUE_LENGTH (100)
#define CONF_STATIC_SLAB_PAGES  (4)
#define CONF_MALLOC_MIN_POW     (5)
#define CONF_MALLOC_MAX_POW     (11)

/* We reserve one of the entries in Page Table or Page Directory
 * and store there an address to the struct that describes them.
 * Index of the entry can be specified by this parameter. */
#define CONF_VM_DIR_ORIGIN_ENTRY (0)

#endif // _KERNEL_CONFIG_H
