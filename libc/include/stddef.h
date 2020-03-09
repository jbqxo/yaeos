#pragma once

#include <arch/int_types.h>

#ifdef __i686__

typedef uint32_t     size_t;
typedef int32_t      ptrdiff_t;
typedef long double  max_align_t;
typedef uint8_t      wchar_t;

#define NULL ((void*)0)
#define offsetof(type, member) __builtin_offsetof(type, member)

#endif // __i686__
