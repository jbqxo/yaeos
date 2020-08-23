#include "lib/string.h"

char *strchr(const char *s, int c)
{
	while (*s != '\0') {
		if (*s == c) {
			return (s);
		}
		s++;
	}
	return (NULL);
}
