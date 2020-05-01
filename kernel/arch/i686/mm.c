#include <arch/mm.h>
#include <arch/platform.h>

#include <multiboot.h>

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

mm_mem_range_cont_t mm_mem_range_cont_init(arch_info_t info)
{
	return ((struct arch_info_i686 *)info)->info->mmap_addr;
}

bool mm_next_mem_range(arch_info_t _info, mm_mem_range_cont_t *cont,
		       struct range_addr *range)
{
	multiboot_info_t *info = ((struct arch_info_i686 *)_info)->info;
	uintptr_t current = *cont;

	if (current >= info->mmap_addr + info->mmap_length) {
		return false;
	}

	multiboot_memory_map_t *c = (void *)current;
	range->base = c->addr;
	range->length = c->len;
	range->type = c->type;

	*cont = current + c->size + sizeof(c->size);

	return true;
}
