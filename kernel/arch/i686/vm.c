#include "arch_i686/vm.h"

#include "arch_i686/intr.h"
#include "arch_i686/kernel.h"

#include "kernel/config.h"
#include "kernel/kernel.h"
#include "kernel/klog.h"
#include "kernel/mm/addr.h"
#include "kernel/mm/mm.h"
#include "kernel/mm/vm.h"

#include "lib/align.h"
#include "lib/cppdefs.h"
#include "lib/cstd/assert.h"
#include "lib/cstd/string.h"
#include "lib/ds/rbtree.h"
#include "lib/sync/barriers.h"

#include <stdbool.h>
#include <stdint.h>

#define PTE_MASK (1023U << 12)
#define PDE_MASK (1023U << 22)
#define EMERGENCY_DIR \
        ((struct i686_vm_pd *)((I686VM_PD_RECURSIVE_NDX << 22) | (I686VM_PD_EMERGENCY_NDX << 12)))

void vm_arch_iter_reserved_vaddresses(void (*fn)(void const *addr, size_t len, void *data),
                                      void *data)
{
        /* Reserved PD entries. */
        fn(I686VM_PD_RECURSIVE_ADDR, PLATFORM_PAGE_SIZE, data);
        fn(I686VM_PD_EMERGENCY_ADDR, PLATFORM_PAGE_SIZE, data);
}

bool vm_arch_is_range_valid(void const *base, size_t len)
{
        uintptr_t const base_addr = (uintptr_t)base;
        uintptr_t const end_addr = base_addr + len;

        uintptr_t const pd_reserve_start = (I686VM_PD_LAST_VALID_PAGE + 1) << 22;

        if ((base_addr >= pd_reserve_start) || (end_addr >= pd_reserve_start)) {
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

__const static inline uint32_t get_pte_ndx(void const *vaddr)
{
        /* The table index consists of 21:12 bits of an address. */
        uint32_t index = ((uintptr_t)vaddr & PTE_MASK) >> 12;
        return (index);
}

__const static inline uint32_t get_pde_ndx(void const *vaddr)
{
        /* The directory index consists of 31:22 bits of an address. */
        uint32_t index = ((uintptr_t)vaddr & PDE_MASK) >> 22;
        return (index);
}

struct i686_vm_pge *i686_vm_get_pge(enum i686_vm_pg_lvls lvl, struct i686_vm_pd *dir,
                                    void const *vaddr)
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
        /* -Wconversion gives a bit controversal warning on an assignment to a bit-field. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
        entry->any.paddr = (uintptr_t)phys_addr >> 12;
#pragma GCC diagnostic pop
}

void *i686_vm_pge_get_addr(struct i686_vm_pge *entry)
{
        kassert(entry->any.is_present);
        return ((void *)(entry->any.paddr << 12));
}

void *i686_vm_get_cr2(void)
{
        void *vaddr;
        asm volatile("mov %%cr2, %0" : "=r"(vaddr));

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

/* Access to the PGE will be performed through the emergency entry.
 * As such, when you're done, you MUST call the free_emergency_entry() function.
 * Also, create_new_dir() uses emergency entry too, so...
 * TODO: Add locks to the emergency page directory entry. */
static struct i686_vm_pge *get_pge_for_vaddr(void *tree_root, void const *vaddr)
{
        struct i686_vm_pd *root_dir = tree_root;
        struct i686_vm_pge *emergency = &root_dir->emergency;

        kassert(!emergency->any.is_present);

        struct i686_vm_pge *pge_root = i686_vm_get_pge(I686VM_PGLVL_DIR, root_dir, vaddr);
        i686_vm_pge_set_addr(emergency, i686_vm_pge_get_addr(pge_root));
        emergency->any.is_present = true;
        emergency->dir.flags |= I686VM_DIR_FLAG_RW;

        barrier_compiler();
        struct i686_vm_pge *pge_table = i686_vm_get_pge(I686VM_PGLVL_TABLE, EMERGENCY_DIR, vaddr);

        return (pge_table);
}

static void free_emergency_entry(void *tree_root)
{
        struct i686_vm_pd *root_dir = tree_root;
        struct i686_vm_pge *emergency = &root_dir->emergency;

        kassert(emergency->any.is_present);

        emergency->any.is_present = false;
        i686_vm_tlb_invlpg(EMERGENCY_DIR);
}

void *vm_arch_resolve_phys_page(void *tree_root, void const *virt_page)
{
        struct i686_vm_pge *e = get_pge_for_vaddr(tree_root, virt_page);

        void *phys_addr = i686_vm_pge_get_addr(e);

        free_emergency_entry(tree_root);

        return (phys_addr);
}

void i686_vm_pg_fault_handler(struct intr_ctx *ctx __unused)
{
        void *const fault_at = i686_vm_get_cr2();
        struct vm_space *fault_space = NULL;
        if (addr_is_high(fault_at)) {
                fault_space = &CURRENT_KERNEL;
        } else {
                kassert(CURRENT_USER != NULL);
                fault_space = CURRENT_USER;
        }

        struct rbtree_node *rbtnode =
                rbtree_search(&fault_space->rb_areas, fault_at, vm_area_rbtcmpfn_area_to_addr);

        if (rbtnode == NULL) {
                LOGF_P("Page fault at the address (%p) not covered by any vm_area!\n", fault_at);
        }

        struct vm_area *fault_area = rbtnode->data;

        if (__likely(fault_area->ops.handle_pg_fault != NULL)) {
                fault_area->ops.handle_pg_fault(fault_area, fault_at);
        } else {
                LOGF_P("Unhandled page fault at %p!\n", fault_at);
        }
}

static void *create_new_dir(struct i686_vm_pd *root_pd)
{
        struct mm_page *page = mm_alloc_page();

        struct i686_vm_pge *e = &root_pd->emergency;
        kassert(!e->any.is_present);

        i686_vm_pge_set_addr(e, page->paddr);
        e->any.is_present = true;
        e->dir.flags |= I686VM_DIR_FLAG_RW;


        barrier_compiler();

        kmemset(EMERGENCY_DIR, 0x0, PLATFORM_PAGE_SIZE);

        e->any.is_present = false;
        i686_vm_tlb_invlpg(EMERGENCY_DIR);

        return (page->paddr);
}

void vm_arch_pt_map(void *tree_root, const void *phys_addr, const void *at_virt_addr,
                    enum vm_flags flags)
{
        kassert(tree_root != NULL);

        struct i686_vm_pge *pde = i686_vm_get_pge(I686VM_PGLVL_DIR, tree_root, at_virt_addr);
        if (!pde->any.is_present) {
                void *new_pd_paddr = create_new_dir(tree_root);
                i686_vm_pge_set_addr(pde, new_pd_paddr);
                pde->dir.flags = i686_vm_to_dir_flags(flags);
                pde->dir.is_present = true;
        }

        struct i686_vm_pge *pte = get_pge_for_vaddr(tree_root, at_virt_addr);
        kassert(!pte->any.is_present);

        i686_vm_pge_set_addr(pte, phys_addr);
        pte->table.flags = i686_vm_to_table_flags(flags);
        pte->any.is_present = true;

        free_emergency_entry(tree_root);
}
