#include <kernel/tty.h>
#include <stddef.h>
#include <stdbool.h>

void i686_init(void) 
{
    tty_init();
    size_t i = 0;
    while(true) {
	tty_putchar(33 + (i++ % 93));
    }
}
