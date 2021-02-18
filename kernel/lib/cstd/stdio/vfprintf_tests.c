// UNITY_TEST DEPENDS ON: kernel/lib/stdio/vfprintf.c
// UNITY_TEST DEPENDS ON: kernel/lib/string/strlen.c
// UNITY_TEST DEPENDS ON: kernel/lib/string/strchr.c

#include "lib/cppdefs.h"
#include "lib/cstd/stdio.h"

#include <stdio.h>
#include <string.h>
#include <unity.h>

#define GUARDVAL 0xAAu

static char buffer[256];
static int pos;

void console_write(const char *msg, size_t length)
{
        for (int i = 0; i < length; i++) {
                buffer[pos++] = msg[i];
        }
}

static void reset(void)
{
        memset(buffer, GUARDVAL, sizeof(buffer));
        pos = 0;
}

void setUp(void)
{
        reset();
}

void tearDown(void)
{}

#define KVFPRINTF(expected, format, ...)                                        \
        do {                                                                    \
                int s = strlen(expected);                                       \
                int r = kfprintf(console_write, (format), ##__VA_ARGS__);       \
                TEST_ASSERT_EQUAL_HEX8(GUARDVAL, buffer[s]);                    \
                buffer[s] = '\0';                                               \
                TEST_ASSERT_EQUAL_STRING((expected), buffer);                   \
                TEST_ASSERT_EQUAL_INT(s, r);                                    \
                TEST_ASSERT_EACH_EQUAL_CHAR_MESSAGE(GUARDVAL, &buffer[s + 1],   \
                                                    ARRAY_SIZE(buffer) - s - 1, \
                                                    "Corrupted guard values");  \
                reset();                                                        \
        } while (0)

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat"
#pragma clang diagnostic ignored "-Wint-conversion"
#pragma clang diagnostic ignored "-Wincompatible-pointer-types"

static void simple(void)
{
        KVFPRINTF("16", "16");
        KVFPRINTF("20% + 80% = 100%", "%d%% + %d%% = %d%%", 20, 80, 100);
}

static void conv_int(void)
{
        KVFPRINTF("2 + 14 = 16", "2 + 14 = %d", 16);
        KVFPRINTF("2 + 14 = -16", "2 + 14 = %d", -16);
        KVFPRINTF("2 + 14 = 16", "2 + 14 = %i", 16);
        KVFPRINTF("2 + 14 = -16", "2 + 14 = %i", -16);

        // Width
        KVFPRINTF("  16", "%4d", 16);

        // Precision
        KVFPRINTF("16", "%.0d", 16);
        KVFPRINTF("", "%.0d", 0);
        KVFPRINTF("0", "%.1d", 0);

        // Length
        KVFPRINTF("-128", "%hhd", 0x7F + 1);
        KVFPRINTF("-32768", "%hd", 0x7FFF + 1);

        // TODO: Test all length modifiers

        // Flags
        KVFPRINTF("16 ", "%-3d", 16);
        KVFPRINTF("+16", "%+d", 16);
        KVFPRINTF("-16", "%+d", -16);
        KVFPRINTF("16", "%d", 16);
        KVFPRINTF("-16", "%d", -16);
        KVFPRINTF(" 16", "% d", 16);
        KVFPRINTF("-16", "% d", -16);
        KVFPRINTF("+16", "% +d", 16);
        KVFPRINTF("-16", "% +d", -16);
        KVFPRINTF("0016", "%04d", 16);
        KVFPRINTF("-016", "%04d", -16);
}

static void conv_uint(void)
{
        KVFPRINTF("2 + 14 = 16", "2 + 14 = %u", 16);

        // Width
        KVFPRINTF("  16", "%4u", 16);

        // Precision
        KVFPRINTF("16", "%.0u", 16);
        KVFPRINTF("", "%.0u", 0);
        KVFPRINTF("0", "%.1u", 0);

        // Length
        KVFPRINTF("0", "%hhu", 0xFF + 1);
        KVFPRINTF("0", "%hu", 0xFFFF + 1);

        // TODO: Test all length modifiers

        // Flag
        KVFPRINTF("16 ", "%-3u", 16);
        KVFPRINTF("0016", "%04u", 16);
}

static void conv_str(void)
{
        KVFPRINTF("Test string!", "Test %s!", "string");

        // Precision
        KVFPRINTF("Test str!", "Test %.3s!", "string");
        KVFPRINTF("Test string!", "Test %.10s!", "string");

        // TODO: Wide characters
}

static void conv_ptr(void)
{
        void *ptr = (uintptr_t)0xABC123;

        KVFPRINTF("0xABC123", "%p", ptr);
        KVFPRINTF("  0xABC123", "%10p", ptr);
        KVFPRINTF("0xABC123  ", "%-10p", ptr);
}

static void conv_uhex(void)
{
        KVFPRINTF("0x1 + 0xe = f", "0x1 + 0xe = %x", 0x1 + 0xe);
        KVFPRINTF("0x1 + 0xE = F", "0x1 + 0xE = %X", 0x1 + 0xE);

        // Width
        KVFPRINTF("  f", "%3x", 0xf);
        KVFPRINTF("  F", "%3X", 0xf);

        // Precision
        KVFPRINTF("f", "%.0x", 0xf);
        KVFPRINTF("", "%.0x", 0);
        KVFPRINTF("0", "%.1x", 0);

        // Length
        KVFPRINTF("0", "%hhx", 0xFF + 1);
        KVFPRINTF("0", "%hx", 0xFFFF + 1);

        // TODO: Test all length modifiers

        // Flags
        KVFPRINTF("10 ", "%-3x", 16);
        KVFPRINTF("0010", "%04x", 16);
        KVFPRINTF("0xf", "%#x", 15);
        KVFPRINTF("0xF", "%#X", 15);
        KVFPRINTF("0x00f", "%#05x", 15);
}

static void conv_uoctal(void)
{
        KVFPRINTF("7 + 3 = 12", "7 + 3 = %o", 07 + 03);

        // Width
        KVFPRINTF("  12", "%4o", 012);

        // Precision
        KVFPRINTF("12", "%.0o", 012);
        KVFPRINTF("", "%.0o", 00);
        KVFPRINTF("0", "%.1o", 00);

        // Length
        KVFPRINTF("0", "%hho", 0400);
        KVFPRINTF("0", "%ho", 0200000);

        // TODO: Test all length modifiers

        // Flags
        KVFPRINTF("12 ", "%-3o", 012);
        KVFPRINTF("0012", "%04o", 012);
}

static void conv_uchar(void)
{
        KVFPRINTF("F", "%c", 'F');
        KVFPRINTF("Some string", "Some str%cng", 'i');
}

int main(void)
{
        UNITY_BEGIN();

        RUN_TEST(simple);
        RUN_TEST(conv_int);
        RUN_TEST(conv_uint);
        RUN_TEST(conv_str);
        RUN_TEST(conv_ptr);
        RUN_TEST(conv_uchar);
        RUN_TEST(conv_uhex);
        RUN_TEST(conv_uoctal);

        UNITY_END();
        return (0);
}
#pragma clang diagnostic pop
