#include "kernel/mm/buddy.h"

#include "arch/platform.h"

#include "kernel/config.h"
#include "kernel/cppdefs.h"

#include <stdlib.h>
#include <string.h>
#include <unity.h>

static char ALLOC_DATA[16 << 20];

static unsigned mylog2(unsigned x)
{
	unsigned a = 0;
	while (x >>= 1) {
		a++;
	}
	return (a);
}

#define SIZE 5
static void *regions[SIZE];
static size_t sizes[SIZE];
static struct buddy_allocator allocator;

void setUp(void)
{
	for (int i = 0; i < SIZE; i++) {
		sizes[i] = PLATFORM_PAGE_SIZE << (3 * i);
		regions[i] = aligned_alloc(PLATFORM_PAGE_SIZE, sizes[i]);
	}
	buddy_init(&allocator, regions, sizes, SIZE, ALLOC_DATA, sizeof(ALLOC_DATA));
}

void tearDown()
{
	for (int i = 0; i < SIZE; i++) {
		free(regions[i]);
	}
}

static void allocation_works(void)
{
	void *mem1 = buddy_alloc(&allocator, 0);
	TEST_ASSERT_MESSAGE(mem1, "Unable to allocate one page.");
	memset(mem1, 0xFF, 1 * PLATFORM_PAGE_SIZE);

	void *mem2 = buddy_alloc(&allocator, 0);
	TEST_ASSERT_MESSAGE(mem1 != mem2, "A block was allocated twice.");
}

static void big_allocation(void)
{
	unsigned order = mylog2(sizes[4] / PLATFORM_PAGE_SIZE);
	void *mem1 = buddy_alloc(&allocator, order);
	TEST_ASSERT_MESSAGE(mem1, "Can't allocate large block");
}

static void cant_allocate_more_than_own(void)
{
	unsigned order = mylog2(sizes[4] / PLATFORM_PAGE_SIZE) + 1;
	void *mem1 = buddy_alloc(&allocator, order);
	TEST_ASSERT_MESSAGE(!mem1, "It seems that the allocator has stolen some memory from "
				   "another universe. That won't do..");
}

static void free_works(void)
{
	// Allocate memory while there is any.
	void *last;
	void *cur;
	while ((cur = buddy_alloc(&allocator, 0))) {
		last = cur;
	}
	buddy_free(&allocator, last, 0);
	last = buddy_alloc(&allocator, 0);
	TEST_ASSERT_MESSAGE(last, "We know that allocator has one more page but it is hiding it!");
}

static void no_missing_memory(void)
{
	unsigned pages = 0;
	while (buddy_alloc(&allocator, 0)) {
		pages++;
	}

	int expected = 0;
	for (int i = 0; i < SIZE; i++) {
		expected += sizes[i];
	}

	TEST_ASSERT_MESSAGE(pages == expected / PLATFORM_PAGE_SIZE, "It seems that some memory has "
								    "been lost");
}

static void memory_page_aligned(void)
{
	void *mem;
	while ((mem = buddy_alloc(&allocator, 0))) {
		unsigned remainder = (uintptr_t)mem % PLATFORM_PAGE_SIZE;
		TEST_ASSERT_MESSAGE(remainder == 0, "Allocated memory isn't aligned at page "
						    "boundary.");
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
	UNITY_END();
	return (0);
}
