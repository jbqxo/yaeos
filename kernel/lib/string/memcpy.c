#include "lib/string.h"

void *kmemcpy(void *restrict _dest, const void *restrict _src, size_t n)
{
	const unsigned char *src = _src;
	unsigned char *dest = _dest;

	for (size_t i = 0; i < n; i++) {
		dest[i] = src[i];
	}
	return (_dest);
}
