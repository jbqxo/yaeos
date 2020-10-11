#include "lib/string.h"

char *kstrchr(char *s, int c)
{
	while (*s != '\0') {
		if (*s == c) {
			return (s);
		}
		s++;
	}
	return (NULL);
}
