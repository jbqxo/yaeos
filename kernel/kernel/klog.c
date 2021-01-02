#include "kernel/klog.h"

#include "kernel/kernel.h"

#include "lib/stdio.h"

#include <stdarg.h>
#include <stdint.h>

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
        return (len);
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
        info.location = (char *)location;
        info.description = (char *)format;
        kernel_panic(&info);
}
