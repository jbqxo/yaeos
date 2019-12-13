#pragma once

#include <stddef.h>

void tty_init(void);
void tty_putchar(char);
void tty_write(const char *data, size_t size);
void tty_writestring(const char *);
