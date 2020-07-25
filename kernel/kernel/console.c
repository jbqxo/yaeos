#include <kernel/console.h>
#include <kernel/elflist.h>
#include <kernel/ds/slist.h>

ELFLIST_EXTERN(struct console, consoles);

static struct console *LIST_HEAD;

void console_init(void) {
	struct console **c;
	ELFLIST_FOREACH(consoles, c) {
		if ((*c)->init != NULL) {
			int rc = (*c)->init(*c);
			if (rc != CONSRC_OK) {
				continue;
			}
		}
		SLIST_NODE_INIT(*c);

		if (LIST_HEAD == NULL) {
			LIST_HEAD = *c;
		} else {
			SLIST_ADD_AFTER(LIST_HEAD, *c);
		}
	}
}

void console_write(const char *msg, size_t len) {
	struct console *c;
	SLIST_FOREACH(LIST_HEAD, c) {
		if (c->write != NULL) {
			c->write(c, msg, len);
		}
	}
}
