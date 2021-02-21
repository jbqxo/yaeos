#include "arch_i686/kernel.h"
#include "arch_i686/vm.h"

#include "kernel/kernel.h"
#include "kernel/mm/kmm.h"
#include "kernel/mm/mm.h"

#include "lib/align.h"
#include "lib/cppdefs.h"
#include "lib/cstd/nonstd.h"
#include "lib/klog.h"
#include "lib/platform_consts.h"

#include <multiboot.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

static uintptr_t max_addr(void)
{
        unsigned width = sizeof(void *) * 8;
        uintptr_t mask = 0;
        for (unsigned i = 0; i < width; i++) {
                mask |= (uintptr_t)(0x1) << i;
        }
        return ((uintptr_t)(-0x1) & mask);
}

typedef void (*chunks_it_fn)(struct multiboot_mmap_entry *c, void *data);

static void chunks_iter(chunks_it_fn fn, void *extra_data)
{
        multiboot_info_t *info = I686_INFO.multiboot;
        struct multiboot_mmap_entry *c = (void *)info->mmap_addr;

        // mmap_* variables are not valid. There is nothing we can do.
        if (!(info->flags & MULTIBOOT_INFO_MEM_MAP)) {
                return;
        }

        while ((uintptr_t)c < info->mmap_addr + info->mmap_length) {
                fn(c, extra_data);
                c = (void *)((uintptr_t)c + c->size + sizeof(c->size));
        }
}

typedef void (*availmem_it_fn)(uintptr_t start, uintptr_t end, uint32_t type, void *data);
struct availmem_data {
        availmem_it_fn fn;
        void *fn_data;
};
static void availmem_iter(struct multiboot_mmap_entry *mmap, void *_data)
{
        // Skip unavailable memory.
        if (mmap->type == MULTIBOOT_MEMORY_BADRAM || mmap->type == MULTIBOOT_MEMORY_RESERVED) {
                return;
        }

        // ... if the chunk is entirely out of our reach, ignore it.
        if (mmap->addr > max_addr()) {
                return;
        }

        struct availmem_data *data = _data;
        uintptr_t kstart = ptr2uint(kernel_arch_to_low(kernel_start));
        uintptr_t kend = ptr2uint(kernel_arch_to_low(kernel_end));
        uintptr_t memstart = mmap->addr;
        uintptr_t memend = memstart + mmap->len;

        /*
         * There are following cases:
         * 1. the kernel before the region => Do Nothing
         * 2. the kernel after the region => Do Nothing
         * 3. the kernel overlap with the begginning of the region => Update: memstart = kend
         * 4. the kernel overlap with the end of the region => Update: memend = kstart
         * 5. the region fully in the kernel => Drop the region altogether
         * 6. the kernel fully in the region => Split the region:
         *     memstart_1 = memstart;
         *     memend_1 = kstart
         *     memstart_2 = kend
         *     memend_2 = memend
         */

        /*
         * Keep in mind that `kstart` points at the actual beginning of the kernel's data.
         * But `kend` actually points at linker's location counter, so we are free to use this byte.
         * This is not really important then to try to set correct boundaries, because
         * in the worst-case scenario only multiboot header would be damaged, and it's no use to us.
         * So the boundaries checks there may be a bit incorrect (1 byte) and this is fine.
         */

        // Case 1.
        if (kend <= memstart) {
                data->fn(memstart, memend, mmap->type, data->fn_data);
                return;
        }

        // Case 2.
        if (kstart >= memend) {
                data->fn(memstart, memend, mmap->type, data->fn_data);
                return;
        }

        // Case 3.
        if (kend > memstart && kstart <= memstart) {
                data->fn(kend, memend, mmap->type, data->fn_data);
                return;
        }

        // Case 4.
        if (kstart < memend && kend >= memend) {
                data->fn(memend, kstart, mmap->type, data->fn_data);
                return;
        }

        // Case 5.
        if (kstart <= memstart && kend >= memend) {
                // Ignore the region.
                return;
        }

        // Case 6.
        if (kstart >= memstart && kend <= memend) {
                data->fn(memstart, kstart, mmap->type, data->fn_data);
                data->fn(kend, memend, mmap->type, data->fn_data);
                return;
        }
        LOGF_P("Unhandled case.\n");
}

static void count(uintptr_t start __unused, uintptr_t end __unused, uint32_t type __unused,
                  void *data)
{
        const size_t length = end - start;
        if (length <= PLATFORM_PAGE_SIZE) {
                return;
        }

        int *counter = data;
        (*counter)++;
}

static void find(uintptr_t start, uintptr_t end, uint32_t type, void *data)
{
        struct mem_chunk **chunks = data;
        struct mem_chunk *chunk = *chunks;
        const size_t length = end - start;

        if (length <= PLATFORM_PAGE_SIZE) {
                return;
        }

        chunk->mem = (void *)start;
        // ... if we can address some part of the chunk, cut remainders out.
        chunk->length = MIN(length, max_addr() - start);
        chunk->type = type;
        (*chunks)++;
}

int mm_arch_chunks_len(void)
{
        int counter = 0;
        struct availmem_data d = (struct availmem_data){ .fn = count, .fn_data = &counter };
        chunks_iter(availmem_iter, &d);
        return (counter);
}

void mm_arch_get_chunks(struct mem_chunk *chunks)
{
        struct availmem_data d = (struct availmem_data){ .fn = find, .fn_data = &chunks };
        chunks_iter(availmem_iter, &d);
}
