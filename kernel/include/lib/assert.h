#ifndef _LIB_ASSERT_H
#define _LIB_ASSERT_H
#undef kassert

#include "kernel/cppdefs.h"
#include "kernel/panic.h"

#ifdef NDEBUG
#define kassert(exp) ((void)0)
#else
#define kassert(exp)                                                                              \
	do {                                                                                     \
		if (__unlikely(!(exp))) {                                                        \
			struct kernel_panic_info __info = { 0 };                                 \
			__info.description = "Assertion failed: assert(" TO_SSTR_MACRO(exp) ")"; \
			__info.location = __FILE__ ":" TO_SSTR_MACRO(__LINE__);                  \
			kernel_panic(&__info);                                                   \
		}                                                                                \
	} while (0)
#endif

#define kstatic_assert(exp, str) _Static_assert(exp, str)

#endif // _LIB_ASSERT_H
