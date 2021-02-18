#include "kernel/ownership.h"

#include "lib/cstd/assert.h"

#include <stddef.h>

void ownership_init(struct ownership *o)
{
        o->owner = NULL;
}

void ownership_add(struct ownership *o, void *owner)
{
        kassert(o->owner == NULL);
        o->owner = owner;
}

void *ownership_get(struct ownership *o)
{
        return (o->owner);
}
