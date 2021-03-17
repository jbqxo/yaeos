// UNITY_TEST DEPENDS ON: kernel/kernel/mm/kmm.c
// UNITY_TEST DEPENDS ON: kernel/lib/ds/slist.c
// UNITY_TEST DEPENDS ON: kernel/test_fakes/panic.c

#include "kernel/mm/kmm.h"

#include "lib/cppdefs.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unity.h>

#define TESTVAL (0xAB)

#define VMM_MEM_LIMIT (6 * PLATFORM_PAGE_SIZE)
static size_t VMM_MEM_USAGE = 0;

size_t const PLATFORM_PAGE_SIZE = 4096;

static void *alloc_page(void)
{
        size_t const s = PLATFORM_PAGE_SIZE;
        if (VMM_MEM_USAGE + s > VMM_MEM_LIMIT) {
                return (NULL);
        }
        VMM_MEM_USAGE += s;

        void *mem = aligned_alloc(PLATFORM_PAGE_SIZE, s);
        return (mem);
}

static void free_page(void *mem)
{
        TEST_ASSERT_NOT_NULL(mem);
        VMM_MEM_USAGE -= PLATFORM_PAGE_SIZE;
        free(mem);
}

void setUp(void)
{
        kmm_init(alloc_page, free_page);
        VMM_MEM_USAGE = 0;
}

