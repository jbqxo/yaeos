#ifndef _LIB_ELFLIST_H
#define _LIB_ELFLIST_H

#include "lib/cppdefs.h"

/**
 * Store pointers to custom information inside the kernel's binary.
 */

/**
  * Used to mark edges of a list.
  */
typedef void *elflist_mark;

/**
 * The macro places the address of a symbol in the list's section and ensures that there are edge marks.
 * Note that it's important how sections are named, as they are sorted by a linker.
 */
#define ELFLIST_NEWDATA(listname, symbol)                                                          \
        __weak elflist_mark __elflist_##listname##_begin __section(".elflist_" #listname "_begin") \
                __used = NULL;                                                                     \
        void *__elflist_##listname##_##symbol __section(".elflist_" #listname "_dat"               \
                                                        "a") __used = &(symbol);                   \
        __weak elflist_mark __elflist_##listname##_end __section(".elflist"                        \
                                                                 "_" #listname "_end")             \
                __used = NULL

#define ELFLIST_EXTERN(listname)                                                                   \
        __weak elflist_mark __elflist_##listname##_begin __section(".elflist_" #listname "_begin") \
                __used;                                                                            \
        __weak elflist_mark __elflist_##listname##_end __section(".elflist_" #listname "_end")     \
                __used = NULL

#define ELFLIST_BEGIN(ptype, listname) ((ptype **)&__elflist_##listname##_begin)
#define ELFLIST_END(ptype, listname)   ((ptype **)&__elflist_##listname##_end)
#define ELFLIST_FIRST(ptype, listname) (ELFLIST_BEGIN(ptype, listname) + 1)
#define ELFLIST_LAST(ptype, listname)  (ELFLIST_END(ptype, listname) - 1)
#define ELFLIST_COUNT(ptype, listname) \
        (ELFLIST_END(ptype, listname) - ELFLIST_FIRST(ptype, listname))
#define ELFLIST_FOREACH(ptype, listname, iterv)                                                \
        for ((iterv) = ELFLIST_FIRST(ptype, listname); (iterv) < ELFLIST_END(ptype, listname); \
             (iterv)++)

#endif /* _LIB_ELFLIST_H */
