#ifndef _KERNEL_ARCH_I686_KERNEL_H
#define _KERNEL_ARCH_I686_KERNEL_H

#include <stdint.h>

extern union vm_arch_page_dir boot_paging_pd asm("boot_paging_pd");
extern union vm_arch_page_dir boot_paging_pt asm("boot_paging_pt");

extern char kernel_bootstack_start[] asm("bootstack_top");
extern char kernel_bootstack_end[] asm("bootstack_bottom");

extern char kernel_vma[] asm("__kernel_vma");
#define KERNEL_VM_OFFSET (ptr2uint(kernel_vma))

extern char kernel_start[] asm("__kernel_start");
extern char kernel_end[] asm("__kernel_end");

extern char kernel_text_start[] asm("__kernel_text_start");
extern char kernel_text_end[] asm("__kernel_text_end");

extern char kernel_rodata_start[] asm("__kernel_rodata_start");
extern char kernel_rodata_end[] asm("__kernel_rodata_end");

extern char kernel_data_start[] asm("__kernel_data_start");
extern char kernel_data_end[] asm("__kernel_data_end");

extern char kernel_bss_start[] asm("__kernel_bss_start");
extern char kernel_bss_end[] asm("__kernel_bss_end");

#endif // _KERNEL_ARCH_I686_KERNEL_H
