/* UNITY_TEST DEPENDS ON: kernel/lib/ds/kvstore.c
 * UNITY_TEST DEPENDS ON: kernel/lib/mm/linear.c
 * UNITY_TEST DEPENDS ON: kernel/test_fakes/panic.c
 */

#include "lib/ds/kvstore.h"

#include "lib/tests/assert_failed.h"
#include "lib/utils.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <unity.h>

void setUp(void)
{}

void tearDown(void)
{}

static struct kvstore *create_kvstore_with_len(size_t const len, void **allocated_mem)
{
        size_t const req_mem = kvstore_predict_reqmem(len);
        *allocated_mem = malloc(req_mem);
        assert(NULL != *allocated_mem);

        /* NOTE: The cast _should_ be fine.  */
        return (kvstore_create(*allocated_mem, len, (kvstore_fn_cmpkeys_t)strcmp));
}

static void store_retrieve_single(void)
{
        void *mem;
        struct kvstore *kvstore = create_kvstore_with_len(1, &mem);

        static char *test_key = "test_key";
        static const uintptr_t test_val = 30;

        kvstore_append(kvstore, test_key, (void *)test_val);
        TEST_ASSERT_EQUAL_INT(1, kvstore_length(kvstore));

        uintptr_t val;
        TEST_ASSERT_TRUE(kvstore_find(kvstore, test_key, (void **)&val));
        TEST_ASSERT_EQUAL_INT(test_val, val);

        free(mem);
}

static void store_retrieve_multiple(void)
{
        static const struct {
                char *key;
                uintptr_t val;
        } testarr[] = { { "one", 10 },  { "two", 20 }, { "three", 30 }, { "four", 40 },
                        { "five", 50 }, { "six", 60 }, { "seven", 70 }, { "eight", 80 },
                        { "nine", 90 }, { "ten", 100 } };
        size_t const testarr_len = ARRAY_SIZE(testarr);

        void *mem;
        struct kvstore *kvstore = create_kvstore_with_len(testarr_len, &mem);

        for (size_t i = 0; i < testarr_len; i++) {
                kvstore_append(kvstore, testarr[i].key, (void *)testarr[i].val);
        }

        TEST_ASSERT_EQUAL_INT(testarr_len, kvstore_length(kvstore));

        for (signed long long i = (signed long long)testarr_len - 1; i >= 0; i--) {
                size_t const u = (size_t)i;
                uintptr_t val;
                TEST_ASSERT_TRUE(kvstore_find(kvstore, testarr[u].key, (void **)&val));
                TEST_ASSERT_EQUAL_INT(testarr[u].val, val);
        }

        free(mem);
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
        size_t const testarr_len = ARRAY_SIZE(testarr);

        void *mem;
        struct kvstore *kvstore = create_kvstore_with_len(testarr_len, &mem);

        for (size_t i = 0; i < 3; i++) {
                kvstore_append(kvstore, testarr[i].key, (void *)testarr[i].val);
        }

        TEST_ASSERT_EQUAL_INT(testarr_len, kvstore_length(kvstore));

        kvstore_remove(kvstore, "four");

        uintptr_t val;
        TEST_ASSERT_FALSE(kvstore_find(kvstore, "four", (void **)&val));

        kvstore_iter(kvstore, kvstore_iter_doesnt_contain, "four");

        free(mem);
}

static void dont_exceed(void)
{
        static const struct {
                char *key;
                uintptr_t val;
        } testarr[] = { { "one", 10 },  { "two", 20 },  { "three", 30 },  { "four", 40 },
                        { "five", 50 }, { "six", 60 },  { "seven", 70 },  { "eight", 80 },
                        { "nine", 90 }, { "ten", 100 }, { "eleven", 110 } };
        size_t const testarr_len = ARRAY_SIZE(testarr) - 1;

        void *mem;
        struct kvstore *kvstore = create_kvstore_with_len(testarr_len, &mem);

        for (size_t i = 0; i < testarr_len; i++) {
                kvstore_append(kvstore, testarr[i].key, (void *)testarr[i].val);
        }

        if (!failed_kassert(kvstore_append(kvstore, testarr[testarr_len].key,
                                           (void *)testarr[testarr_len].val))) {
                TEST_FAIL();
        }

        free(mem);
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
