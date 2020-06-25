#include <kernel/kernel.h>

#include <arch/vga.h>
#include <arch/vm.h>
#include <arch/platform.h>
#include <arch/descriptors.h>
#include <arch/mm.h>

#include <multiboot.h>

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

// TODO: Maybe add some "smart" recursive macrosses to patch the stack automatically?
/**
 * @brief Adjust the frame with given offset.
 * @param level The depth of the frame to patch.
 * @param offset The offset to apply to the addresses in the frame.
 */
#define PATCH_FRAME(level, offset)                                             \
	do {                                                                   \
		uint32_t *frame = __builtin_frame_address(level);              \
		/* Patch return address */                                     \
		uint32_t *ra = &frame[1];                                      \
		*ra += offset;                                                 \
		/* Patch previous ebp */                                       \
		uint32_t *bp = &frame[0];                                      \
		*bp += offset;                                                 \
	} while (false)

/**
 * @brief Map the pages in the range of addresses to the high memory.
 * 
 * @param page_dir Page Directory to modify
 * @param flags Flags to apply to every page.
 */
static void map_addr_range(void *page_dir, const void *start, const void *end,
			   enum VM_TABLE_FLAGS flags)
{
	// TODO: Enforce page boundary checks
	uintptr_t paddr = (uintptr_t)start;
	while (paddr < (uintptr_t)end) {
		uintptr_t vaddr = KERNEL_VMA + paddr;
		vm_map(page_dir, (void *)vaddr, (void *)paddr, flags);
		paddr += PLATFORM_PAGE_SIZE;
	}
}

/**
 * @brief Map kernel sections from low memory to high memory.
 */
static void map_kernel(void *page_dir)
{
	map_addr_range(page_dir, (void *)LOW(&kernel_text_start[0]),
		       (void *)LOW(&kernel_text_end[0]), VM_TABLE_FLAG_PRESENT);
	map_addr_range(page_dir, (void *)LOW(&kernel_rodata_start[0]),
		       (void *)LOW(&kernel_rodata_end[0]),
		       VM_TABLE_FLAG_PRESENT);
	map_addr_range(page_dir, (void *)LOW(&kernel_data_start[0]),
		       (void *)LOW(&kernel_data_end[0]),
		       VM_TABLE_FLAG_RW | VM_TABLE_FLAG_PRESENT);
	map_addr_range(page_dir, (void *)LOW(&kernel_bss_start[0]),
		       (void *)LOW(&kernel_bss_end[0]),
		       VM_TABLE_FLAG_PRESENT | VM_TABLE_FLAG_RW);
}

/**
 * @brief Map platform specific regions from low memory to high memory.
 */
static void map_platform(void *page_dir)
{
	// Map low 1MiB
	const uintptr_t start = 0x0;
	const uintptr_t end = 1 * 1024 * 1024;
	map_addr_range(page_dir, (void *)start, (void *)end,
		       VM_TABLE_FLAG_PRESENT | VM_TABLE_FLAG_RW);
}

/**
 * @brief Setup paging to actualy boot the kernel.
 */
static void setup_boot_paging(void)
{
	void *boot_pt = (void *)LOW(&boot_paging_pt[0]);
	void *boot_pd = (void *)LOW(&boot_paging_pd[0]);

	// Identity mapping
	void *identity = vm_dir_entry_addr(boot_pd, 0x0);
	vm_set_dir_entry(identity, boot_pt,
			 VM_DIR_FLAG_PRESENT | VM_DIR_FLAG_RW);

	// Map the kernel to higher half of address space
	void *hh = vm_dir_entry_addr(boot_pd, (void *)KERNEL_VMA);
	vm_set_dir_entry(hh, boot_pt, VM_DIR_FLAG_PRESENT | VM_DIR_FLAG_RW);

	map_kernel(boot_pd);
	map_platform(boot_pd);

	vm_paging_set(boot_pd);
	vm_paging_enable(KERNEL_VMA);

	// Patch i686_init(...)
	PATCH_FRAME(1, KERNEL_VMA);
	// Patch setup_boot_paging(...)
	PATCH_FRAME(0, KERNEL_VMA);

	// Undo identity mapping
	vm_set_dir_entry(boot_pd, 0x0, 0x0);
	vm_tlb_flush();
}

/**
 * @brief Patch some multiboot information in order to use it from high memory.
 * 
 * @param info Multiboot info block.
 */
static void patch_multiboot_info(multiboot_info_t *info)
{
	info->mmap_addr = HIGH(info->mmap_addr);
}

extern void call_global_ctors(void) asm("_init");
extern void call_global_dtors(void) asm("_fini");

static struct arch_info_i686 i686_info;

void i686_init(multiboot_info_t *info, uint32_t magic)
{
	if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
		// TODO: Panic
		return;
	}
	setup_boot_paging();
	boot_setup_gdt();
	boot_setup_idt();

	i686_info.info = (void *)HIGH(info);
	patch_multiboot_info(i686_info.info);

	call_global_ctors();
	kernel_init((arch_info_t)&i686_info);
	call_global_dtors();
}
