#ifndef _KERNEL_MODULES_H
#define _KERNEL_MODULES_H

#include "lib/ds/slist.h"

#include <stdbool.h>

struct module {
        char const *name;

        struct module_fns {
                bool (*available)(void);
                void (*load)(void);
                void (*unload)(void);
        } fns;

        struct slist_ref state_list;
};

void modules_init(void);

void modules_load_available(void);

#endif /* _KERNEL_MODULES_H */
