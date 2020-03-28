#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Tests for any character for which isalpha or isdigit is true.
int isalnum(int c);

// Returns true only for the characters for which isupper or islower is true.
int isalpha(int c);

// Returns true only for the standard blank characters(' ' and '\t').
int isblank(int c);

// Tests for any control character.
int iscntrl(int c);

// Tests for any decimal-digit character.
int isdigit(int c);

// The isgraph function tests for any printing character except space.
int isgraph(int c);

// Returns true only for the lowercase letters.
int islower(int c);

// Tests for any printing character including space.
int isprint(int c);

// Returns true for every printing character for which neither isspace nor isalnum is true.
int ispunct(int c);

// Tests for any character that is a standard white-space character.
int isspace(int c);

// Returns true only for the uppercase letters.
int isupper(int c);

// Tests for any hexadecimal-digit character
int isxdigit(int c);

// Converts an uppercase letter to a corresponding lowercase letter.
int tolower(int c);

// Converts a lowercase letter to a corresponding uppercase letter.
int toupper(int c);

#ifdef __cplusplus
}
#endif
