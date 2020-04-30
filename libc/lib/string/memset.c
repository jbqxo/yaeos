#include <string.h>
#include <stdint.h>

void *memset(void *_data, int val, size_t len) {
    unsigned char *data = _data;
    for(size_t i = 0; i < len; i++) {
        data[i] = (uint8_t)val;
    }
    return data;
}
