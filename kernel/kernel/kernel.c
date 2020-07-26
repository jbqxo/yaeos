#include <kernel/klog.h>
#include <kernel/kernel.h>
#include <kernel/console.h>
#include <kernel/mm/alloc.h>
#include <kernel/mm/pmm.h>
#include <kernel/cppdefs.h>
#include <kernel/config.h>
#include <string.h>

static void init_allocator(void *platform_info)
{
	int chunks_len = pmm_arch_available_chunks(platform_info);
	struct pmm_chunk *chunks = __builtin_alloca(chunks_len * sizeof(*chunks));
	pmm_arch_get_chunks(platform_info, chunks);

	for (int i = 0; i < chunks_len; i++) {
		void *start = chunks[i].mem;
		void *end = (void *)((uintptr_t)start + chunks[i].length);
		LOGF_I("Available memory: %p .. %p\n", start, end);
	}

	void **ch_array = __builtin_alloca(chunks_len * sizeof(*ch_array));
	size_t *sizes_array = __builtin_alloca(chunks_len * sizeof(*sizes_array));

	for (int i = 0; i < chunks_len; i++) {
		ch_array[i] = chunks[i].mem;
		sizes_array[i] = chunks[i].length;
	}

	struct buddy_allocator *a = buddy_init(ch_array, sizes_array, chunks_len, BUDDY_LOWMEM);
	if (!a) {
		LOGF_P("Failed to initialize Buddy Allocator!!!");
		return;
	}
}

void kernel_init(void *platform_info)
{
	console_init();
	LOGF_I("Platform layer has been initialized\n");
	init_allocator(platform_info);
}
