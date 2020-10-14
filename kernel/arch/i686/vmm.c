#include "arch_i686/vmm.h"

#include "kernel/klog.h"
#include "kernel/mm/kmm.h"
#include "kernel/mm/vmm.h"

#include <stdbool.h>
#include <stdint.h>

// TODO: Rename vm to vmm_archi686

void *vm_table_entry_addr(void *table, void *vaddr)
{
        // The table index consists of 21:12 bits of an address.
        const uintptr_t MASK = 0x003FF000U;
        uint32_t index = ((uintptr_t)vaddr & MASK) >> 12;
        return (void *)((uintptr_t)table + index * 4);
}

void *vm_dir_entry_addr(void *dir, void *vaddr)
{
        // The directory index consists of 31:22 bits of an address.
        const uintptr_t MASK = 0xFFC00000U;
        uint32_t index = ((uintptr_t)vaddr & MASK) >> 22;
        return (void *)((uintptr_t)dir + index * 4);
}

void vm_tlb_flush(void)
{
        asm volatile("movl %cr0, %eax;"
                     "movl %eax, %cr0");
}

void vm_tlb_invlpg(void *addr)
{
        // TODO: Fallback to CR3->Reg->CR3 if the invlpg instruction unavailable
        asm volatile("invlpg (%0)" ::"r"(addr) : "memory");
}

void vm_set_table_entry(void *table_entry, void *phys_addr, int flags)
{
        uint32_t *entry = table_entry;
        *entry = ((uintptr_t)phys_addr) | flags;
}

void vm_set_dir_entry(void *dir_entry, void *table_addr, int flags)
{
        uint32_t *entry = dir_entry;
        *entry = ((uintptr_t)table_addr) | flags;
}

void vm_map(void *page_dir, void *virt_addr, void *phys_addr, int flags)
{
        uint32_t *pdir_entry = vm_dir_entry_addr(page_dir, virt_addr);
        // TODO: Assert that the table is present.
        // Discard PDE flags.
        void *ptable = (void *)((uintptr_t)*pdir_entry & -0x1000);
        uint32_t *pte = vm_table_entry_addr(ptable, virt_addr);
        vm_set_table_entry(pte, phys_addr, flags);
}

struct {
        struct kmm_cache page_dirs;
        struct kmm_cache page_trees;
} CACHES;

static void construct_page_dir(void *pd)
{
        kmemset(pd, 0, sizeof(vmm_arch_page_dir));
}

static void construct_page_tree(void *pt)
{
        struct vmm_arch_page_tree *ptree = pt;
        for (int i = 0; i < PLATFORM_PAGEDIR_COUNT; i++) {
                ptree->pagedirs[i] = NULL;
                ptree->pagedirs_lengths[i] = 0;
        }
}

void vmm_arch_init(void)
{
        kmm_cache_create("Page Directories", sizeof(vmm_arch_page_dir), PLATFORM_PAGE_SIZE, 0,
                         construct_page_dir, NULL);
        kmm_cache_create("Page Trees", sizeof(struct vmm_arch_page_tree), 0, 0, construct_page_tree,
                         NULL);
}

static void patch_pagetree(struct vmm_arch_page_tree *pt, struct vm_space *vspace)
{
        struct vm_mapping *it;
        VM_SPACE_MAPPINGS_FOREACH (vspace, it) {
                kassert(pt->pagedirs[0] != NULL);
                kassert(pt->pagedirs_lengths[0] > 0);

                uint32_t *ped = vm_dir_entry_addr(pt->pagedirs[0], it->start);
                const uintptr_t table_mask = 0xFFFFF000U;
                void *table_base = uint2ptr(*ped & table_mask);

                uint32_t *pet = vm_table_entry_addr(table_base, it->start);
                // Clear all flags. Especially "Present".
                *pet = 0;

                const uintptr_t pet_addr_mask = 0xFFFFF000U;
                *pet |= (pet_addr_mask & ptr2uint(it)) << 11;

                if (it->flags & VMMM_FLAGS_WRITE) {
                        *pet |= VM_TABLE_FLAG_RW;
                }

                if (it->flags & VMMM_FLAGS_USERSPACE) {
                        *pet |= VM_TABLE_FLAG_USER;
                }
        }
}

struct vmm_arch_page_tree *vmm_arch_create_pt(struct vm_space *userspace,
                                              struct vm_space *kernelspace)
{
        // TODO: Use same page directories for a kernel space in all vm spaces.

        // We aren't going to create a valid page tree from the start.
        // Every PDE (page directory entry) will has PRESENT field == 0.
        // Instead of the actual physical address in the final PDE there will be an address of it's mapping.

        // i686 has only two page tree directories.
        // Create top-level directory here, and create second-level directories when necessary.
        struct vmm_arch_page_tree *pt = kmm_cache_alloc(&CACHES.page_trees);
        memset(pt, 0, sizeof(*pt));
        pt->pagedirs[0] = kmm_cache_alloc(&CACHES.page_dirs);
        pt->pagedirs_lengths[0] = 1;

        patch_pagetree(pt, userspace);
        patch_pagetree(pt, kernelspace);
}

void vmm_arch_free_pt(struct vmm_arch_page_tree *pt)
{
        kmm_cache_free(&CACHES.page_trees, pt);
}

void vmm_arch_load_pt(struct vmm_arch_page_tree *pt)
{
        // BUG: CR3 must contain physical address!!!
        kassert(false);
        vm_paging_set(pt->pagedirs[0]);
}

static void load_missing_page(void *vaddr)
{
        // Can't implement it right now, because we need to get a vm space of the current task,
        // but I haven't even began to implement multi-tasking.
        LOGF_P("Encountered a missing page. TODO: Handle it\n");
}

static void pagefault_handler(struct intr_ctx *ctx)
{
        bool not_present = ctx->err_code & (0x1 << 0);
        bool is_writeop = ctx->err_code & (0x1 << 1);
        bool is_usermode = ctx->err_code & (0x1 << 2);
        bool is_fetchop = ctx->err_code & (0x1 << 4);
        bool is_other = ctx->err_code & ((0x1 << 3) | (0x1 << 5) | (0x1 << 15));

        if (not_present) {
                void *vaddr = uint2ptr(vm_get_cr2());
                load_missing_page(vaddr);
        } else {
                LOGF_P("Don't know how to handle a page fault. Error code is %d\n", ctx->err_code);
        }
}

void vm_register_pagefault_handler(void)
{
        intr_handler_cpu(INTR_CPU_PAGEFAULT, pagefault_handler);
}

uintptr_t vm_get_cr2(void)
{
        uintptr_t vaddr;
        asm("mov %%cr2, %0" : "=r"(vaddr));

        return (vaddr);
}
