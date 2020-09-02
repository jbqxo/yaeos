#include "kernel/ds/cbuffer.h"
#include "kernel/cppdefs.h"

#include <stdbool.h>
#include <unity.h>

void setUp(void)
{}

void tearDown(void)
{}

static void can_store_single_element(void)
{
	int testval = 0xFF;
	CBUFFER_DECLARE(typeof(testval), 1) buffer;
	CBUFFER_INIT(&buffer);
	CBUFFER_PUSH(&buffer, testval);

	int result;
	CBUFFER_POP(&buffer, result, 0);

	TEST_ASSERT_EQUAL_INT(testval, result);
}

static void can_store_multiple_elements(void)
{
	static int testarr[] = { 255, 62, 92, 4112, 1245, 2315, 54336, -1, -2000 };
	static const size_t testarr_sz = ARRAY_SIZE(testarr);

	CBUFFER_DECLARE(typeof(testarr[0]), testarr_sz) buffer;
	CBUFFER_INIT(&buffer);
	for (int i = 0; i < testarr_sz; i++) {
		bool success = CBUFFER_PUSH(&buffer, testarr[i]);
		TEST_ASSERT(success);
	}

	for (int i = 0; i < testarr_sz; i++) {
		int retval;
		CBUFFER_POP(&buffer, retval, 0);
		TEST_ASSERT_EQUAL_INT(testarr[i], retval);
	}
}

static void can_handle_multiple_read_writes(void)
{
	static int testarr[] = { 255, 62, 92, 4112, 1245, 2315, 54336, -1, -2000 };
	static const size_t testarr_sz = ARRAY_SIZE(testarr);

	CBUFFER_DECLARE(typeof(testarr[0]), testarr_sz) buffer;
	CBUFFER_INIT(&buffer);
	for (int i = 0; i < testarr_sz; i++) {
		bool success = CBUFFER_PUSH(&buffer, testarr[i]);
		TEST_ASSERT(success);
	}

	for (int i = 0; i < 2; i++) {
		int retval;
		CBUFFER_POP(&buffer, retval, 0);
		TEST_ASSERT_EQUAL_INT(testarr[i], retval);
	}

	static int newelems[] = { 788, 799 };
	for (int i = 0; i < 2; i++) {
		bool success = CBUFFER_PUSH(&buffer, newelems[i]);
		TEST_ASSERT(success);
	}

	static int newexpect[] = { 92, 4112, 1245, 2315, 54336, -1, -2000, 788, 799 };
	for (int i = 0; i < testarr_sz; i++) {
		int retval;
		CBUFFER_POP(&buffer, retval, 0);
		TEST_ASSERT_EQUAL_INT(newexpect[i], retval);
	}
}

static void cant_overflow(void)
{
	static int testarr[] = { 255, 62, 92, 4112, 1245, 2315, 54336, -1, -2000 };
	static const size_t testarr_sz = ARRAY_SIZE(testarr);

	CBUFFER_DECLARE(typeof(testarr[0]), testarr_sz - 2) buffer;
	CBUFFER_INIT(&buffer);
	for (int i = 0; i < testarr_sz - 2; i++) {
		bool success = CBUFFER_PUSH(&buffer, testarr[i]);
		TEST_ASSERT(success);
	}

	bool success = CBUFFER_PUSH(&buffer, testarr[testarr_sz - 2]);
	TEST_ASSERT(!success);
	success = CBUFFER_PUSH(&buffer, testarr[testarr_sz - 1]);
	TEST_ASSERT(!success);

	for (int i = 0; i < testarr_sz - 2; i++) {
		int retval;
		CBUFFER_POP(&buffer, retval, 0);
		TEST_ASSERT_EQUAL_INT(testarr[i], retval);
	}

	int retval;
	CBUFFER_POP(&buffer, retval, 0);
	TEST_ASSERT_EQUAL_INT(0, retval);

	CBUFFER_POP(&buffer, retval, 0);
	TEST_ASSERT_EQUAL_INT(0, retval);
}

static void return_default_on_fail(void)
{
	CBUFFER_DECLARE(int, 1) buffer;
	CBUFFER_INIT(&buffer);
	bool success = CBUFFER_PUSH(&buffer, 10);
	TEST_ASSERT(success);

	int retval;
	CBUFFER_POP(&buffer, retval, 0);
	TEST_ASSERT_EQUAL_INT(10, retval);

	CBUFFER_POP(&buffer, retval, 0);
	TEST_ASSERT_EQUAL_INT(0, retval);
}

int main(void)
{
	UNITY_BEGIN();
	RUN_TEST(can_store_single_element);
	RUN_TEST(can_store_multiple_elements);
	RUN_TEST(can_handle_multiple_read_writes);
	RUN_TEST(cant_overflow);
	RUN_TEST(return_default_on_fail);
	UNITY_END();
	return (0);
}
