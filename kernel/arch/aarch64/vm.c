#include "kernel/mm/vm.h"

#include "kernel/klog.h"


void vm_arch_ptree_map(union vm_arch_page_dir *tree_root, const void *phys_addr,
                       const void *at_virt_addr, enum vm_flags flags)
{
        LOGF_P("Yet to be implemented\n");
}
