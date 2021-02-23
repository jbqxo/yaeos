#ifndef _LIB_CPPDEFS_H
#define _LIB_CPPDEFS_H

#include <stdint.h>

#define __unused          __attribute__((__unused__))
#define __used            __attribute__((__used__))
#define __packed          __attribute__((__packed__))
#define __weak            __attribute__((__weak__))
#define __noinline        __attribute__((noinline))
#define __noreturn        __attribute__((noreturn))
#define __force_inline    inline __attribute__((always_inline))
#define __naked           __attribute__((naked))
#define __section(target) __attribute__((__section__(target)))
#define __aligned(b)      __attribute__((__aligned__(b)))
#define __likely(exp)     __builtin_expect((exp), 1)
#define __unlikely(exp)   __builtin_expect((exp), 0)

#define __asmexport __attribute__((extract_offset))

#define TO_SSTR(X)       #X
#define TO_SSTR_MACRO(X) TO_SSTR(X)

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

/* Shamelessly took the idea from the Linux. It's so cool actually...  */
#define container_of(ptr, type, field_name)                    \
        ({                                                     \
                void *__fptr = (void *)(ptr);                  \
                (type *)(__fptr - offsetof(type, field_name)); \
        })

union uiptr {
        void *ptr;
        uintptr_t num;
};

static inline union uiptr ptr2uiptr(void *ptr)
{
        return ((union uiptr){ .ptr = ptr });
}

static inline union uiptr uint2uiptr(uintptr_t num)
{
        return ((union uiptr){ .num = num });
}

static inline void *uint2ptr(uintptr_t address)
{
        return (uint2uiptr(address).ptr);
}

static inline uintptr_t ptr2uint(void *address)
{
        return (ptr2uiptr(address).num);
}

#endif /* _LIB_CPPDEFS_H */
