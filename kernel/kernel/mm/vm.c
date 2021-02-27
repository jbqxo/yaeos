#include "kernel/mm/vm.h"

#include "kernel/kernel.h"
#include "kernel/klog.h"
#include "kernel/mm/highmem.h"
#include "kernel/mm/kheap.h"

#include "lib/align.h"
#include "lib/cppdefs.h"
#include "lib/ds/rbtree.h"
#include "lib/ds/slist.h"
#include "lib/cstd/string.h"

void vm_pgfault_handle_default(struct vm_area *area, void *addr)
{
        LOGF_P("Page fault at the address %p inside area %p-%p!\n", addr, area->base_vaddr,
               uint2ptr(ptr2uint(area->base_vaddr) + area->length - 1));
}

void vm_pgfault_handle_direct(struct vm_area *area, void *addr)
{
        kassert(area != NULL);

        const union uiptr fault_addr = ptr2uiptr(addr);

        const void *virt_page_addr = uint2ptr(align_rounddown(fault_addr.num, PLATFORM_PAGE_SIZE));
        const void *phys_page_addr = highmem_to_low(virt_page_addr);

        vm_arch_pt_map(area->owner->root_dir, phys_page_addr, virt_page_addr, area->flags);
}

void *vm_new_page_directory(void)
{
        void *pd = kheap_alloc_page();
        /* TODO: A page from the heap should be already 0ed? */
        kmemset(pd, 0x0, PLATFORM_PAGEDIR_SIZE);
        return (pd);
}
