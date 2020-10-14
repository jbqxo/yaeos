#ifndef _KERNEL_TIMER_H
#define _KERNEL_TIMER_H

#include "kernel/ds/slist.h"

typedef void (*callback_fn)(void);

struct int_timer {
        const char *name;
#define TIMER_RC_OK   (0x0)
#define TIMER_RC_FAIL (-0x1)
        int (*init)(callback_fn);
        void (*deinit)(void);
        void (*inter_after)(unsigned ms);
};

void timer_init(void);
void timer_call_after(unsigned ms, callback_fn);
void timer_call_every(unsigned ms, callback_fn);

#endif // _KERNEL_TIMER_H
