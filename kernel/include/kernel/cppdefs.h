#ifndef _KERNEL_CPPDEFS_H
#define _KERNEL_CPPDEFS_H

#define __unused                __attribute__((__unused__))
#define __used                  __attribute__((__used__))
#define __dead_code             __attribute__((__noreturn__))
#define __packed                __attribute__((__packed__))
#define __section(target)       __attribute__((__section__(target)))
#define __aligned(b)            __attribute__((__aligned__(b)))
#define __likely(exp)           __builtin_expect((exp), 1)
#define __unlikely(exp)         __builtin_expect((exp), 0)

#endif // _KERNEL_CPPDEFS_H
