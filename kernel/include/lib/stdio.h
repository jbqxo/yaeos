#ifndef _LIBC_STDIO_H
#define _LIBC_STDIO_H

#ifdef __libk__
#include <kernel/console.h>
#endif

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#define restrict __restrict
#endif

// TODO: Add proper definition of FILE structure before coming to the user-space.
typedef struct STRUCT_FILE {
} FILE;

#ifdef __libc__
int vfprintf(FILE *restrict stream, const char *restrict format, va_list arg);
int fprintf(FILE *restrict stream, const char *restrict format, ...)
	__attribute__((format(printf, 2, 3)));
#elif __libk__
int vfprintf(const char *restrict format, va_list arg);
int fprintf(const char *restrict format, ...) __attribute__((format(printf, 1, 2)));
#endif

#ifdef __cplusplus
#undef restrict
}
#endif

#endif // _LIBC_STDIO_H
