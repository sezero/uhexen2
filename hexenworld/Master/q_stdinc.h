/*
	q_stdinc.h
	includes the minimum necessary stdc headers,
	defines common and / or missing types.
	NOTE:	for net stuff use net_sys.h,
		for byte order use q_endian.h,
		for math stuff use mathlib.h.

	$Id: q_stdinc.h,v 1.3 2008-01-10 21:25:08 sezero Exp $

	Copyright (C) 1996-1997  Id Software, Inc.
	Copyright (C) 2007-2008  O.Sezer <sezero@users.sourceforge.net>

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

	See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to:

		Free Software Foundation, Inc.
		51 Franklin St, Fifth Floor,
		Boston, MA  02110-1301  USA
*/

#ifndef __QSTDINC_H
#define __QSTDINC_H

#include <sys/types.h>
#include <stddef.h>
#include <limits.h>

/* NOTES on TYPE SIZES:
   Quake/Hexen II engine relied on 32 bit int type size
   with ILP32 (not LP32) model in mind.  We now support
   LP64 and LLP64, too. We expect:
   sizeof (char)	== 1
   sizeof (short)	== 2
   sizeof (int)		== 4
   sizeof (float)	== 4
   sizeof (long)	== 4 / 8
   sizeof (pointer *)	== 4 / 8
   For this, we need stdint.h (or inttypes.h)
   FIXME: On some platforms, only inttypes.h is available.
   FIXME: Properly replace certain short and int usage
	  with int16_t and int32_t.
 */
#if defined(_MSC_VER)
#include "msinttypes/stdint.h"
#else	/* not M$ compiler: */
#include <stdint.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#if !(defined(_WIN32) || defined(_WIN64))
#include <strings.h>	/* strcasecmp and strncasecmp	*/
#endif	/* ! PLATFORM_WINDOWS */

/*==========================================================================*/

#ifndef NULL
#if defined(__cplusplus)
#define	NULL		0
#else
#define	NULL		((void *)0)
#endif
#endif

#define	Q_MAXCHAR	((char)0x7f)
#define	Q_MAXSHORT	((short)0x7fff)
#define	Q_MAXINT	((int)0x7fffffff)
#define	Q_MAXLONG	((int)0x7fffffff)
#define	Q_MAXFLOAT	((int)0x7fffffff)

#define	Q_MINCHAR	((char)0x80)
#define	Q_MINSHORT	((short)0x8000)
#define	Q_MININT	((int)0x80000000)
#define	Q_MINLONG	((int)0x80000000)
#define	Q_MINFLOAT	((int)0x7fffffff)

/* Make sure the types really have the right
 * sizes: These macros are from SDL headers.
 */
#define	COMPILE_TIME_ASSERT(name, x)	\
	typedef int dummy_ ## name[(x) * 2 - 1]

COMPILE_TIME_ASSERT(char, sizeof(char) == 1);
COMPILE_TIME_ASSERT(float, sizeof(float) == 4);
COMPILE_TIME_ASSERT(long, sizeof(long) >= 4);
COMPILE_TIME_ASSERT(int, sizeof(int) == 4);
COMPILE_TIME_ASSERT(short, sizeof(short) == 2);


/*==========================================================================*/

typedef unsigned char		byte;

#undef true
#undef false
#if defined(__cplusplus)
typedef bool	qboolean;
#else
typedef	enum	{false, true} qboolean;
#endif


/*==========================================================================*/

/* math */
#if defined(DOUBLEVEC_T)
typedef double	vec_t;
#else	/* float type */
typedef float	vec_t;
#endif	/* DOUBLEVEC_T */
typedef vec_t	vec3_t[3];
typedef vec_t	vec5_t[5];
typedef int	fixed4_t;
typedef int	fixed8_t;
typedef int	fixed16_t;


/*==========================================================================*/

/* compatibility with M$ types */
#if !(defined(_WIN32) || defined(_WIN64))

#define	PASCAL
#define	FAR
#define	APIENTRY

#endif	/* ! WINDOWS */

/*==========================================================================*/


#endif	/* __QSTDINC_H */

