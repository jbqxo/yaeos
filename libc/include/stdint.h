#pragma once

#include <arch/int_types.h>

#ifdef __i686__
/* Minimum-width integer types */
typedef int8_t  int_least8_t;
typedef int16_t int_least16_t;
typedef int32_t int_least32_t;
typedef int64_t int_least64_t;

typedef uint8_t  uint_least8_t;
typedef uint16_t uint_least16_t;
typedef uint32_t uint_least32_t;
typedef uint64_t uint_least64_t;

/* Fastest minimum-width integer types */
typedef int8_t  int_fast8_t;
typedef int32_t int_fast16_t;
typedef int32_t int_fast32_t;
typedef int64_t int_fast64_t;

typedef uint8_t  uint_fast8_t;
typedef uint32_t uint_fast16_t;
typedef uint32_t uint_fast32_t;
typedef uint64_t uint_fast64_t;

/* Integer types capable of holding object pointers */
typedef int_least64_t  intptr_t;
typedef uint_least32_t uintptr_t;

/* Greatest-width integer types */
typedef int_least64_t  intmax_t;
typedef uint_least64_t uintmax_t;

/* Limits of minimum-width integer types */
#define INT_LEAST8_MIN  INT8_MIN
#define INT_LEAST16_MIN INT16_MIN
#define INT_LEAST32_MIN INT32_MIN
#define INT_LEAST64_MIN INT64_MIN

#define INT_LEAST8_MAX  INT8_MAX
#define INT_LEAST16_MAX INT16_MAX
#define INT_LEAST32_MAX INT32_MAX
#define INT_LEAST64_MAX INT64_MAX

#define UINT_LEAST8_MAX  INT8_MAX
#define UINT_LEAST16_MAX INT16_MAX
#define UINT_LEAST32_MAX INT32_MAX
#define UINT_LEAST64_MAX INT64_MAX

/* Limits of fastest minimum-width integer types */
#define INT_FAST8_MIN  INT8_MIN
#define INT_FAST16_MIN INT32_MIN
#define INT_FAST32_MIN INT32_MIN
#define INT_FAST64_MIN INT64_MIN

#define INT_FAST8_MAX  INT8_MAX
#define INT_FAST16_MAX INT32_MAX
#define INT_FAST32_MAX INT32_MAX
#define INT_FAST64_MAX INT64_MAX

#define UINT_FAST8_MAX  UINT8_MAX
#define UINT_FAST16_MAX UINT32_MAX
#define UINT_FAST32_MAX UINT32_MAX
#define UINT_FAST64_MAX UINT64_MAX

/* Limits of integer types capable of holding object pointers */
#define INTPTR_MIN  INT_LEAST64_MIN
#define INTPTR_MAX  INT_LEAST64_MAX
#define UINTPTR_MAX UINT_LEAST32_MAX

/* Limits of greatest-width integer types */
#define INTMAX_MIN  INT_LEAST64_MIN
#define INTMAX_MAX  INT_LEAST64_MAX
#define UINTMAX_MAX UINT_LEAST64_MAX

/* Limits of other integer types */

#define PTRDIFF_MIN
#define PTRDIFF_MAX

#define SIG_ATOMIC_MIN
#define SIG_ATOMIC_MAX

#define SIZE_MAX

#define WCHAR_MIN
#define WCHAR_MAX

#define WINT_MIN
#define WINT_MAX

/* Macros for minimum-width integer constants */
#define INT8_C(c)  c
#define INT16_C(c) c
#define INT32_C(c) c
#define INT64_C(c) c ## ll

#define UINT8_C(c)  c ## u
#define UINT16_C(c) c ## u
#define UINT32_C(c) c ## u
#define UINT64_C(c) c ## ull

/* Macros for greatest-width integer constants */
#define INTMAX_C(c)  c ## ll
#define UINTMAX_C(c) c ## ull

#endif // __i686__
