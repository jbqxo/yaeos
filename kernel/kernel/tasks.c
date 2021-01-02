#include "kernel/tasks.h"

#include "kernel/config.h"
#include "kernel/mm/kmm.h"

static struct kmm_cache *CACHE_TASKS;
static struct kmm_cache *CACHE_STACKS;

static struct task *BOOTTASK;

void tasks_init(void)
{
        CACHE_TASKS = kmm_cache_create("tasks_tasks", sizeof(struct task), 0, 0, NULL, NULL);
        CACHE_STACKS = kmm_cache_create("tasks_stacks", CONF_STACK_SIZE, 0, 0, NULL, NULL);

        BOOTTASK = NULL;
        tasks_arch_init();
}

__noreturn static void on_return(struct task *t __unused)
{
        kassert(false);

        // Every task will eventally "return" to this function.
        int32_t returncode;
        asm("movl %%eax, %[rc]" : [rc] "=rm"(returncode));
}

struct task *tasks_create_new(char *restrict name, task_entrypoint_fn fn)
{
        struct task *t = kmm_cache_alloc(CACHE_TASKS);
        void *stack = kmm_cache_alloc(CACHE_STACKS);

        t->display_name = name;
}

struct task *tasks_get_boottask(void)
{
        if (BOOTTASK == NULL) {
                struct task *boottask = kmm_cache_alloc(CACHE_TASKS);
                tasks_arch_get_execstate(&boottask->state);
                // TODO: Assing correct vm space to the boottask.
                boottask->userspace = NULL;
                boottask->display_name = "Boot task";
                BOOTTASK = boottask;
        }

        return (BOOTTASK);
}
