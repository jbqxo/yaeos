// UNITY_TEST DEPENDS ON: kernel/lib/string/memcpy.c

#include "lib/string.h"

#include <stdbool.h>
#include <stddef.h>
#include <unity.h>

#define GUARDVAL 0xAAu

void setUp(void)
{}

void tearDown(void)
{}

static void kmemcpy_simple(void)
{
        static const unsigned char src[] = "Test string";
        unsigned char dest[sizeof(src) + 32];
        for (size_t i = 0; i < 32; i++) {
                dest[sizeof(src) + i] = GUARDVAL;
        }

        void *res = kmemcpy(dest, src, sizeof(src));
        TEST_ASSERT_EQUAL_PTR(dest, res);
        TEST_ASSERT_EQUAL_STRING(src, dest);
        TEST_ASSERT_EACH_EQUAL_CHAR_MESSAGE(GUARDVAL, &dest[sizeof(src)], 32,
                                            "Guard value is ruined");
}

static void kmemcpy_empty_str(void)
{
        static const unsigned char src[] = "";
        unsigned char dest[sizeof(src) + 32];
        for (size_t i = 0; i < 32; i++) {
                dest[sizeof(src) + i] = GUARDVAL;
        }

        void *res = kmemcpy(dest, src, sizeof(src));
        TEST_ASSERT_EQUAL_PTR(dest, res);
        TEST_ASSERT_EQUAL_STRING(src, dest);
        TEST_ASSERT_EACH_EQUAL_CHAR_MESSAGE(GUARDVAL, &dest[sizeof(src)], 32,
                                            "Guard value is ruined");
}

int main(void)
{
        UNITY_BEGIN();
        RUN_TEST(kmemcpy_simple);
        RUN_TEST(kmemcpy_empty_str);
        UNITY_END();
        return (0);
}
