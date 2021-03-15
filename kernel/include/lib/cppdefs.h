#ifndef _LIB_CPPDEFS_H
#define _LIB_CPPDEFS_H

#include <stdint.h>

#define __unused              __attribute__((__unused__))
#define __maybe_unused        __attribute__((unused))
#define __used                __attribute__((__used__))
#define __packed              __attribute__((__packed__))
#define __weak                __attribute__((__weak__))
#define __noinline            __attribute__((noinline))
#define __const               __attribute__((const))
#define __noreturn            __attribute__((noreturn))
#define __force_inline        inline __attribute__((always_inline))
#define __returns_nonnull     __attribute__((returns_nonnull))
#define __warn_unused         __attribute__((warn_unused_result))
#define __naked               __attribute__((naked))
#define __pure                __attribute__((pure))
/* TODO: Add an option to specify an associated free method when GCC 11.0 is released. */
#define __malloc              __attribute__((malloc, warn_unused_result))
#define __alias(target)       __attribute__((alias (target)))
#define __alloc_align(argpos) __attribute__((alloc_align(argpos)))
#define __section(target)     __attribute__((__section__(target)))
#define __aligned(b)          __attribute__((__aligned__(b)))
#define __likely(exp)         __builtin_expect((exp), 1)
#define __unlikely(exp)       __builtin_expect((exp), 0)

#define __asmexport __attribute__((extract_offset))

#endif /* _LIB_CPPDEFS_H */
