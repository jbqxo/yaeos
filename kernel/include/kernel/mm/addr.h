#ifndef _KERNEL_MM_ADDR_H
#define _KERNEL_MM_ADDR_H

#include <stdbool.h>
#include <stdint.h>


/* Represents a physical address. */
typedef void *phys_addr_t;

/* Represents a virtual address. */
typedef void *virt_addr_t;

/**
 * @brief Set's an offset where the "high memory" starts.
 * @note Must be set at the beginning of the boot process. But only after the early paging.
 */
void addr_set_offset(uintptr_t offset);

/**
 * @brief Get current offset where the "high memory" starts.
 */
uintptr_t addr_get_offset(void);

/**
 * @brief Translates an address from the high memory to its low memory counterpart.
 * @return An address in the low memory.
 */
void *addr_to_low(const void *high_addr);

/**
 * @brief Translates an address from the low memory to its high memory counterpart.
 * @return An address in the high memory.
 */
void *addr_to_high(const void *low_addr);

/**
 * @brief Determines if an address belongs to the high memory.
 */
bool addr_is_high(const void *addr);

struct vm_area;

/**
 * @brief Maps a physical page that is a direct counterpart to a virtual page from the high memory.
 */
void addr_pgfault_handler_maplow(struct vm_area *area, virt_addr_t addr);

#endif /* _KERNEL_MM_ADDR_H */
