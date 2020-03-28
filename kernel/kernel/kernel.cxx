#include <kernel/kernel.h>
#include <kernel/tty.h>


extern "C" {
    void kernel_init(void) {
        tty_descriptor_t d = tty_platform_get_descriptor();
        tty_print(d, "Platform layer has been initialized\n");
    }
}
