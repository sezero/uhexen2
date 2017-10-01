/*
 * compiler.h
 * compiler specific definitions and settings
 * used in the uhexen2 (Hammer of Thyrion) tree.
 * - standalone header
 * - doesn't and must not include any other headers
 * - shouldn't depend on arch_def.h, q_stdinc.h, or
 *   any other headers
 *
 * $Id$
 *
 * Copyright (C) 2007-2011  O.Sezer <sezero@users.sourceforge.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef HX2_COMPILER_H
#define HX2_COMPILER_H

#if defined(__GNUC__)
#define FUNC_PRINTF(x,y)	__attribute__((__format__(__printf__,x,y)))
#else
#define FUNC_PRINTF(x,y)
#endif

/* argument format attributes for function pointers are supported for gcc >= 3.1 */
#if defined(__GNUC__) && (__GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ > 0))
#define FUNCP_PRINTF	FUNC_PRINTF
#else
#define FUNCP_PRINTF(x,y)
#endif

/* llvm's optnone function attribute started with clang-3.5.0 */
#if defined(__clang__) && \
           (__clang_major__ > 3 || (__clang_major__ == 3 && __clang_minor__ >= 5))
#define FUNC_NO_OPTIMIZE	__attribute__((__optnone__))
/* function optimize attribute is added starting with gcc 4.4.0 */
#elif defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ > 3))
#define FUNC_NO_OPTIMIZE	__attribute__((__optimize__("0")))
#else
#define FUNC_NO_OPTIMIZE
#endif

#if defined(__GNUC__)
#define FUNC_NORETURN	__attribute__((__noreturn__))
#elif defined(_MSC_VER) && (_MSC_VER >= 1200)
#define FUNC_NORETURN		__declspec(noreturn)
#elif defined(__WATCOMC__)
#define FUNC_NORETURN /* use the 'aborts' aux pragma */
#else
#define FUNC_NORETURN
#endif

#if defined(__GNUC__) && ((__GNUC__ > 3) || (__GNUC__ == 3 && __GNUC_MINOR__ >= 1))
#define FUNC_NOINLINE	__attribute__((__noinline__))
#elif defined(_MSC_VER) && (_MSC_VER >= 1300)
#define FUNC_NOINLINE		__declspec(noinline)
#else
#define FUNC_NOINLINE
#endif

#if defined(__GNUC__) && ((__GNUC__ > 4) || (__GNUC__ == 4 && __GNUC_MINOR__ >= 5))
#define FUNC_NOCLONE	__attribute__((__noclone__))
#else
#define FUNC_NOCLONE
#endif

#if !(defined(__GNUC__) &&  (__GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 8)))
#define __extension__
#endif	/* __GNUC__ */

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#define	__thisfunc__	__func__
#elif defined(__GNUC__) && __GNUC__ < 3
#define	__thisfunc__	__FUNCTION__
#elif defined(__GNUC__) && __GNUC__ > 2
#define	__thisfunc__	__func__
#elif defined(__WATCOMC__)
#define	__thisfunc__	__FUNCTION__
#elif defined(__LCC__)
#define	__thisfunc__	__func__
#elif defined(_MSC_VER) && _MSC_VER >= 1300	/* VC7++ */
#define	__thisfunc__	__FUNCTION__
#else	/* stupid fallback */
/*#define	__thisfunc__	__FILE__*/
#error	__func__ or __FUNCTION__ compiler token not supported? define one...
#endif

/* Some compilers, such as OpenWatcom, and possibly other compilers
 * from the DOS universe, define __386__ but not __i386__
 */
#if defined(__386__) && !defined(__i386__)
#define __i386__		1
#endif

/* inline keyword: */
#if defined(_MSC_VER) && !defined(__cplusplus)
#define inline __inline
#endif	/* _MSC_VER */


#endif	/* HX2_COMPILER_H */

