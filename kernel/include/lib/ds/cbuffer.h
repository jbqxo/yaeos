#ifndef _KERNEL_DS_CBUFFER_H
#define _KERNEL_DS_CBUFFER_H

#include "lib/cstd/string.h"

#include <stdint.h>

#define CBUFFER_DECLARE(valuetype, length) \
        struct {                           \
                valuetype array[(length)]; \
                unsigned r_idx;            \
                unsigned w_idx;            \
                unsigned count;            \
        }

#define CBUFFER_INIT(cbuffer)                                            \
        do {                                                             \
                kmemset(&(cbuffer)->array, 0, sizeof((cbuffer)->array)); \
                (cbuffer)->r_idx = 0;                                    \
                (cbuffer)->w_idx = 0;                                    \
                (cbuffer)->count = 0;                                    \
        } while (0)

#define CBUFFER_LENGTH(cbuffer)    ARRAY_SIZE((cbuffer)->array)
#define CBUFFER_NEXT_WIDX(cbuffer) (((cbuffer)->w_idx + 1) % CBUFFER_LENGTH(cbuffer))
#define CBUFFER_NEXT_RIDX(cbuffer) (((cbuffer)->r_idx + 1) % CBUFFER_LENGTH(cbuffer))

#define CBUFFER_PUSH(cbuffer, value)                               \
        ((cbuffer)->count < CBUFFER_LENGTH(cbuffer));              \
        do {                                                       \
                if ((cbuffer)->count == CBUFFER_LENGTH(cbuffer)) { \
                        break;                                     \
                }                                                  \
                (cbuffer)->array[(cbuffer)->w_idx] = (value);      \
                (cbuffer)->w_idx = CBUFFER_NEXT_WIDX(cbuffer);     \
                (cbuffer)->count++;                                \
        } while (0)

#define CBUFFER_POP(cbuffer, dest, defaultval)                 \
        do {                                                   \
                if ((cbuffer)->count == 0) {                   \
                        (dest) = (defaultval);                 \
                        break;                                 \
                }                                              \
                (dest) = (cbuffer)->array[(cbuffer)->r_idx];   \
                (cbuffer)->r_idx = CBUFFER_NEXT_RIDX(cbuffer); \
                (cbuffer)->count--;                            \
        } while (0)

#endif // _KERNEL_DS_CBUFFER_H
