#include "kernel/kernel.h"

#include "arch_i686/kernel.h"

#include "kernel/klog.h"

#include "lib/cppdefs.h"

void *kernel_arch_vm_offset(void)
{
        return (kernel_vma);
}

void *kernel_arch_to_low(const void *high_addr)
{
        union uiptr u = ptr2uiptr(high_addr);
        u.num -= ptr2uint(kernel_vma);
        return (u.ptr);
}

void *kernel_arch_to_high(const void *low_addr)
{
        union uiptr u = ptr2uiptr(low_addr);
        u.num += ptr2uint(kernel_vma);
        return (u.ptr);
}

bool kernel_arch_is_highmem(const void *addr)
{
        return (ptr2uint(addr) >= ptr2uint(kernel_vma));
}

union vm_arch_page_dir *kernel_arch_get_early_pg_root(void)
{
        return (&boot_paging_pd);
}

void kernel_arch_get_segment(enum kernel_segments seg, void **start, void **end)
{
        switch (seg) {
        case KSEGMENT_TEXT: {
                *start = kernel_text_start;
                *end = kernel_text_end;
        } break;
        case KSEGMENT_RODATA: {
                *start = kernel_rodata_start;
                *end = kernel_rodata_end;
        } break;
        case KSEGMENT_DATA: {
                *start = kernel_data_start;
                *end = kernel_data_end;
        } break;
        case KSEGMENT_BSS: {
                *start = kernel_bss_start;
                *end = kernel_bss_end;
        } break;
        default: LOGF_P("Requested info about an unknown segment\n");
        }
}
