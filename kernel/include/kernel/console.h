#ifndef _KERNEL_CONSOLE_H
#define _KERNEL_CONSOLE_H

#include "lib/ds/slist.h"

#include <stdbool.h>
#include <stddef.h>

struct console {
#define CONSRC_OK       (0)
#define CONSRC_NOTREADY (-1)
        const char *name;
        void (*write)(struct console *, const char *msg, size_t len);
        void (*clear)(struct console *);
        int (*init)(struct console *);
        void *data;
#define CONSFLAG_EARLY (0x1)
        unsigned flags;

        struct slist_ref active_consoles;
};

void console_init(void);
void console_write(const char *msg, size_t len);
void console_clear(void);

#endif /* _KERNEL_CONSOLE_H */
