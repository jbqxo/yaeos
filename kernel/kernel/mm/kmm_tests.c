// UNITY_TEST DEPENDS ON: kernel/kernel/mm/pool.c
// UNITY_TEST DEPENDS ON: kernel/tests_utils.c

#include <kernel/cppdefs.h>
#include <arch/platform.h>
#include <kernel/mm/kmm.h>

#include <stdint.h>
#include <string.h>
#include <unity.h>
#include <stddef.h>
#include <stdlib.h>
#include <assert.h>

#define TESTVAL (0xAB)

#define VMM_MEM_LIMIT (6 * PLATFORM_PAGE_SIZE)
static size_t VMM_MEM_USAGE = 0;

void setUp(void)
{
	kmm_init();
	VMM_MEM_USAGE = 0;
}

void tearDown(void)
{
}

void *vmm_alloc_pages(size_t count, int flags __unused)
{
	// TODO: Improve VMM emulation?
	assert(count <= 1);

	size_t s = PLATFORM_PAGE_SIZE * count;
	if (VMM_MEM_USAGE + s > VMM_MEM_LIMIT) {
		return (NULL);
	}
	VMM_MEM_USAGE += s;

	return (aligned_alloc(PLATFORM_PAGE_SIZE, s));
}

void vmm_free_pages_at(void *address)
{
	TEST_ASSERT_NOT_NULL(address);
	VMM_MEM_USAGE -= PLATFORM_PAGE_SIZE;
	free(address);
}

static void small_allocations(void)
{
	typedef int small_t;

	small_t *(*ptrs)[VMM_MEM_LIMIT / sizeof(small_t)] = malloc(sizeof(*ptrs));
	size_t allocated = 0;
	small_t expected_value;
	memset(&expected_value, TESTVAL, sizeof(expected_value));

	struct kmm_cache *c = kmm_cache_create("test_cache", sizeof(small_t), 0, 0, NULL, NULL);
	TEST_ASSERT(c);

	TEST_MESSAGE("Allocating all available memory...");
	small_t *new;
	while ((new = kmm_cache_alloc(c))) {
		memset(new, TESTVAL, sizeof(*new));
		(*ptrs)[allocated] = new;
		allocated++;
	}
	TEST_ASSERT_MESSAGE(allocated > 0, "Couldn't allocate single small object");

	for (int i = 0; i < allocated; i++) {
		small_t *m = (*ptrs)[i];
		TEST_ASSERT_EQUAL_HEX8_ARRAY(&expected_value, m, sizeof(*m));

		kmm_cache_free(c, m);
	}

	kmm_cache_destroy(c);

	free(ptrs);
}

static void large_allocations(void)
{
	typedef char big_t[1024];

	big_t *(*ptrs)[VMM_MEM_LIMIT / sizeof(big_t)] = malloc(sizeof(*ptrs));
	size_t allocated = 0;
	big_t expected_value;
	memset(&expected_value, TESTVAL, sizeof(expected_value));

	struct kmm_cache *c = kmm_cache_create("test_cache", sizeof(big_t), 0, 0, NULL, NULL);
	TEST_ASSERT(c);

	TEST_MESSAGE("Allocating all available memory...");
	big_t *new;
	while ((new = kmm_cache_alloc(c))) {
		memset(new, TESTVAL, sizeof(*new));
		(*ptrs)[allocated] = new;
		allocated++;
	}
	TEST_ASSERT_MESSAGE(allocated > 0, "Couldn't allocate single large object");

	for (int i = 0; i < allocated; i++) {
		big_t *m = (*ptrs)[i];
		TEST_ASSERT_EQUAL_HEX8_ARRAY(&expected_value, m, sizeof(*m));

		kmm_cache_free(c, m);
	}

	kmm_cache_destroy(c);
	free(ptrs);
}

static void no_leaks(void)
{
	typedef uint32_t elem_t;

	elem_t *(*ptrs)[VMM_MEM_LIMIT / sizeof(elem_t)] = malloc(sizeof(*ptrs));
	size_t allocated = 0;
	elem_t expected_value;
	memset(&expected_value, TESTVAL, sizeof(expected_value));

	struct kmm_cache *c = kmm_cache_create("test_cache", sizeof(elem_t), 0, 0, NULL, NULL);
	TEST_ASSERT(c);

	TEST_MESSAGE("Allocating all available memory...");
	elem_t *new;
	while ((new = kmm_cache_alloc(c))) {
		memset(new, TESTVAL, sizeof(*new));
		(*ptrs)[allocated] = new;
		allocated++;
	}
	TEST_ASSERT_MESSAGE(allocated > 0, "Couldn't allocate single small object");

	for (int i = 0; i < allocated; i++) {
		elem_t *m = (*ptrs)[i];
		TEST_ASSERT_EQUAL_HEX8_ARRAY(&expected_value, m, sizeof(*m));

		kmm_cache_free(c, m);
	}

	size_t repeated_alloc = 0;
	while ((new = kmm_cache_alloc(c))) {
		memset(new, TESTVAL, sizeof(*new));
		(*ptrs)[repeated_alloc] = new;
		repeated_alloc++;
	}

	TEST_ASSERT_EQUAL_INT_MESSAGE(allocated, repeated_alloc,
				      "Missing elements on a second allocation");

	for (int i = 0; i < repeated_alloc; i++) {
		elem_t *m = (*ptrs)[i];
		TEST_ASSERT_EQUAL_HEX8_ARRAY(&expected_value, m, sizeof(*m));

		kmm_cache_free(c, m);
	}

	kmm_cache_destroy(c);
	free(ptrs);
}

static void memory_aligned(void)
{
	typedef uint32_t elem_t;
	const size_t required_align = 32;
	const size_t testset_len = 5;

	elem_t *(*ptrs)[testset_len] = malloc(sizeof(*ptrs));

	struct kmm_cache *cache =
		kmm_cache_create("test_cache", sizeof(elem_t), required_align, 0, NULL, NULL);
}

static void ctor_and_dtor(void)
{
}

static void reclaiming(void)
{
}

static void cache_coloring(void)
{
}

int main(void)
{
	UNITY_BEGIN();
	RUN_TEST(small_allocations);
	RUN_TEST(large_allocations);
	RUN_TEST(no_leaks);
	UNITY_END();
	return 0;
}
