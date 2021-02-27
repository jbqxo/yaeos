#include "arch_i686/vm.h"

#include "arch_i686/intr.h"
#include "arch_i686/kernel.h"

#include "kernel/config.h"
#include "kernel/kernel.h"
#include "kernel/klog.h"
#include "kernel/mm/highmem.h"
#include "kernel/mm/vm.h"

#include "lib/cppdefs.h"
#include "lib/cstd/assert.h"
#include "lib/ds/rbtree.h"

#include <stdbool.h>
#include <stdint.h>

void *vm_arch_get_early_pgroot(void)
{
        return (&boot_paging_pd);
}

enum i686_vm_table_flags i686_vm_to_table_flags(enum vm_flags area_flags)
{
        enum i686_vm_table_flags f = 0;
        f |= area_flags & VM_WRITE ? I686VM_TABLE_FLAG_RW : 0;
        f |= area_flags & VM_USER ? I686VM_TABLE_FLAG_USER : 0;
        return (f);
}

enum i686_vm_dir_flags i686_vm_to_dir_flags(enum vm_flags area_flags)
{
        enum i686_vm_dir_flags f = 0;
        f |= area_flags & VM_WRITE ? I686VM_DIR_FLAG_RW : 0;
        f |= area_flags & VM_USER ? I686VM_DIR_FLAG_USER : 0;
        return (f);
}

static uint32_t get_pte_ndx(void const *vaddr)
{
        /* The table index consists of 21:12 bits of an address. */
        uintptr_t const MASK = 0x003FF000U;
        uint32_t index = (ptr2uint(vaddr) & MASK) >> 12;
        return (index);
}

static uint32_t get_pde_ndx(void const *vaddr)
{
        /* The directory index consists of 31:22 bits of an address. */
        uint32_t index = ptr2uint(vaddr) >> 22;
        return (index);
}

struct i686_vm_pge *i686_vm_get_pge(enum i686_vm_pg_lvls lvl, struct i686_vm_pd *dir, void *vaddr)
{
        uint32_t ndx = 0;
        switch (lvl) {
        case I686VM_PGLVL_DIR: {
                ndx = get_pde_ndx(vaddr);
        } break;
        case I686VM_PGLVL_TABLE: {
                ndx = get_pte_ndx(vaddr);
        } break;
        default: kassert(false);
        }

        return (&dir->entries[ndx]);
}

void i686_vm_tlb_flush(void)
{
        asm volatile("movl %cr0, %eax;"
                     "movl %eax, %cr0");
}

void i686_vm_tlb_invlpg(void *addr)
{
        asm volatile("invlpg (%0)" ::"r"(addr) : "memory");
}

void i686_vm_pge_set_addr(struct i686_vm_pge *entry, const void *phys_addr)
{
        entry->any.paddr = ptr2uint(phys_addr) >> 12;
}

void *i686_vm_pge_get_addr(struct i686_vm_pge *entry)
{
        kassert(entry->any.is_present);
        return (uint2ptr(entry->any.paddr << 12));
}

void *i686_vm_get_cr2(void)
{
        void *vaddr;
        asm("mov %%cr2, %0" : "=r"(vaddr));

        return (vaddr);
}

void i686_vm_setup_recursive_mapping(struct i686_vm_pd *phys_addr_dir)
{
        struct i686_vm_pge *e = &phys_addr_dir->itself;
        i686_vm_pge_set_addr(e, phys_addr_dir);
        e->any.is_present = true;
        e->dir.flags |= I686VM_DIR_FLAG_RW;
}

void *vm_arch_get_phys_page(void const *virt_page)
{
        size_t const pte_ndx = get_pte_ndx(virt_page);

        /* Use the recursive mapping at the end of the directory to get the address. */
        struct i686_vm_pd *dir = uint2ptr((ptr2uint(virt_page) | 0x003FF000U) & 0xFFFFF000U);

        kassert(dir->entries[pte_ndx].any.is_present);
        return (i686_vm_pge_get_addr(&dir->entries[pte_ndx]));
}

void vm_arch_load_spaces(const struct vm_space *user, const struct vm_space *kernel)
{
        LOGF_P("vm_arch_load_space is not implemented!\n");
}

void i686_vm_pg_fault_handler(struct intr_ctx *__unused ctx)
{
        union uiptr addr = ptr2uiptr(i686_vm_get_cr2());
        struct vm_space *fault_space = NULL;
        if (highmem_is_high(addr.ptr)) {
                fault_space = &CURRENT_KERNEL;
        } else {
                kassert(CURRENT_USER != NULL);
                fault_space = CURRENT_USER;
        }

        struct rbtree_node *rbtnode =
                rbtree_search(&fault_space->rb_areas, addr.ptr, vm_area_rbtcmpfn_area_to_addr);

        if (rbtnode == NULL) {
                LOGF_P("Page fault at the address (%p) not covered by any vm_area!\n", addr.ptr);
        }

        struct vm_area *fault_area = rbtnode->data;

        if (__likely(fault_area->ops.handle_pg_fault != NULL)) {
                fault_area->ops.handle_pg_fault(fault_area, addr.ptr);
        } else {
                LOGF_P("Unhandled page fault at %p!\n", addr.ptr);
        }
}

void vm_arch_pt_map(void *tree_root, const void *phys_addr, const void *at_virt_addr,
                    enum vm_flags flags)
{
        kassert(tree_root != NULL);

        const union uiptr vaddr = ptr2uiptr(at_virt_addr);

        struct i686_vm_pge *pde = i686_vm_get_pge(I686VM_PGLVL_DIR, tree_root, vaddr.ptr);
        if (!pde->any.is_present) {
                /* This is definitely the wrong place to do this.
                 * TODO: Organize Virtual Memory management properly. */
                void *new_pd_vaddr = vm_new_page_directory();
                void *new_pd_paddr = vm_arch_get_phys_page(new_pd_vaddr);
                i686_vm_pge_set_addr(pde, new_pd_paddr);
                pde->dir.flags = i686_vm_to_dir_flags(flags);
                pde->dir.is_present = true;
        }

        struct i686_vm_pd *pt = i686_vm_pge_get_addr(pde);
        pt = highmem_to_high(pt);

        struct i686_vm_pge *pte = i686_vm_get_pge(I686VM_PGLVL_TABLE, pt, vaddr.ptr);
        kassert(!pte->any.is_present);

        pte->table.flags = i686_vm_to_table_flags(flags);
        pte->any.is_present = true;
        i686_vm_pge_set_addr(pte, phys_addr);
}
