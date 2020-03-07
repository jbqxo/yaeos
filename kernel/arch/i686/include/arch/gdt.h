#pragma once

#include <stdint.h>
#include <stdbool.h>

/**
 * struct gdt_entry - the structure contains description of one segment descriptor.
 * @limit_low: the lowest part of the segment limit.
 * @base_low: the lowest part of the segment base address.
 * @accessed: the cpu sets the field to true when the segment is accessed.
 * @writable: whether to allow write access.
 * @direction_conforming: direction when code=false, conforming bit otherwise.
 * @code: whether the segment contains code or data.
 * @code_or_data: whether the segment descriptor is for a system management.
 * @privelege: contains the ring level.
 * @present: whether the segment is present in memory.
 * @limit_high: the highest part of the segment limit.
 * @available: you can store your vast arrays of information here.
 * @must_be_false: reserved for IA-32e. On IA-32 must be always 0.
 * @size: represents different size flags. See Intel's Vol. 3A: 3-11
 * @granularity: determines granularity of the limit field. If set to 1 the limit is in 4KiB blocks.
 * @base_high: the highest part of the segment base address.
 */
struct gdt_entry {
    uint16_t limit_low : 16;
    uint32_t base_low : 24;
    bool accessed : 1;
    bool writable : 1;
    bool direction_conforming : 1;
    bool code : 1;
    bool code_or_data : 1;
    uint8_t privelege : 2;
    bool present : 1;
    uint16_t limit_high : 4;
    bool available : 1;
    bool must_be_false : 1;
    bool size : 1;
    bool granularity : 1;
    uint8_t base_high : 8;
} __attribute__((packed));

/**
 * struct gdt_ptr - the structure specifies the address and the size of the gdt.
 * @limit: limit of the table.
 * @base: the address of the table.
 */
struct gdt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

/**
 * set_gdt() - set a gdt table.
 * @table - pointer to the gdt table.
 * @data_offset - offset in the gdt table for data selector.
 * @code_offset - offset in the gdt table for code selector.
 */
void set_gdt(struct gdt_ptr *table, uint16_t data_offset, uint16_t code_offset);
