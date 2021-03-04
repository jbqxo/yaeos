#include "kernel/modules.h"

#include "lib/cstd/assert.h"
#include "lib/ds/slist.h"
#include "lib/elflist.h"
#include "lib/cppdefs.h"

#include <stddef.h>

ELFLIST_DECLARE(modules);

static struct slist_ref INACTIVE_MODULES;
static struct slist_ref LOADED_MODULES;

static void register_module(struct module *m)
{
        kassert(m != NULL);
        kassert(m->fns.available != NULL && m->fns.available());

        slist_init(&m->state_list);
        slist_insert(&INACTIVE_MODULES, &m->state_list);
}

static void modules_iterate_over_available(void (*fn)(struct module *))
{
        struct module **it = NULL;
        ELFLIST_FOREACH (struct module, modules, it) {
                struct module *m = *it;
                if (m->fns.available != NULL && m->fns.available()) {
                        fn(m);
                }
        }
}

void modules_init(void)
{
        slist_init(&INACTIVE_MODULES);
        slist_init(&LOADED_MODULES);

        modules_iterate_over_available(register_module);
}

void modules_load_available(void)
{
        SLIST_FOREACH(it, slist_next(&INACTIVE_MODULES)) {
                struct module *m = container_of(it, struct module, state_list);
                if (m->fns.load != NULL) {
                        m->fns.load();
                }
        }
}
