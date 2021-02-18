#include "arch_i686/vmm.h"

#include "arch_i686/intr.h"

#include "kernel/config.h"
#include "kernel/cppdefs.h"
#include "kernel/ds/rbtree.h"
#include "kernel/kernel.h"
#include "kernel/klog.h"
#include "kernel/mm/vmm.h"

#include "lib/assert.h"

#include <stdbool.h>
#include <stdint.h>

struct vm_arch_page_entry *vm_table_entry(union vm_arch_page_dir *dir, void *vaddr)
{
        // The table index consists of 21:12 bits of an address.
        const uintptr_t MASK = 0x003FF000U;
        uint32_t index = ((uintptr_t)vaddr & MASK) >> 12;

        return (&dir->dir.entries[index]);
}

struct vm_arch_page_entry *vm_dir_entry(union vm_arch_page_dir *dir, void *vaddr)
{
        // The directory index consists of 31:22 bits of an address.
        uint32_t index = ptr2uint(vaddr) >> 22;

        return (&dir->dir.entries[index]);
}

void vm_tlb_flush(void)
{
        asm volatile("movl %cr0, %eax;"
                     "movl %eax, %cr0");
}

void vm_tlb_invlpg(void *addr)
{
        asm volatile("invlpg (%0)" ::"r"(addr) : "memory");
}

void vm_pt_set_addr(struct vm_arch_page_entry *entry, const void *phys_addr)
{
        entry->present.any.paddr = ptr2uint(phys_addr) >> 12;
}

void *vm_pt_get_addr(struct vm_arch_page_entry *entry)
{
        kassert(entry->is_present);
        return (uint2ptr(entry->present.any.paddr << 12));
}

void *vm_get_cr2(void)
{
        void *vaddr;
        asm("mov %%cr2, %0" : "=r"(vaddr));

        return (vaddr);
}

void vm_arch_load_spaces(const struct vm_space *user, const struct vm_space *kernel)
{
        LOGF_P("vm_arch_load_space is not implemented!\n");
}

void vm_i686_pg_fault_handler(struct intr_ctx *__unused ctx)
{
        union uiptr addr = ptr2uiptr(vm_get_cr2());
        struct vm_space *fault_space = NULL;
        if (kernel_arch_is_highmem(addr.ptr)) {
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

void vm_arch_ptree_map(union vm_arch_page_dir *tree_root, const void *phys_addr,
                       const void *at_virt_addr, enum vm_flags flags)
{
        kassert(tree_root != NULL);

        const union uiptr vaddr = ptr2uiptr(at_virt_addr);

        struct vm_arch_page_entry *pde = vm_dir_entry(tree_root, vaddr.ptr);
        kassert(pde->is_present);

        union vm_arch_page_dir *pt = vm_pt_get_addr(pde);
        pt = kernel_arch_to_high(pt);

        struct vm_arch_page_entry *pte = vm_table_entry(pt, vaddr.ptr);
        kassert(!pte->is_present);

        pte->is_present = true;
        pte->present.table.flags = 0;
        pte->present.table.flags |= flags & VM_USER ? VM_TABLE_FLAG_USER : 0;
        pte->present.table.flags |= flags & VM_WRITE ? VM_TABLE_FLAG_RW : 0;
        vm_pt_set_addr(pte, phys_addr);
}
