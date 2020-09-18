#include "kernel/cppdefs.h"
#include "kernel/kernel.h"
#include "kernel/klog.h"
#include "kernel/mm/vmm.h"

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unity.h>



__weak void klog_logf_at(enum LOG_LEVEL lvl, const char *restrict path, const char *restrict func,
			 const char *restrict line, const char *restrict format, ...)
{
	char *message = malloc(4096 * sizeof(*message));
	char *buffer = malloc(2048 * sizeof(*buffer));
	assert(buffer);
	assert(message);

	va_list ap;
	va_start(ap, format);
	vsnprintf(buffer, 2048, format, ap);
	va_end(ap);

	snprintf(message, 4096, "LOG: Level: %c. Function: %s:%s. Message: %s", lvl_to_char(lvl),
		 func, line, buffer);
	TEST_MESSAGE(message);

	free(message);
	free(buffer);
}

__weak void kernel_panic(struct kernel_panic_info *info)
{
	char description_buffer[4096] = "Description: ";
	char location_buffer[4096] = "Location: ";
	strcat(description_buffer, info->description);
	strcat(location_buffer, info->location);
	TEST_MESSAGE("Kernel panic has been raised");
	TEST_MESSAGE(description_buffer);
	TEST_MESSAGE(location_buffer);
	TEST_FAIL();
}
