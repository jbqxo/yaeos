#ifndef _LIB_DS_CBUFFER_H
#define _LIB_DS_CBUFFER_H

#include "lib/cstd/string.h"

#include <stdint.h>

#define CBUFFER_DECLARE(valuetype, length) \
        struct {                           \
                size_t r_idx;              \
                size_t w_idx;              \
                size_t count;              \
                valuetype array[(length)]; \
        }

#define CBUFFER_INIT(cbuffer)                                            \
        do {                                                             \
                (cbuffer)->r_idx = 0;                                    \
                (cbuffer)->w_idx = 0;                                    \
                (cbuffer)->count = 0;                                    \
                kmemset(&(cbuffer)->array, 0, sizeof((cbuffer)->array)); \
        } while (0)

#define CBUFFER_LENGTH(cbuffer)    ARRAY_SIZE((cbuffer)->array)
#define CBUFFER_NEXT_WIDX(cbuffer) (((cbuffer)->w_idx + 1) % CBUFFER_LENGTH(cbuffer))
#define CBUFFER_NEXT_RIDX(cbuffer) (((cbuffer)->r_idx + 1) % CBUFFER_LENGTH(cbuffer))

#define CBUFFER_PUSH(cbuffer, value)                                              \
        ({                                                                        \
                bool __cbuf_success = (cbuffer)->count < CBUFFER_LENGTH(cbuffer); \
                do {                                                              \
                        if ((cbuffer)->count == CBUFFER_LENGTH(cbuffer)) {        \
                                break;                                            \
                        }                                                         \
                        (cbuffer)->array[(cbuffer)->w_idx] = (value);             \
                        (cbuffer)->w_idx = CBUFFER_NEXT_WIDX(cbuffer);            \
                        (cbuffer)->count++;                                       \
                } while (0);                                                      \
                __cbuf_success;                                                   \
        })

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

#endif /* _LIB_DS_CBUFFER_H */
