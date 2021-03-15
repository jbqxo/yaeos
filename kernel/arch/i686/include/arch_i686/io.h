#ifndef _KERNEL_ARCH_I686_IO_H
#define _KERNEL_ARCH_I686_IO_H

#include <stdint.h>

/* Cast ports to uint16_t to force use of "dx" register instead of "dh" or "dl". */

#define iowrite(port, value) \
        asm volatile("out %[v], %[p]" : : [v] "a,a"(value), [p] "N,d"((uint16_t)port));
#define ioread(port, type)                                                                 \
        ({                                                                                 \
                type v;                                                                    \
                asm volatile("in %[p], %[d]" : [d] "=a,a"(v) : [p] "N,d"((uint16_t)port)); \
                v;                                                                         \
        })

#endif /* _KERNEL_ARCH_I686_IO_H */
