#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>

int isalnum(int c)
{
	return isalpha(c) | isdigit(c);
}

int isalpha(int c)
{
	return islower(c) | isupper(c);
}

int isblank(int c)
{
	return c == ' ' | c == '\t';
}

int iscntrl(int c)
{
	// ASCII code of the last control character
	// in the sequence placed at the start of the ASCII.
	int unit_sep_code = 31;
	if (c <= unit_sep_code) {
		return true;
	}

	int del_code = 127;
	return c == del_code;
}

int isdigit(int c)
{
	return '0' <= c && c <= '9';
}

int isgraph(int c)
{
	// TODO(Maxim Lyapin): Check if this is correct implementation.
	return (c != ' ') & (isalnum(c) | ispunct(c));
}

int islower(int c)
{
	return 'a' <= c && c <= 'z';
}

int isprint(int c)
{
	return (c == ' ') | isgraph(c);
}

int ispunct(int c)
{
	if ('!' <= c & c <= '/') {
		return true;
	}

	if (':' <= c & c <= '@') {
		return true;
	}

	if ('[' <= c & c <= '`') {
		return true;
	}

	return '{' <= c & c <= '~';
}

int isspace(int c)
{
	// The standard white-space characters according to C99 standard.
	static char white_space_chars[] = { ' ', '\f', '\n', '\r', '\t', '\v' };
	static size_t wsc_len = sizeof(white_space_chars) / sizeof(char);

	for (size_t i = 0; i < wsc_len; ++i) {
		if (c == white_space_chars[i]) {
			return true;
		}
	}

	return false;
}

int isupper(int c)
{
	return 'A' <= c && c <= 'Z';
}

int isxdigit(int c)
{
	if ('0' <= c & c <= '9') {
		return true;
	}

	if ('a' <= c & c <= 'f') {
		return true;
	}

	return 'A' <= c & c <= 'F';
}

int tolower(int c)
{
	if (!isupper(c)) {
		return c;
	}

	int diff = 'a' - 'A';
	return c + diff;
}

int toupper(int c)
{
	if (!islower(c)) {
		return c;
	}

	int diff = 'a' - 'A';
	return c - diff;
}
