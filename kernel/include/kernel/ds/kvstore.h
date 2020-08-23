#ifndef _KERNEL_DS_KVSTORE_H
#define _KERNEL_DS_KVSTORE_H

#include "lib/string.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define KVSTATIC_DECLARE(keytype, valuetype, length, key_cmp_func) \
	struct {                                                   \
		keytype keys[(length)];                            \
		keytype key_nil;                                   \
		valuetype values[(length)];                        \
		typeof(key_cmp_func) *cmp;                         \
	}

#define KVSTATIC_CAP(kvvar) (sizeof((kvvar)->keys) / sizeof(*((kvvar)->keys)))

#define KVSTATIC_INIT(kvvar, key_nil_value, val_init, key_cmp_func) \
	do {                                                        \
		int __i;                                            \
		typeof((kvvar)->key_nil) __k;                       \
		typeof((kvvar)->values[0]) __v;                     \
		(kvvar)->cmp = (key_cmp_func);                      \
		(kvvar)->key_nil = (key_nil_value);                 \
		KVSTATIC_FOREACH0 (kvvar, __i, __k, __v) {          \
			(kvvar)->keys[__i] = (kvvar)->key_nil;      \
			(kvvar)->values[__i] = (val_init);          \
		}                                                   \
	} while (0)

// Let's hope that clang will figure this mess out (eliminate checks).

#define KVSTATIC_FOREACH0(kvvar, ivar, itkey, itval)                                         \
	for ((ivar) = 0, (itkey) = (kvvar)->keys[(ivar)], (itval) = (kvvar)->values[(ivar)]; \
	     (ivar) < KVSTATIC_CAP(kvvar); (ivar) += 1,                                      \
	    (itkey) = (kvvar)->keys[(ivar) < KVSTATIC_CAP(kvvar) ? (ivar) : (ivar)-1],       \
	    (itval) = (kvvar)->values[(ivar) < KVSTATIC_CAP(kvvar) ? (ivar) : (ivar)-1])

#define KVSTATIC_GETIDX(kvvar, key, dest)                    \
	do {                                                 \
		(dest) = -1;                                 \
		int __i;                                     \
		typeof((kvvar)->key_nil) __k;                \
		typeof((kvvar)->values[0]) __v;              \
		KVSTATIC_FOREACH0 (kvvar, __i, __k, __v) {   \
			if ((kvvar)->cmp((key), __k) == 0) { \
				(dest) = __i;                \
				break;                       \
			}                                    \
		}                                            \
	} while (0)

#define KVSTATIC_ADD(kvvar, newkey, newvalue)                           \
	do {                                                            \
		int __freeidx = -1;                                     \
		int __i;                                                \
		typeof((kvvar)->key_nil) __k;                           \
		typeof((kvvar)->values[0]) __v;                         \
		KVSTATIC_FOREACH0 (kvvar, __i, __k, __v) {              \
			if ((kvvar)->cmp(__k, (kvvar)->key_nil) == 0) { \
				__freeidx = __i;                        \
				break;                                  \
			}                                               \
		}                                                       \
		if (__freeidx == -1) {                                  \
			break;                                          \
		}                                                       \
		(kvvar)->keys[__freeidx] = (newkey);                    \
		(kvvar)->values[__freeidx] = (newvalue);                \
	} while (0)

#define KVSTATIC_DEL(kvvar, key)                                 \
	do {                                                     \
		int __idx;                                       \
		KVSTATIC_GETIDX(kvvar, key, __idx);              \
		if (__idx >= 0 && __idx < KVSTATIC_CAP(kvvar)) { \
			(kvvar)->keys[__idx] = (kvvar)->key_nil; \
		}                                                \
	} while (0)

#define KVSTATIC_GET(kvvar, key, dest, failvalue)                \
	do {                                                     \
		int __idx;                                       \
		KVSTATIC_GETIDX(kvvar, key, __idx);              \
		if (__idx >= 0 && __idx < KVSTATIC_CAP(kvvar)) { \
			(dest) = (kvvar)->values[__idx];         \
		} else {                                         \
			(dest) = (failvalue);                    \
		}                                                \
	} while (0)

#define KVSTATIC_SET(kvvar, key, value)                                \
	do {                                                           \
		int __idx;                                             \
		KVSTATIC_GETIDX(kvvar, key, __idx);                    \
		if (__idx >= 0 && __idx < KVSTATIC_CAP(kvvar)) {       \
			(kvvar)->values[__idx] = (value);              \
		} else {                                               \
			assert("kvstatic_set: key hasn't been found"); \
		}                                                      \
	} while (0)

#define KVSTATIC_FOREACH(kvvar, ivar, itkey, itval)   \
	KVSTATIC_FOREACH0 (kvvar, ivar, itkey, itval) \
		if ((kvvar)->cmp((itkey), (kvvar)->key_nil))

#define KVSTATIC_LEN(kvvar, dest)                         \
	do {                                              \
		int __i;                                  \
		typeof((kvvar)->key_nil) __k;             \
		typeof((kvvar)->values[0]) __v;           \
		(dest) = 0;                               \
		KVSTATIC_FOREACH (kvvar, __i, __k, __v) { \
			(dest)++;                         \
		}                                         \
	} while (0)

#endif // _KERNEL_DS_KVSTORE_H
