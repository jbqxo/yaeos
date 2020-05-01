#pragma once

#include <stdint.h>

#include <arch/platform.h>

#ifdef __cplusplus
extern "C" {
#endif

extern uint32_t boot_paging_pt[] asm("boot_paging_pt");
extern uint32_t boot_paging_pd[] asm("boot_paging_pd");

extern char bootstack_top[] asm("bootstack_top");
extern char bootstack_bottom[] asm("bootstack_bottom");

extern const char kernel_vma[] asm("_kernel_vma");
extern const char kernel_start[] asm("_kernel_start");
extern const char kernel_end[] asm("_kernel_end");

extern const char kernel_text_start[] asm("_kernel_text_start");
extern const char kernel_text_end[] asm("_kernel_text_end");

extern const char kernel_rodata_start[] asm("_kernel_rodata_start");
extern const char kernel_rodata_end[] asm("_kernel_rodata_end");

extern char kernel_data_start[] asm("_kernel_data_start");
extern char kernel_data_end[] asm("_kernel_data_end");

extern char kernel_bss_start[] asm("_kernel_bss_start");
extern char kernel_bss_end[] asm("_kernel_bss_end");

#define KERNEL_VMA ((uintptr_t)&kernel_vma[0])

#define HIGH(addr) ((uintptr_t)(KERNEL_VMA + (uintptr_t)(addr)))
#define LOW(addr) ((uintptr_t)((uintptr_t)(addr) - KERNEL_VMA))

enum VM_TABLE_FLAGS {
	VM_TABLE_FLAG_PRESENT = 0x1 << 0,
	VM_TABLE_FLAG_RW = 0x1 << 1,
	VM_TABLE_FLAG_USER = 0x1 << 2,
	VM_TABLE_FLAG_CACHE_WT = 0x1 << 3,
	VM_TABLE_FLAG_CACHE_OFF = 0x1 << 4,
	VM_TABLE_FLAG_ACCESSED = 0x1 << 5,
	VM_TABLE_FLAG_DIRTY = 0x1 << 6,
	VM_TABLE_FLAG_PAT = 0x1 << 7,
	VM_TABLE_FLAG_GLOBAL = 0x1 << 8,
};

enum VM_DIR_FLAGS {
	VM_DIR_FLAG_PRESENT = 0x1 << 0,
	VM_DIR_FLAG_RW = 0x1 << 1,
	VM_DIR_FLAG_USER = 0x1 << 2,
	VM_DIR_FLAG_CACHE_WT = 0x1 << 3,
	VM_DIR_FLAG_CACHE_OFF = 0x1 << 4,
	VM_DIR_FLAG_ACCESSED = 0x1 << 5,
	VM_DIR_FLAG_4MB = 0x1 << 7,
	VM_DIR_FLAG_GLOBAL = 0x1 << 8,
};

/**
 * vm_table_entry_addr() - Calculate an address of the page table entry for a virtual address.
 * @table: Table address.
 * @vaddr: Virtual address.
 *
 * Return: Address of the Page Table entry.
 */
static inline void *vm_table_entry_addr(void *table, void *vaddr)
{
	// The table index consists of 21:12 bits of an address.
	const uintptr_t MASK = 0x003FF000u;
	uint32_t index = ((uintptr_t)vaddr & MASK) >> 12;
	return (void *)((uintptr_t)table + index * 4);
}

/**
 * vm_dir_entry_addr() - Calculate an address of the page directory entry for a virtual address.
 * @dir: Directory address.
 * @vaddr: Virtual address.
 *
 * Return: Address of the requested entry inside the page table.
 */
static inline void *vm_dir_entry_addr(void *dir, void *vaddr)
{
	// The directory index consists of 31:22 bits of an address.
	const uintptr_t MASK = 0xFFC00000u;
	uint32_t index = ((uintptr_t)vaddr & MASK) >> 22;
	return (void *)((uintptr_t)dir + index * 4);
}

static inline void vm_tlb_flush(void)
{
	asm volatile("movl %cr0, %eax;"
		     "movl %eax, %cr0");
}

static inline void vm_tlb_invlpg(void *addr)
{
	// TODO: Fallback to CR3->Reg->CR3 if the invlpg instruction unavailable
	asm volatile("invlpg (%0)" ::"r"(addr) : "memory");
}

/**
 * vm_set_table_entry() - Set table entry to point at specified page.
 * @table_entry: Page Table entry to set.
 * @phys_addr: Page address to point to.
 * @flags: PTE flags.
 */
void vm_set_table_entry(void *table_entry, void *phys_addr,
			enum VM_TABLE_FLAGS flags);

/**
 * vm_set_dir_entry() - Set directory entry to point at specified page table.
 * @dir_entry: Page Directory entry to set.
 * @table_addr: Page Table to point to.
 * @flags: PDE flags.
 */
void vm_set_dir_entry(void *dir_entry, void *table_addr,
		      enum VM_DIR_FLAGS flags);

/**
 * vm_map() - Map virtual address to physical address.
 * @page_dir: Active Page Directory.
 * @virt_addr: Virtual Address to map from.
 * @phys_addr: Physical Addres to map to.
 * @flags: Page Table Entry flags.
 *
 * The function assumes that the page table is present in page directory.
 * Otherwise, behaviour is undefined.
 */
void vm_map(void *page_dir, void *virt_addr, void *phys_addr,
	    enum VM_TABLE_FLAGS flags);

/**
 * vm_paging_set() - Set given Page Directory as active.
 * @dir: Page Directory to activate.
 */
void vm_paging_set(void *dir);

/**
 * vm_paging_enable() - Enable Paging.
 * @hh_offset: Offset of the kernel itself.
 */
void vm_paging_enable(uintptr_t hh_offset);

#ifdef __cplusplus
}
#endif
