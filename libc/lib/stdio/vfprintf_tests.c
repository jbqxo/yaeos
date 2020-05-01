#include <stdio.h>
#include <unity.h>
#include <string.h>

#define GUARDVAL 0xAAu

// Kernel space testing
#ifdef __libk__
static char buffer[256];
static int pos;

void tty_putchar(tty_descriptor_t d, char c)
{
	buffer[pos++] = c;
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
{
}

#define VFPRINTF(expected, format, ...)                                        \
	do {                                                                   \
		int s = strlen(expected);                                      \
		int r = fprintf(0, (format), ##__VA_ARGS__);                   \
		TEST_ASSERT_EQUAL_HEX8(GUARDVAL, buffer[s]);                   \
		buffer[s] = '\0';                                              \
		TEST_ASSERT_EQUAL_STRING((expected), buffer);                  \
		TEST_ASSERT_EQUAL_INT(s, r);                                   \
		reset();                                                       \
	} while (0);

static void simple(void)
{
	VFPRINTF("16", "16");
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

	// Length: hh
	VFPRINTF("-128", "%hhd", 0x7F + 1);

	// Length: h
	VFPRINTF("-32768", "%hd", 0x7FFF + 1);

	// TODO: Test all length modifiers

	// Flag: -
	VFPRINTF("16 ", "%-3d", 16);

	// Flag: +
	VFPRINTF("+16", "%+d", 16);
	VFPRINTF("-16", "%+d", -16);
	VFPRINTF("16", "%d", 16);
	VFPRINTF("-16", "%d", -16);

	// Flag: _space_
	VFPRINTF(" 16", "% d", 16);
	VFPRINTF("-16", "% d", -16);
	VFPRINTF("+16", "% +d", 16);
	VFPRINTF("-16", "% +d", -16);

	// Flag: 0
	VFPRINTF("0016", "%04d", 16);
	VFPRINTF("-016", "%04d", -16);
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
}
static void conv_uoctal(void)
{
}
static void conv_udec(void)
{
}
static void conv_uchar(void)
{
}
static void conv_written_output(void)
{
}
static void conv_percentage(void)
{
}

int main(void)
{
	UNITY_BEGIN();

	RUN_TEST(simple);
	RUN_TEST(conv_int);
	RUN_TEST(conv_str);
	RUN_TEST(conv_ptr);

	return UNITY_END();
}
#endif //__libk__

// TODO: Implement unit tests for user-space libc
#ifdef __libc__
void setUp(void)
{
}

void tearDown(void)
{
}

int main(void)
{
	UNITY_BEGIN();
	return UNITY_END();
}
#endif // __libc__
