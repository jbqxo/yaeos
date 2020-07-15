#include <kernel/mm.h>

static struct page_allocator {
	void *data;
	page_alloc_fn alloc_fn;
	page_free_fn free_fn;
} PAGE_ALLOCATOR;

void mm_alloc_init(void *data, page_alloc_fn afn, page_free_fn ffn)
{
	PAGE_ALLOCATOR = (struct page_allocator){ data, afn, ffn };
}

void *mm_page_alloc(unsigned pages) {
	return PAGE_ALLOCATOR.alloc_fn(PAGE_ALLOCATOR.data, pages);
}

void mm_page_free(void *mem, unsigned pages) {
	return PAGE_ALLOCATOR.free_fn(PAGE_ALLOCATOR.data, mem, pages);
}
