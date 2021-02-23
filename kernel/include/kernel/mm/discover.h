#ifndef _KERNEL_MM_DISCOVER_H
#define _KERNEL_MM_DISCOVER_H

#include <stddef.h>

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

/**
 * Get the number of available chunks of physical memory.
 */
int mm_discover_chunks_len(void);

/**
 * Fill the given array of chunks with the information about available chunks of physical memory.
 * @param chunks An array of chunks where the information will be stored.
 * @note it's expected that chunks array is of sufficient size to hold info about all chunks.
 * You can get the number of available chunks with the `pmm_arch_chunks_len` function.
 */
void mm_discover_get_chunks(struct mem_chunk *chunks);

#endif /* _KERNEL_MM_DISCOVER_H */
