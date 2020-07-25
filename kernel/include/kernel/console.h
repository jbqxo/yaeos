#ifndef _KERNEL_CONSOLE_H
#define _KERNEL_CONSOLE_H

#include <stddef.h>
#include <stdbool.h>
#include <kernel/ds/slist.h>

struct console {
#define CONSRC_OK (0)
#define CONSRC_NOTREADY (-1)
	const char *name;
	void (*write)(struct console *,const char *msg, size_t len);
	int (*init)(struct console *);
#define CONSFLAG_EARLY (0x1)
	unsigned flags;

	SLIST_FIELD(console);
};

void console_init(void);
void console_write(const char *msg, size_t len);

#endif // _KERNEL_CONSOLE_H
