#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>

#ifdef __libc__
typedef FILE *output_t;

static int print_char(FILE *stream, char c)
{
}
#endif

#ifdef __libk__
typedef tty_descriptor_t output_t;

static int print_char(tty_descriptor_t d, char c)
{
	tty_putchar(d, c);
	return 1;
}
#endif

static void putn(output_t out, const char *str, unsigned n) {
	for (int i = 0; i < n; i++) {
		print_char(out, str[i]);
	}
}

enum conv_flags {
	CF_MINUS = 0x1 << 0,
	CF_PLUS  = 0x1 << 1,
	CF_SPACE = 0x1 << 2,
	CF_HASH  = 0x1 << 3,
	CF_ZERO  = 0x1 << 4
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
	CL_LLONG,
	CL_INTMAX,
	CL_SIZET,
	CL_PTRDIFF,
	CL_LDOUBLE,
	CL_NONE
};
#define WIDTH_EMPTY -2
#define WIDTH_VAR   -1
#define PREC_EMPTY  -2
#define PREC_VAR    -1
struct conv_spec {
	enum conv_flags flags;
	int width;
	int precision;
	enum conv_length length;
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
				result.length = CL_CHAR;
				format++;
			} else {
				result.length = CL_SHORT;
			}
		} break;
		case 'l': {
			if (format[1] == 'l') {
				result.length = CL_LLONG;
				format++;
			} else {
				result.length = CL_LONG;
			}
		} break;
		case 'j': result.length = CL_INTMAX; break;
		case 'z': result.length = CL_SIZET; break;
		case 't': result.length = CL_PTRDIFF; break;
		case 'L': result.length = CL_LDOUBLE; break;
		default: result.length = CL_NONE;
		}

		if (result.length != CL_NONE) {
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

struct argument {
	union {
		uintmax_t i;
		char *str;
	} val;
	bool negative;
};

static struct argument fetch_arg(struct conv_spec s, va_list *args,
				 struct argument *dest)
{
	struct argument arg = { 0 };
	switch (s.spec) {
	case CS_INT: {
		int val = va_arg(*args, int);
		switch (s.length) {
#define CASE_BODY(type)                                                        \
	do {                                                                   \
		type v = (type)val;                                            \
		arg.val.i = v;                                                 \
		if (v < 0) {                                                   \
			arg.negative = true;                                   \
			arg.val.i *= -1;                                       \
		}                                                              \
	} while (0)
		case CL_CHAR: CASE_BODY(char); break;
		case CL_SHORT: CASE_BODY(short); break;
		case CL_LONG: CASE_BODY(long); break;
		case CL_LLONG: CASE_BODY(long long); break;
		case CL_INTMAX: CASE_BODY(intmax_t); break;
		case CL_SIZET: CASE_BODY(size_t); break;
		case CL_NONE: CASE_BODY(int); break;
		case CL_PTRDIFF: CASE_BODY(ptrdiff_t); break;
#undef CASE_BODY
		}
	} break;
	case CS_PTR: arg.val.i = va_arg(*args, int); break;
	case CS_STR: arg.val.str = va_arg(*args, char*); break;
	case CS_UCHAR: arg.val.i = va_arg(*args, int); break;
	}
	return arg;
}

struct conv_spec_funcs {
	// Print the format specifier.
	void (*print)(output_t out, struct conv_spec *s, struct argument *a);
	// Calculate the length of the result of print function.
	int (*length)(struct conv_spec *s, struct argument *a);
	// Null-terminated string to prepend to the value ("0x" for ptr/hex values, for example). Could be NULL.
	const char *prefix;
};

static int length_for_intnumbase(uintmax_t num, unsigned base, unsigned max_num_digits) {
	for (int i = max_num_digits - 1; i >= 0; i--) {
		uintmax_t power = 1;
		for (int j = 0; j < i; j++) {
			power *= base;
		}
		if (num >= power) {
			return i + 1;
		}
	}
	return 1;
}

static int int_length(struct conv_spec *s, struct argument *a)
{
	// Special case:
	// The result of converting a zero value with a precision of zero is no characters.
	if (s->precision == 0 && a->val.i == 0) {
		return 0;
	}

	int length = length_for_intnumbase(a->val.i, 10, 20);

	if (s->precision != PREC_EMPTY) {
		if (s->precision >= length) {
			return s->precision;
		}
	}

	return length;
}

static void int_print(output_t out, struct conv_spec *s, struct argument *a)
{
	char buffer[20];
	int buffer_size = sizeof(buffer) / sizeof(*buffer);
	int buffer_i = buffer_size;

	// Special case:
	// The result of converting a zero value with a precision of zero is no characters.
	if (s->precision == 0 && a->val.i == 0) {
		return;
	}

	do {
		buffer_i--;
		buffer[buffer_i] = a->val.i % 10 + '0';
	} while ((a->val.i /= 10) != 0 && buffer_i > 0);

	if (s->precision != PREC_EMPTY) {
		int already_printed = buffer_size - buffer_i;
		s->precision -= already_printed;
		for (int i = 0; i < s->precision; i++) {
			print_char(out, '0');
		}
	}

	while (buffer_i < buffer_size) {
		print_char(out, buffer[buffer_i]);
		buffer_i++;
	}
}

static int ptr_length(struct conv_spec *s, struct argument *a) {
	return length_for_intnumbase(a->val.i, 16, 16);
}

static void ptr_print(output_t out, struct conv_spec *s, struct argument *a) {
	char buffer[16];
	int buffer_size = sizeof(buffer) / sizeof(*buffer);
	int buffer_i = buffer_size;

	do {
		buffer_i--;
		unsigned char n = a->val.i % 16;
		if (n < 10) {
			buffer[buffer_i] = n + '0';
		} else {
			buffer[buffer_i] = (n - 10) + 'A';
		}
	} while((a->val.i /= 16) != 0 && buffer_i > 0);

	while (buffer_i < buffer_size) {
		print_char(out, buffer[buffer_i]);
		buffer_i++;
	}
}

static int str_length(struct conv_spec *s, struct argument *a) {
	int l = strlen(a->val.str);
	if (s->precision != PREC_EMPTY && s->precision < l) {
		return s->precision;
	}
	return l;
}

static void str_print(output_t out, struct conv_spec *s, struct argument *a) {
	const char *str = a->val.str;
	// TODO: Find a way to reuse the result.
	int len = str_length(s, a);

	for (int i = 0; i < len; i++) {
		print_char(out, str[i]);
	}
}

static int char_length(struct conv_spec *s, struct argument *a) {
	return 1;
}

static void char_print(output_t out, struct conv_spec *s, struct argument *a) {
	print_char(out, a->val.i);
}

static struct conv_spec_funcs cs_funcs_table[] = {
	[CS_INT] = (struct conv_spec_funcs){ .print = int_print,
					     .length = int_length,
					     .prefix = NULL },
	[CS_PTR] = (struct conv_spec_funcs){ .print = ptr_print,
					     .length = ptr_length,
					     .prefix = "0x" },
	[CS_STR] = (struct conv_spec_funcs){ .print = str_print,
					     .length = str_length,
					     .prefix = NULL },
	[CS_UCHAR] = (struct conv_spec_funcs){ .print = char_print,
					       .length = char_length,
					       .prefix = NULL }
};

/**
 * @brief Print visible characters, specified by flags.
 * 
 * @return int Number of written characters.
 */
static int put_flags(output_t out, struct conv_spec s,
			     struct argument arg)
{
	int printed = 0;
	if (arg.negative) {
		print_char(out, '-');
		printed++;
	} else if (s.flags & CF_PLUS) {
		print_char(out, '+');
		printed++;
	} else if (s.flags & CF_SPACE) {
		print_char(out, ' ');
		printed++;
	}
	return printed;
}

/**
 * @brief Print given conversion specifier field.
 *
 * @return int Number of written characters.
 */
static int print_conv_spec(output_t out, struct conv_spec s, va_list *args)
{
	struct argument arg = fetch_arg(s, args, &arg);
	int num_len = cs_funcs_table[s.spec].length(&s, &arg);
	int printed = 0;

	const char *prefix = cs_funcs_table[s.spec].prefix;
	unsigned prefix_len = 0;
	if (prefix) {
		prefix_len = strlen(prefix);
	}

	bool flag_present = arg.negative || (s.flags & (CF_PLUS | CF_SPACE));
	int width_to_fill = s.width - num_len - prefix_len - (flag_present ? 1 : 0);

	// If Width != 0 and the flag '-' was not specified, result must be right-justified
	if (!(s.flags & CF_MINUS) && s.width != WIDTH_EMPTY) {
		if (s.flags & CF_ZERO) {
			putn(out, prefix, prefix_len);
			printed += prefix_len;
			printed += put_flags(out, s, arg);
		}
		while (width_to_fill > 0) {
			// remained_width could be < 0, so decrement in the loop
			char c = s.flags & CF_ZERO ? '0' : ' ';
			print_char(out, c);
			printed++;
			width_to_fill--;
		}
	}
	if (!(s.flags & CF_ZERO)) {
		putn(out, prefix, prefix_len);
		printed += prefix_len;
		printed += put_flags(out, s, arg);
	}
	cs_funcs_table[s.spec].print(out, &s, &arg);
	printed += num_len;

	// If Width != 0 and the flag '-' was specified, result must be left-justified
	if (s.flags & CF_MINUS && s.width != WIDTH_EMPTY) {
		while (width_to_fill > 0) {
			// remained_width could be < 0, so decrement in the loop
			print_char(out, ' ');
			printed++;
			width_to_fill--;
		}
	}
	return printed;
}

#ifdef __libc__
int vfprintf(FILE *restrict stream, const char *restrict format, va_list args)
#elif __libk__
int vfprintf(tty_descriptor_t d, const char *restrict format, va_list args)
#endif
{
	va_list ap;
	va_copy(ap, args);

	int printed = 0;
	int i = 0;
	while (format[i] != '\0') {
		if (format[i] == '%') {
			const char *new_pos = &format[i];
			struct conv_spec s = parse_conv_spec(&new_pos);
#ifdef __libc__
			printed += print_conv_spec(stream, s, &ap);
#elif __libk__
			printed += print_conv_spec(d, s, &ap);
#endif
			i = new_pos - format;
		} else {
			int res;
#ifdef __libc__
			res = print_char(stream, format[i]);
#elif __libk__
			res = print_char(d, format[i]);
#endif
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
