/* Hacked from DJGPP v2.04 for use with older DJGPP versions */

/* Copyright (C) 2013 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
#ifndef __dj_stdint__h_
#define __dj_stdint__h_

typedef signed char int_least8_t;
typedef unsigned char uint_least8_t;
typedef signed char int_fast8_t;
typedef unsigned char uint_fast8_t;
typedef signed char int8_t;
typedef unsigned char uint8_t;

typedef signed short int int_least16_t;
typedef unsigned short int uint_least16_t;
typedef signed int int_fast16_t;
typedef unsigned int uint_fast16_t;
typedef signed short int int16_t;
typedef unsigned short int uint16_t;

typedef signed int int_least32_t;
typedef unsigned int uint_least32_t;
typedef signed int int_fast32_t;
typedef unsigned int uint_fast32_t;
typedef signed int int32_t;
typedef unsigned int uint32_t;

__extension__ typedef signed long long int int_least64_t;
__extension__ typedef unsigned long long int uint_least64_t;
__extension__ typedef signed long long int int_fast64_t;
__extension__ typedef unsigned long long int uint_fast64_t;
__extension__ typedef signed long long int int64_t;
__extension__ typedef unsigned long long int uint64_t;

typedef long int intptr_t;
typedef unsigned long int uintptr_t;

__extension__ typedef signed long long int intmax_t;
__extension__ typedef unsigned long long int uintmax_t;

/* ANSI/ISO C99 says these should not be visible in C++ unless
   explicitly requested.  */

#if !defined(__cplusplus) || defined(__STDC_LIMIT_MACROS)

#define INT_LEAST8_MAX	 127
#define UINT_LEAST8_MAX	 255
#define INT_FAST8_MAX	 127
#define UINT_FAST8_MAX	 255
#define INT8_MAX	 127
#define UINT8_MAX	 255 
#define INT_LEAST8_MIN	 (-128)
#define INT_FAST8_MIN	 (-128)
#define INT8_MIN	 (-128)

#define INT_LEAST16_MAX	 32767
#define UINT_LEAST16_MAX 65535
#define INT_FAST16_MAX	 2147483647
#define UINT_FAST16_MAX	 4294967295U
#define INT16_MAX	 32767
#define UINT16_MAX	 65535
#define INT_LEAST16_MIN	 (-32768)
#define INT_FAST16_MIN	 (-2147483647-1)
#define INT16_MIN	 (-32768)

#define INT_LEAST32_MAX	 2147483647
#define UINT_LEAST32_MAX 4294967295U
#define INT_FAST32_MAX	 2147483647
#define UINT_FAST32_MAX	 4294967295U
#define INT32_MAX	 2147483647L
#define UINT32_MAX	 4294967295UL
#define INT_LEAST32_MIN	 (-2147483647-1)
#define INT_FAST32_MIN	 (-2147483647-1)
#define INT32_MIN	 (-2147483647L-1)

#define INT_LEAST64_MAX	 9223372036854775807LL
#define UINT_LEAST64_MAX 18446744073709551615ULL
#define INT_FAST64_MAX	 9223372036854775807LL
#define UINT_FAST64_MAX	 18446744073709551615ULL
#define INT64_MAX	 9223372036854775807LL
#define UINT64_MAX	 18446744073709551615ULL
#define INT_LEAST64_MIN	 (-9223372036854775807LL-1LL)
#define INT_FAST64_MIN	 (-9223372036854775807LL-1LL)
#define INT64_MIN	 (-9223372036854775807LL-1LL)

#define INTPTR_MAX	2147483647L
#define INTPTR_MIN	(-2147483647L-1L)
#define UINTPTR_MAX	0xffffffffUL

#define INTMAX_MAX	9223372036854775807LL
#define UINTMAX_MAX	18446744073709551615ULL
#define INTMAX_MIN	(-9223372036854775807LL-1LL)

#define PTRDIFF_MAX	2147483647
#define PTRDIFF_MIN	(-2147483647-1)
#define SIG_ATOMIC_MAX	2147483647
#define SIG_ATOMIC_MIN	(-2147483647-1)
#define SIZE_MAX	4294967295U

  /* These are defined by limits.h, so make them conditional.  */
#ifndef WCHAR_MAX
#define WCHAR_MAX	65535
#endif
#ifndef WCHAR_MIN
#define WCHAR_MIN	0
#endif
#ifndef WINT_MAX
#define WINT_MAX	2147483647
#endif
#ifndef WINT_MIN
#define WINT_MIN	(-2147483647-1)
#endif

#endif /* !__cplusplus || __STDC_LIMIT_MACROS */


#if !defined(__cplusplus) || defined(__STDC_CONSTANT_MACROS)

#define INT8_C(x)	((int8_t)x)
#define UINT8_C(x)	((uint8_t)x ## U)
#define INT16_C(x)	((int16_t)x)
#define UINT16_C(x)	((uint16_t)x ## U)
#define INT32_C(x)	x ## L
#define UINT32_C(x)	x ## UL
#define INT64_C(x)	x ## LL
#define UINT64_C(x)	x ## ULL

#define INTMAX_C(x)	x ## LL
#define UINTMAX_C(x)	x ## ULL

#endif /* !__cplusplus || __STDC_CONSTANT_MACROS */

#endif /* !__dj_stdint__h_ */
