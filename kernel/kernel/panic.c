#include "kernel/panic.h"

#include "kernel/console.h"

#include "lib/cstd/stdio.h"

static int conwrite(const char *msg, size_t len)
{
        console_write(msg, len);
        return (len);
}

__noreturn void kernel_panic(struct kernel_panic_info *info)
{
        console_clear();
        kfprintf(conwrite, "Whoopsie. The kernel is on fire... Bye.\n");
        kfprintf(conwrite, "Cause: %s\n", info->description);
        if (info->location != NULL) {
                kfprintf(conwrite, "Location: %s\n", info->location);
        }

        {
                int regs_len;
                KVSTATIC_LEN(&info->regs, regs_len);
                if (regs_len > 0) {
                        kfprintf(conwrite, "Registers:");
                        int i;
                        const char *key;
                        size_t reg;
                        KVSTATIC_FOREACH (&info->regs, i, key, reg) {
                                if (i % 4 == 0) {
                                        conwrite("\n", 1);
                                }
                                kfprintf(conwrite, "%s: %08X ", key, reg);
                        }
                }
        }
        halt(false);
}
