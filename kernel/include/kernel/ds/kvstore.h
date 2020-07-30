#ifndef _KERNEL_DS_KVSTORE_H
#define _KERNEL_DS_KVSTORE_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <lib/string.h>

#define KVSTATIC_DECLARE(valuetype, length)                                                        \
	struct {                                                                                   \
		char *keys[(length)];                                                              \
		valuetype values[(length)];                                                        \
	}

#define KVSTATIC_LEN(kvvar) (sizeof((kvvar)->keys) / sizeof(*((kvvar)->keys)))

#define KVSTATIC_INIT(kvvar, val_init)                                                             \
	do {                                                                                       \
		int __i;                                                                           \
		char *__k;                                                                         \
		typeof((kvvar)->values[0]) __v;                                                    \
		KVSTATIC_FOREACH0(kvvar, __i, __k, __v)                                            \
		{                                                                                  \
			(kvvar)->keys[__i] = (void *)0;                                            \
			(kvvar)->values[__i] = (val_init);                                         \
		}                                                                                  \
	} while (0)

#define KVSTATIC_FOREACH0(kvvar, ivar, itkey, itval)                                               \
	for ((ivar) = 0, (itkey) = (kvvar)->keys[(ivar)], (itval) = (kvvar)->values[(ivar)];       \
	     (ivar) < KVSTATIC_LEN(kvvar);                                                         \
	     (ivar) += 1, (itkey) = (kvvar)->keys[(ivar)], (itval) = (kvvar)->values[(ivar)])

#define KVSTATIC_GETIDX(kvvar, key, dest)                                                          \
	do {                                                                                       \
		(dest) = -1;                                                                       \
		int __i;                                                                           \
		char *__k;                                                                         \
		typeof((kvvar)->values[0]) __v;                                                    \
		KVSTATIC_FOREACH0(kvvar, __i, __k, __v)                                            \
		{                                                                                  \
			if (strcmp((key), __k) == 0) {                                             \
				(dest) = __i;                                                      \
				break;                                                             \
			}                                                                          \
		}                                                                                  \
	} while (0)

#define KVSTATIC_ADD(kvvar, newkey, newvalue)                                                      \
	do {                                                                                       \
		int __i;                                                                           \
		char *__k;                                                                         \
		typeof((kvvar)->values[0]) __v;                                                    \
		KVSTATIC_FOREACH0(kvvar, __i, __k, __v)                                            \
		{                                                                                  \
			if (__k == (void *)0) {                                                    \
				break;                                                             \
			}                                                                          \
		}                                                                                  \
		(kvvar)->keys[__i] = (newkey);                                                     \
		(kvvar)->values[__i] = (newvalue);                                                 \
	} while (0)

#define KVSTATIC_DEL(kvvar, key)                                                                   \
	do {                                                                                       \
		int __idx;                                                                         \
		KVSTATIC_GETIDX(kvvar, key, __idx);                                                \
		if (__idx >= 0 && __idx < KVSTATIC_LEN(kvvar)) {                                   \
			(kvvar)->keys[__idx] = (void *)0;                                          \
		}                                                                                  \
	} while (0)

#define KVSTATIC_GET(name, key, dest, failvalue)                                                   \
	do {                                                                                       \
		int __idx;                                                                         \
		KVSTATIC_GETIDX(kvvar, key, __idx);                                                \
		if (__idx >= 0 && __idx < KVSTATIC_LEN(kvvar)) {                                   \
			(dest) = (kvvar)->values[__idx];                                           \
		} else {                                                                           \
			(dest) = (failvalue);                                                      \
		}                                                                                  \
	} while (0)

#define KVSTATIC_FOREACH(kvvar, ivar, itkey, itval)                                                \
	KVSTATIC_FOREACH0(kvvar, ivar, itkey, itval)                                               \
	if ((itkey) != (void *)0)

#endif // _KERNEL_DS_KVSTORE_H
