#include "lib/ds/kvstore.h"

#include "lib/align.h"
#include "lib/cppdefs.h"
#include "lib/cstd/assert.h"
#include "lib/mm/linear.h"

size_t kvstore_predict_reqmem(size_t const cap)
{
        struct kvstore fake;

        size_t space = 0;
        space += sizeof(fake);
        space += cap * sizeof(*fake.keys);
        space += cap * sizeof(*fake.values);

        return (space);
}

struct kvstore *kvstore_create(void *const mem, size_t const cap, kvstore_fn_cmpkeys_t const cmpfn)
{
        kassert(cmpfn != NULL);

        struct linear_alloc alloc;
        linear_alloc_init(&alloc, mem, kvstore_predict_reqmem(cap));

        struct kvstore *kv = linear_alloc_alloc(&alloc, sizeof(*kv));
        kv->keys = linear_alloc_alloc(&alloc, cap * sizeof(*kv->keys));
        kv->cap = cap;
        kv->values = linear_alloc_alloc(&alloc, cap * sizeof(*kv->values));
        kv->cmpfn = cmpfn;

        for (size_t i = 0; i < cap; i++) {
                kv->keys[i] = NULL;
                kv->values[i] = NULL;
        }

        kassert(({
                size_t const mem_occupied = linear_alloc_occupied(&alloc);
                mem_occupied == kvstore_predict_reqmem(cap);
        }));

        return (kv);
}

size_t kvstore_capacity(struct kvstore const *kv)
{
        kassert(kv != NULL);
        return (kv->cap);
}

void kvstore_iter(struct kvstore const *kv, kvstore_fn_iter_t itfn, void *data)
{
        kassert(kv != NULL);
        kassert(itfn != NULL);

        for (size_t i = 0; i < kv->cap; i++) {
                if (kv->keys[i] != NULL) {
                        itfn(kv->keys[i], kv->values[i], data);
                }
        }
}

bool kvstore_find(struct kvstore const *kv, void const *key, void **result_val)
{
        kassert(kv != NULL);
        kassert(key != NULL);

        for (size_t i = 0; i < kv->cap; i++) {
                if (NULL != kv->keys[i] && 0 == kv->cmpfn(kv->keys[i], key)) {
                        *result_val = kv->values[i];
                        return (true);
                }
        }

        return (false);
}

void kvstore_append(struct kvstore *kv, void *key, void *val)
{
        kassert(kv != NULL);
        /* We use the NULL value to mark an empty key. */
        kassert(key != NULL);
        /* We can't deal with duplicate keys. */
        void *present_key __unused = NULL;
        kassert(!kvstore_find(kv, key, &present_key));

        bool found __maybe_unused = false;
        size_t ndx = 0;
        for (; ndx < kv->cap; ndx++) {
                if (kv->keys[ndx] == NULL) {
                        found = true;
                        break;
                }
        }

        kassert(found == true);
        kassert(kv->keys[ndx] == NULL);
        kassert(kv->values[ndx] == NULL);

        kv->keys[ndx] = key;
        kv->values[ndx] = val;
}

void kvstore_remove(struct kvstore *kv, void const *key)
{
        kassert(kv != NULL);
        kassert(key != NULL);

        for (size_t i = 0; i < kv->cap; i++) {
                if (kv->cmpfn(kv->keys[i], key) == 0) {
                        kv->keys[i] = NULL;
                        kv->values[i] = NULL;
                }
        }
}

void kvstore_change(struct kvstore *kv, void const *key, void *val)
{
        kassert(kv != NULL);
        kassert(key != NULL);

        for (size_t i = 0; i < kv->cap; i++) {
                if (kv->cmpfn(kv->keys[i], key) == 0) {
                        kv->values[i] = val;
                }
        }
}

size_t kvstore_length(struct kvstore const *kv)
{
        kassert(kv != NULL);

        size_t len = 0;

        for (size_t i = 0; i < kv->cap; i++) {
                if (kv->keys[i] != NULL) {
                        len++;
                }
        }

        return (len);
}
