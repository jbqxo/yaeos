#include "kernel/panic.h"

#include "kernel/console.h"

#include "lib/cppdefs.h"
#include "lib/cstd/stdio.h"

#include "lib/cstd/inttypes.h"

static int conwrite(const char *msg, size_t len)
{
        console_write(msg, len);
        return (len);
}

static void print_register(void *key, void *value, void *data __unused)
{
        char const *reg = key;
        uintptr_t val = (uintptr_t)(value);
        kfprintf(conwrite, "%s: %08" PRIXPTR "\n", reg, val);
}

__noreturn void kernel_panic(struct kernel_panic_info *info)
{
        kfprintf(conwrite, "\n\n");
        kfprintf(conwrite, "You've been whoopsed.\n");
        kfprintf(conwrite, "Reason: %s\n", info->description);
        if (info->location != NULL) {
                kfprintf(conwrite, "Location: %s\n", info->location);
        }

        if (info->regs != NULL) {
                size_t regs_len = kvstore_length(info->regs);
                if (regs_len > 0) {
                        kfprintf(conwrite, "Registers:");
                        kvstore_iter(info->regs, print_register, NULL);
                }
        }
        while (true) {
                asm volatile("");
        }
}
