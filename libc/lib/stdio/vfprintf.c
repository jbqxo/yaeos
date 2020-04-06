#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>

#ifdef __libc__
static int print_char(FILE *stream, char c)
{
}
#define PRINT_CHAR(character, desc, stream) print_char((stream), (character))
#endif

#ifdef __libk__
static int print_char(tty_descriptor_t d, char c)
{
	tty_putchar(d, c);
	return 1;
}
#define PRINT_CHAR(character, desc, stream) print_char((desc), (character))
#endif

enum conv_flags {
	CF_MINUS = 0x1 << 0,
	CF_PLUS = 0x1 << 1,
	CF_SPACE = 0x1 << 2,
	CF_HASH = 0x1 << 3,
	CF_ZERO = 0x1 << 4
};
enum conv_specifiers {
	CS_INT,
	CS_UOCTAL,
	CS_UDEC,
	CS_UHEX,
	CS_UCHAR,
	CS_STR,
	CS_PTR,
	CS_WRITTEN_OUT,
	CS_PERCENTAGE,
	CS_INVALID
};
enum conv_length {
	CL_CHAR,
	CL_SHORT,
	CL_LONG,
	CL_LONG_LONG,
	CL_INTMAX,
	CL_SIZET,
	CL_PTRDIFF,
	CL_LONG_DOUBLE,
	CL_NONE
};
#define WIDTH_EMPTY -2
#define WIDTH_VAR -1
#define PREC_EMPTY -2
#define PREC_VAR -1
struct conv_spec {
	enum conv_flags flags;
	// Could be -1, which means that width will be passed as argument.
	int width;
	int precision;
	enum conv_length len;
	enum conv_specifiers spec;
};

#ifdef __i686__
static unsigned conv_positive_atoi(const char *s, unsigned len)
{
	// The number cannot be negative.
	// The segment must contain only digits.
	int res = 0;
	switch (len) {
	case 10: res += (s[len - 10] - '0') * 1000000000u;
	case 9: res += (s[len - 9] - '0') * 100000000u;
	case 8: res += (s[len - 8] - '0') * 10000000u;
	case 7: res += (s[len - 7] - '0') * 1000000u;
	case 6: res += (s[len - 6] - '0') * 100000u;
	case 5: res += (s[len - 5] - '0') * 10000u;
	case 4: res += (s[len - 4] - '0') * 1000u;
	case 3: res += (s[len - 3] - '0') * 100u;
	case 2: res += (s[len - 2] - '0') * 10u;
	case 1: res += (s[len - 1] - '0');
	}
	return res;
}
#endif // __i686__

static struct conv_spec parse_conv_spec(const char **_format)
{
	const char *format = *_format;
	struct conv_spec result = { 0 };
	// The first symbol must be '%'.
	format++;

	// Flags
	while (*format != '\0') {
		bool done = false;
		switch (*format) {
		case '-': {
			result.flags |= CF_MINUS;
			format++;
		} break;
		case '+': {
			result.flags |= CF_PLUS;
			format++;
		} break;
		case ' ': {
			result.flags |= CF_SPACE;
			format++;
		} break;
		case '#': {
			result.flags |= CF_HASH;
			format++;
		} break;
		case '0': {
			result.flags |= CF_ZERO;
			format++;
		} break;
		default: done = true;
		}
		if (done) {
			break;
		}
	}

	// Field width
	{
		if (*format == '*') {
			result.width = WIDTH_VAR;
			format++;
			goto skip_width;
		}
		const char *begin = format;
		unsigned len = 0;
		while (begin[len] >= '0' && begin[len] <= '9')
			len++;
		if (len == 0) {
			// Width was not specified.
			result.width = WIDTH_EMPTY;
			goto skip_width;
		}

		result.width = conv_positive_atoi(begin, len);
		format += len;

	skip_width:;
	}

	// Precision
	{
		if (*format != '.') {
			result.precision = PREC_EMPTY;
			goto skip_precision;
		} else {
			format++;
		}

		if (*format == '*') {
			result.precision = PREC_VAR;
			format++;
			goto skip_precision;
		}

		const char *begin = format;
		unsigned len = 0;
		while (begin[len] >= '0' && begin[len] <= '9')
			len++;
		if (len == 0) {
			// "If only the period specified, the precision is taken as zero."
			result.precision = 0;
			goto skip_precision;
		}
		result.precision = conv_positive_atoi(begin, len);
		format += len;

	skip_precision:;
	}

