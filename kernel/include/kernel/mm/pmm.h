#ifndef _KERNEL_MM_PMM_H
#define _KERNEL_MM_PMM_H

#include "kernel/ds/slist.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

///
/// Describes a chunk of physical memory.
///
struct pmm_chunk {
	void *mem; //! Location of a chunk.
	size_t length;

#define MEM_TYPE_AVAIL    (0x1)
#define MEM_TYPE_RESERVED (0x2)
#define MEM_TYPE_ACPI     (0x3)
#define MEM_TYPE_HIBER    (0x4)
#define MEM_TYPE_UNAVAIL  (0x5)
	uint8_t type;
};

///
/// Get the number of available chunks of physical memory.
///
int pmm_arch_chunks_len(void);

///
/// Fill the given array of chunks with the information about available chunks of physical memory.
///
/// @param chunks An array of chunks where the information will be stored.
/// @note it's expected that chunks array is of sufficient size to hold info about all chunks.
/// You can get the number of available chunks with the `pmm_arch_chunks_len` function.
///
void pmm_arch_get_chunks(struct pmm_chunk *chunks);

///
/// Holds a result of physical memory allocation by some physical allocator.
/// @note For internal usage between PMM and physical allocators.
///
struct pmm_alloc_result {
	uintptr_t paddr;
	bool success;
};

///
/// Holds a result of physical memory allocation by PMM.
///
struct pmm_page {
	uintptr_t paddr;
	struct pmm_allocator *alloc; //! Allocator that owns allocated memory.
};

///
/// Describes a physical allocator that can be used by PMM.
///
/// When PMM is requested to allocate some memory, it will try to
/// find an allocator which suits given requirements (DMA, low memory, and so on).
///
struct pmm_allocator {
	const char *name;
#define PMM_RESTRICT_DMA         (0x1 << 0)
#define PMM_RESTRICT_SINGLE_ONLY (0x1 << 1)
	int restrict_flags;

	/// Custom data pointer
	void *data;

	struct pmm_alloc_result (*page_alloc)(void *data);
	void (*page_free)(void *data, uintptr_t);

	SLIST_FIELD(struct pmm_allocator) allocators;
};

///
/// Initializes physical memory allocator with the given physical memory allocators.
///
void pmm_init(struct pmm_allocator *allocators, size_t alloc_length);

#define PMM_FLAG_DMA (0x1 << 0)
struct pmm_page *pmm_alloc_page(int flags);

void pmm_free(struct pmm_page *);

#endif // _KERNEL_MM_PMM_H
