#include "kernel/timer.h"

#include "kernel/config.h"
#include "kernel/elflist.h"

#include "lib/ds/cbuffer.h"

ELFLIST_EXTERN(struct int_timer, timers);

struct event {
        callback_fn cb;
        unsigned wake_time;
};

static CBUFFER_DECLARE(struct event, CONF_TIMER_QUEUE_LENGTH) QUEUE;
static struct int_timer *TIMER;
static unsigned TIME;

static void callback(void)
{}

void timer_init(void)
{
        struct int_timer **t;
        ELFLIST_FOREACH (timers, t) {
                if ((*t)->init != NULL) {
                        int rc = (*t)->init(callback);
                        if (rc != TIMER_RC_OK) {
                                continue;
                        }
                }
                TIMER = *t;
                break;
        }

        CBUFFER_INIT(&QUEUE);
}

void timer_call_after(unsigned ms, callback_fn f)
{
        struct event ev = (struct event){ .cb = f, .wake_time = TIME + ms };
}
