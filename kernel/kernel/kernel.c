#include "kernel/kernel.h"

#include "arch/platform.h"

#include "kernel/config.h"
#include "kernel/console.h"
#include "kernel/cppdefs.h"
#include "kernel/ds/kvstore.h"
#include "kernel/klog.h"
#include "kernel/mm/pmm.h"

#include "lib/stdio.h"

static int conwrite(const char *msg, size_t len)
{
	console_write(msg, len);
	return (len);
}

__noreturn void kernel_panic(struct kernel_panic_info *info)
{
	console_clear();
	fprintf(conwrite, "Whoopsie. The kernel is on fire... Bye.\n");
	fprintf(conwrite, "Cause: %s\n", info->description);
	if (info->location != NULL) {
		fprintf(conwrite, "Location: %s\n", info->location);
	}

	{
		int regs_len;
		KVSTATIC_LEN(&info->regs, regs_len);
		if (regs_len > 0) {
			fprintf(conwrite, "Registers:");
			int i;
			char *key;
			size_t reg;
			KVSTATIC_FOREACH (&info->regs, i, key, reg) {
				if (i % 4 == 0) {
					conwrite("\n", 1);
				}
				fprintf(conwrite, "%s: %08X ", key, reg);
			}
		}
	}
	halt(false);
}

void kernel_init()
{
	console_init();
	LOGF_I("Platform layer has been initialized\n");
	//pmm_init();
}

#if 0
void mm_init(void)
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

	buddy_init(&GLOBAL_ALLOC, ch_array, sizes_array, chunks_count, &ALLOCS_DATA,
		   sizeof(ALLOCS_DATA));

}
#endif
