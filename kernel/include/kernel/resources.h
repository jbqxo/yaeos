#ifndef _KERNEL_RESOURCES_H
#define _KERNEL_RESOURCES_H

#include <stddef.h>

enum resource_type {
        RESOURCE_TYPE_MEMORY,
        RESOURCE_TYPE_DEV_REGS,
        RESOURCE_TYPE_DEV_BUFFER,
};

struct resource {
        char *device_id;
        enum resource_type type;
        union {
                struct {
                        void *base;
                        size_t len;
                } mem_reg;
                struct {
                        void *base;
                        size_t len;
                } dev_buffer;
        } data;
};

typedef void (*resources_iter_fn_t)(struct resource *r);
void resources_iter(resources_iter_fn_t iter_fn);

void resources_register_res(struct resource r);

#endif /* _KERNEL_RESOURCES_H */
