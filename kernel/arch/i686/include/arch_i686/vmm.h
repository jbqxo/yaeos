#ifndef _KERNEL_ARCH_I686_VM_H
#define _KERNEL_ARCH_I686_VM_H

#include "arch_i686/intr.h"
#include "arch_i686/platform.h"

#include "kernel/cppdefs.h"
#include "kernel/mm/vmm.h"

enum vm_table_flags {
        VM_TABLE_FLAG_PRESENT   = 0x1 << 0,
        VM_TABLE_FLAG_RW        = 0x1 << 1,
        VM_TABLE_FLAG_USER      = 0x1 << 2,
        VM_TABLE_FLAG_CACHE_WT  = 0x1 << 3,
        VM_TABLE_FLAG_CACHE_OFF = 0x1 << 4,
        VM_TABLE_FLAG_ACCESSED  = 0x1 << 5,
        VM_TABLE_FLAG_DIRTY     = 0x1 << 6,
        VM_TABLE_FLAG_PAT       = 0x1 << 7,
};

enum vm_dir_flags {
        VM_DIR_FLAG_PRESENT   = 0x1 << 0,
        VM_DIR_FLAG_RW        = 0x1 << 1,
        VM_DIR_FLAG_USER      = 0x1 << 2,
        VM_DIR_FLAG_CACHE_WT  = 0x1 << 3,
        VM_DIR_FLAG_CACHE_OFF = 0x1 << 4,
        VM_DIR_FLAG_ACCESSED  = 0x1 << 5,
        VM_DIR_FLAG_4MiB      = 0x1 << 7,
        VM_DIR_FLAG_GLOBAL    = 0x1 << 8,
};

struct vm_arch_page_entry {
        union {
                struct {
                        bool is_present : 1;
                        uint32_t : 31;
                };
                union {
                        struct {
                                bool is_present : 1;
                                uint32_t : 11;
                                uintptr_t paddr : 20;
                        } any;
                        struct {
                                bool is_present : 1;
                                enum vm_dir_flags flags : 7;
                                uint8_t kernel_data : 4;
                                uintptr_t paddr : 20;
                        } dir;
                        struct {
                                bool is_present : 1;
                                enum vm_table_flags flags : 8;
                                uint8_t kernel_data : 3;
                                uintptr_t paddr : 20;
                        } table;
                } present;
                union {
                } absent;
                /* See commentary on CONF_VM_DIR_ORIGIN_ENTRY */
                union {
                        struct vm_space *space;
                        struct vm_region *region;
                } origin;
        };
};

/**
* @brief Calculate an address of the page table entry for a virtual address.
*
* @param table Table address.
* @param vaddr Virtual address.
* @return Address of the Page Table entry.
*/
struct vm_arch_page_entry *vm_table_entry(struct vm_arch_page_entry *table, void *vaddr);

/**
* @brief Calculate an address of the page directory entry for a virtual address.
*
* @dir Directory address.
* @vaddr Virtual address.
* @return Address of the requested entry inside the page table.
*/
struct vm_arch_page_entry *vm_dir_entry(struct vm_arch_page_entry *dir, void *vaddr);

void vm_tlb_flush(void);
void vm_tlb_invlpg(void *addr);

/**
* @brief Sets table entry to point at specified page.
*
* @param entry Page Table entry to set.
* @param phys_addr Page address to point to.
*/
void vm_pt_set_addr(struct vm_arch_page_entry *entry, void *phys_addr);

void *vm_pt_get_addr(struct vm_arch_page_entry *entry);

/**
* @brief Set given Page Tree as active.
*
* @param pt Page Tree to activate.
*/
void vm_set_active_pt(void *pt);

/**
* @brief Enable Paging.
*
* @param hh_offset Offset of the kernel itself.
*/
void vm_paging_enable(uintptr_t hh_offset);

#endif // _KERNEL_ARCH_I686_VM_H
