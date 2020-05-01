#include <kernel/klog.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <x86intrin.h>

static struct klog_state {
	tty_descriptor_t descriptor;
} STATE;

void klog_init(tty_descriptor_t d)
{
	STATE.descriptor = d;
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
	return last_delim + 1;
}

static char lvl_to_char(enum LOG_LEVEL lvl)
{
	static char LOOKUP_TABLE[LOG_PANIC + 1] = { [LOG_DEBUG] = 'D',
						    [LOG_INFO] = 'I',
						    [LOG_WARN] = 'W',
						    [LOG_ERR] = 'E',
						    [LOG_PANIC] = 'P' };
	return LOOKUP_TABLE[lvl];
}

void klog_logf_at(enum LOG_LEVEL lvl, const char *restrict path, int line,
		  const char *restrict format, ...)
{
	uint64_t cycle = __rdtsc();
	fprintf(STATE.descriptor, "[%llu:%c] %s:%d | ", cycle, lvl_to_char(lvl),
		find_filename(path), line);

	va_list ap;
	va_start(ap, format);
	vfprintf(STATE.descriptor, format, ap);
	va_end(ap);
}
