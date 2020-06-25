#include "kernel/klog.h"
#include <kernel/kernel.h>
#include <kernel/tty.h>
#include <kernel/arch/mm.h>

void kernel_init(arch_info_t *info)
{
	tty_descriptor_t d = tty_platform_get_descriptor();
	klog_init(d);

	LOGF_I("Platform layer has been initialized\n");

	mm_mem_range_cont_t cont = mm_mem_range_cont_init(info);
	struct range_addr ra;
	while (mm_next_mem_range(info, &cont, &ra)) {
		void *start = (void *)ra.base;
		void *end = (void *)(ra.base + ra.length);
		if (ra.type == TYPE_MEMORY) {
			LOGF_I("Found usable memory range: %p - %p\n", start,
			       end);
		} else {
			LOGF_D("Found unusable memory range: %p - %p\n", start,
			       end);
		}
	}
}
