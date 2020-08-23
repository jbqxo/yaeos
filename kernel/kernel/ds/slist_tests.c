#include "kernel/ds/slist.h"

#include <stddef.h>
#include <unity.h>

struct node {
	int val;
	SLIST_FIELD(struct node) list;
};

static SLIST_HEAD(, struct node) HEAD;

void setUp(void)
{
	SLIST_INIT(&HEAD);
}

void tearDown(void)
{}

static void single_element_insertion_head(void)
{
	struct node test = (struct node){ .val = 10 };
	SLIST_INSERT_HEAD(&HEAD, &test, list);
	TEST_ASSERT(SLIST_FIRST(&HEAD)->val == test.val);
	TEST_ASSERT(SLIST_FIRST(&HEAD) == &test);
}

static void single_element_insertion_middle(void)
{
	struct node testobjs[] = {
		(struct node){ .val = 10 },
		(struct node){ .val = 11 },
		(struct node){ .val = 12 },
	};
	SLIST_INSERT_HEAD(&HEAD, &testobjs[0], list);
	SLIST_INSERT_AFTER(SLIST_FIRST(&HEAD), &testobjs[2], list);

	SLIST_INSERT_AFTER(SLIST_FIRST(&HEAD), &testobjs[1], list);

	struct node *itern;
	int i = 0;
	SLIST_FOREACH (itern, &HEAD, list) {
		TEST_ASSERT(itern->val == testobjs[i].val);
		TEST_ASSERT(itern == &testobjs[i]);
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
	size_t testobjs_len = sizeof(testobjs) / sizeof(*testobjs);

	SLIST_INSERT_HEAD(&HEAD, &testobjs[0], list);
	for (int i = 1; i < testobjs_len; i++) {
		SLIST_INSERT_AFTER(&testobjs[i - 1], &testobjs[i], list);
	}

	SLIST_REMOVE_HEAD(&HEAD, list);
	struct node *itern = NULL;
	int i = 1;
	SLIST_FOREACH (itern, &HEAD, list) {
		TEST_ASSERT(testobjs[i].val == itern->val);
		TEST_ASSERT(&testobjs[i] == itern);
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
	size_t testobjs_len = sizeof(testobjs) / sizeof(*testobjs);

	SLIST_INSERT_HEAD(&HEAD, &testobjs[0], list);
	for (int i = 1; i < testobjs_len; i++) {
		SLIST_INSERT_AFTER(&testobjs[i - 1], &testobjs[i], list);
	}

	SLIST_REMOVE_AFTER(&testobjs[0], list);

	TEST_ASSERT(testobjs[0].val == SLIST_FIRST(&HEAD)->val);
	TEST_ASSERT(&testobjs[0] == SLIST_FIRST(&HEAD));

	TEST_ASSERT(testobjs[2].val == SLIST_NEXT(SLIST_FIRST(&HEAD), list)->val);
	TEST_ASSERT(&testobjs[2] == SLIST_NEXT(SLIST_FIRST(&HEAD), list));
}

static void single_element_removal(void)
{
	struct node testobjs[] = {
		(struct node){ .val = 10 },
		(struct node){ .val = 11 },
		(struct node){ .val = 12 },
	};
	size_t testobjs_len = sizeof(testobjs) / sizeof(*testobjs);

	SLIST_INSERT_HEAD(&HEAD, &testobjs[0], list);
	for (int i = 1; i < testobjs_len; i++) {
		SLIST_INSERT_AFTER(&testobjs[i - 1], &testobjs[i], list);
	}

	SLIST_REMOVE(&HEAD, &testobjs[2], list);
	struct node *itern = NULL;
	int i = 0;
	SLIST_FOREACH (itern, &HEAD, list) {
		TEST_ASSERT(i < 2);
		TEST_ASSERT(testobjs[i].val == itern->val);
		TEST_ASSERT(&testobjs[i] == itern);
		i++;
	}
}

static void survive_empty_list_traversal(void)
{
	struct node *itern = NULL;
	SLIST_FOREACH (itern, &HEAD, list) {
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
