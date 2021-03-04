#ifndef _KERNEL_CONSOLE_H
#define _KERNEL_CONSOLE_H

#include "lib/ds/slist.h"

#include <stdbool.h>
#include <stddef.h>

struct console {
        const char *name;
        struct slist_ref active_consoles;
        void *data;

        struct {
                void (*write)(struct console *, const char *msg, size_t len);
                void (*clear)(struct console *);
        } ops;
};

void consoles_init(void);
void console_register(struct console *c);
void console_write(const char *msg, size_t len);
void console_clear(void);

#endif /* _KERNEL_CONSOLE_H */
