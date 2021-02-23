#ifndef _LIB_DS_KVSTORE_H
#define _LIB_DS_KVSTORE_H

#include "lib/cppdefs.h"
#include "lib/cstd/string.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef int (*kvstore_fn_cmpkeys_t)(void const *lhs_key, void const *rhs_key);
typedef void (*kvstore_fn_iter_t)(void *key, void *value, void *data);

struct kvstore {
        void **keys;
        size_t cap;
        void **values;
        kvstore_fn_cmpkeys_t cmpfn;
};

size_t kvstore_predict_reqmem(size_t cap);

struct kvstore *kvstore_create(void *mem, size_t capacity, kvstore_fn_cmpkeys_t cmpfn);
size_t kvstore_capacity(struct kvstore const *kv);
void kvstore_iter(struct kvstore const *kv, kvstore_fn_iter_t itfn, void *data);
bool kvstore_find(struct kvstore const *kv, void const *key, void **result_val);
size_t kvstore_length(struct kvstore const *kv);

void kvstore_append(struct kvstore *kv, void *key, void *val);
void kvstore_remove(struct kvstore *kv, void const *key);
void kvstore_change(struct kvstore *kv, void const *key, void *val);

#endif /* _LIB_DS_KVSTORE_H */
