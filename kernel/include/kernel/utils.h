#ifndef _KERNEL_UTILS_H
#define _KERNEL_UTILS_H

#include <kernel/cppdefs.h>

/**
 * @brief Return the nearest address that is bigger than the address and fit the alignment.
 */
static inline union uiptr align_roundup(union uiptr from, uintptr_t alignment)
{
	if (alignment == 0) {
		return (from);
	}
	from.i += alignment - 1;
	from.i &= -alignment;
	return (from);
}

#define MAX(x, y) ((x) < (y) ? (y) : (x))
#define MIN(x, y) ((x) > (y) ? (y) : (x))

#endif // _KERNEL_UTILS_H
