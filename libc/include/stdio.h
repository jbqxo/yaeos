#pragma once

#ifdef __libk__
#include <kernel/tty.h>
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
int vfprintf(tty_descriptor_t d, const char *restrict format, va_list arg);
int fprintf(tty_descriptor_t d, const char *restrict format, ...)
	__attribute__((format(printf, 2, 3)));
#endif

#ifdef __cplusplus
#undef restrict
}
#endif
