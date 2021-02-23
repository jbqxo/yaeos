#include "kernel/kernel.h"

#include "arch_i686/kernel.h"

#include "kernel/klog.h"
#include "kernel/platform_consts.h"

#include "lib/cppdefs.h"

size_t const PLATFORM_PAGE_SIZE = 4096;
size_t const PLATFORM_REGISTERS_COUNT = 20;
size_t const PLATFORM_PAGEDIR_SIZE = PLATFORM_PAGE_SIZE;
size_t const PLATFORM_PAGEDIR_COUNT = 2;
size_t const PLATFORM_PAGEDIR_PAGES = 1024;

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
