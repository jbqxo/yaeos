#include <kernel/tty.h>

#include <arch/vga.h>
#include <arch/vm.h>
#include <arch/platform.h>
#include <arch/gdt.h>

#include <stddef.h>
#include <stdbool.h>
#include <string.h>

// TODO: Maybe add some "smart" recursive macrosses to patch the stack automatically?
// TODOO: Make sure my home address is hidden on GitHub after that. Just in case...
#define PATCH_FRAME(level, offset)                                             \
	do {                                                                   \
		uint32_t *frame = __builtin_frame_address(level);              \
		/* Patch return address */                                     \
		uint32_t *ra = &frame[1];                                     \
		*ra += offset;                                                 \
		/* Patch previous ebp */                                       \
		uint32_t *bp = &frame[0];                                      \
		*bp += offset;                                                 \
	} while (false)

static void map_addr_range(void *page_dir, const void *start, const void *end,
			   enum VM_TABLE_FLAGS flags)
{
	uintptr_t paddr = (uintptr_t)start;
	while (paddr < (uintptr_t)end) {
		uintptr_t vaddr = KERNEL_VMA + paddr;
		vm_map(page_dir, (void *)vaddr, (void *)paddr, flags);
		paddr += PLATFORM_PAGE_SIZE;
	}
}

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

static void map_platform(void *page_dir)
{
	void *paddr = (void *)VGA_BUFFER_ADDR;
	void *vaddr = (void *)HIGH(VGA_BUFFER_ADDR);
	vm_map(page_dir, vaddr, paddr,
	       VM_TABLE_FLAG_PRESENT | VM_TABLE_FLAG_RW);
}

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

	// It is important to do it in reverse order!
	// Patch i686_init(...)
	PATCH_FRAME(1, KERNEL_VMA);
	// Patch setup_boot_paging(...)
	PATCH_FRAME(0, KERNEL_VMA);

	// Undo identity mapping
	vm_set_dir_entry(boot_pd, 0x0, 0x0);
	vm_tlb_flush();
}

static struct gdt_entry gdt_table[3];

/**
 * setup_gdt() - setup system's gdt with flat memory model.
 */
void setup_gdt(void)
{
	static const size_t LIMIT = 0xFFFFFFFF;

	struct gdt_entry *null_descriptor = &gdt_table[0];
	struct gdt_entry *code_descriptor = &gdt_table[1];
	struct gdt_entry *data_descriptor = &gdt_table[2];

	memset(null_descriptor, 0, sizeof(*null_descriptor));

	code_descriptor->limit_low = LIMIT & 0xFFFF;
	code_descriptor->limit_high = (LIMIT >> 0x10) & 0x0F;
	code_descriptor->base_low = 0;
	code_descriptor->base_high = 0;
	code_descriptor->accessed = false;
	code_descriptor->writable = false;
	code_descriptor->direction_conforming = false;
	code_descriptor->code = true;
	code_descriptor->code_or_data = true;
	code_descriptor->privelege = 0;
	code_descriptor->present = true;
	code_descriptor->must_be_false = false;
	code_descriptor->size = true;
	code_descriptor->granularity = true;

	data_descriptor->limit_low = LIMIT & 0xFFFF;
	data_descriptor->limit_high = (LIMIT >> 0x10) & 0x0F;
	data_descriptor->base_low = 0;
	data_descriptor->base_high = 0;
	data_descriptor->accessed = false;
	data_descriptor->writable = true;
	data_descriptor->direction_conforming = false;
	data_descriptor->code = false;
	data_descriptor->code_or_data = true;
	data_descriptor->privelege = 0;
	data_descriptor->present = true;
	data_descriptor->must_be_false = false;
	data_descriptor->size = true;
	data_descriptor->granularity = true;

	struct gdt_ptr p = { .limit = sizeof(gdt_table) - 1,
			     .base = (uint32_t)&gdt_table[0] };
	ptrdiff_t code_offset = (uintptr_t)code_descriptor - (uintptr_t)&gdt_table[0];
	ptrdiff_t data_offset = (uintptr_t)data_descriptor - (uintptr_t)&gdt_table[0];
	set_gdt(&p, data_offset, code_offset);
}

void i686_init(void)
{
	setup_boot_paging();
	setup_gdt();

	tty_init();
	size_t i = 0;
	while (true) {
		tty_putchar(33 + (i++ % 93));
	}
}
