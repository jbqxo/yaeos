#include "arch_i686/vm.h"

#include "arch_i686/intr.h"
#include "arch_i686/kernel.h"

#include "kernel/config.h"
#include "kernel/kernel.h"
#include "kernel/klog.h"
#include "kernel/mm/highmem.h"
#include "kernel/mm/mm.h"
#include "kernel/mm/vm.h"

#include "lib/cppdefs.h"
#include "lib/cstd/assert.h"
#include "lib/cstd/string.h"
#include "lib/ds/rbtree.h"
#include "lib/sync/barriers.h"

#include <stdbool.h>
#include <stdint.h>

#define PTE_MASK (1023U << 12)
#define PDE_MASK (1023U << 22)

void vm_arch_iter_reserved_vaddresses(void (*fn)(void const *addr, size_t len, void *data),
                                      void *data)
{
        /* Reserved PD entries. */
        fn(I686VM_PD_RECURSIVE_ADDR, PLATFORM_PAGE_SIZE, data);
        fn(I686VM_PD_EMERGENCY_ADDR, PLATFORM_PAGE_SIZE, data);

        /* Page Tables recursive mappings. */
        size_t page_directories = PLATFORM_PAGEDIR_PAGES;
        /* We've already reserved the whole Top Entry in the directory.
         * Note, page tables don't have emergency entries, only recursive. */
        page_directories -= 1;

        for (size_t i = 0; i < page_directories; i++) {
                uintptr_t start = (i << 22) | PTE_MASK;
                fn(uint2ptr(start), PLATFORM_PAGE_SIZE, data);
        }
}

bool vm_arch_is_range_valid(void const *base, size_t len)
{
        union uiptr const start = ptr2uiptr(base);
        union uiptr const end = uint2uiptr(start.num + len);

        uintptr_t const pd_reserve_start = (I686VM_PD_LAST_VALID_PAGE + 1) << 22;

        if (start.num >= pd_reserve_start || end.num >= pd_reserve_start) {
                return (false);
        }

        /* Check for overlaps with Page Table's recursive mappings. */
        if (((start.num & PTE_MASK) == PTE_MASK) || ((end.num & PTE_MASK) == PTE_MASK)) {
                return (false);
        }

        /* Not sure about this one.
         * Oh well, will debug it anyway... */
        if (end.num - start.num >= PTE_MASK) {
                return (false);
        }

        return (true);
}

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
        uint32_t index = (ptr2uint(vaddr) & PTE_MASK) >> 12;
        return (index);
}

static uint32_t get_pde_ndx(void const *vaddr)
{
        /* The directory index consists of 31:22 bits of an address. */
        uint32_t index = (ptr2uint(vaddr) & PDE_MASK) >> 22;
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
        barrier_compiler();
        asm volatile("movl %cr0, %eax;"
                     "movl %eax, %cr0");
        barrier_compiler();
}

void i686_vm_tlb_invlpg(void *addr)
{
        barrier_compiler();
        asm volatile("invlpg (%0)" ::"r"(addr) : "memory");
        barrier_compiler();
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

void i686_vm_setup_recursive_mapping(struct i686_vm_pd *dir_actual, void *dir_paddr)
{
        struct i686_vm_pge *e = &dir_actual->recursive;
        kassert(!e->any.is_present);

        i686_vm_pge_set_addr(e, dir_paddr);
        e->any.is_present = true;
        e->dir.flags |= I686VM_DIR_FLAG_RW;
}

static struct i686_vm_pge *get_pge_for_vaddr(void const *vaddr)
{
        size_t const pte_ndx = get_pte_ndx(vaddr);
        /* Use the recursive mapping at the end of the directory to get the address. */
        struct i686_vm_pd *dir = uint2ptr((ptr2uint(vaddr) | 0x003FF000U) & 0xFFFFF000U);

        return (&dir->entries[pte_ndx]);
}

void *vm_arch_get_phys_page(void const *virt_page)
{
        struct i686_vm_pge *e = get_pge_for_vaddr(virt_page);

        kassert(e->any.is_present);
        return (i686_vm_pge_get_addr(e));
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

static void *create_new_dir(struct i686_vm_pd *root_pd)
{
        struct mm_page *page = mm_alloc_page();

        struct i686_vm_pge *e = &root_pd->emergency;
        i686_vm_pge_set_addr(e, page->paddr);
        e->any.is_present = true;
        e->dir.flags |= I686VM_DIR_FLAG_RW;

        uintptr_t const emerg_vaddr = (I686VM_PD_RECURSIVE_NDX << 22) |
                                      (I686VM_PD_EMERGENCY_NDX << 12);
        void *const vaddr = uint2ptr(emerg_vaddr);

        barrier_compiler();

        kmemset(vaddr, 0x0, PLATFORM_PAGE_SIZE);

        i686_vm_setup_recursive_mapping(vaddr, page->paddr);

        e->any.is_present = false;

        return (page->paddr);
}

void vm_arch_pt_map(void *tree_root, const void *phys_addr, const void *at_virt_addr,
                    enum vm_flags flags)
{
        kassert(tree_root != NULL);

        const union uiptr vaddr = ptr2uiptr(at_virt_addr);

        struct i686_vm_pge *pde = i686_vm_get_pge(I686VM_PGLVL_DIR, tree_root, vaddr.ptr);
        if (!pde->any.is_present) {
                void *new_pd_paddr = create_new_dir(tree_root);
                i686_vm_pge_set_addr(pde, new_pd_paddr);
                pde->dir.flags = i686_vm_to_dir_flags(flags);
                pde->dir.is_present = true;
        }

        struct i686_vm_pge *pte = get_pge_for_vaddr(vaddr.ptr);
        kassert(!pte->any.is_present);

        pte->table.flags = i686_vm_to_table_flags(flags);
        pte->any.is_present = true;
        i686_vm_pge_set_addr(pte, phys_addr);
}
