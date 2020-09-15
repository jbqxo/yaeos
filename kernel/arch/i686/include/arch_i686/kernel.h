#ifndef _KERNEL_ARCH_I686_KERNEL_H
#define _KERNEL_ARCH_I686_KERNEL_H

#include <stdint.h>

extern uint32_t boot_paging_pt[] asm("boot_paging_pt");
extern uint32_t boot_paging_pd[] asm("boot_paging_pd");

extern char bootstack_top[] asm("bootstack_top");
extern char bootstack_bottom[] asm("bootstack_bottom");

extern const char kernel_vma[] asm("__kernel_vma");
extern const char kernel_start[] asm("__kernel_start");
extern const char kernel_end[] asm("__kernel_end");

extern const char kernel_text_start[] asm("__kernel_text_start");
extern const char kernel_text_end[] asm("__kernel_text_end");

extern const char kernel_rodata_start[] asm("__kernel_rodata_start");
extern const char kernel_rodata_end[] asm("__kernel_rodata_end");

extern char kernel_data_start[] asm("__kernel_data_start");
extern char kernel_data_end[] asm("__kernel_data_end");

extern char kernel_bss_start[] asm("__kernel_bss_start");
extern char kernel_bss_end[] asm("__kernel_bss_end");

#define KERNEL_VMA ((uintptr_t)&kernel_vma[0])

#define HIGH(addr) ((uintptr_t)(KERNEL_VMA + (uintptr_t)(addr)))
#define LOW(addr)  ((uintptr_t)((uintptr_t)(addr)-KERNEL_VMA))

#endif // _KERNEL_ARCH_I686_KERNEL_H
