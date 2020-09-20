#include <stdio.h>
#include <string.h>
#include <unity.h>

#define GUARDVAL 0xAAu

static char buffer[256];
static int pos;

int console_write(const char *msg, size_t length)
{
	for (int i = 0; i < length; i++) {
		buffer[pos++] = msg[i];
	}
	return (length);
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

#define VFPRINTF(expected, format, ...)                                        \
	do {                                                                   \
		int s = strlen(expected);                                      \
		int r = fprintf(console_write, (format), ##__VA_ARGS__);       \
		TEST_ASSERT_EQUAL_HEX8(GUARDVAL, buffer[s]);                   \
		buffer[s] = '\0';                                              \
		TEST_ASSERT_EQUAL_STRING((expected), buffer);                  \
		TEST_ASSERT_EQUAL_INT(s, r);                                   \
		TEST_ASSERT_EACH_EQUAL_CHAR_MESSAGE(GUARDVAL, &buffer[s + 1],  \
						    ARRAY_SIZE(buffer) - s,    \
						    "Corrupted guard values"); \
		reset();                                                       \
	} while (0)

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat"
#pragma clang diagnostic ignored "-Wint-conversion"
#pragma clang diagnostic ignored "-Wincompatible-pointer-types"

static void simple(void)
{
	VFPRINTF("16", "16");
	VFPRINTF("20% + 80% = 100%", "%d%% + %d%% = %d%%", 20, 80, 100);
}

static void conv_int(void)
{
	VFPRINTF("2 + 14 = 16", "2 + 14 = %d", 16);
	VFPRINTF("2 + 14 = -16", "2 + 14 = %d", -16);
	VFPRINTF("2 + 14 = 16", "2 + 14 = %i", 16);
	VFPRINTF("2 + 14 = -16", "2 + 14 = %i", -16);

	// Width
	VFPRINTF("  16", "%4d", 16);

	// Precision
	VFPRINTF("16", "%.0d", 16);
	VFPRINTF("", "%.0d", 0);
	VFPRINTF("0", "%.1d", 0);

	// Length
	VFPRINTF("-128", "%hhd", 0x7F + 1);
	VFPRINTF("-32768", "%hd", 0x7FFF + 1);

	// TODO: Test all length modifiers

	// Flags
	VFPRINTF("16 ", "%-3d", 16);
	VFPRINTF("+16", "%+d", 16);
	VFPRINTF("-16", "%+d", -16);
	VFPRINTF("16", "%d", 16);
	VFPRINTF("-16", "%d", -16);
	VFPRINTF(" 16", "% d", 16);
	VFPRINTF("-16", "% d", -16);
	VFPRINTF("+16", "% +d", 16);
	VFPRINTF("-16", "% +d", -16);
	VFPRINTF("0016", "%04d", 16);
	VFPRINTF("-016", "%04d", -16);
}

static void conv_uint(void)
{
	VFPRINTF("2 + 14 = 16", "2 + 14 = %u", 16);

	// Width
	VFPRINTF("  16", "%4u", 16);

	// Precision
	VFPRINTF("16", "%.0u", 16);
	VFPRINTF("", "%.0u", 0);
	VFPRINTF("0", "%.1u", 0);

	// Length
	VFPRINTF("0", "%hhu", 0xFF + 1);
	VFPRINTF("0", "%hu", 0xFFFF + 1);

	// TODO: Test all length modifiers

	// Flag
	VFPRINTF("16 ", "%-3u", 16);
	VFPRINTF("0016", "%04u", 16);
}

static void conv_str(void)
{
	VFPRINTF("Test string!", "Test %s!", "string");

	// Precision
	VFPRINTF("Test str!", "Test %.3s!", "string");
	VFPRINTF("Test string!", "Test %.10s!", "string");

	// TODO: Wide characters
}

static void conv_ptr(void)
{
	void *ptr = (uintptr_t)0xABC123;

	VFPRINTF("0xABC123", "%p", ptr);
	VFPRINTF("  0xABC123", "%10p", ptr);
	VFPRINTF("0xABC123  ", "%-10p", ptr);
}

static void conv_uhex(void)
{
	VFPRINTF("0x1 + 0xe = f", "0x1 + 0xe = %x", 0x1 + 0xe);
	VFPRINTF("0x1 + 0xE = F", "0x1 + 0xE = %X", 0x1 + 0xE);

	// Width
	VFPRINTF("  f", "%3x", 0xf);
	VFPRINTF("  F", "%3X", 0xf);

	// Precision
	VFPRINTF("f", "%.0x", 0xf);
	VFPRINTF("", "%.0x", 0);
	VFPRINTF("0", "%.1x", 0);

	// Length
	VFPRINTF("0", "%hhx", 0xFF + 1);
	VFPRINTF("0", "%hx", 0xFFFF + 1);

	// TODO: Test all length modifiers

	// Flags
	VFPRINTF("10 ", "%-3x", 16);
	VFPRINTF("0010", "%04x", 16);
	VFPRINTF("0xf", "%#x", 15);
	VFPRINTF("0xF", "%#X", 15);
	VFPRINTF("0x00f", "%#05x", 15);
}

static void conv_uoctal(void)
{
	VFPRINTF("7 + 3 = 12", "7 + 3 = %o", 07 + 03);

	// Width
	VFPRINTF("  12", "%4o", 012);

	// Precision
	VFPRINTF("12", "%.0o", 012);
	VFPRINTF("", "%.0o", 00);
	VFPRINTF("0", "%.1o", 00);

	// Length
	VFPRINTF("0", "%hho", 0400);
	VFPRINTF("0", "%ho", 0200000);

	// TODO: Test all length modifiers

	// Flags
	VFPRINTF("12 ", "%-3o", 012);
	VFPRINTF("0012", "%04o", 012);
}

static void conv_uchar(void)
{
	VFPRINTF("F", "%c", 'F');
	VFPRINTF("Some string", "Some str%cng", 'i');
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
