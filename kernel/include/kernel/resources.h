#ifndef _KERNEL_RESOURCES_H
#define _KERNEL_RESOURCES_H

#include "kernel/modules.h"

#include "lib/cppdefs.h"
#include "lib/ds/slist.h"

#include <stddef.h>

enum resource_type {
        RESOURCE_TYPE_MEMORY,
        RESOURCE_TYPE_DEV_BUFFER,
};

struct resource {
        char const *device_id;
        char const *resource_id;
#define resource_of(LISTPTR) container_of((LISTPTR), struct resource, list)
        struct slist_ref list;

        union resource_owner {
                enum resource_owner_kind {
                        RES_OWNER_NONE = 0x0,
                        RES_OWNER_KERNEL = 0x1,
                } state;
                struct module *module;
        } owner;

        enum resource_type type;
        union resource_data {
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

void resources_init(void);

struct resource *resources_claim_by_id(char const *device_id, char const *resource_id,
                                       struct module *owner);
struct resource *resources_claim_by_type(enum resource_type type, struct module *owner);

struct resource *resources_kclaim_by_id(char const *device_id, char const *resource_id);
struct resource *resources_kclaim_by_type(enum resource_type type);

void resources_register(char const *device_id, char const *resource_id, enum resource_type type,
                        union resource_data data);

#endif /* _KERNEL_RESOURCES_H */
