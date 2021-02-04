#ifndef _KERNEL_MM_H
#define _KERNEL_MM_H

#include "kernel/cppdefs.h"
#include "kernel/ds/slist.h"
#include "kernel/mm/buddy.h"
#include "kernel/mm/pmm.h"

#include <stdint.h>

/* Describes a chunk of physical memory. */
struct mem_chunk {
        void *mem; /**< Location of a chunk. */
        size_t length;

        enum mem_chunk_type {
                MEM_TYPE_AVAIL,
                MEM_TYPE_RESERVED,
                MEM_TYPE_ACPI,
                MEM_TYPE_HIBER,
                MEM_TYPE_UNAVAIL,
        } type;
};

struct mm_page {
        void *paddr;

        enum page_flags {
                PAGEFLAG_WRITE = 0x1 << 0,
                PAGEFLAG_USERSPACE = 0x1 << 1,
                PAGEFLAG_KERNEL = 0x1 << 2,
        } flags;

        enum page_state {
                PAGESTATE_FREE,
                PAGESTATE_OCCUPIED,
                PAGESTATE_SYSTEM_IMPORTANT, /**< Reserved by important system resource. */
        } state;
};

void mm_page_init_free(struct mm_page *, void *phys_addr);

struct mm_zone {
        void *start;
        size_t length;

        struct mm_page *pages;
        size_t pages_count;

        struct linear_alloc *priv_alloc;
        struct buddy_manager *buddym;

        SLIST_FIELD(struct mm_zone) list;
};

struct mm_zone *mm_zone_create_from(struct mem_chunk *chunk);

void mm_zone_register(struct mm_zone *);

struct mm_page *mm_alloc_page_from(struct mm_zone *zone);

struct mm_page *mm_alloc_page(void);

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
