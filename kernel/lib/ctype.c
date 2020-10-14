#include "lib/ctype.h"

#include <stdbool.h>
#include <stddef.h>

int kisalnum(int c)
{
        return (kisalpha(c) | kisdigit(c));
}

int kisalpha(int c)
{
        return (kislower(c) | kisupper(c));
}

int kisblank(int c)
{
        return (c == ' ' | c == '\t');
}

int kiscntrl(int c)
{
        // ASCII code of the last control character
        // in the sequence placed at the start of the ASCII.
        int unit_sep_code = 31;
        if (c <= unit_sep_code) {
                return (true);
        }

        int del_code = 127;
        return (c == del_code);
}

int kisdigit(int c)
{
        return ('0' <= c && c <= '9');
}

int kisgraph(int c)
{
        // TODO(Maxim Lyapin): Check if this is correct implementation.
        return ((c != ' ') & (kisalnum(c) | kispunct(c)));
}

int kislower(int c)
{
        return ('a' <= c && c <= 'z');
}

int kisprint(int c)
{
        return ((c == ' ') | kisgraph(c));
}

int kispunct(int c)
{
        if ('!' <= c & c <= '/') {
                return (true);
        }

        if (':' <= c & c <= '@') {
                return (true);
        }

        if ('[' <= c & c <= '`') {
                return (true);
        }

        return ('{' <= c & c <= '~');
}

int kisspace(int c)
{
        // The standard white-space characters according to C99 standard.
        static char white_space_chars[] = { ' ', '\f', '\n', '\r', '\t', '\v' };
        static size_t wsc_len = sizeof(white_space_chars) / sizeof(char);

        for (size_t i = 0; i < wsc_len; ++i) {
                if (c == white_space_chars[i]) {
                        return (true);
                }
        }

        return (false);
}

int kisupper(int c)
{
        return ('A' <= c && c <= 'Z');
}

int kisxdigit(int c)
{
        if ('0' <= c & c <= '9') {
                return (true);
        }

        if ('a' <= c & c <= 'f') {
                return (true);
        }

        return ('A' <= c & c <= 'F');
}

int ktolower(int c)
{
        if (!kisupper(c)) {
                return (c);
        }

        int diff = 'a' - 'A';
        return (c + diff);
}

int ktoupper(int c)
{
        if (!kislower(c)) {
                return (c);
        }

        int diff = 'a' - 'A';
        return (c - diff);
}
