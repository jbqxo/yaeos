#ifndef _LIB_SYNC_BARRIERS_H
#define _LIB_SYNC_BARRIERS_H

#define barrier_compiler() asm volatile ("" ::: "memory")

#endif /* _LIB_SYNC_BARRIERS_H */