void tearDown(void)
{
        kmm_cache_trim_all();
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

        for (size_t i = 0; i < allocated; i++) {
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

        for (size_t i = 0; i < allocated; i++) {
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

        for (size_t i = 0; i < allocated; i++) {
                elem_t *m = (*ptrs)[i];
                TEST_ASSERT_EQUAL_HEX8_ARRAY(&expected_value, m, sizeof(*m));

                kmm_cache_free(c, m);
        }

        size_t repeated_alloc = 0;
        TEST_MESSAGE("Allocating all available memory...");
        while ((new = kmm_cache_alloc(c))) {
                memset(new, TESTVAL, sizeof(*new));
                (*ptrs)[repeated_alloc] = new;
                repeated_alloc++;
        }

        TEST_ASSERT_EQUAL_INT_MESSAGE(allocated, repeated_alloc,
                                      "Missing elements on a second allocation");

        for (size_t i = 0; i < repeated_alloc; i++) {
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
        const size_t required_alignment = 32;
#define TESTSET_LEN 5

        elem_t *ptrs[TESTSET_LEN] = { 0 };

        struct kmm_cache *cache =
                kmm_cache_create("test_cache", sizeof(elem_t), required_alignment, 0, NULL, NULL);
        TEST_ASSERT(cache);
        for (size_t i = 0; i < TESTSET_LEN; i++) {
                ptrs[i] = kmm_cache_alloc(cache);
                TEST_ASSERT(NULL != ptrs[i]);
                TEST_ASSERT_EQUAL_INT_MESSAGE(0, (uintptr_t)ptrs[i] % required_alignment,
                                              "Alignment request hasn't been honored");
        }
        for (size_t i = 0; i < TESTSET_LEN; i++) {
                kmm_cache_free(cache, ptrs[i]);
        }
        kmm_cache_destroy(cache);
}

static const unsigned constructor__foo = 0xFE;
static const unsigned constructor__bar = 0xEF;
struct constructor__type {
        unsigned foo;
        unsigned bar;
        void *pfoo;
};

static void constructor__ctor(void *mem)
{
        struct constructor__type *obj = mem;
        obj->foo = constructor__foo;
        obj->bar = constructor__bar;
        obj->pfoo = mem;
}

static void constructor(void)
{
        struct constructor__type *obj;
        struct kmm_cache *cache =
                kmm_cache_create("test_cache", sizeof(*obj), 0, 0, constructor__ctor, NULL);
        TEST_ASSERT(cache);

        obj = kmm_cache_alloc(cache);
        TEST_ASSERT(obj);

        TEST_ASSERT_EQUAL_INT_MESSAGE(constructor__foo, obj->foo,
                                      "'foo' field hasn't been initialized");
        TEST_ASSERT_EQUAL_INT_MESSAGE(constructor__bar, obj->bar,
                                      "'bar' field hasn't been initialized");
        TEST_ASSERT_EQUAL_PTR_MESSAGE(obj, obj->pfoo, "'pfoo' field hasn't been initialized");

        kmm_cache_free(cache, obj);
        kmm_cache_destroy(cache);
}

static bool destroyed = false;
static int *dtor__obj = NULL;

static void destructor__dtor(void *mem)
{
        if (mem == dtor__obj) {
                destroyed = true;
        }
}

static void destructor(void)
{
        struct kmm_cache *cache =
                kmm_cache_create("test_cache", sizeof(*dtor__obj), 0, 0, NULL, destructor__dtor);
        TEST_ASSERT(cache);

        dtor__obj = kmm_cache_alloc(cache);
        TEST_ASSERT(dtor__obj);

        kmm_cache_free(cache, dtor__obj);
        kmm_cache_trim(cache);
        kmm_cache_destroy(cache);

        TEST_ASSERT_MESSAGE(destroyed, "Destructor wasn't called.");
}

static void trimming(void)
{
        typedef uint32_t elem_t;

        elem_t *(*ptrs)[VMM_MEM_LIMIT / sizeof(elem_t)] = malloc(sizeof(*ptrs));
        size_t allocated = 0;

        struct kmm_cache *first_cache =
                kmm_cache_create("first_cache", sizeof(elem_t), 0, 0, NULL, NULL);
        TEST_ASSERT(first_cache);
        struct kmm_cache *second_cache =
                kmm_cache_create("second_cache", sizeof(elem_t), 0, 0, NULL, NULL);
        TEST_ASSERT(second_cache);

        TEST_MESSAGE("Allocating all available memory...");
        elem_t *new;
        while ((new = kmm_cache_alloc(first_cache))) {
                (*ptrs)[allocated] = new;
                allocated++;
        }
        TEST_ASSERT_MESSAGE(allocated > 0, "Couldn't allocate single small object");

        TEST_ASSERT_MESSAGE(
                kmm_cache_alloc(second_cache) == NULL,
                "Somehow we managed to allocate memory while it's supposed to be occupied");

        for (size_t i = 0; i < allocated; i++) {
                elem_t *m = (*ptrs)[i];
                kmm_cache_free(first_cache, m);
        }

        /* Trim unused memory from other caches. */
        kmm_cache_trim_all();

        size_t repeated_alloc = 0;
        TEST_MESSAGE("Allocating all available memory...");
        while ((new = kmm_cache_alloc(second_cache))) {
                (*ptrs)[repeated_alloc] = new;
                repeated_alloc++;
        }

        TEST_ASSERT_EQUAL_INT_MESSAGE(
                allocated, repeated_alloc,
                "It seems we weren't able to reclaim memory from the first cache");

        for (size_t i = 0; i < repeated_alloc; i++) {
                elem_t *m = (*ptrs)[i];
                kmm_cache_free(second_cache, m);
        }

        kmm_cache_destroy(first_cache);
        kmm_cache_destroy(second_cache);
        free(ptrs);
}

static void cache_coloring(void)
{
        typedef char elem_t[37];
#define CACHELINE_LEN 64
        size_t colour_hits[CACHELINE_LEN] = { 0 };

        size_t allocated = 0;
        elem_t *(*ptrs)[VMM_MEM_LIMIT / sizeof(elem_t)] = malloc(sizeof(*ptrs));

        struct kmm_cache *cache = kmm_cache_create("test_cache", sizeof(elem_t), 0, 0, NULL, NULL);

        TEST_MESSAGE("Allocating all available memory...");
        elem_t *new;
        while ((new = kmm_cache_alloc(cache))) {
                size_t cacheline_offset = (uintptr_t) new % 64;
                colour_hits[cacheline_offset]++;

                (*ptrs)[allocated] = new;
                allocated++;
        }

        TEST_MESSAGE("Cache colour hits (colour, times): ");
        size_t colours = 0;
        for (size_t colour = 0; colour < CACHELINE_LEN; colour++) {
                size_t hits = colour_hits[colour];
                if (hits > 0) {
                        TEST_PRINTF("(%d, %d) ", colour, hits);
                        colours++;
                }
        }
        TEST_ASSERT_GREATER_THAN_INT_MESSAGE(
                1, colours, "We have pretty monotonic cache. Though this test isn't very robust");

        for (size_t i = 0; i < allocated; i++) {
                elem_t *m = (*ptrs)[i];
                kmm_cache_free(cache, m);
        }

        kmm_cache_destroy(cache);
        free(ptrs);
}

int main(void)
{
        UNITY_BEGIN();
        RUN_TEST(small_allocations);
        RUN_TEST(large_allocations);
        RUN_TEST(no_leaks);
        RUN_TEST(memory_aligned);
        RUN_TEST(constructor);
        RUN_TEST(destructor);
        RUN_TEST(trimming);
        RUN_TEST(cache_coloring);
        UNITY_END();
        return (0);
}
