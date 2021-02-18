#include "lib/cstd/string.h"

void *kmemcpy(void *restrict _dest, const void *restrict _src, size_t n)
{
        const unsigned char *src = _src;
        unsigned char *dest = _dest;

        for (size_t i = 0; i < n; i++) {
                dest[i] = src[i];
        }
        return (_dest);
}

// A compiler may make use of the memcpy function.
void *memcpy(void *restrict _dest, const void *restrict _src, size_t n)
{
        return (kmemcpy(_dest, _src, n));
}
