#include "kernel/mm/addr.h"

#include "kernel/mm/vm.h"
#include "kernel/platform_consts.h"

#include "lib/cstd/assert.h"

#include <stdbool.h>
#include <stdint.h>

static bool READY = false;
static uintptr_t OFFSET = 0;

void addr_set_offset(uintptr_t offset)
{
        kassert(!READY);
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

void addr_pgfault_handler_maplow(struct vm_area *area __unused, void *addr)
{
        kassert(area != NULL);

        uintptr_t const fault_addr = (uintptr_t)addr;

        const void *virt_page_addr = (void *)align_rounddown(fault_addr, PLATFORM_PAGE_SIZE);
        const void *phys_page_addr = addr_to_low(virt_page_addr);

        vm_arch_pt_map(area->owner->root_dir, phys_page_addr, virt_page_addr, area->flags);
}
