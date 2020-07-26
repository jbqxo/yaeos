#include <arch/mm/pmm.h>
#include <kernel/config.h>
#include <kernel/cppdefs.h>
#include <kernel/klog.h>
#include <kernel/mm/pmm.h>
#include <kernel/mm/alloc.h>

#include <kernel/ds/slist.h>

static char ALLOCS_DATA[CONF_BUDDY_BITMAP_SIZE] __section(".bss");
static struct buddy_allocator GLOBAL_ALLOC;

void pmm_init(void)
{
	int chunks_count = pmm_arch_available_chunks();
	struct pmm_chunk *chunks = __builtin_alloca(chunks_count * sizeof(*chunks));
	pmm_arch_get_chunks(chunks);

	void **ch_array = __builtin_alloca(chunks_count * sizeof(*ch_array));
	size_t *sizes_array = __builtin_alloca(chunks_count * sizeof(*sizes_array));

	for (int i = 0; i < chunks_count; i++) {
		LOGF_I("Memory: %p; Length: %#zX\n", chunks[i].mem, chunks[i].length);
		ch_array[i] = chunks[i].mem;
		sizes_array[i] = chunks[i].length;
	}

	buddy_init(&GLOBAL_ALLOC, ch_array, sizes_array, chunks_count, &ALLOCS_DATA, sizeof(ALLOCS_DATA));
}

pmm_pages_t pmm_alloc(unsigned order)
{
	return ((pmm_pages_t){ .paddr = buddy_alloc(&GLOBAL_ALLOC, order), .order = order });
}

void pmm_free(pmm_pages_t p)
{
	buddy_free(&GLOBAL_ALLOC, p.paddr, p.order);
}
