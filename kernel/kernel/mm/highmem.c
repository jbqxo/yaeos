#include "kernel/mm/highmem.h"

static bool HIGHMEM_READY = false;
static uintptr_t HIGHMEM_OFFSET = 0;

void highmem_set_offset(uintptr_t offset)
{
        HIGHMEM_OFFSET = offset;
        HIGHMEM_READY = true;
}

void *highmem_get_offset(void)
{
        kassert(HIGHMEM_READY);

        return (uint2ptr(HIGHMEM_OFFSET));
}

void *highmem_to_low(const void *high_addr)
{
        kassert(HIGHMEM_READY);

        union uiptr addr = ptr2uiptr(high_addr);
        kassert(addr.num > HIGHMEM_OFFSET);

        addr.num -= HIGHMEM_OFFSET;
        return (addr.ptr);
}

void *highmem_to_high(const void *low_addr)
{
        kassert(HIGHMEM_READY);

        union uiptr addr = ptr2uiptr(low_addr);
        kassert(addr.num < HIGHMEM_OFFSET);
        addr.num += HIGHMEM_OFFSET;
        return (addr.ptr);
}

bool highmem_is_high(const void *addr)
{
        kassert(HIGHMEM_READY);

        return (ptr2uint(addr) >= HIGHMEM_READY);
}
