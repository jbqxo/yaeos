#ifndef _KERNEL_ARCH_I686_VM_H
#define _KERNEL_ARCH_I686_VM_H

#include "arch_i686/intr.h"

#include "kernel/mm/vm.h"
#include "kernel/platform_consts.h"

#include "lib/cppdefs.h"

enum i686_vm_table_flags {
        I686VM_TABLE_FLAG_RW = 0x1 << 0,
        I686VM_TABLE_FLAG_USER = 0x1 << 1,
        I686VM_TABLE_FLAG_CACHE_WT = 0x1 << 2,
        I686VM_TABLE_FLAG_CACHE_OFF = 0x1 << 3,
        I686VM_TABLE_FLAG_ACCESSED = 0x1 << 4,
        I686VM_TABLE_FLAG_DIRTY = 0x1 << 5,
        I686VM_TABLE_FLAG_PAT = 0x1 << 6,
};

enum i686_vm_dir_flags {
        I686VM_DIR_FLAG_RW = 0x1 << 0,
        I686VM_DIR_FLAG_USER = 0x1 << 1,
        I686VM_DIR_FLAG_CACHE_WT = 0x1 << 2,
        I686VM_DIR_FLAG_CACHE_OFF = 0x1 << 3,
        I686VM_DIR_FLAG_ACCESSED = 0x1 << 4,
        I686VM_DIR_FLAG_4MiB = 0x1 << 6,
};

/**
 * @brief Converts vm_flags to i686_table flags.
 */
enum i686_vm_table_flags i686_vm_to_table_flags(enum vm_flags area_flags);

/**
 * @brief Converts vm_flags to i686_dir flags.
 */
enum i686_vm_dir_flags i686_vm_to_dir_flags(enum vm_flags area_flags);

struct i686_vm_pge {
        union {
                struct {
                        bool is_present : 1;
                        uint32_t : 11;
                        uintptr_t paddr : 20;
                } any;
                struct {
                        union {
                                struct {
                                        bool is_present : 1;
                                        enum i686_vm_dir_flags flags : 7;
                                        uint8_t kernel_data : 4;
                                        uintptr_t paddr : 20;
                                };
                                struct {
                                        bool is_present : 1;
                                        enum i686_vm_dir_flags flags : 7;
                                        bool global : 1;
                                        uint8_t kernel_data : 3;
                                        uintptr_t paddr : 20;
                                } if_4mib;
                        };
                } dir;
                struct {
                        bool is_present : 1;
                        enum i686_vm_table_flags flags : 7;
                        bool global : 1;
                        uint8_t kernel_data : 3;
                        uintptr_t paddr : 20;
                } table;
        };
};

#define I686VM_PD_LAST_VALID_PAGE (1021U)
#define I686VM_PD_EMERGENCY_NDX   (1022U)
#define I686VM_PD_EMERGENCY_ADDR  ((void *)(I686VM_PD_EMERGENCY_NDX << 22))
#define I686VM_PD_RECURSIVE_NDX   (1023U)
#define I686VM_PD_RECURSIVE_ADDR  ((void *)(I686VM_PD_RECURSIVE_NDX << 22))

struct i686_vm_pd {
        struct i686_vm_pge entries[I686VM_PD_LAST_VALID_PAGE + 1];
        struct i686_vm_pge emergency; /**< Page Tables don't have this. */
        struct i686_vm_pge recursive;
};

kstatic_assert(sizeof(struct i686_vm_pd) == 4096, "Wrong size of the Page Dir struct.");

enum i686_vm_pg_lvls {
        I686VM_PGLVL_DIR,
        I686VM_PGLVL_TABLE,
};

/**
* @brief Calculate an address of the page entry for a virtual address.
*
* @param lvl Tree level.
* @param dir Table address.
* @param vaddr Virtual address.
* @return Address of the Page Table entry.
*/
struct i686_vm_pge *i686_vm_get_pge(enum i686_vm_pg_lvls lvl, struct i686_vm_pd *dir, void const *vaddr);

void i686_vm_tlb_flush(void);

void i686_vm_tlb_invlpg(void *addr);

/**
* @brief Sets table entry to point at specified page.
*
* @param entry Page Table entry to set.
* @param phys_addr Page address to point to.
*/
void i686_vm_pge_set_addr(struct i686_vm_pge *entry, const void *phys_addr);

void *i686_vm_pge_get_addr(struct i686_vm_pge *entry);

/**
* @brief Set given Page Tree as active.
*
* @param pt Page Tree to activate.
*/
void i686_vm_set_pt(struct i686_vm_pd *root_dir);

/**
* @brief Enable Paging.
*
* @param hh_offset Offset of the kernel itself.
*/
void i686_vm_paging_enable(uintptr_t hh_offset);

void *i686_vm_get_cr2(void);

/**
 * @brief Page fault handler.
 */
void i686_vm_pg_fault_handler(struct intr_ctx *ctx);

void i686_vm_setup_recursive_mapping(struct i686_vm_pd *dir, void *dir_paddr);

#endif /* _KERNEL_ARCH_I686_VM_H */
