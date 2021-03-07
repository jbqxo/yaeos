#include "lib/cstd/string.h"
#include "lib/cppdefs.h"

#include <stdint.h>

void *kmemset(void *_data, int val, size_t len)
{
        uint8_t *data = _data;
        for (size_t i = 0; i < len; i++) {
                data[i] = (uint8_t)val;
        }
        return (data);
}

void *memset(void *_data, int val, size_t len) __alias("kmemset");
