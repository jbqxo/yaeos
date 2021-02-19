#ifndef _LIB_STDIO_H
#define _LIB_STDIO_H

#include <stdarg.h>
#include <stddef.h>

typedef int (*fprintf_fn)(const char *data, size_t len);

int kvfprintf(fprintf_fn f, const char *restrict format, va_list arg);
int kfprintf(fprintf_fn f, const char *restrict format, ...) __attribute__((format(printf, 2, 3)));

#endif // _LIB_STDIO_H
