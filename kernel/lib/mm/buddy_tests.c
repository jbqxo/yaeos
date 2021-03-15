/* UNITY_TEST DEPENDS ON: kernel/lib/mm/buddy.c
 * UNITY_TEST DEPENDS ON: kernel/lib/mm/linear.c
 * UNITY_TEST DEPENDS ON: kernel/lib/ds/bitmap.c
 * UNITY_TEST DEPENDS ON: kernel/lib/cstd/string/memset.c
 * UNITY_TEST DEPENDS ON: kernel/test_fakes/panic.c
 */

#include "lib/mm/buddy.h"

#include "lib/cppdefs.h"
#include "lib/utils.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unity.h>

size_t const PLATFORM_PAGE_SIZE = 4096U;

static struct buddy_manager buddym;
static struct linear_alloc lin_alloc;
static void *mem;

static size_t const number_of_pages = (16 * 1024 * 1024) / PLATFORM_PAGE_SIZE;

void setUp(void)
{
        /* Allocate slightly more in case that the function returned the wrong value. */
        size_t const mem_space = buddy_predict_req_space(number_of_pages) + 0x100;
        mem = malloc(mem_space);
        assert(NULL != mem);

        linear_alloc_init(&lin_alloc, mem, mem_space);
        buddy_init(&buddym, number_of_pages, &lin_alloc);

        TEST_ASSERT_EQUAL_size_t(mem_space, linear_alloc_occupied(&lin_alloc));
}

void tearDown()
{
        free(mem);
}

static void allocation_works(void)
{
        uint32_t ndx1 = 0;
        TEST_ASSERT_TRUE_MESSAGE(buddy_alloc(&buddym, 0, &ndx1), "Unable to allocate single page");

        uint32_t ndx2 = 0;
        TEST_ASSERT_TRUE(buddy_alloc(&buddym, 0, &ndx2));
        TEST_ASSERT_MESSAGE(ndx1 != ndx2, "A block was allocated twice.");
}

static void big_allocation(void)
{
        unsigned order = log2_floor(number_of_pages);
        uint32_t ndx = 0;
        TEST_ASSERT_TRUE_MESSAGE(buddy_alloc(&buddym, order, &ndx), "Can't allocate large block");
}

static void cant_allocate_more_than_own(void)
{
        unsigned order = log2_ceil(number_of_pages) + 1;
        uint32_t ndx = 0;
        TEST_ASSERT_FALSE_MESSAGE(buddy_alloc(&buddym, order, &ndx),
                                  "It seems that the allocator has stolen some memory from "
                                  "another universe. That won't do..");
}

static void no_missing_memory(void)
{
        for (size_t i = 0; i < number_of_pages; i++) {
                uint32_t tmp __unused;
                TEST_ASSERT_TRUE_MESSAGE(buddy_alloc(&buddym, 0, &tmp),
                                         "It seems that some memory has been lost.");
        }
}

static void free_works(void)
{
        for (size_t i = 0; i < number_of_pages; i++) {
                uint32_t tmp __unused;
                TEST_ASSERT_TRUE(buddy_alloc(&buddym, 0, &tmp));
        }

        buddy_free(&buddym, 0, 0);

        uint32_t ndx = UINT32_MAX;
        TEST_ASSERT_TRUE_MESSAGE(
                buddy_alloc(&buddym, 0, &ndx),
                "We know that the allocator has one more page but it is hiding it!");
}

int main(void)
{
        UNITY_BEGIN();
        RUN_TEST(allocation_works);
        RUN_TEST(big_allocation);
        RUN_TEST(cant_allocate_more_than_own);
        RUN_TEST(no_missing_memory);
        RUN_TEST(free_works);
        UNITY_END();
        return (0);
}
