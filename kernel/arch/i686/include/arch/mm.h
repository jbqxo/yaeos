#pragma once

#include <kernel/kernel.h>
#include <multiboot.h>

#include <stdbool.h>
#include <stdint.h>

enum range_addr_type {
	TYPE_MEMORY = 0x1,
	TYPE_RESERVED = 0x2,
	TYPE_ACPI = 0x3,
	TYPE_NVS = 0x4,
	TYPE_UNUSABLE = 0x5,
	TYPE_DISABLED = 0x6,
	TYPE_PERSISTENT = 0x7
};

/**
 * @brief Contains information about system memory region.
 * 
 */
struct range_addr {
	uint64_t base;
	union {
		uint64_t length;
		bool valid;
	};
	enum range_addr_type type : 32;
	// TODO: Handle ACPI Extended attributes
	uint32_t ext_attr;
} __attribute__((packed));

/**
 * @brief Represents a continuation value, required to fetch system memory map.
 * 
 * Must be preinitialized with the mm_mem_range_cont_init function.
 * Should be used with the mm_next_mm_range function.
 */
typedef uintptr_t mm_mem_range_cont_t;

/**
 * @brief Returns the new continuation value, that should be passed to mm_next_mem_range.
 * 
 * @param info The information from an architecture-dependent level.
 * @return mm_mem_range_cont_t New continuation value.
 */
mm_mem_range_cont_t mm_mem_range_cont_init(arch_info_t info);

/**
 * @brief Retrieve next system memory region.
 * 
 * @param info The information from an architecture-dependent level.
 * @param continuation A continuation value. Should be preinitialized with mm_mem_range_cont_init.
 * @param range Destination.
 * @return true More memory regions are available.
 * @return false There are no more available memory regions.
 */
bool mm_next_mem_range(arch_info_t info, mm_mem_range_cont_t *continuation,
		       struct range_addr *range);