	// Length modifier
	{
		switch (*format) {
		case 'h': {
			if (format[1] == 'h') {
				result.len = CL_CHAR;
				format++;
			} else {
				result.len = CL_SHORT;
			}
		} break;
		case 'l': {
			if (format[1] == 'l') {
				result.len = CL_LONG_LONG;
				format++;
			} else {
				result.len = CL_LONG;
			}
		} break;
		case 'j': result.len = CL_INTMAX; break;
		case 'z': result.len = CL_SIZET; break;
		case 't': result.len = CL_PTRDIFF; break;
		case 'L': result.len = CL_LONG_DOUBLE; break;
		default: result.len = CL_NONE;
		}

		if (result.len != CL_NONE) {
			format++;
		}
	}

	// Conversion specifier
	{
		switch (*format) {
		case 'd':
		case 'i': result.spec = CS_INT; break;

		case 'o': result.spec = CS_UOCTAL; break;
		case 'u': result.spec = CS_UDEC; break;
		case 'x':
		case 'X': result.spec = CS_UHEX; break;

		case 'c': result.spec = CS_UCHAR; break;

		case 's': result.spec = CS_STR; break;

		case 'p': result.spec = CS_PTR; break;

		case 'n': result.spec = CS_WRITTEN_OUT; break;

		case '%': result.spec = CS_PERCENTAGE; break;
		default: result.spec = CS_INVALID;
		}
		format++;
	}

	*_format = format;
	return result;
}

static bool is_spec_correct(struct conv_spec s)
{
	return true;
}

#ifdef __libc__
static int print_conv_spec(FILE *restrict stream, struct conv_spec s,
			   va_list *args)
#elif __libk__
static int print_conv_spec(tty_descriptor_t d, struct conv_spec s,
			   va_list *args)
#endif
{
	int printed = 0;
	switch (s.spec) {
	case CS_INT: {
		char buffer[21];
		size_t buff_sz = sizeof(buffer) / sizeof(*buffer);
		buffer[20] = '\0';
		int bi = sizeof(buffer) / sizeof(*buffer) - 1;
		int value = va_arg(*args, int);
		bool negative = value < 0;
		if (negative) {
			value *= -1;
		}

		do {
			bi--;
			buffer[bi] = value % 10 + '0';
		} while ((value /= 10) != 0 && bi > 0);
		if (negative) {
			// There is always some space for a minus sign in negative numbers. (I hope)
			bi--;
			buffer[bi] = '-';
		}

		while (buffer[bi] != '\0') {
			PRINT_CHAR(buffer[bi], d, stream);
			bi++;
			printed++;
		}
	} break;
	case CS_UCHAR: {
		int v = va_arg(*args, int);
		PRINT_CHAR((unsigned char)v, d, stream);
		printed++;
	} break;
	case CS_STR: {
		int max_len = s.precision;
		if (max_len == PREC_VAR) {
			max_len = va_arg(*args, int);
		}
		const char *str = va_arg(*args, char *);
		if (max_len == PREC_EMPTY) {
			while (*str != '\0') {
				PRINT_CHAR(*str, d, stream);
				printed++;
				str++;
			}
		} else {
			int i;
			for (i = 0; i < max_len && str[i] != '\0'; i++) {
				PRINT_CHAR(str[i], d, stream);
			}
			printed += i;
		}
	} break;
	}
	return printed;
}

#ifdef __libc__
int vfprintf(FILE *restrict stream, const char *restrict format, va_list args)
#elif __libk__
int vfprintf(tty_descriptor_t d, const char *restrict format, va_list args)
#endif
{
	// TODO: I don't understand why I can't just take an address of args from function arguments.
	// In both cases the structure should be on the stack, as far as I understand.
	va_list ap;
	va_copy(ap, args);

	int printed = 0;
	int i = 0;
	while (format[i] != '\0') {
		if (format[i] == '%') {
			const char *new_pos = &format[i];
			struct conv_spec s = parse_conv_spec(&new_pos);
			if (is_spec_correct(s)) {
#ifdef __libc__
				printed += print_conv_spec(stream, s, &ap);
#elif __libk__
				printed += print_conv_spec(d, s, &ap);
#endif
			}
			i = new_pos - format;
		} else {
			int res = PRINT_CHAR(format[i], d, stream);
			if (res < 0) {
				printed = res;
				goto failed;
			}
			printed += res;
			i++;
		}
	}

failed:
	return printed;
}

#ifdef __libc__
int fprintf(FILE *restrict stream, const char *restrict format, ...)
#elif __libk__
int fprintf(tty_descriptor_t d, const char *restrict format, ...)
#endif
{
	va_list args;
	va_start(args, format);

#ifdef __libc__
	int i = vfprintf(stream, format, args);
#elif __libk__
	int i = vfprintf(d, format, args);
#endif

	va_end(args);
	return i;
}
