#ifndef _LIB_TESTS_ASSERT_FAILED_H
#define _LIB_TESTS_ASSERT_FAILED_H

#include <setjmp.h>
#include <stdbool.h>
#include <unity.h>

extern bool UNIT_TEST_KASSERT_SET;
extern jmp_buf UNIT_TEST_KASSERT_JMPBUF;

enum expect_failed_kassert_state {
        EFK_STATE_FIRST = 0,
        EFK_STATE_SUCCESS = 1,
        EFK_STATE_FAILURE = 2,
};

#ifndef NDEBUG
/**
 * @brief Expect that a kassert will fail during the execution of the expression.
 * @return True if an assertion failed; false otherwise. */
#define failed_kassert(EXPRESSION_TEST)                                                     \
        ({                                                                                  \
                UNIT_TEST_KASSERT_SET = true;                                               \
                enum expect_failed_kassert_state _state =                                   \
                        (enum expect_failed_kassert_state)setjmp(UNIT_TEST_KASSERT_JMPBUF); \
                if (_state == EFK_STATE_FIRST) {                                            \
                        (EXPRESSION_TEST);                                                  \
                        longjmp(UNIT_TEST_KASSERT_JMPBUF, EFK_STATE_FAILURE);               \
                }                                                                           \
                UNIT_TEST_KASSERT_SET = false;                                              \
                _state == EFK_STATE_SUCCESS;                                                \
        })
#else
#define failed_kassert(EXPRESSION_TEST) \
        (TEST_MESSAGE("Assertions are disabled on NDEBUG builds."), true)
#endif /* NDEBUG */

#define failed_kassert_kasserted() (longjmp(UNIT_TEST_KASSERT_JMPBUF, EFK_STATE_SUCCESS))

#define failed_kassert_expecting() (UNIT_TEST_KASSERT_SET)

#endif /* _LIB_TESTS_ASSERT_FAILED_H */
