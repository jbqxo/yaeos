#include "lib/cstd/assert.h"

#include "lib/cstd/stdio.h"

#include <stdbool.h>

static fprintf_fn OUT_FN = NULL;

void assertion_init(fprintf_fn outfn)
{
        OUT_FN = outfn;
}

__noreturn void assertion_fail(char const *failed_expression, char const *location)
{
        kfprintf(OUT_FN, "\n");
        kfprintf(OUT_FN, "Assertion failed: %s\n", failed_expression);
        kfprintf(OUT_FN, "At: %s\n", location);
        while (true) {
                asm volatile("");
        }
}
