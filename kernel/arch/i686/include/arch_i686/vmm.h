#ifndef _KERNEL_ARCH_I686_VM_H
#define _KERNEL_ARCH_I686_VM_H

#include "arch_i686/platform.h"
#include "arch_i686/intr.h"

#include "kernel/cppdefs.h"

#define VM_TABLE_FLAG_PRESENT   (0x1 << 0)
#define VM_TABLE_FLAG_RW        (0x1 << 1)
#define VM_TABLE_FLAG_USER      (0x1 << 2)
#define VM_TABLE_FLAG_CACHE_WT  (0x1 << 3)
#define VM_TABLE_FLAG_CACHE_OFF (0x1 << 4)
#define VM_TABLE_FLAG_ACCESSED  (0x1 << 5)
#define VM_TABLE_FLAG_DIRTY     (0x1 << 6)
#define VM_TABLE_FLAG_PAT       (0x1 << 7)

#define VM_DIR_FLAG_PRESENT   (0x1 << 0)
#define VM_DIR_FLAG_RW        (0x1 << 1)
#define VM_DIR_FLAG_USER      (0x1 << 2)
#define VM_DIR_FLAG_CACHE_WT  (0x1 << 3)
#define VM_DIR_FLAG_CACHE_OFF (0x1 << 4)
#define VM_DIR_FLAG_ACCESSED  (0x1 << 5)
#define VM_DIR_FLAG_4MB       (0x1 << 7)
#define VM_DIR_FLAG_GLOBAL    (0x1 << 8)

///
/// Calculate an address of the page table entry for a virtual address.
///
/// @param table Table address.
/// @param vaddr Virtual address.
/// @return Address of the Page Table entry.
///
void *vm_table_entry_addr(void *table, void *vaddr);

///
/// Calculate an address of the page directory entry for a virtual address.
///
/// @dir Directory address.
/// @vaddr Virtual address.
/// @return Address of the requested entry inside the page table.
///
void *vm_dir_entry_addr(void *dir, void *vaddr);

void vm_tlb_flush(void);

void vm_tlb_invlpg(void *addr);

///
/// Sets table entry to point at specified page.
///
/// @param table_entry Page Table entry to set.
/// @param phys_addr Page address to point to.
/// @param flags PTE flags.
///
void vm_set_table_entry(void *table_entry, void *phys_addr, int flags);

///
/// Set directory entry to point at specified page table.
///
/// @param dir_entry Page Directory entry to set.
/// @param table_addr Page Table to point to.
/// @param flags PDE flags.
///
void vm_set_dir_entry(void *dir_entry, void *table_addr, int flags);

///
/// Map virtual address to physical address.
///
/// @param page_dir Active Page Directory.
/// @param virt_addr Virtual Address to map from.
/// @param phys_addr Physical Addres to map to.
/// @param flags Page Table Entry flags.
///
/// The function assumes that the page table is present in page directory.
/// Otherwise, behaviour is undefined.
///
void vm_map(void *page_dir, void *virt_addr, void *phys_addr, int flags);

///
/// Set given Page Directory as active.
///
/// @param dir Page Directory to activate.
///
void vm_paging_set(void *dir);

///
/// Enable Paging.
///
/// @param hh_offset Offset of the kernel itself.
void vm_paging_enable(uintptr_t hh_offset);

void vm_register_pagefault_handler(void);

///
/// Get contents of the cr2 register.
/// The register containst the linear address that caused a page fault.
///
uintptr_t vm_get_cr2(void);

#endif // _KERNEL_ARCH_I686_VM_H
