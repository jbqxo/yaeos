#pragma once

#include <arch/int_types.h>

#ifdef __i686__

typedef int32_t size_t;
typedef int32_t ptrdiff_t;
typedef long double max_align_t;
typedef char wchar_t;

#define NULL ((void*)0)
#define offsetof(type, member_designator) ((size_t)(&(((type*)0)->member_designator)))

#endif // __i686__
