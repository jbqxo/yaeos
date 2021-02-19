#include "lib/console.h"

#include "lib/ds/slist.h"
#include "lib/elflist.h"

ELFLIST_EXTERN(struct console, consoles);

static SLIST_HEAD(, struct console) ACTIVE_CONSOLES;

void console_init(void)
{
        struct console **c;
        SLIST_INIT(&ACTIVE_CONSOLES);
        ELFLIST_FOREACH (consoles, c) {
                if ((*c)->init != NULL) {
                        int rc = (*c)->init(*c);
                        if (rc != CONSRC_OK) {
                                continue;
                        }
                }
                SLIST_INSERT_HEAD(&ACTIVE_CONSOLES, *c, active_consoles);
        }
}

void console_clear()
{
        struct console *c;
        SLIST_FOREACH (c, &ACTIVE_CONSOLES, active_consoles) {
                if (c->clear != NULL) {
                        c->clear(c);
                }
        }
}

void console_write(const char *msg, size_t len)
{
        struct console *c;
        SLIST_FOREACH (c, &ACTIVE_CONSOLES, active_consoles) {
                if (c->write != NULL) {
                        c->write(c, msg, len);
                }
        }
}
