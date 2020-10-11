#ifndef _LIB_CTYPE_H
#define _LIB_CTYPE_H

// Tests for any character for which isalpha or isdigit is true.
int kisalnum(int c);

// Returns true only for the characters for which isupper or islower is true.
int kisalpha(int c);

// Returns true only for the standard blank characters(' ' and '\t').
int kisblank(int c);

// Tests for any control character.
int kiscntrl(int c);

// Tests for any decimal-digit character.
int kisdigit(int c);

// The isgraph function tests for any printing character except space.
int kisgraph(int c);

// Returns true only for the lowercase letters.
int kislower(int c);

// Tests for any printing character including space.
int kisprint(int c);

// Returns true for every printing character for which neither isspace nor isalnum is true.
int kispunct(int c);

// Tests for any character that is a standard white-space character.
int kisspace(int c);

// Returns true only for the uppercase letters.
int kisupper(int c);

// Tests for any hexadecimal-digit character
int kisxdigit(int c);

// Converts an uppercase letter to a corresponding lowercase letter.
int ktolower(int c);

// Converts a lowercase letter to a corresponding uppercase letter.
int ktoupper(int c);

#endif // _LIBC_CTYPE_H
