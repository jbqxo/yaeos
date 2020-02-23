#include <arch/vm.h>
#include <stdint.h>

void vm_set_table_entry(void *table_entry, void *phys_addr,
                        enum VM_TABLE_FLAGS flags) {
    uint32_t *entry = table_entry;
    *entry = ((uintptr_t)phys_addr) | flags;
}

void vm_set_dir_entry(void *dir_entry, void *table_addr,
                      enum VM_DIR_FLAGS flags) {
    uint32_t *entry = dir_entry;
    *entry = ((uintptr_t)table_addr) | flags;
}

void vm_map(void *page_dir, void *virt_addr, void *phys_addr,
            enum VM_TABLE_FLAGS flags) {
    uint32_t *pdir_entry = vm_dir_entry_addr(page_dir, virt_addr);
    // TODO: Assert that the table is present.
    // Discard PDE flags.
    void *ptable = (void*)((uintptr_t)*pdir_entry & -0x1000);
    uint32_t *ptable_entry = vm_table_entry_addr(ptable, virt_addr);
    vm_set_table_entry(ptable_entry, phys_addr, flags);
}
