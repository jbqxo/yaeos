#include "kernel/tasks.h"

#include "arch_i686/kernel.h"
#include "arch_i686/tasks.h"
#include "arch_i686/vmm.h"

#include <stddef.h>

// TODO: Track current tasks for all processors.
static struct task *CPU_TASK;

void tasks_arch_init(void)
{
        CPU_TASK = tasks_get_boottask();
}

void *tasks_arch_get_bootstack(void)
{
        return (bootstack_top);
}

struct task *tasks_arch_get_currenttask(void)
{
        return (CPU_TASK);
}

void tasks_arch_set_currenttask(struct task *task)
{
        CPU_TASK = task;
}

void *tasks_arch_phys_pt_root(struct task *t)
{
        kassert(t->pt->pagedirs_lengths[0] > 0);
        void *vroot = &t->pt->pagedirs[0];
        void *proot = vmm_virtual_to_physical(t->pt, vroot);
        return (proot);
}
