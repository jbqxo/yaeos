#include "kernel/resources.h"

#include "lib/cppdefs.h"
#include "lib/cstd/assert.h"
#include "lib/cstd/string.h"
#include "lib/ds/slist.h"
#include "lib/utils.h"

#include <stdbool.h>

#define MAX_RESOURCES (256u)

static struct resource RESOURCES_STORAGE[MAX_RESOURCES] = { 0 };
static size_t RESOURCES_NEXT_FREE_NDX = 0;

static struct {
        struct slist_ref claimed_head;
        struct slist_ref free_head;
} RESOURCES;

void resources_init(void)
{
        slist_init(&RESOURCES.claimed_head);
        slist_init(&RESOURCES.free_head);
}

static struct resource *resources_get_new(void)
{
        size_t const max_ndx = ARRAY_SIZE(RESOURCES_STORAGE) - 1;

        if (RESOURCES_NEXT_FREE_NDX > max_ndx) {
                kassert(false);
                return (NULL);
        }

        struct resource *new = &RESOURCES_STORAGE[RESOURCES_NEXT_FREE_NDX];
        RESOURCES_NEXT_FREE_NDX++;
        return (new);
}

static struct resource *claim_by_id(char const *device_id, char const *resource_id,
                                    union resource_owner owner)
{
        bool found = false;
        struct slist_ref *cursor = &RESOURCES.free_head;
        while (slist_next(cursor) != NULL) {
                struct slist_ref *next = slist_next(cursor);
                struct resource *r = resource_of(next);

                bool const match_dev = kstrcmp(device_id, r->device_id) == 0;
                bool const match_res = kstrcmp(resource_id, r->resource_id) == 0;
                if (match_dev && match_res) {
                        found = true;
                        break;
                }

                cursor = next;
        }

        if (!found) {
                return (NULL);
        }

        struct slist_ref *resource_ref = slist_next(cursor);
        slist_remove_next(cursor);
        slist_insert(&RESOURCES.claimed_head, resource_ref);

        struct resource *r = resource_of(resource_ref);
        kassert(r->owner.state == RES_OWNER_NONE);
        r->owner = owner;

        return (r);
}

static struct resource *claim_by_type(enum resource_type type, union resource_owner owner)
{
        bool found = false;
        struct slist_ref *cursor = &RESOURCES.free_head;
        while (slist_next(cursor) != NULL) {
                struct slist_ref *next = slist_next(cursor);
                struct resource *r = resource_of(next);

                if (r->type == type) {
                        found = true;
                        break;
                }

                cursor = next;
        }

        if (!found) {
                return (NULL);
        }

        struct slist_ref *resource_ref = slist_next(cursor);
        slist_remove_next(cursor);
        slist_insert(&RESOURCES.claimed_head, resource_ref);

        struct resource *r = resource_of(resource_ref);
        kassert(r->owner.state == RES_OWNER_NONE);
        r->owner = owner;

        return (r);
}

struct resource *resources_claim_by_id(char const *device_id, char const *resource_id,
                                       struct module *owner)
{
        union resource_owner o = { .module = owner };
        return (claim_by_id(device_id, resource_id, o));
}

struct resource *resources_claim_by_type(enum resource_type type, struct module *owner)
{
        union resource_owner o = { .module = owner };
        return (claim_by_type(type, o));
}

struct resource *resources_kclaim_by_id(char const *device_id, char const *resource_id)
{
        union resource_owner o = { .state = RES_OWNER_KERNEL };
        return (claim_by_id(device_id, resource_id, o));
}

struct resource *resources_kclaim_by_type(enum resource_type type)
{
        union resource_owner o = { .state = RES_OWNER_KERNEL };
        return (claim_by_type(type, o));
}

void resources_register(char const *device_id, char const *resource_id, enum resource_type type,
                        union resource_data data)
{
        struct resource *new = resources_get_new();
        rkassert(new != NULL);

        slist_init(&new->list);
        new->device_id = device_id;
        new->resource_id = resource_id;
        new->type = type;
        new->data = data;
        new->owner.state = RES_OWNER_NONE;

        slist_insert(&RESOURCES.free_head, &new->list);
}
