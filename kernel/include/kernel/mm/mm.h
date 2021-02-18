#ifndef _KERNEL_MM_H
#define _KERNEL_MM_H

#include "kernel/cppdefs.h"
#include "kernel/mm/buddy.h"
#include "kernel/mm/pmm.h"
#include "kernel/mm/vmm.h"

#include "lib/ds/slist.h"

#include <stdint.h>

/**
 * Describes a chunk of physical memory.
 */
struct mem_chunk {
        void *mem; /**< Location of a chunk. */
        size_t length;

        /* Too tied to x86.
         * TODO: More generic memory types. */
        enum mem_chunk_type {
                MEM_TYPE_AVAIL = 1,
                MEM_TYPE_RESERVED = 2,
                MEM_TYPE_ACPI = 3,
                MEM_TYPE_HIBER = 4,
                MEM_TYPE_UNAVAIL = 5,
        } type;
};

struct mm_page {
        void *paddr;

        enum page_state {
                PAGESTATE_FREE,
                PAGESTATE_OCCUPIED,
                PAGESTATE_FIXED, /**< A page must remain in memory. */
        } state;
};

void mm_page_init_free(struct mm_page *, void *phys_addr);

struct mm_zone {
        void *start;
        size_t length;

        struct mm_page *pages;
        size_t pages_count;

        struct linear_alloc *alloc;
        struct buddy_manager *buddym;
        /* There is a vm area for every zone that covers linear allocator space.
         * Basically, it maps a virtual address to it's physical counterpart. */
        struct vm_area info_area;

        SLIST_FIELD(struct mm_zone) list;
};

struct mm_zone *mm_zone_create(void *phys_start, size_t length, struct vm_space *kernel_vmspace);

void mm_zone_register(struct mm_zone *);

struct mm_zone *mm_zone_new(void);

struct mm_page *mm_alloc_page_from(struct mm_zone *zone);

struct mm_page *mm_alloc_page(void);

struct mm_page *mm_get_page_by_paddr(void *phys_addr);

void mm_init(void);

/**
 * Get the number of available chunks of physical memory.
 */
int mm_arch_chunks_len(void);

/**
 * Fill the given array of chunks with the information about available chunks of physical memory.
 * @param chunks An array of chunks where the information will be stored.
 * @note it's expected that chunks array is of sufficient size to hold info about all chunks.
 * You can get the number of available chunks with the `pmm_arch_chunks_len` function.
 */
void mm_arch_get_chunks(struct mem_chunk *chunks);

#endif // _KERNEL_MM_H_
