#include "offsets/kernel/tasks.h"

.section .text

.global tasks_arch_switch
.type   tasks_arch_switch, @function

tasks_arch_switch:
        // Save previous task's state.
        // EAX, ECX, EDX, EIP are saved by the caller.
        push %ebx
        push %esi
        push %edi
        push %ebp

        // EAX contains an address of current task.
        call tasks_arch_get_currenttask
#define TASK_ESP (OFFSETS__TASK__STATE + OFFSETS__TASK_ARCH_EXECSTATE__ESP)
        movl %esp, TASK_ESP(%edi)

        // Load new task's state.
        // ESI contains a pointer to a new task.
        movl 20(%esp), %esi

        pushl %esi
        call tasks_arch_set_currenttask

        // Load a physical address of a PT root into the EAX register.
        call tasks_arch_phys_pt_root
        addl $4, %esp

        // Load new ESP.
        movl TASK_ESP(%esi), %esp
        // Load current task's PT root.
        movl %cr3, %ecx
        // Compare new task's PT root with the current task's root.
        cmpl %ecx, %eax
        // Skip unnecessary TLB flush.
        je 1f
                // Load the new PT, and cause TLB flush.
                movl %eax, %cr3
1:

        // TODO: Adjust ESP0 in the TSS.

        popl %ebp
        popl %edi
        popl %esi
        popl %ebx
        // Load EIP from the stack.
        ret

.size tasks_arch_switch, . - tasks_arch_switch

.global tasks_arch_get_execstate
.type tasks_arch_get_execstate, @function
tasks_arch_get_execstate:
        // Load an address of the structure.
        movl 4(%esp), %eax

        // Save EIP.
        movl (%esp), %edx
        movl %edx, OFFSETS__TASK_ARCH_EXECSTATE__EIP(%eax)

        // Save EBP.
        movl %ebp, OFFSETS__TASK_ARCH_EXECSTATE__EBP(%eax)

        // Save ESP.
        movl %esp, %edx
        // Substract the return address and an argument.
        subl $8, %edx
        movl %edx, OFFSETS__TASK_ARCH_EXECSTATE__ESP(%eax)
        ret

.size tasks_arch_get_execstate, . - tasks_arch_get_execstate
