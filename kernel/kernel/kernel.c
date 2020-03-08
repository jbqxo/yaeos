#include <kernel/kernel.h>
#include <kernel/tty.h>

void kernel_init(void) {
    tty_init();
    tty_writestring("Platform layer has been initialized\n");
}
