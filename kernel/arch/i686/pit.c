#include "arch_i686/intr.h"
#include "arch_i686/io.h"

#include "kernel/timer.h"

#include "lib/cppdefs.h"
#include "lib/elflist.h"

#include <stddef.h>

#define PIT_FREQUENCY (1193182)
#define PIT_CH0       (uint8_t)(0x40)
#define PIT_CMD       (uint8_t)(0x43)
#define PIT_TCOUNT    (0x30)

static callback_fn CALLBACK = NULL;

static void callback(struct intr_ctx *c __unused)
{
        CALLBACK();
}

static uint16_t divisor(unsigned ms)
{
        uint16_t hz = 1000 / ms;
        if (hz < 19) {
                // It's the slowest PIT can go.
                return (65535);
        }
        return (PIT_FREQUENCY / hz);
}

int pit_init(callback_fn f)
{
        CALLBACK = f;
        intr_handler_pic(0, callback);
        return (TIMER_RC_OK);
}

void pit_inter_after(unsigned ms)
{
        iowrite(PIT_CMD, (uint8_t)PIT_TCOUNT);

        uint16_t div = divisor(ms);
        uint8_t low = div & 0xFF;
        uint8_t high = (div >> 8) & 0xFF;
        iowrite(PIT_CH0, low);
        iowrite(PIT_CH0, high);
}

struct int_timer pit_timer = (struct int_timer){
        .name = "pit",
        .init = pit_init,
        .deinit = NULL,
        .inter_after = pit_inter_after,
};
ELFLIST_NEWDATA(timers, pit_timer);
