#ifndef _KERNEL_ARCH_MM_VMM_H
#define _KERNEL_ARCH_MM_VMM_H

#include "arch/platform.h"

#include "kernel/mm/vmm.h"

#include <stddef.h>

///
/// Page directory of which a page tree consists.
///
typedef char vmm_arch_page_dir[PLATFORM_PAGEDIR_SIZE];

///
/// Contains an information required for maintaining and using a page tree.
struct vmm_arch_page_tree {
	// Store it to free resources occupied by pagedirs.
	vmm_arch_page_dir *pagedirs
		[PLATFORM_PAGEDIR_COUNT]; //! A set of pagedirs of which the page tree consists.
	size_t pagedirs_lengths[PLATFORM_PAGEDIR_COUNT];
};

void vmm_arch_init(void);

///
/// Create a page tree from a userspace and a kernelspace.
///
struct vmm_arch_page_tree *vmm_arch_create_pt(struct vm_space *userspace,
					      struct vm_space *kernelspace);

///
/// Free resources occupied by a page tree.
///
void vmm_arch_free_pt(struct vmm_arch_page_tree *);

///
/// Load a page tree.
///
void vmm_arch_load_pt(struct vmm_arch_page_tree *);

#endif // _KERNEL_ARCH_MM_VMM_H
