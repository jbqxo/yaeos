#include <kernel/klog.h>

#include <stdarg.h>
#include <stdint.h>
#include <x86intrin.h>

#include <lib/stdio.h>

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
	return last_delim + 1;
}

static int write(const char *msg, size_t len)
{
	console_write(msg, len);
	return len;
}

static char lvl_to_char(enum LOG_LEVEL lvl)
{
	static char LOOKUP_TABLE[LOG_PANIC + 1] = {
		[LOG_DEBUG] = 'D', [LOG_INFO] = 'I',  [LOG_WARN] = 'W',
		[LOG_ERR] = 'E',   [LOG_PANIC] = 'P',
	};
	return LOOKUP_TABLE[lvl];
}

void klog_logf_at(enum LOG_LEVEL lvl, const char *path, const char *func,
		  const char *line, const char *format, ...)
{
	uint64_t cycle = __rdtsc();
	fprintf(write, "[%llu:%c] %s:%s:%s | ", cycle, lvl_to_char(lvl), find_filename(path), func,
		line);

	va_list ap;
	va_start(ap, format);
	vfprintf(write, format, ap);
	va_end(ap);
}
