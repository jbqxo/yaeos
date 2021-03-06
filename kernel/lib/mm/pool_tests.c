/*
 * UNITY_TEST DEPENDS ON: kernel/lib/mm/pool.c
 * UNITY_TEST DEPENDS ON: kernel/lib/ds/slist.c
 * UNITY_TEST DEPENDS ON: kernel/test_fakes/panic.c
 */

#include "lib/mm/pool.h"

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <unity.h>

#define SET_SIZE 1000
static int64_t SET[SET_SIZE];
static struct mem_pool POOL;

void setUp()
{
        mem_pool_init(&POOL, SET, sizeof(int64_t) * SET_SIZE, sizeof(int64_t), sizeof(int64_t));
}

void tearDown()
{
}

static void can_allocate_and_free_single_element(void)
{
        int *result = mem_pool_alloc(&POOL);
        TEST_ASSERT_NOT_NULL(result);

        *result = -1;

        mem_pool_free(&POOL, result);
}

static void can_allocate_and_free_many_elements(void)
{
        int64_t *results[SET_SIZE];
        for (size_t i = 0; i < SET_SIZE; i++) {
                results[i] = mem_pool_alloc(&POOL);
                TEST_ASSERT_NOT_NULL(results[i]);

                *results[i] = -1;
        }

        for (size_t i = 0; i < SET_SIZE; i++) {
                mem_pool_free(&POOL, results[i]);
        }
}

static void receive_an_error_when_there_are_no_free_blocks(void)
{
        for (size_t i = 0; i < SET_SIZE; i++) {
                int64_t *result = mem_pool_alloc(&POOL);
                TEST_ASSERT_NOT_NULL(result);

                *result = -1;
        }
        void *mem = mem_pool_alloc(&POOL);
        TEST_ASSERT_NULL(mem);
}

int main(void)
{
        UNITY_BEGIN();
        RUN_TEST(can_allocate_and_free_single_element);
        RUN_TEST(can_allocate_and_free_many_elements);
        RUN_TEST(receive_an_error_when_there_are_no_free_blocks);
        UNITY_END();
        return (0);
}
