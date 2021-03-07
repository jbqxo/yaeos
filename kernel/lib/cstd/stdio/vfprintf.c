// UNITY_TEST DEPENDS ON: kernel/lib/string/strchr.c
// UNITY_TEST DEPENDS ON: kernel/lib/string/strlen.c

#include "lib/cppdefs.h"
#include "lib/cstd/stdio.h"
#include "lib/cstd/string.h"
#include "lib/utils.h"

#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define CF_MINUS (0x1 << 0)
#define CF_PLUS  (0x1 << 1)
#define CF_SPACE (0x1 << 2)
#define CF_HASH  (0x1 << 3)
#define CF_ZERO  (0x1 << 4)

enum conv_specifiers {
        CS_INT,
        CS_UOCTAL,
        CS_UDEC,
        CS_UHEX,
        CS_UHEX_BIG,
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

struct conv_spec {
        int flags;
        union {
/* Width can hold maximum of SIZE_MAX - 2. */
#define WIDTH_MAX (SIZE_MAX - 2)
                size_t width;
                enum width_special {
                        WIDTH_VAR = -1,
                        WIDTH_EMPTY = -2,
                } special;
        } width;
        union {
/* Precision can hold maximum of SIZE_MAX - 2. */
#define PRECISION_MAX (SIZE_MAX - 2)
                size_t precision;
                enum precision_special {
                        PREC_VAR = -1,
                        PREC_EMPTY = -2,
                } special;
        } precision;
        enum conv_length length;
        enum conv_specifiers spec;
};

static unsigned vfprintf_atoi(const char *s)
{
        unsigned res = 0;

        for (;; s++) {
                switch (*s) {
                /* Won't overflow because of the case's condition. */
                case '0' ... '9': res = 10 * res + (unsigned)(*s - '0'); break;
                default: return res;
                }
        }
}

static struct conv_spec parse_conv_spec(const char *format, size_t *spec_len)
{
        const char *cursor = format;
        struct conv_spec result = { 0 };
        // The first symbol must be '%'.
        cursor++;

        // Flags
        while (*cursor != '\0') {
                bool done = false;
                switch (*cursor) {
                case '-': {
                        result.flags |= CF_MINUS;
                        cursor++;
                } break;
                case '+': {
                        result.flags |= CF_PLUS;
                        cursor++;
                } break;
                case ' ': {
                        result.flags |= CF_SPACE;
                        cursor++;
                } break;
                case '#': {
                        result.flags |= CF_HASH;
                        cursor++;
                } break;
                case '0': {
                        result.flags |= CF_ZERO;
                        cursor++;
                } break;
                default: done = true;
                }
                if (done) {
                        break;
                }
        }

        // Field width

        {
                if (*cursor == '*') {
                        result.width.special = WIDTH_VAR;
                        cursor++;
                        goto skip_width;
                }
                const char *begin = cursor;
                size_t len = 0;
                while (begin[len] >= '0' && begin[len] <= '9') {
                        len++;
                }
                kassert(len <= WIDTH_MAX);
                if (len == 0) {
                        // Width was not specified.
                        result.width.special = WIDTH_EMPTY;
                        goto skip_width;
                }

                result.width.width = vfprintf_atoi(begin);
                cursor += len;

        skip_width:;
        }

        // Precision
        {
                if (*cursor != '.') {
                        result.precision.special = PREC_EMPTY;
                        goto skip_precision;
                } else {
                        cursor++;
                }

                if (*cursor == '*') {
                        result.precision.special = PREC_VAR;
                        cursor++;
                        goto skip_precision;
                }

                const char *begin = cursor;
                size_t len = 0;
                while (begin[len] >= '0' && begin[len] <= '9') {
                        len++;
                }
                kassert(len <= PRECISION_MAX);
                if (len == 0) {
                        // "If only the period specified, the precision is taken as zero."
                        result.precision.precision = 0;
                        goto skip_precision;
                }
                result.precision.precision = vfprintf_atoi(begin);
                cursor += len;

        skip_precision:;
        }

