#include "lib/cstd/string.h"

#include <stddef.h>

size_t kstrlen(const char *str)
{
        size_t i = 0;
        while (str[i] != 0) {
                i++;
        }
        return (i);
}
