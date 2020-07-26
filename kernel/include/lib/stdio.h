#ifndef _LIB_STDIO_H
#define _LIB_STDIO_H

#include <kernel/console.h>
#include <stdarg.h>

typedef int (*fprintf_fn)(const char *data, size_t len);

int vfprintf(fprintf_fn f, const char *restrict format, va_list arg);
int fprintf(fprintf_fn f, const char *restrict format, ...) __attribute__((format(printf, 2, 3)));

#endif // _LIB_STDIO_H
