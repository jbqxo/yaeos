#include <kernel/mm/pmm.h>
#include <kernel/klog.h>
#include <kernel/kernel.h>
#include <kernel/console.h>
#include <kernel/cppdefs.h>
#include <kernel/config.h>
#include <arch/platform.h>
#include <lib/stdio.h>
#include <kernel/ds/kvstore.h>

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
	if (info->location != (void*)0) {
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
			KVSTATIC_FOREACH(&info->regs, i, key, reg) {
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
	pmm_init();
}
