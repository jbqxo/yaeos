#include "arch_i686/early_paging.h"

#include "arch_i686/vm.h"
#include "arch_i686/kernel.h"
#include "kernel/kernel.h"

#include "lib/align.h"
#include "lib/cppdefs.h"

#define TO_LOW(addr)  (void *)((uintptr_t)(addr) - (KERNEL_VM_OFFSET))
#define TO_HIGH(addr) (void *)((uintptr_t)(addr) + (KERNEL_VM_OFFSET))

/**
 * @brief Adjust the frame with given offset.
 * @param level The depth of the frame to patch.
 * @param offset The offset to apply to the addresses in the frame.
 */
#define PATCH_FRAME(level, offset)                                \
        do {                                                      \
                uint32_t *frame = __builtin_frame_address(level); \
                /* Patch return address */                        \
                uint32_t *ra = &frame[1];                         \
                *ra += (offset);                                  \
                /* Patch previous ebp */                          \
                uint32_t *bp = &frame[0];                         \
                *bp += (offset);                                  \
        } while (false)

/**
 * @brief Map the pages in the range of addresses to the high memory.
 *
 * @param page_dir Page Directory to modify
 * @param flags Flags to apply to every page.
 */
static void map_addr_range(union vm_arch_page_dir *page_dir, const void *start, const void *end,
                           enum vm_table_flags flags)
{
        size_t const *page_size = TO_LOW(&PLATFORM_PAGE_SIZE);
        union uiptr page_addr = uint2uiptr(align_rounddown(ptr2uint(start), *page_size));
        union uiptr upto_page = uint2uiptr(align_roundup(ptr2uint(end), *page_size));

        while (page_addr.num < upto_page.num) {
                const union uiptr virt_addr = uint2uiptr(KERNEL_VM_OFFSET + page_addr.num);

                struct vm_arch_page_entry *pde = vm_dir_entry(page_dir, virt_addr.ptr);
                union vm_arch_page_dir *pt = vm_pt_get_addr(pde);
                struct vm_arch_page_entry *pte = vm_table_entry(pt, virt_addr.ptr);

                vm_pt_set_addr(pte, page_addr.ptr);
                pte->is_present = true;
                pte->present.table.flags = flags;

                page_addr.num += *page_size;
        }
}

static void map_segment(enum kernel_segments seg, union vm_arch_page_dir *pdir,
                        enum vm_table_flags flags)
{
        union uiptr start = ptr2uiptr(NULL);
        union uiptr end = ptr2uiptr(NULL);
        kernel_arch_get_segment(seg, &start.ptr, &end.ptr);

        start.ptr = TO_LOW(start.ptr);
        end.ptr = TO_LOW(end.ptr);

        map_addr_range(pdir, start.ptr, end.ptr, flags);
}

/**
 * @brief Map kernel sections from low memory to high memory.
 */
static void map_kernel(union vm_arch_page_dir *page_dir)
{
        map_segment(KSEGMENT_TEXT, page_dir, 0);
        map_segment(KSEGMENT_DATA, page_dir, VM_TABLE_FLAG_RW);
        map_segment(KSEGMENT_RODATA, page_dir, 0);
        map_segment(KSEGMENT_BSS, page_dir, VM_TABLE_FLAG_RW);
}

/**
 * @brief Map platform specific regions from low memory to high memory.
 */
static void map_platform(union vm_arch_page_dir *page_dir)
{
        // Map low 1MiB
        const union uiptr start = uint2uiptr(0x0);
        const union uiptr end = uint2uiptr(1 * 1024 * 1024);
        map_addr_range(page_dir, start.ptr, end.ptr, VM_TABLE_FLAG_RW);
}

__noinline void setup_boot_paging(void)
{
        union uiptr pt_addr = ptr2uiptr(&boot_paging_pt);
        union uiptr pd_addr = ptr2uiptr(&boot_paging_pd);

        pt_addr.ptr = TO_LOW(pt_addr.ptr);
        pd_addr.ptr = TO_LOW(pd_addr.ptr);

        union vm_arch_page_dir *pd = pd_addr.ptr;
        union vm_arch_page_dir *pt = pt_addr.ptr;

        // Identity mapping
        struct vm_arch_page_entry *pde_low = vm_dir_entry(pd, 0x0);
        pde_low->is_present = true;
        vm_pt_set_addr(pde_low, pt);
        pde_low->present.dir.flags = VM_DIR_FLAG_RW;

        // Map the kernel to higher half of address space
        struct vm_arch_page_entry *pde_high = vm_dir_entry(pd, uint2ptr(KERNEL_VM_OFFSET));
        pde_high->is_present = true;
        vm_pt_set_addr(pde_high, pt);
        pde_high->present.dir.flags = VM_DIR_FLAG_RW;

        map_kernel(pd);
        map_platform(pd);

        vm_set_active_pt(pd);
        vm_paging_enable(KERNEL_VM_OFFSET);

        // Patch i686_init(...)
        PATCH_FRAME(1, KERNEL_VM_OFFSET);
        // Patch setup_boot_paging(...)
        PATCH_FRAME(0, KERNEL_VM_OFFSET);

        // Undo identity mapping
        pde_low->is_present = false;
        vm_tlb_flush();
}

void patch_multiboot_info(multiboot_info_t *info)
{
        union uiptr addr = uint2uiptr(info->mmap_addr);
        addr.ptr = TO_HIGH(addr.ptr);
        info->mmap_addr = addr.num;
}
