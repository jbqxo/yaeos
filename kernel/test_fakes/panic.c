#include "kernel/panic.h"

#include "kernel/kernel.h"
#include "kernel/klog.h"
#include "kernel/mm/vm.h"

#include "lib/cppdefs.h"

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unity.h>

static char lvl_to_char(enum LOG_LEVEL lvl)
{
        static char LOOKUP_TABLE[LOG_PANIC + 1] = {
                [LOG_DEBUG] = 'D', [LOG_INFO] = 'I',  [LOG_WARN] = 'W',
                [LOG_ERR] = 'E',   [LOG_PANIC] = 'P',
        };
        return (LOOKUP_TABLE[lvl]);
}

__weak void klog_logf_at(enum LOG_LEVEL lvl, const char *restrict path __unused,
                         const char *restrict func, const char *restrict line,
                         const char *restrict format, ...)
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

__noreturn void assertion_fail(char const *failed_expression, char const *location)
{
        fprintf(stderr, "Assertion failed: %s\n", failed_expression);
        fprintf(stderr, "At: %s\n", location);
        exit(EXIT_FAILURE);
}
