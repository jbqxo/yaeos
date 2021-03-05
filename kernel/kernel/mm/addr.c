#include "kernel/mm/addr.h"

#include "lib/cstd/assert.h"

#include <stdbool.h>
#include <stdint.h>

static bool READY = false;
static uintptr_t OFFSET = 0;

void addr_set_offset(uintptr_t offset)
{
        OFFSET = offset;
        READY = true;
}

uintptr_t addr_get_offset(void)
{
        kassert(READY);

        return (OFFSET);
}

void *addr_to_low(const void *high_addr)
{
        kassert(READY);

        uintptr_t addr = (uintptr_t)high_addr;
        kassert(addr > OFFSET);

        addr -= OFFSET;
        return ((void *)addr);
}

void *addr_to_high(const void *low_addr)
{
        kassert(READY);

        uintptr_t addr = (uintptr_t)low_addr;
        kassert(addr < OFFSET);
        addr += OFFSET;
        return ((void *)addr);
}

bool addr_is_high(const void *addr)
{
        kassert(READY);

        return ((uintptr_t)addr >= OFFSET);
}
