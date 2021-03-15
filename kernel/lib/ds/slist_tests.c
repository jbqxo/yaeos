/* UNITY_TEST DEPENDS ON: kernel/lib/ds/slist.c
 * UNITY_TEST DEPENDS ON: kernel/test_fakes/panic.c
 */

#include "lib/ds/slist.h"

#include "lib/cppdefs.h"
#include "lib/utils.h"

#include <stddef.h>
#include <unity.h>

struct node {
        int val;
#define node_of(PTR) (container_of((PTR), struct node, list))
        struct slist_ref list;
};

static struct slist_ref HEAD;

void setUp(void)
{
        slist_init(&HEAD);
}

void tearDown(void)
{}

static void single_element_insertion_head(void)
{
        struct node test = (struct node){ .val = 10 };
        slist_insert(&HEAD, &test.list);
        TEST_ASSERT(slist_next(&HEAD) == &test.list);
        TEST_ASSERT(node_of(slist_next(&HEAD))->val == test.val);
}

static void single_element_insertion_middle(void)
{
        struct node testobjs[] = {
                (struct node){ .val = 10 },
                (struct node){ .val = 11 },
                (struct node){ .val = 12 },
        };
        slist_insert(&HEAD, &testobjs[0].list);
        slist_insert(&testobjs[0].list, &testobjs[2].list);
        slist_insert(&testobjs[0].list, &testobjs[1].list);

        size_t i = 0;
        SLIST_FOREACH (it, slist_next(&HEAD)) {
                struct node *n = node_of(it);
                TEST_ASSERT(n->val == testobjs[i].val);
                TEST_ASSERT(n == &testobjs[i]);

                i++;
        }
}

static void single_element_removal_head(void)
{
        struct node testobjs[] = {
                (struct node){ .val = 10 },
                (struct node){ .val = 11 },
                (struct node){ .val = 12 },
        };
        size_t testobjs_len = ARRAY_SIZE(testobjs);

        slist_insert(&HEAD, &testobjs[0].list);
        for (size_t i = 1; i < testobjs_len; i++) {
                slist_insert(&testobjs[i - 1].list, &testobjs[i].list);
        }

        slist_remove(&HEAD, &testobjs[0].list);
        size_t i = 1;
        SLIST_FOREACH (it, &HEAD) {
                struct node *n = node_of(it);
                TEST_ASSERT(testobjs[i].val == n->val);
                TEST_ASSERT(&testobjs[i] == n);
                i++;
        }
}

static void single_element_removal_middle(void)
{
        struct node testobjs[] = {
                (struct node){ .val = 10 },
                (struct node){ .val = 11 },
                (struct node){ .val = 12 },
        };
        size_t testobjs_len = ARRAY_SIZE(testobjs);

        slist_insert(&HEAD, &testobjs[0].list);
        for (size_t i = 1; i < testobjs_len; i++) {
                slist_insert(&testobjs[i - 1].list, &testobjs[i].list);
        }

        slist_remove(&HEAD, &testobjs[1].list);

        struct slist_ref *first_ref = slist_next(&HEAD);
        struct slist_ref *second_ref = slist_next(first_ref);

        TEST_ASSERT(&testobjs[0] == node_of(first_ref));
        TEST_ASSERT(testobjs[0].val == node_of(first_ref)->val);

        TEST_ASSERT(&testobjs[2] == node_of(second_ref));
        TEST_ASSERT(testobjs[2].val == node_of(second_ref)->val);
}

static void single_element_removal(void)
{
        struct node testobjs[] = {
                (struct node){ .val = 10 },
                (struct node){ .val = 11 },
                (struct node){ .val = 12 },
        };
        size_t testobjs_len = ARRAY_SIZE(testobjs);

        slist_insert(&HEAD, &testobjs[0].list);
        for (size_t i = 1; i < testobjs_len; i++) {
                slist_insert(&testobjs[i - 1].list, &testobjs[i].list);
        }

        slist_remove(&HEAD, &testobjs[2].list);
        size_t i = 0;
        SLIST_FOREACH (it, &HEAD) {
                struct node *n = node_of(it);

                TEST_ASSERT(i < 2);
                TEST_ASSERT(&testobjs[i] == n);
                TEST_ASSERT(testobjs[i].val == n->val);
                i++;
        }
}

static void survive_empty_list_traversal(void)
{
        SLIST_FOREACH (it, &HEAD) {
                TEST_FAIL();
        }
}

int main(void)
{
        UNITY_BEGIN();
        RUN_TEST(single_element_insertion_head);
        RUN_TEST(single_element_insertion_middle);
        RUN_TEST(single_element_removal_head);
        RUN_TEST(single_element_removal_middle);
        RUN_TEST(single_element_removal);
        RUN_TEST(survive_empty_list_traversal);
        UNITY_END();
        return (0);
}
