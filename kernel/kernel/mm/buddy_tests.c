#include <unity.h>
#include <kernel/mm.h>
#include <kernel/cppdefs.h>
#include <arch/platform.h>
#include <stdlib.h>
#include <string.h>

const size_t PLATFORM_PAGE_SIZE = 4096;
// TODO: Test BUDDY_LOWMEM too
const uintptr_t PLATFORM_KERNEL_VMA = 0;

// Allocator uses this symbol inside when it receives BUDDY_LOWMEM flag.
const char dumb_kernel_vma asm("_kernel_vma");

#define SIZE 5
static void *regions[SIZE];
static size_t sizes[SIZE];
static struct buddy_allocator *allocator = NULL;

void setUp(void)
{
	for (int i = 0; i < SIZE; i++) {
		sizes[i] = PLATFORM_PAGE_SIZE << (3 * i);
		regions[i] = aligned_alloc(PLATFORM_PAGE_SIZE, sizes[i]);
	}
	allocator = buddy_init(regions, sizes, SIZE, 0);
	TEST_ASSERT_MESSAGE(allocator, "Unable to initialize allocator.");
}

void tearDown()
{
	for (int i = 0; i < SIZE; i++) {
		free(regions[i]);
	}
}

static void allocation_works(void)
{
	void *mem1 = buddy_alloc(allocator, 1);
	TEST_ASSERT_MESSAGE(mem1, "Unable to allocate one page.");
	memset(mem1, 0xFF, 1 * PLATFORM_PAGE_SIZE);

	void *mem2 = buddy_alloc(allocator, 1);
	TEST_ASSERT_MESSAGE(mem1 != mem2, "A block was allocated twice.");
}

static void big_allocation(void)
{
	void *mem1 = buddy_alloc(allocator, sizes[4] / PLATFORM_PAGE_SIZE);
	TEST_ASSERT_MESSAGE(mem1, "Can't allocate large block");
}

static void cant_allocate_more_than_own(void)
{
	void *mem1 = buddy_alloc(allocator, sizes[4] / PLATFORM_PAGE_SIZE + 1);
	TEST_ASSERT_MESSAGE(
		!mem1,
		"It seems that the allocator has stolen some memory from another universe. That won't do..");
}

static void free_works(void)
{
	// Allocate memory while there is any.
	void *last;
	void *cur;
	while ((cur = buddy_alloc(allocator, 1))) {
		last = cur;
	}
	buddy_free(allocator, last, 1);
	last = buddy_alloc(allocator, 1);
	TEST_ASSERT_MESSAGE(last, "We know that allocator has one more page but it is hiding it!");
}

static void no_missing_memory(void)
{
	unsigned pages = 0;
	while (buddy_alloc(allocator, 1)) {
		pages++;
	}

	// Some memory is occupied by the allocator itself.
	int expected = -((allocator->data_size + PLATFORM_PAGE_SIZE - 1) & -PLATFORM_PAGE_SIZE);
	for (int i = 0; i < SIZE; i++) {
		expected += sizes[i];
	}

	TEST_ASSERT_MESSAGE(pages == expected / PLATFORM_PAGE_SIZE,
			    "It seems that some memory has been lost");
}

static void memory_page_aligned(void)
{
	void *mem;
	while ((mem = buddy_alloc(allocator, 1))) {
		unsigned remainder = (uintptr_t)mem % PLATFORM_PAGE_SIZE;
		TEST_ASSERT_MESSAGE(remainder == 0,
				    "Allocated memory isn't aligned at page boundary.");
	}
}

int main(void)
{
	UNITY_BEGIN();
	RUN_TEST(allocation_works);
	RUN_TEST(big_allocation);
	RUN_TEST(cant_allocate_more_than_own);
	RUN_TEST(free_works);
	RUN_TEST(no_missing_memory);
	RUN_TEST(memory_page_aligned);
	return UNITY_END();
}
