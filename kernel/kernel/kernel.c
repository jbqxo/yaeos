#include "kernel/kernel.h"

#include "kernel/config.h"
#include "kernel/console.h"
#include "kernel/cppdefs.h"
#include "kernel/klog.h"
#include "kernel/mm/kmm.h"

#include "lib/stdio.h"

struct vmm_space kvm_space;

void kernel_init()
{
        console_init();
        LOGF_I("Platform layer has been initialized\n");

        kvm_space = vmm_space_new(KERNEL_VMA);
        kmm_init();
        vmm_init();
        kmm_init_kmalloc();
        LOGF_I("Kernel Memory Manager has been initialized\n");

        void *somemem = kmalloc(PLATFORM_PAGE_SIZE - 0x100);
        kassert(somemem);
}

#if 0
void mm_init(void)
{
        int chunks_count = pmm_arch_available_chunks();
        struct pmm_chunk *chunks = __builtin_alloca(chunks_count * sizeof(*chunks));
        pmm_arch_get_chunks(chunks);

        void **ch_array = __builtin_alloca(chunks_count * sizeof(*ch_array));
        size_t *sizes_array = __builtin_alloca(chunks_count * sizeof(*sizes_array));

        for (int i = 0; i < chunks_count; i++) {
                LOGF_I("Memory: %p; Length: %#zX\n", chunks[i].mem, chunks[i].length);
                ch_array[i] = chunks[i].mem;
                sizes_array[i] = chunks[i].length;
        }

        buddy_init(&GLOBAL_ALLOC, ch_array, sizes_array, chunks_count, &ALLOCS_DATA,
                   sizeof(ALLOCS_DATA));

}
#endif
