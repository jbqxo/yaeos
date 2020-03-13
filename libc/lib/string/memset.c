#include <string.h>
#include <stdint.h>

void *memset(void *_data, int val, size_t len) {
    // TODO: There is a vast room for optimizations.
    unsigned char *data = _data;
    for(;len; len--) {
        data[len] = (uint8_t)val;
    }
    return data;
}