        // Length modifier
        {
                switch (*cursor) {
                case 'h': {
                        if (cursor[1] == 'h') {
                                result.length = CL_CHAR;
                                cursor++;
                        } else {
                                result.length = CL_SHORT;
                        }
                } break;
                case 'l': {
                        if (cursor[1] == 'l') {
                                result.length = CL_LLONG;
                                cursor++;
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
                        cursor++;
                }
        }

        // Conversion specifier
        {
                switch (*cursor) {
                case 'd':
                case 'i': result.spec = CS_INT; break;
                case 'o': result.spec = CS_UOCTAL; break;
                case 'u': result.spec = CS_UDEC; break;
                case 'x': result.spec = CS_UHEX; break;
                case 'X': result.spec = CS_UHEX_BIG; break;
                case 'c': result.spec = CS_UCHAR; break;
                case 's': result.spec = CS_STR; break;
                case 'p': result.spec = CS_PTR; break;
                case 'n': result.spec = CS_WRITTEN_OUT; break;
                case '%': result.spec = CS_PERCENTAGE; break;
                default: result.spec = CS_INVALID;
                }
                cursor++;
        }

        *spec_len = (uintptr_t)cursor - (uintptr_t)format;
        return (result);
}

struct argument {
        union {
                uintmax_t ud;
                intmax_t d;
                char *str;
        } val;
        bool negative;
};

static struct argument fetch_arg(struct conv_spec s, va_list *args)
{
        struct argument arg = { 0 };
        switch (s.spec) {
        case CS_INT: {
                switch (s.length) {
#define CASE_BODY(__type, __va_type)                         \
        do {                                                 \
                __type v = (__type)va_arg(*args, __va_type); \
                arg.val.d = v;                               \
                if (v < 0) {                                 \
                        arg.negative = true;                 \
                        arg.val.d *= -1;                     \
                }                                            \
        } while (0)
                case CL_CHAR: CASE_BODY(char, int); break;
                case CL_SHORT: CASE_BODY(short, int); break;
                case CL_LONG: CASE_BODY(long, long); break;
                case CL_LLONG: CASE_BODY(long long, long long); break;
                case CL_INTMAX: CASE_BODY(intmax_t, intmax_t); break;
                case CL_NONE: CASE_BODY(int, int); break;
                case CL_SIZET:
                case CL_PTRDIFF: CASE_BODY(ptrdiff_t, ptrdiff_t); break;
                default: kassert(false);
#undef CASE_BODY
                }
        } break;
        case CS_UHEX:
        case CS_UHEX_BIG:
        case CS_UOCTAL:
        case CS_UDEC: {
                switch (s.length) {
                case CL_CHAR: arg.val.d = va_arg(*args, int); break;
                case CL_SHORT: arg.val.d = va_arg(*args, int); break;
                case CL_LONG: arg.val.d = va_arg(*args, long); break;
                case CL_LLONG: arg.val.d = va_arg(*args, long long); break;
                case CL_INTMAX: arg.val.d = va_arg(*args, intmax_t); break;
                case CL_SIZET: arg.val.d = va_arg(*args, size_t); break;
                case CL_NONE: arg.val.d = va_arg(*args, int); break;
                case CL_PTRDIFF: arg.val.d = va_arg(*args, ptrdiff_t); break;
                default: kassert(false);
                }
        } break;
        case CS_PTR: arg.val.d = va_arg(*args, uintptr_t); break;
        case CS_STR: arg.val.str = va_arg(*args, char *); break;
        case CS_UCHAR: arg.val.d = va_arg(*args, int); break;
        default: kassert(false);
        }
        return (arg);
}

struct conv_spec_funcs {
        // Print the format specifier.
        int (*print)(fprintf_fn f, struct conv_spec *s, struct argument *a);
        // Calculate the length of the result of print function.
        size_t (*length)(struct conv_spec *s, struct argument *a);
        // Function that returns string to prepend to the value ("0x" for ptr/hex values, for example)
        // and it's length. Could be NULL.
        const char *(*prefix)(struct conv_spec *s, struct argument *a, size_t *length);
};

__const static size_t length_for_intnumbase(uintmax_t num, unsigned base, unsigned max_num_digits)
{
        kassert(max_num_digits <= INT_MAX);
        for (int i = (int)max_num_digits - 1; i >= 0; i--) {
                uintmax_t power = 1;
                for (int j = 0; j < i; j++) {
                        power *= base;
                }
                if (num >= power) {
                        /* i can't be negative, so the cast is fine. */
                        return ((size_t)i + 1);
                }
        }
        return (1);
}

static size_t oct_length(struct conv_spec *s, struct argument *a)
{
        // Special case:
        // The result of converting a zero value with a precision of zero is no characters.
        if (s->precision.precision == 0 && a->val.ud == 0) {
                return (0);
        }

        unsigned const length = length_for_intnumbase(a->val.ud, 8, 22);

        if (s->precision.special != PREC_EMPTY) {
                if (s->precision.precision >= length) {
                        return (s->precision.precision);
                }
        }

        return (length);
}

static int oct_print(fprintf_fn f, struct conv_spec *s, struct argument *a)
{
        unsigned char buffer[22];
        size_t const buffer_size = ARRAY_SIZE(buffer);
        size_t buffer_i = buffer_size;

        // Special case:
        // The result of converting a zero value with a precision of zero is no characters.
        if (s->precision.precision == 0 && a->val.d == 0) {
                return (0);
        }

        kassert(buffer_i > 0);
        do {
                buffer_i--;
                buffer[buffer_i] = a->val.ud % 8 + '0';
        } while ((a->val.ud /= 8) != 0 && buffer_i > 0);

        size_t const val_len = buffer_size - buffer_i;
        size_t printed = 0;

        if (s->precision.special != PREC_EMPTY) {
                intptr_t padding = (intptr_t)(s->precision.precision - val_len);
                for (intptr_t i = 0; i < padding; i++) {
                        int rc = f("0", 1);
                        if (__unlikely(rc < 0)) {
                                return (rc);
                        }
                        printed += (unsigned)rc;
                }
        }

        int rc = f((char*)&buffer[buffer_i], val_len);
        if (__unlikely(rc < 0)) {
                return (rc);
        }

        kassert(printed <= INT_MAX);
        return (rc + (int)printed);
}

static size_t dec_length(struct conv_spec *s, struct argument *a)
{
        // Special case:
        // The result of converting a zero value with a precision of zero is no characters.
        if (s->precision.precision == 0 && a->val.d == 0) {
                return (0);
        }

        size_t const length = length_for_intnumbase(a->val.ud, 10, 20);

        if (s->precision.special != PREC_EMPTY) {
                if (s->precision.precision >= length) {
                        return (s->precision.precision);
                }
        }

        return (length);
}

static int dec_print(fprintf_fn f, struct conv_spec *s, struct argument *a)
{
        unsigned char buffer[20] = { 0 };
        size_t const buffer_size = ARRAY_SIZE(buffer);
        size_t buffer_i = buffer_size;

        // Special case:
        // The result of converting a zero value with a precision of zero is no characters.
        if (s->precision.precision == 0 && a->val.d == 0) {
                return (0);
        }

        kassert(buffer_i > 0);
        do {
                buffer_i--;
                buffer[buffer_i] = (unsigned char)(a->val.ud % 10 + '0');
        } while ((a->val.ud /= 10) != 0 && buffer_i > 0);

        size_t const value_len = buffer_size - buffer_i;
        size_t printed = 0;

        if (s->precision.special != PREC_EMPTY) {
                /* Padding may be negative if precision is less than the length of the value. */
                intptr_t const padding = (intptr_t)(s->precision.precision - value_len);
                for (intptr_t i = 0; i < padding; i++) {
                        int rc = f("0", 1);
                        if (__unlikely(rc < 0)) {
                                return (rc);
                        }
                        printed += (unsigned)rc;
                }
        }

        int rc = f((char *)&buffer[buffer_i], value_len);
        if (__unlikely(rc < 0)) {
                return (rc);
        }

        kassert(printed <= INT_MAX);
        return (rc + (int)printed);
}

static size_t hex_length(struct conv_spec *s, struct argument *a)
{
        if (s->spec == CS_UHEX || s->spec == CS_UHEX_BIG) {
                if (s->precision.precision == 0 && a->val.ud == 0) {
                        return (0);
                }
        }
        return (length_for_intnumbase(a->val.ud, 16, 16));
}

static int hex_print(fprintf_fn f, struct conv_spec *s, struct argument *a)
{
        unsigned char buffer[16] = { 0 };
        size_t const buffer_size = ARRAY_SIZE(buffer);

        size_t buffer_i = buffer_size;

        // A little bit hacky, but it makes no sense to write the same code for the second and third time.
        if (s->spec == CS_UHEX || s->spec == CS_UHEX_BIG) {
                if (s->precision.precision == 0 && a->val.ud == 0) {
                        return (0);
                }
        }

        /* 'A' for CS_UHEX_BIG and CS_PTR */
        unsigned char const hex_offset = s->spec == CS_UHEX ? 'a' : 'A';

        kassert(buffer_i > 0);
        do {
                buffer_i--;
                unsigned char n = (unsigned char)(a->val.ud % 16);
                if (n < 10) {
                        buffer[buffer_i] = n + '0';
                } else {
                        buffer[buffer_i] = (unsigned char)((n - 10) + hex_offset);
                }
        } while ((a->val.ud /= 16) != 0 && buffer_i > 0);

        size_t const value_len = buffer_size - buffer_i;

        return (f((char *)&buffer[buffer_i], value_len));
}

static const char *hex_prefix(struct conv_spec *s, struct argument *a __unused, size_t *len)
{
        static const char zx[] = "0x";
        // Do not count null-terminator.
        static const unsigned zx_len = sizeof(zx) - 1;

        if (s->spec == CS_PTR) {
                *len = zx_len;
                return (zx);
        }

        if (s->flags & (CF_HASH)) {
                *len = zx_len;
                return (zx);
        }

        *len = 0;
        return ("");
}

static size_t str_length(struct conv_spec *s, struct argument *a)
{
        size_t len = kstrlen(a->val.str);
        if (s->precision.special != PREC_EMPTY && s->precision.precision < len) {
                return (s->precision.precision);
        }
        return (len);
}

static int str_print(fprintf_fn f, struct conv_spec *s, struct argument *a)
{
        const char *str = a->val.str;
        // TODO: Find a way to reuse the result.
        size_t len = str_length(s, a);

        return (f(str, len));
}

static size_t char_length(struct conv_spec *s __unused, struct argument *a __unused)
{
        return (1);
}

static int char_print(fprintf_fn f, struct conv_spec *s __unused, struct argument *a)
{
        return (f((char *)&a->val.d, 1));
}

static struct conv_spec_funcs cs_funcs_table[] = {
        [CS_INT] = (struct conv_spec_funcs){ .print = dec_print,
                                             .length = dec_length,
                                             .prefix = NULL },
        [CS_UDEC] = (struct conv_spec_funcs){ .print = dec_print,
                                              .length = dec_length,
                                              .prefix = NULL },
        [CS_PTR] = (struct conv_spec_funcs){ .print = hex_print,
                                             .length = hex_length,
                                             .prefix = hex_prefix },
        [CS_UHEX] = (struct conv_spec_funcs){ .print = hex_print,
                                              .length = hex_length,
                                              .prefix = hex_prefix },
        [CS_UHEX_BIG] = (struct conv_spec_funcs){ .print = hex_print,
                                                  .length = hex_length,
                                                  .prefix = hex_prefix },
        [CS_STR] = (struct conv_spec_funcs){ .print = str_print,
                                             .length = str_length,
                                             .prefix = NULL },
        [CS_UCHAR] = (struct conv_spec_funcs){ .print = char_print,
                                               .length = char_length,
                                               .prefix = NULL },
        [CS_UOCTAL] = (struct conv_spec_funcs){ .print = oct_print,
                                                .length = oct_length,
                                                .prefix = NULL },
};

/**
 * @brief Print visible characters, specified by flags.
 * 
 * @return int Number of written characters.
 */
static int put_flags(fprintf_fn f, struct conv_spec s, struct argument arg)
{
        int printed = 0;
        if (arg.negative) {
                int rc = f("-", 1);
                if (__unlikely(rc < 0)) {
                        return (rc);
                }
                printed += rc;
        } else if (s.flags & CF_PLUS) {
                int rc = f("-", 1);
                if (__unlikely(rc < 0)) {
                        return (rc);
                }
                printed += rc;
        } else if (s.flags & CF_SPACE) {
                int rc = f("-", 1);
                if (__unlikely(rc < 0)) {
                        return (rc);
                }
                printed += rc;
        }
        return (printed);
}

/**
 * @brief Print given conversion specifier field.
 *
 * @return int Number of written characters.
 */
static int print_conv_spec(fprintf_fn f, struct conv_spec s, va_list *args)
{
        if (s.spec == CS_PERCENTAGE) {
                return (f("%", 1));
        }

        struct argument arg = fetch_arg(s, args);
        size_t const num_len = cs_funcs_table[s.spec].length(&s, &arg);
        size_t printed = 0;

        const char *prefix = NULL;
        size_t prefix_len = 0;
        if (cs_funcs_table[s.spec].prefix) {
                prefix = cs_funcs_table[s.spec].prefix(&s, &arg, &prefix_len);
        }

        bool flag_present = arg.negative || (s.flags & (CF_PLUS | CF_SPACE));

        kassert(s.width.width < INTPTR_MAX);
        kassert(num_len < INTPTR_MAX);
        kassert(prefix_len < INTPTR_MAX);
        intptr_t width_to_fill = (intptr_t)s.width.width - (intptr_t)num_len -
                                 (intptr_t)prefix_len - (flag_present ? 1 : 0);

        // If Width != 0 and the flag '-' was not specified, result must be right-justified
        if (!(s.flags & CF_MINUS) && s.width.special != WIDTH_EMPTY) {
                if (s.flags & CF_ZERO) {
                        if (prefix != NULL) {
                                int rc = f(prefix, prefix_len);
                                if (__unlikely(rc < 0)) {
                                        return (rc);
                                }
                                printed += (unsigned)rc;
                        }
                        int rc = put_flags(f, s, arg);
                        if (__unlikely(rc < 0)) {
                                return (rc);
                        }
                        printed += (unsigned)rc;
                }
                while (width_to_fill > 0) {
                        // remained_width could be < 0, so decrement in the loop
                        char c = s.flags & CF_ZERO ? '0' : ' ';
                        int rc = f(&c, 1);
                        if (__unlikely(rc < 0)) {
                                return (rc);
                        }
                        printed += (unsigned)rc;
                        width_to_fill--;
                }
        }
        if (!(s.flags & CF_ZERO)) {
                if (prefix != NULL) {
                        int rc = f(prefix, prefix_len);
                        if (__unlikely(rc < 0)) {
                                return (rc);
                        }
                        printed += (unsigned)rc;
                }
                int rc = put_flags(f, s, arg);
                if (__unlikely(rc < 0)) {
                        return (rc);
                }
                printed += (unsigned)rc;
        }
        int rc = cs_funcs_table[s.spec].print(f, &s, &arg);
        if (__unlikely(rc < 0)) {
                return (rc);
        }
        printed += (unsigned)rc;

        // If Width != 0 and the flag '-' was specified, result must be left-justified
        if (s.flags & CF_MINUS && s.width.special != WIDTH_EMPTY) {
                while (width_to_fill > 0) {
                        // remained_width could be < 0, so decrement in the loop
                        int frc = f(" ", 1);
                        if (__unlikely(frc < 0)) {
                                return (frc);
                        }
                        printed += (unsigned)frc;
                        width_to_fill--;
                }
        }

        kassert(printed <= INT_MAX);
        return ((int)printed);
}

int kvfprintf(fprintf_fn f, const char *restrict format, va_list args)
{
        va_list ap;
        va_copy(ap, args);
        size_t printed = 0;

        while (*format != '\0') {
                if (*format == '%') {
                        size_t spec_len = 0;
                        struct conv_spec s = parse_conv_spec(format, &spec_len);

                        int rc = print_conv_spec(f, s, &ap);
                        if (__unlikely(rc < 0)) {
                                return (rc);
                        }

                        printed += (unsigned)rc;
                        format += spec_len;
                } else {
                        /* TODO: Clean this. */
                        union {
                                char *nc;
                                char const *c;
                        } tmp;
                        tmp.c = format;
                        tmp.nc = kstrchr(tmp.nc, '%');
                        const char *cspec = tmp.c;

                        size_t toprint;
                        if (cspec != NULL) {
                                toprint = (uintptr_t)cspec - (uintptr_t)format;
                        } else {
                                toprint = kstrlen(format);
                        }

                        int rc = f(format, toprint);
                        if (__unlikely(rc < 0)) {
                                return (rc);
                        }

                        printed += (unsigned)rc;
                        format += toprint;
                }
        }
        va_end(ap);
        kassert(printed <= INT_MAX);
        return ((int)printed);
}

int kfprintf(fprintf_fn f, const char *restrict format, ...)
{
        va_list args;
        va_start(args, format);
        int i = kvfprintf(f, format, args);
        va_end(args);
        return (i);
}
