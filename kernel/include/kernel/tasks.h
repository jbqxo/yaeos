#ifndef _KERNEL_TASKS_H
#define _KERNEL_TASKS_H

#include "kernel/mm/vmm.h"

#ifdef __i686__
#include "arch_i686/tasks.h"
#endif // __i686__

typedef int (*task_entrypoint_fn)(void);

struct task {
        uint32_t pid;
        char *display_name;
        struct vmm_space *userspace;
        struct vmm_arch_page_tree *pt;

        struct task_arch_execstate state __asmexport;
};

///
/// Initialize architecture independent tasks management.
///
void tasks_init(void);

///
/// Returns a task structure for the task that was started at the system's boot.
///
struct task *tasks_get_boottask(void);

///
/// Creates a new task and executes given function.
///
struct task *tasks_create_new(char *name, task_entrypoint_fn);

///
/// Initialize architecture dependent tasks management.
/// @note Intended to be called by `tasks_init`.
///
void tasks_arch_init(void);

///
/// Returns an addres of a top of the stack used by a boot task.
///
void *tasks_arch_get_bootstack(void);

///
/// Returns architecture specific info about the execution state.
///
void tasks_arch_get_execstate(struct task_arch_execstate *);

///
/// Returns the task being executed on the current CPU.
///
struct task *tasks_arch_get_currenttask(void);

///
/// Sets the task being executed on the current CPU.
///
void tasks_arch_set_currenttask(struct task *);

///
/// Switch to the task.
/// @note Must be called with disabled irq.
///
void tasks_arch_switch(struct task *to);

///
/// Get physical address of a root of a page tree suitable for use in CR3 register
///
void *tasks_arch_phys_pt_root(struct task *);

void tasks_arch_init_task(struct task *);

#endif // _KERNEL_TASKS_H
