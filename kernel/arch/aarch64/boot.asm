#include "kernel/config.h"

#define PLATFORM_PAGE_SIZE 4096

.section .bootstack, "aw", @nobits
.global bootstack_bottom
.global bootstack_top
bootstack_bottom:
        .skip CONF_STACK_SIZE
bootstack_top:


.section .text

.global _start
.type _start, @function
_start:
        bl hang_on_all_but_first
        bl switch_to_el1
        bl prepare_bss
        bl aarch64_init
1:
        wfe
        b 1b
.size _start, . - _start

.type hang_on_all_but_first, @function
hang_on_all_but_first:
        mrs x1, mpidr_el1
        and x1, x1, #3
        cbz x1, 2f
1:
        wfe
        b 1b
2:
        ret
.size hang_on_all_but_first, . - hang_on_all_but_first

.type switch_to_el1, @function
switch_to_el1:
        /* TODO implement */
        ret
.size switch_to_el1, . - switch_to_el1

.type prepare_bss, @function
prepare_bss:
        /* Setup stack */
        ldr x5, =bootstack_bottom
        mov sp, x5

        /* Fill the .bss segment with 0s. Just in case. */
        ldr x5, =__kernel_bss_end
        ldr x6, =__kernel_bss_start
        sub x5, x5, x6
        cbz x5, 2f
1:
        str xzr, [x6]
        sub x5, x5, #1
        cbnz x5, 1b
2:
        ret
.size prepare_bss, . - prepare_bss
