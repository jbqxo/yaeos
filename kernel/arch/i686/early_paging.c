#include "arch_i686/early_paging.h"

#include "arch_i686/kernel.h"
#include "arch_i686/vm.h"

#include "kernel/kernel.h"

#include "lib/align.h"

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
static void map_addr_range(struct i686_vm_pd *page_dir, const void *start, const void *end,
                           enum i686_vm_table_flags flags)
{
        size_t const *page_size = TO_LOW(&PLATFORM_PAGE_SIZE);
        void *page = (void *)align_rounddown((uintptr_t)start, *page_size);
        void *const upto_page = (void *)align_roundup((uintptr_t)end, *page_size);

        while (page < upto_page) {
                void *const high_page = TO_HIGH(page);

                struct i686_vm_pge *pde = i686_vm_get_pge(I686VM_PGLVL_DIR, page_dir, high_page);
                struct i686_vm_pd *pt = i686_vm_pge_get_addr(pde);
                struct i686_vm_pge *pte = i686_vm_get_pge(I686VM_PGLVL_TABLE, pt, high_page);

                i686_vm_pge_set_addr(pte, page);
                pte->any.is_present = true;
                pte->table.flags = flags;

                page += *page_size;
        }
}

static void map_segment(enum kernel_segments seg, struct i686_vm_pd *pdir,
                        enum i686_vm_table_flags flags)
{
        void *start = NULL;
        void *end = NULL;
        kernel_arch_get_segment(seg, &start, &end);

        start = TO_LOW(start);
        end = TO_LOW(end);

        map_addr_range(pdir, start, end, flags);
}

/**
 * @brief Map kernel sections from low memory to high memory.
 */
static void map_kernel(struct i686_vm_pd *page_dir)
{
        map_segment(KSEGMENT_TEXT, page_dir, 0);
        map_segment(KSEGMENT_DATA, page_dir, I686VM_TABLE_FLAG_RW);
        map_segment(KSEGMENT_RODATA, page_dir, 0);
        map_segment(KSEGMENT_BSS, page_dir, I686VM_TABLE_FLAG_RW);
}

/**
 * @brief Map platform specific regions from low memory to high memory.
 */
static void map_platform(struct i686_vm_pd *page_dir)
{
        /* Map low 1MiB */
        void *const start = NULL;
        void *const end = (void *)(1 * 1024 * 1024);
        map_addr_range(page_dir, start, end, I686VM_TABLE_FLAG_RW);
}

__noinline void setup_boot_paging(void)
{
        struct i686_vm_pd *pd = TO_LOW(&boot_paging_pd);
        struct i686_vm_pd *pt = TO_LOW(&boot_paging_pt);

        i686_vm_setup_recursive_mapping(pd, pd);
        i686_vm_setup_recursive_mapping(pt, pt);

        /* Identity mapping. */
        struct i686_vm_pge *pde_low = i686_vm_get_pge(I686VM_PGLVL_DIR, pd, 0x0);
        pde_low->any.is_present = true;
        i686_vm_pge_set_addr(pde_low, pt);
        pde_low->dir.flags = I686VM_DIR_FLAG_RW;

        /* Map the kernel to higher half of address space. */
        void *koffset = (void*)KERNEL_VM_OFFSET;
        struct i686_vm_pge *pde_high = i686_vm_get_pge(I686VM_PGLVL_DIR, pd, koffset);
        *pde_high = *pde_low;

        map_kernel(pd);
        map_platform(pd);

        i686_vm_set_pt(pd);
        i686_vm_paging_enable(KERNEL_VM_OFFSET);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wframe-address"
        /* Patch i686_init(...) */
        PATCH_FRAME(1, KERNEL_VM_OFFSET);
        /* Patch setup_boot_paging(...) */
        PATCH_FRAME(0, KERNEL_VM_OFFSET);
#pragma GCC diagnostic pop

        /* Undo identity mapping */
        pde_low->any.is_present = false;
        i686_vm_tlb_flush();
}

void patch_multiboot_info(multiboot_info_t *info)
{
        info->mmap_addr = (multiboot_uint32_t) TO_HIGH(
                (void *)info->mmap_addr); /* NOTE: Shouldn't overflow on i686. */
}
