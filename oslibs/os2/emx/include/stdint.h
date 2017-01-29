#ifndef STDINT_H
#define STDINT_H

#if defined(__INNOTEK_LIBC__) || defined(__KLIBC__)
#include_next <stdint.h>
#else

/* Exact-width types. */
typedef signed char        int8_t;
typedef unsigned char      uint8_t;
typedef short              int16_t;
typedef unsigned short     uint16_t;
typedef long               int32_t;
typedef unsigned long      uint32_t;
typedef long long          int64_t;
typedef unsigned long long uint64_t;

/* Minimum-width types. */
typedef signed   char      int_least8_t;
typedef signed   short     int_least16_t;
typedef signed   long      int_least32_t;
typedef signed   long long int_least64_t;

typedef unsigned char      uint_least8_t;
typedef unsigned short     uint_least16_t;
typedef unsigned long      uint_least32_t;
typedef unsigned long long uint_least64_t;

/* Fastest minimum-width types. */
typedef signed   int       int_fast8_t;
typedef signed   int       int_fast16_t;
typedef signed   long      int_fast32_t;
typedef signed   long long int_fast64_t;

typedef unsigned int       uint_fast8_t;
typedef unsigned int       uint_fast16_t;
typedef unsigned long      uint_fast32_t;
typedef unsigned long long uint_fast64_t;

/* Integer types able to hold *object* pointers. */
typedef long intptr_t;
typedef unsigned long uintptr_t;

/* Greatest-width types. */
typedef long long intmax_t;
typedef unsigned long long uintmax_t;

/* Limit macros */

/* Exact-width types. */
#define INT8_MIN   (-128)
#define INT8_MAX   127
#define INT16_MIN  (-32767-1)
#define INT16_MAX  32767
#define INT32_MIN  (-2147483647L-1)
#define INT32_MAX  2147483647L
#define INT64_MIN  (-9223372036854775807LL-1)
#define INT64_MAX  9223372036854775807LL

#define UINT8_MAX   255
#define UINT16_MAX  65535
#define UINT32_MAX  4294967295UL
#define UINT64_MAX  18446744073709551615ULL

/* Minimum-width types. */
#define INT_LEAST8_MIN   (-128)
#define INT_LEAST8_MAX   127
#define INT_LEAST16_MIN  (-32767-1)
#define INT_LEAST16_MAX  32767
#define INT_LEAST32_MIN  (-2147483647L-1)
#define INT_LEAST32_MAX  2147483647L
#define INT_LEAST64_MIN  (-9223372036854775807LL-1)
#define INT_LEAST64_MAX  9223372036854775807LL

#define UINT_LEAST8_MAX   255
#define UINT_LEAST16_MAX  65535
#define UINT_LEAST32_MAX  4294967295UL
#define UINT_LEAST64_MAX  18446744073709551615ULL

/* Fast minimum-width types. */
#define INT_FAST8_MIN   (-2147483647-1)
#define INT_FAST8_MAX   2147483647
#define INT_FAST16_MIN  (-2147483647-1)
#define INT_FAST16_MAX  2147483647
#define INT_FAST32_MIN  (-2147483647L-1)
#define INT_FAST32_MAX  2147483647L
#define INT_FAST64_MIN  (-9223372036854775807LL-1)
#define INT_FAST64_MAX  9223372036854775807LL

#define UINT_FAST8_MAX   4294967295U
#define UINT_FAST16_MAX  4294967295U
#define UINT_FAST32_MAX  4294967295UL
#define UINT_FAST64_MAX  18446744073709551615ULL

/* Integer types able to hold object pointers. */
#define INTPTR_MIN   (-2147483647L-1)
#define INTPTR_MAX   2147483647L
#define UINTPTR_MAX  4294967295UL

/* Greatest-width types. */
#define INTMAX_MIN   (-9223372036854775807LL-1)
#define INTMAX_MAX   9223372036854775807LL
#define UINTMAX_MAX  18446744073709551615ULL

/* Limits of wide character types */
#define WCHAR_MIN       0
#define WCHAR_MAX       65535U
#define WINT_MIN        WCHAR_MIN
#define WINT_MAX        WCHAR_MAX

/* Limits of other integer types. */
#define PTRDIFF_MIN     (-2147483647-1)
#define PTRDIFF_MAX     2147483647
#define SIG_ATOMIC_MIN  (-2147483647-1)
#define SIG_ATOMIC_MAX  2147483647
#define SIZE_MAX        4294967295U

/* Constant macros */

#define INT8_C(x)    (x)
#define INT16_C(x)   (x)
#define INT32_C(x)   (x##L)
#define INT64_C(x)   (x##LL)

#define UINT8_C(x)   (x)
#define UINT16_C(x)  (x)
#define UINT32_C(x)  (x##UL)
#define UINT64_C(x)  (x##ULL)

#define INTMAX_C(x)  (x##LL)
#define UINTMAX_C(x) (x##ULL)

#endif
#endif
