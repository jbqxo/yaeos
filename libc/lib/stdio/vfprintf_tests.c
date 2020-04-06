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

void setUp(void)
{
	memset(buffer, GUARDVAL, sizeof(buffer));
	pos = 0;
}

void tearDown(void)
{
}

#define VFPRINTF(expected, format, ...)                                        \
	do {                                                                   \
		int s = strlen(expected);                                      \
		TEST_ASSERT_EQUAL_INT(s, fprintf(0, (format), ##__VA_ARGS__));  \
		TEST_ASSERT_EQUAL_HEX8(GUARDVAL, buffer[s]);                   \
		buffer[s] = '\0';                                              \
		TEST_ASSERT_EQUAL_STRING((expected), buffer);                    \
		setUp();                                                       \
	} while (0);

static void vfprintf_simple(void)
{
	VFPRINTF("16", "16");
	VFPRINTF("2 + 14 = 16", "2 + 14 = %d", 16);
	VFPRINTF("2 + 14 = -16", "2 + 14 = %d", -16);
	VFPRINTF("Hello world!", "Hello %s!", "world");
}

// TODO: It would be cool to cover whole set of behaviour.
static void vfprintf_flag_minus(void)
{
	VFPRINTF("16 ", "%-3d", 16);
	VFPRINTF(" 16", "%3d", 16);
}

static void vfprintf_flag_plus(void)
{
	VFPRINTF("+16", "%+d", 16);
	VFPRINTF("-16", "%+d", -16);
	VFPRINTF("16", "%d", 16);
	VFPRINTF("-16", "%d", -16);
}

static void vfprintf_flag_space(void)
{
	VFPRINTF(" 16", "% d", 16);
	VFPRINTF("+16", "% +d", 16);
	VFPRINTF("-16", "% +d", -16);
}

static void vfprintf_flag_hash(void)
{
	VFPRINTF("0x10", "%#x", 16);
	VFPRINTF(" 0x10", "%#x", 16);
}

static void vfprintf_flag_zero(void)
{
	VFPRINTF("0016", "%04d", 16);
	VFPRINTF("-016", "%04d", -16);
}

static void vfprintf_conv_spec_int(void)
{
	VFPRINTF("16", "%d", 16);
	VFPRINTF("16", "%.0d", 16);
	VFPRINTF("", "%.0d", 0);
	VFPRINTF("0", "%.1d", 0);
}

int main(void)
{
	UNITY_BEGIN();

	RUN_TEST(vfprintf_simple);

	RUN_TEST(vfprintf_flag_minus);
	RUN_TEST(vfprintf_flag_plus);
	RUN_TEST(vfprintf_flag_space);
	RUN_TEST(vfprintf_flag_hash);
	RUN_TEST(vfprintf_flag_zero);

	RUN_TEST(vfprintf_conv_spec_int);

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
