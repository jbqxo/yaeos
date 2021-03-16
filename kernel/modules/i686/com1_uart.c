/* Copy-pasted from https://wiki.osdev.org/Serial_Ports
 * Just want working UART for now for unit-tests.
 * TODO: Remove or reimplement. */

#include "arch_i686/io.h"

#include "kernel/console.h"
#include "kernel/modules.h"

#include "lib/cppdefs.h"
#include "lib/elflist.h"

#include <stddef.h>

#define PORT 0x3f8 /* COM1 */

static int init_serial(void)
{
        iowrite(PORT + 1, 0x00); /* Disable all interrupts */
        iowrite(PORT + 3, 0x80); /* Enable DLAB (set baud rate divisor) */
        iowrite(PORT + 0, 0x03); /* Set divisor to 3 (lo byte) 38400 baud */
        iowrite(PORT + 1, 0x00);                  /* (hi byte) */
        iowrite(PORT + 3, 0x03); /* 8 bits, no parity, one stop bit */
        iowrite(PORT + 2, 0xC7); /* Enable FIFO, clear them, with 14-byte threshold */
        iowrite(PORT + 4, 0x0B); /* IRQs enabled, RTS/DSR set */
        iowrite(PORT + 4, 0x1E); /* Set in loopback mode, test the serial chip */
        iowrite(PORT + 0,
                0xAE); /* Test serial chip (send byte 0xAE and check if serial returns same byte) */

        /* Check if serial is faulty (i.e: not same byte as sent) */
        if (ioread(PORT + 0, unsigned char) != 0xAE) {
                return 1;
        }

        /* If serial is not faulty set it in normal operation mode
         * (not-loopback with IRQs enabled and OUT#1 and OUT#2 bits enabled)
         */
        iowrite(PORT + 4, 0x0F);
        return 0;
}

static int is_transmit_empty()
{
        return ioread(PORT + 5, unsigned char) & 0x20;
}

static void write_serial(char a)
{
        while (is_transmit_empty() == 0) {
                asm volatile("");
        }

        iowrite(PORT, a);
}

static void writestr_serial(struct console *c __unused, const char *msg, size_t len)
{
        for (size_t i = 0; i < len; i++) {
                write_serial(msg[i]);
        }
}

static struct console serial_console = (struct console){
        .name = "serial_console",
        .ops.write = writestr_serial,
};

static bool com1_available(void)
{
#ifdef __i386__
        return (true);
#else
        return (false);
#endif
}

static void com1_load(void)
{
        init_serial();
        console_register(&serial_console);
}

static struct module com1_uart = (struct module){
        .name = "com1_uart",
        .fns.available = com1_available,
        .fns.load = com1_load,
};
ELFLIST_NEWDATA(modules, com1_uart);
