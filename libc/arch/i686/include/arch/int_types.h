#pragma once

/* This file contains Exact-width integers for i686 */

#ifdef __i686__

/* Exact-width integer types */
typedef signed char       int8_t ;
typedef signed short int int16_t ;
typedef signed int       int32_t ;
typedef signed long      int64_t ;

typedef unsigned char       uint8_t ;
typedef unsigned short int uint16_t ;
typedef unsigned int       uint32_t ;
typedef unsigned long      uint64_t ;

/* Limits of exact-width integer types */
#define INT8_MAX (0x7f)
#define INT16_MAX (0x7fff)
#define INT32_MAX (0x7fffffff)
#define INT64_MAX (0x7fffffffffffffff)

#define INT8_MIN  (-1-INT8_MAX)
#define INT16_MIN (-1-INT16_MAX)
#define INT32_MIN (-1-INT32_MAX)
#define INT64_MIN (-1-INT64_MAX)

#define UINT8_MAX  (0x7fu)
#define UINT16_MAX (0x7fffu)
#define UINT32_MAX (0x7fffffffu)
#define UINT64_MAX (0x7fffffffffffffffu)

#endif // __i686__
