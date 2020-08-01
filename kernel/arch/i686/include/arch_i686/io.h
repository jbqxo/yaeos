#ifndef _KERNEL_ARCH_I686_IO_H
#define _KERNEL_ARCH_I686_IO_H

#include <stdint.h>

#define iowrite(port, value) asm volatile("out %[v], %[p]" : : [v] "a"(value), [p] "Nd"(port));
#define ioread(port, destination)                                                                  \
	asm volatile("in %[p], %[d]" : [d] "=a"(destination) : [p] "Nd"(port));

#endif // _KERNEL_ARCH_I686_IO_H
