#include "kernel/kernel.h"

#include "arch_i686/resources.h"
#include "arch_i686/descriptors.h"
#include "arch_i686/early_paging.h"
#include "arch_i686/exceptions.h"
#include "arch_i686/intr.h"
#include "arch_i686/kernel.h"
#include "arch_i686/vm.h"

#include "kernel/kernel.h"
#include "kernel/klog.h"
#include "kernel/mm/highmem.h"
#include "kernel/platform_consts.h"

#include "lib/cppdefs.h"

size_t const PLATFORM_PAGE_SIZE = 4096;
size_t const PLATFORM_REGISTERS_COUNT = 20;
size_t const PLATFORM_PAGEDIR_SIZE = PLATFORM_PAGE_SIZE;
size_t const PLATFORM_PAGEDIR_COUNT = 2;
size_t const PLATFORM_PAGEDIR_PAGES = 1024;

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

struct arch_info_i686 I686_INFO;

void i686_init(multiboot_info_t *info, uint32_t magic)
{
        if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
                // TODO: Panic
                return;
        }

        setup_boot_paging();

        highmem_set_offset(ptr2uint(kernel_vma));

        boot_setup_gdt();
        boot_setup_idt();
        intr_init();
        i686_setup_exception_handlers();
        intr_handler_cpu(INTR_CPU_PAGEFAULT, i686_vm_pg_fault_handler);

        union uiptr info_addr = ptr2uiptr(info);
        info_addr.ptr = highmem_to_high(info_addr.ptr);
        I686_INFO.multiboot = info_addr.ptr;
        patch_multiboot_info(I686_INFO.multiboot);

        i686_register_resources();

        kernel_init();
}
