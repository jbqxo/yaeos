#include "kernel/mm/vm.h"

#include "kernel/kernel.h"
#include "kernel/klog.h"
#include "kernel/mm/addr.h"
#include "kernel/mm/kheap.h"

#include "lib/align.h"
#include "lib/cppdefs.h"
#include "lib/cstd/string.h"
#include "lib/ds/rbtree.h"
#include "lib/ds/slist.h"

void vm_pgfault_handle_default(struct vm_area *area, void *addr)
{
        LOGF_P("Page fault at the address %p inside area %p-%p!\n", addr, area->base,
               (void *)((uintptr_t)area->base + area->length - 1));
}
