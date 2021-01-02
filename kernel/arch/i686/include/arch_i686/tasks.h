#ifndef _KERNEL_ARCH_I686_TASKS_H
#define _KERNEL_ARCH_I686_TASKS_H

#include "kernel/cppdefs.h"

#include <stdint.h>

struct task_arch_execstate {
        uint32_t esp __asmexport;
        uint32_t ebp __asmexport;
        uint32_t eip __asmexport;
};

#endif // _KERNEL_ARCH_I686_TASKS_H
