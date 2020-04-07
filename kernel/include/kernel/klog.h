#pragma once

#include <kernel/tty.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#define restrict __restrict
#endif

#define LOGF(level, message, ...)                                              \
	klog_logf_at((level), __FILE__, __LINE__, (message), ##__VA_ARGS__)

#define LOGF_D(message, ...) LOGF(LOG_DEBUG, (message), ##__VA_ARGS__)
#define LOGF_I(message, ...) LOGF(LOG_INFO, (message), ##__VA_ARGS__)
#define LOGF_W(message, ...) LOGF(LOG_WARN, (message), ##__VA_ARGS__)
#define LOGF_E(message, ...) LOGF(LOG_ERR, (message), ##__VA_ARGS__)
#define LOGF_P(message, ...) LOGF(LOG_PANIC, (message), ##__VA_ARGS__)

enum LOG_LEVEL {
	LOG_DEBUG = 0x0,
	LOG_INFO = 0x1,
	LOG_WARN = 0x2,
	LOG_ERR = 0x3,
	LOG_PANIC = 0x4
};

void klog_init(tty_descriptor_t descriptor);
void klog_logf_at(enum LOG_LEVEL lvl, const char *restrict path, int line,
		  const char *restrict format, ...)
	__attribute__((format(printf, 4, 5)));

#ifdef __cplusplus
#undef restrict
}
#endif
