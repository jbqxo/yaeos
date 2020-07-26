#include <kernel/mm/pmm.h>
#include <kernel/klog.h>
#include <kernel/kernel.h>
#include <kernel/console.h>
#include <kernel/cppdefs.h>
#include <kernel/config.h>
#include <string.h>


void kernel_init()
{
	console_init();
	LOGF_I("Platform layer has been initialized\n");
	pmm_init();
}
