#include "kernel/klog.h"

#include "kernel/panic.h"

#include "lib/cstd/stdio.h"

#include <limits.h>
#include <stdarg.h>
#include <stdint.h>

static char lvl_to_char(enum LOG_LEVEL lvl)
{
        static char LOOKUP_TABLE[LOG_PANIC + 1] = {
                [LOG_DEBUG] = 'D', [LOG_INFO] = 'I',  [LOG_WARN] = 'W',
                [LOG_ERR] = 'E',   [LOG_PANIC] = 'P',
        };
        return (LOOKUP_TABLE[lvl]);
}

static const char *find_filename(const char *path)
{
        const char *it = path;
        const char *last_delim = path;
        while (*it != '\0') {
                if (*it == '/') {
                        last_delim = it;
                }
                it++;
        }
        return (last_delim + 1);
}

static int write(const char *msg, size_t len)
{
        console_write(msg, len);
        kassert(len <= INT_MAX);
        return ((int)len);
}

void klog_logf_at(enum LOG_LEVEL lvl, const char *restrict path, const char *restrict func,
                  const char *restrict line, const char *restrict format, ...)
{
        uint64_t cycle = 0;
        kfprintf(write, "[%llu:%c] %s:%s:%s | ", cycle, lvl_to_char(lvl), find_filename(path), func,
                 line);

        va_list ap;
        va_start(ap, format);
        kvfprintf(write, format, ap);
        va_end(ap);
}

__noreturn void klog_logf_panic(const char *location, const char *format, ...)
{
        // TODO: Formatting
        struct kernel_panic_info info = { 0 };
        info.location = location;
        info.description = format;
        kernel_panic(&info);

        __builtin_unreachable();
}
