#include "arch_i686/vmm.h"

#include "kernel/cppdefs.h"
#include "kernel/klog.h"
#include "kernel/mm/vmm.h"
#include "kernel/config.h"

#include "lib/assert.h"

#include <stdbool.h>
#include <stdint.h>

struct vm_arch_page_entry *vm_table_entry(struct vm_arch_page_entry *table, void *vaddr)
{
        // The table index consists of 21:12 bits of an address.
        const uintptr_t MASK = 0x003FF000U;
        uint32_t index = ((uintptr_t)vaddr & MASK) >> 12;

        if (__unlikely(CONF_VM_DIR_ORIGIN_ENTRY == index)) {
                return (NULL);
        }

        return (&table[index]);
}

struct vm_arch_page_entry *vm_dir_entry(struct vm_arch_page_entry *dir, void *vaddr)
{
        // The directory index consists of 31:22 bits of an address.
        uint32_t index = ptr2uint(vaddr) >> 22;

        if (__unlikely(CONF_VM_DIR_ORIGIN_ENTRY == index)) {
                return (NULL);
        }

        return (&dir[index]);
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

void vm_pt_set_addr(struct vm_arch_page_entry *entry, void *phys_addr)
{
        entry->present.any.paddr = ptr2uint(phys_addr) >> 12;
}

void *vm_pt_get_addr(struct vm_arch_page_entry *entry)
{
        kassert(entry->is_present);
        return (uint2ptr(entry->present.any.paddr << 12));
}


uintptr_t vm_get_cr2(void)
{
        uintptr_t vaddr;
        asm("mov %%cr2, %0" : "=r"(vaddr));

        return (vaddr);
}

void vmm_arch_change_space(struct vm_space *space)
{
        vm_set_active_pt(space->space_dir_paddr);
}

bool vm_arch_space_map_region(struct vm_region *reg, struct vm_space *space, void *map_point,
                              enum vm_flags flags)
{
        struct vm_arch_page_entry *pe = vm_dir_entry(space->space_dir, map_point);
        if (__unlikely(pe == NULL)) {
                return (false);
        }

        if (pe->is_present) {
                return (false);
        }

        vm_pt_set_addr(pe, reg->region_dir_paddr);

        enum vm_dir_flags f = 0;
        f |= flags & VM_WRITE ? VM_DIR_FLAG_RW : 0;
        f |= flags & VM_USER ? VM_DIR_FLAG_USER : 0;
        pe->present.dir.flags = f;
        pe->present.dir.is_present = true;

        return (true);
}

bool vm_arch_region_map_page(struct mm_page *page, struct vm_region *reg, void *map_point, enum vm_flags flags)
{
        struct vm_arch_page_entry *pe = vm_table_entry(reg->region_dir, map_point);
        if (__unlikely(pe == NULL)) {
                return (false);
        }

        if (pe->is_present) {
                return (false);
        }

        vm_pt_set_addr(pe, page->paddr);

        enum vm_table_flags f = 0;
        f |= flags & VM_WRITE ? VM_TABLE_FLAG_RW : 0;
        f |= flags & VM_USER ? VM_TABLE_FLAG_USER : 0;
        pe->present.table.flags = f;
        pe->present.dir.is_present = true;

        return (true);
}

void *vm_space_get_paddr(struct vm_space *space, void *vaddr)
{
        struct vm_arch_page_entry *entry = vm_dir_entry(space->space_dir, vaddr);
        kassert(entry->is_present);

        struct mm_page *page = mm_get_page_by_paddr(vm_pt_get_addr(entry));
        if (page == NULL) {
                return (NULL);
        }

        struct vm_region *reg = ownership_get(&page->owners);
        kassert(reg != NULL);

        entry = vm_table_entry(reg->region_dir, vaddr);
        return (vm_pt_get_addr(entry));
}

struct vm_space *vm_arch_dir_get_space(struct vm_arch_page_entry *dir)
{
        dir += CONF_VM_DIR_ORIGIN_ENTRY;
        kassert(!dir->is_present);

        return (dir->origin.space);
}

struct vm_region *vm_arch_dir_get_region(struct vm_arch_page_entry *dir)
{
        dir += CONF_VM_DIR_ORIGIN_ENTRY;
        kassert(!dir->is_present);

        return (dir->origin.region);
}
