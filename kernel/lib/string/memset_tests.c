// UNITY_TEST DEPENDS ON: kernel/lib/string/memset.c

#include "lib/string.h"

#include <stdbool.h>
#include <stddef.h>
#include <unity.h>

#define GUARDVAL 0xAAu

void setUp(void)
{}

void tearDown(void)
{}

static void memset_simple(void)
{
	const size_t length = 64;
	const unsigned char val = 'A';

	unsigned char dest[length + 32];
	for (size_t i = 0; i < 32; i++) {
		dest[length + i] = GUARDVAL;
	}

	void *res = memset(dest, val, length);
	TEST_ASSERT_EQUAL_PTR(dest, res);
	TEST_ASSERT_EACH_EQUAL_CHAR(val, dest, length);
	TEST_ASSERT_EACH_EQUAL_CHAR_MESSAGE(GUARDVAL, &dest[length], 32, "Guard value is ruined");
}

static void memset_empty_str(void)
{
	unsigned char dest[32];
	for (size_t i = 0; i < sizeof(dest); i++) {
		dest[i] = GUARDVAL;
	}

	void *res = memset(dest, 'A', 0);
	TEST_ASSERT_EQUAL_PTR(dest, res);
	TEST_ASSERT_EACH_EQUAL_CHAR_MESSAGE(GUARDVAL, dest, 32, "Guard value is ruined");
}

int main(void)
{
	UNITY_BEGIN();
	RUN_TEST(memset_simple);
	RUN_TEST(memset_empty_str);
	UNITY_END();
	return (0);
}
