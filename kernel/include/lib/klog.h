#ifndef _LIB_KLOG_H
#define _LIB_KLOG_H

#include "lib/console.h"
#include "lib/cppdefs.h"

#include <stdarg.h>

#define LOGF(level, message, ...) \
        klog_logf_at((level), __FILE__, __func__, TO_SSTR_MACRO(__LINE__), (message), ##__VA_ARGS__)

#define LOGF_D(message, ...) LOGF(LOG_DEBUG, (message), ##__VA_ARGS__)
#define LOGF_I(message, ...) LOGF(LOG_INFO, (message), ##__VA_ARGS__)
#define LOGF_W(message, ...) LOGF(LOG_WARN, (message), ##__VA_ARGS__)
#define LOGF_E(message, ...) LOGF(LOG_ERR, (message), ##__VA_ARGS__)
#define LOGF_P(message, ...) \
        klog_logf_panic(__FILE__ ":" TO_SSTR_MACRO(__LINE__), (message), ##__VA_ARGS__)

enum LOG_LEVEL {
        LOG_DEBUG = 0x0,
        LOG_INFO = 0x1,
        LOG_WARN = 0x2,
        LOG_ERR = 0x3,
        LOG_PANIC = 0x4,
};

void klog_logf_at(enum LOG_LEVEL lvl, const char *restrict path, const char *restrict func,
                  const char *restrict line, const char *restrict format, ...)
        __attribute__((format(printf, 5, 6)));

__noreturn void klog_logf_panic(const char *location, const char *restrict format, ...)
        __attribute__((format(printf, 2, 3)));

#endif // _LIB_KLOG_H
