/* UNITY_TEST DEPENDS ON: kernel/lib/ds/kvstore.c
 * UNITY_TEST DEPENDS ON: kernel/lib/mm/linear.c
 * UNITY_TEST DEPENDS ON: kernel/test_fakes/panic.c
 */

#include "lib/ds/kvstore.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <unity.h>

#define KVSTORE_LEN 10
static void *kvstore_mem;
static struct kvstore *kvstore;

void setUp(void)
{
        size_t const req_mem = kvstore_predict_reqmem(KVSTORE_LEN);
        kvstore_mem = malloc(req_mem);
        assert(NULL != kvstore_mem);

        kvstore = kvstore_create(kvstore_mem, KVSTORE_LEN, strcmp);
}

void tearDown(void)
{
        free(kvstore_mem);
}

static void store_retrieve_single(void)
{
        static char *test_key = "test_key";
        static const uintptr_t test_val = 30;

        kvstore_append(kvstore, test_key, (void*)test_val);
        TEST_ASSERT_EQUAL_INT(1, kvstore_length(kvstore));

        uintptr_t val;
        TEST_ASSERT_TRUE(kvstore_find(kvstore, test_key, (void **)&val));
        TEST_ASSERT_EQUAL_INT(test_val, val);
}

static void store_retrieve_multiple(void)
{
        static const struct {
                char *key;
                uintptr_t val;
        } testarr[KVSTORE_LEN] = { { "one", 10 },  { "two", 20 }, { "three", 30 }, { "four", 40 },
                                   { "five", 50 }, { "six", 60 }, { "seven", 70 }, { "eight", 80 },
                                   { "nine", 90 }, { "ten", 100 } };

        for (size_t i = 0; i < KVSTORE_LEN; i++) {
                kvstore_append(kvstore, testarr[i].key, (void*)testarr[i].val);
        }

        TEST_ASSERT_EQUAL_INT(KVSTORE_LEN, kvstore_length(kvstore));

        for (size_t i = KVSTORE_LEN - 1; i >= 0; i--) {
                uintptr_t val;
                TEST_ASSERT_TRUE(kvstore_find(kvstore, testarr[i].key, (void**)&val));
                TEST_ASSERT_EQUAL_INT(testarr[i].val, val);
        }
}

static void kvstore_iter_doesnt_contain(void *key __unused, void *value __unused, void *data)
{
        TEST_ASSERT(strcmp(key, data) != 0);
}

static void remove_pair(void)
{
        static const struct {
                char *key;
                uintptr_t val;
        } testarr[3] = { { "one", 1 }, { "four", 4 }, { "six", 6 } };

        for (size_t i = 0; i < 3; i++) {
                kvstore_append(kvstore, testarr[i].key, (void*)testarr[i].val);
        }

        TEST_ASSERT_EQUAL_INT(KVSTORE_LEN, kvstore_length(kvstore));

        kvstore_remove(kvstore, "four");

        uintptr_t val;
        TEST_ASSERT_FALSE(kvstore_find(kvstore, "four", (void**)&val));

        kvstore_iter(kvstore, kvstore_iter_doesnt_contain, "four");
}

static void dont_exceed(void)
{
        static const struct {
                char *key;
                int val;
        } testarr[KVSTORE_LEN + 1] = { { "one", 10 },   { "two", 20 },    { "three", 30 },
                                       { "four", 40 },  { "five", 50 },   { "six", 60 },
                                       { "seven", 70 }, { "eight", 80 },  { "nine", 90 },
                                       { "ten", 100 },  { "eleven", 110 } };

        for (size_t i = 0; i < KVSTORE_LEN + 1; i++) {
                kvstore_append(kvstore, testarr[i].key, (void*)testarr[i].val);
        }

        TEST_ASSERT_EQUAL_INT(KVSTORE_LEN, kvstore_length(kvstore));

        uintptr_t val;
        TEST_ASSERT_FALSE(kvstore_find(kvstore, "eleven", (void**)&val));

        kvstore_iter(kvstore, kvstore_iter_doesnt_contain, "eleven");
}

int main(void)
{
        UNITY_BEGIN();
        RUN_TEST(store_retrieve_single);
        RUN_TEST(store_retrieve_multiple);
        RUN_TEST(remove_pair);
        RUN_TEST(dont_exceed);
        UNITY_END();
        return (0);
}
