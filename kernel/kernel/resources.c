#include "kernel/resources.h"

#include "lib/cppdefs.h"
#include "lib/cstd/string.h"
#include "lib/cstd/assert.h"

#include <stdbool.h>

#define MAX_RESOURCES (256u)

static struct resource RESOURCES_STORAGE[MAX_RESOURCES];
static size_t RESOURCES_NEXT_FREE_NDX = 0;

static struct resource *resources_get_free(void)
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

void resources_iter(resources_iter_fn_t iter_fn)
{
        bool no_resources = RESOURCES_NEXT_FREE_NDX == 0;
        if (no_resources) {
                return;
        }

        size_t const last_ndx = RESOURCES_NEXT_FREE_NDX - 1;

        for (size_t i = 0; i <= last_ndx; i++) {
                struct resource *r = &RESOURCES_STORAGE[i];
                iter_fn(r);
        }
}

void resources_register_res(struct resource r)
{
        struct resource *new = resources_get_free();
        kmemcpy(new, &r, sizeof(*new));
}
