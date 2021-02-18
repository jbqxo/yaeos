#include "lib/cstd/string.h"

int kstrcmp(const char *s1, const char *s2)
{
        const unsigned char *lhs = (const unsigned char *)s1;
        const unsigned char *rhs = (const unsigned char *)s2;
        while (lhs && rhs && *lhs == *rhs) {
                lhs++;
                rhs++;
        }

        return (*lhs - *rhs);
}

int kstrncmp(const char *s1, const char *s2, size_t n)
{
        const unsigned char *lhs = (const unsigned char *)s1;
        const unsigned char *rhs = (const unsigned char *)s2;
        while (n > 0 && lhs && rhs && *lhs == *rhs) {
                lhs++;
                rhs++;
                n--;
        }

        return (*lhs - *rhs);
}
