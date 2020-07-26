#include <stdbool.h>
#include <unity.h>
#include <stddef.h>

#include <lib/string.h>

#define GUARDVAL 0xAAu

void setUp(void)
{
}

void tearDown(void)
{
}

static void memcpy_simple(void)
{
	static const unsigned char src[] = "Test string";
	unsigned char dest[sizeof(src) + 32];
	for (size_t i = 0; i < 32; i++) {
		dest[sizeof(src) + i] = GUARDVAL;
	}

	void *res = memcpy(dest, src, sizeof(src));
	TEST_ASSERT_EQUAL_PTR(dest, res);
	TEST_ASSERT_EQUAL_STRING(src, dest);
	TEST_ASSERT_EACH_EQUAL_CHAR_MESSAGE(GUARDVAL, &dest[sizeof(src)], 32,
					    "Guard value is ruined");
}

static void memcpy_empty_str(void)
{
	static const unsigned char src[] = "";
	unsigned char dest[sizeof(src) + 32];
	for (size_t i = 0; i < 32; i++) {
		dest[sizeof(src) + i] = GUARDVAL;
	}

	void *res = memcpy(dest, src, sizeof(src));
	TEST_ASSERT_EQUAL_PTR(dest, res);
	TEST_ASSERT_EQUAL_STRING(src, dest);
	TEST_ASSERT_EACH_EQUAL_CHAR_MESSAGE(GUARDVAL, &dest[sizeof(src)], 32,
					    "Guard value is ruined");
}

int main(void)
{
	UNITY_BEGIN();
	RUN_TEST(memcpy_simple);
	RUN_TEST(memcpy_empty_str);
	return UNITY_END();
}
