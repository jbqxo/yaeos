#ifndef _LIB_ASSERT_H
#define _LIB_ASSERT_H
#undef kassert

#include "lib/cppdefs.h"
#include "lib/cstd/stdio.h"

__noreturn void assertion_fail(char const *failed_expression, char const *location);
void assertion_init(fprintf_fn);

#ifdef NDEBUG
#define kassert(exp) ((void)0)
#else
#define kassert(exp)                                                                              \
        do {                                                                                      \
                if (__unlikely(!(exp))) {                                                         \
                        assertion_fail(TO_SSTR_MACRO(exp), __FILE__ ":" TO_SSTR_MACRO(__LINE__)); \
                }                                                                                 \
        } while (0)
#endif

#define kstatic_assert(exp, str) _Static_assert(exp, str)

#endif // _LIB_ASSERT_H
