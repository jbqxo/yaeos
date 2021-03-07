#ifndef _LIB_CTYPE_H
#define _LIB_CTYPE_H

#include "lib/cppdefs.h"

/* Tests for any character for which isalpha or isdigit is true. */
__const int kisalnum(int c);

/* Returns true only for the characters for which isupper or islower is true. */
__const int kisalpha(int c);

/* Returns true only for the standard blank characters(' ' and '\t'). */
__const int kisblank(int c);

/* Tests for any control character. */
__const int kiscntrl(int c);

/* Tests for any decimal-digit character. */
__const int kisdigit(int c);

/* The isgraph function tests for any printing character except space. */
__const int kisgraph(int c);

/* Returns true only for the lowercase letters. */
__const int kislower(int c);

/* Tests for any printing character including space. */
__const int kisprint(int c);

/* Returns true for every printing character for which neither isspace nor isalnum is true. */
__const int kispunct(int c);

/* Tests for any character that is a standard white-space character. */
__const int kisspace(int c);

/* Returns true only for the uppercase letters. */
__const int kisupper(int c);

/* Tests for any hexadecimal-digit character */
__const int kisxdigit(int c);

/* Converts an uppercase letter to a corresponding lowercase letter. */
__const int ktolower(int c);

/* Converts a lowercase letter to a corresponding uppercase letter. */
__const int ktoupper(int c);

#endif /* _LIBC_CTYPE_H */
