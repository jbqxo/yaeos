#pragma once

#include <stddef.h>

typedef void *tty_descriptor_t;

tty_descriptor_t tty_platform_get_descriptor(void);

void tty_putchar(tty_descriptor_t desc, char c);

void tty_write(tty_descriptor_t desc, const char *data, size_t size);
void tty_writeln(tty_descriptor_t desc, const char *data, size_t size);

void tty_print(tty_descriptor_t desc, const char *str);
void tty_println(tty_descriptor_t desc, const char *str);
