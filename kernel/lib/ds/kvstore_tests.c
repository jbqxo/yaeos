#include "lib/ds/kvstore.h"

#include <stddef.h>
#include <string.h>
#include <unity.h>

static const int kvstore_len = 10;
static KVSTATIC_DECLARE(char *, int, kvstore_len, strcmp) kvstore;

void setUp(void)
{
        KVSTATIC_INIT(&kvstore, "", 0, strcmp);
}

void tearDown(void)
{}

static void store_retrieve_single(void)
{
        static char *test_key = "test_key";
        static const int test_val = 30;

        KVSTATIC_ADD(&kvstore, test_key, test_val);
        int len;
        KVSTATIC_LEN(&kvstore, len);
        TEST_ASSERT_EQUAL_INT(1, len);

        int val;
        KVSTATIC_GET(&kvstore, test_key, val, -1);
        TEST_ASSERT_EQUAL_INT(test_val, val);
}

static void store_retrieve_multiple(void)
{
        static const struct {
                char *key;
                int val;
        } testarr[kvstore_len] = { { "one", 10 },  { "two", 20 }, { "three", 30 }, { "four", 40 },
                                   { "five", 50 }, { "six", 60 }, { "seven", 70 }, { "eight", 80 },
                                   { "nine", 90 }, { "ten", 100 } };

        for (int i = 0; i < kvstore_len; i++) {
                KVSTATIC_ADD(&kvstore, testarr[i].key, testarr[i].val);
        }

        int len;
        KVSTATIC_LEN(&kvstore, len);
        TEST_ASSERT_EQUAL_INT(kvstore_len, len);

        for (int i = kvstore_len - 1; i >= 0; i--) {
                int val;
                KVSTATIC_GET(&kvstore, testarr[i].key, val, -1);
                TEST_ASSERT_EQUAL_INT(testarr[i].val, val);
        }
}

static void traverse(void)
{
        static const struct {
                char *key;
                int val;
        } testarr[3] = { { "one", 1 }, { "four", 4 }, { "six", 6 } };

        bool validatearr_expected[7] = { false, true, false, false, true, false, true };
        bool validatearr[7] = { 0 };

        for (int i = 0; i < 3; i++) {
                KVSTATIC_ADD(&kvstore, testarr[i].key, testarr[i].val);
        }

        int i;
        char *key;
        int val;
        KVSTATIC_FOREACH (&kvstore, i, key, val) {
                validatearr[val] = true;
        }

        for (int i = 0; i < 7; i++) {
                TEST_ASSERT(validatearr[i] == validatearr_expected[i]);
        }
}

static void remove_pair(void)
{
        static const struct {
                char *key;
                int val;
        } testarr[3] = { { "one", 1 }, { "four", 4 }, { "six", 6 } };

        for (int i = 0; i < 3; i++) {
                KVSTATIC_ADD(&kvstore, testarr[i].key, testarr[i].val);
        }

        int len;
        KVSTATIC_LEN(&kvstore, len);
        TEST_ASSERT_EQUAL_INT(3, len);

        KVSTATIC_DEL(&kvstore, "four");

        int val;
        KVSTATIC_GET(&kvstore, "four", val, -1);
        TEST_ASSERT_EQUAL_INT(-1, val);

        int i;
        char *key;
        KVSTATIC_FOREACH (&kvstore, i, key, val) {
                TEST_ASSERT(strcmp(key, "four") != 0);
                TEST_ASSERT_NOT_EQUAL(4, val);
        }
}

static void dont_exceed(void)
{
        static const struct {
                char *key;
                int val;
        } testarr[kvstore_len + 1] = { { "one", 10 },   { "two", 20 },    { "three", 30 },
                                       { "four", 40 },  { "five", 50 },   { "six", 60 },
                                       { "seven", 70 }, { "eight", 80 },  { "nine", 90 },
                                       { "ten", 100 },  { "eleven", 110 } };

        for (int i = 0; i < kvstore_len + 1; i++) {
                KVSTATIC_ADD(&kvstore, testarr[i].key, testarr[i].val);
        }

        int len;
        KVSTATIC_LEN(&kvstore, len);
        TEST_ASSERT_EQUAL_INT(kvstore_len, len);

        int val;
        KVSTATIC_GET(&kvstore, "eleven", val, -1);
        TEST_ASSERT_EQUAL(-1, val);

        int i;
        char *key;
        KVSTATIC_FOREACH (&kvstore, i, key, val) {
                TEST_ASSERT(strcmp(key, "eleven") != 0);
                TEST_ASSERT_NOT_EQUAL(110, val);
        }
}

static void get_failvalue_assert(void)
{
        int val;
        KVSTATIC_GET(&kvstore, "any", val, 289);
        TEST_ASSERT_EQUAL_INT(289, val);
}

int main(void)
{
        UNITY_BEGIN();
        RUN_TEST(store_retrieve_single);
        RUN_TEST(store_retrieve_multiple);
        RUN_TEST(traverse);
        RUN_TEST(remove_pair);
        RUN_TEST(dont_exceed);
        RUN_TEST(get_failvalue_assert);
        UNITY_END();
        return (0);
}
