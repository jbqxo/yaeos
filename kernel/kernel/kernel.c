#include "kernel/kernel.h"

#include "kernel/config.h"
#include "kernel/console.h"
#include "kernel/cppdefs.h"
#include "kernel/klog.h"
#include "kernel/mm/kmm.h"
#include "kernel/mm/mm.h"

#include "lib/stdio.h"

static void kickstart_mm(void)
{
        int chunks_count = mm_arch_chunks_len();
        struct mem_chunk *chunks = __builtin_alloca(chunks_count * sizeof(*chunks));
        mm_arch_get_chunks(chunks);

        mm_init();

        for (int i = 0; i < chunks_count; i++) {
                struct mm_zone *zone = mm_zone_create_from(&chunks[i]);
                mm_zone_register(zone);
        }
}

void kernel_init()
{
        console_init();
        LOGF_I("Platform layer has been initialized\n");

        kickstart_mm();

        kmm_init();
        vmm_init();
        kmm_init_kmalloc();
        LOGF_I("Kernel Memory Manager has been initialized\n");

        void *somemem = kmalloc(PLATFORM_PAGE_SIZE >> 2);
        kassert(somemem);
}
