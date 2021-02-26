#include "kernel/console.h"

#include "lib/cppdefs.h"
#include "lib/ds/slist.h"
#include "lib/elflist.h"

ELFLIST_EXTERN(consoles);

static struct slist_ref ACTIVE_CONSOLES;

void console_init(void)
{
        struct console **c;
        slist_init(&ACTIVE_CONSOLES);
        ELFLIST_FOREACH (struct console, consoles, c) {
                if ((*c)->init != NULL) {
                        int rc = (*c)->init(*c);
                        if (rc != CONSRC_OK) {
                                continue;
                        }
                }
                slist_insert(&ACTIVE_CONSOLES, &(*c)->active_consoles);
        }
}

void console_clear()
{
        SLIST_FOREACH (it, &ACTIVE_CONSOLES) {
                struct console *c = container_of(it, struct console, active_consoles);
                if (c->clear != NULL) {
                        c->clear(c);
                }
        }
}

void console_write(const char *msg, size_t len)
{
        SLIST_FOREACH (it, &ACTIVE_CONSOLES) {
                struct console *c = container_of(it, struct console, active_consoles);
                if (c->write != NULL) {
                        c->write(c, msg, len);
                }
        }
}
