#include "kernel/console.h"

#include "lib/cppdefs.h"
#include "lib/cstd/assert.h"
#include "lib/ds/slist.h"
#include "lib/elflist.h"
#include "lib/utils.h"

static struct slist_ref ACTIVE_CONSOLES;

void consoles_init(void)
{
        slist_init(&ACTIVE_CONSOLES);
}

void console_register(struct console *c)
{
        kassert(c != NULL);
        slist_insert(&ACTIVE_CONSOLES, &c->active_consoles);
}

void console_clear()
{
        SLIST_FOREACH (it, slist_next(&ACTIVE_CONSOLES)) {
                struct console *c = container_of(it, struct console, active_consoles);
                if (c->ops.clear != NULL) {
                        c->ops.clear(c);
                }
        }
}

void console_write(const char *msg, size_t len)
{
        SLIST_FOREACH (it, slist_next(&ACTIVE_CONSOLES)) {
                struct console *c = container_of(it, struct console, active_consoles);
                if (c->ops.write != NULL) {
                        c->ops.write(c, msg, len);
                }
        }
}
