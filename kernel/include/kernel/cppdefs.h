#ifndef _KERNEL_CPPDEFS_H
#define _KERNEL_CPPDEFS_H

#include <stdint.h>

#define __unused                __attribute__((__unused__))
#define __used                  __attribute__((__used__))
#define __dead_code             __attribute__((__noreturn__))
#define __packed                __attribute__((__packed__))
#define __weak                  __attribute__((__weak__))
#define __noinline              __attribute__((noinline))
#define __noreturn              __attribute__((noreturn))
#define __always_inline         __attribute__((always_inline))
#define __section(target)       __attribute__((__section__(target)))
#define __aligned(b)            __attribute__((__aligned__(b)))
#define __likely(exp)           __builtin_expect((exp), 1)
#define __unlikely(exp)         __builtin_expect((exp), 0)

#define TO_SSTR(X) #X
#define TO_SSTR_MACRO(X) TO_SSTR(X)

union uiptr {
	void *p;
	uintptr_t i;
};

#define UIPTR(x)                                                                                   \
	(_Generic((x),                                                                             \
		 void *: (union uiptr){ .p = (void*)(x) },                                         \
		 uintptr_t: (union uiptr){ .i = (uintptr_t)(x) }))

#endif // _KERNEL_CPPDEFS_H
