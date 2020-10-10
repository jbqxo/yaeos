#ifndef _KERNEL_KERNEL_H
#define _KERNEL_KERNEL_H

#include "kernel/config.h"
#include "kernel/mm/vmm.h"

#ifdef __i686__
#include "arch_i686/kernel.h"
#endif // __i686__

struct vm_space kvm_space;

#endif // _KERNEL_KERNEL_H
