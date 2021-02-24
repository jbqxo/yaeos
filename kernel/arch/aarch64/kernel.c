#include "kernel/kernel.h"
#include "kernel/klog.h"
#include "kernel/platform_consts.h"

size_t const PLATFORM_PAGE_SIZE = 4096;

/* TODO: Change */
size_t const PLATFORM_REGISTERS_COUNT = 20;
size_t const PLATFORM_PAGEDIR_SIZE = PLATFORM_PAGE_SIZE;
size_t const PLATFORM_PAGEDIR_COUNT = 2;
size_t const PLATFORM_PAGEDIR_PAGES = 1024;

void kernel_arch_get_segment(enum kernel_segments seg, void **start, void **end)
{
        LOGF_P("Yet to be implemented\n");
}

union vm_arch_page_dir *kernel_arch_get_early_pg_root(void)
{
        LOGF_P("Yet to be implemented\n");
        return (NULL);
}

void aarch64_init(void)
{}
