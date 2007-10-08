/*
	q_types.h
	common type definitions

	$Id: q_types.h,v 1.5 2007-10-08 20:20:11 sezero Exp $
*/

#ifndef __QTYPES_H
#define __QTYPES_H

#include <sys/types.h>

/* NOTES on TYPE SIZES:
   Quake/Hexen II engine relied on 32 bit int type size
   with ILP32 (not LP32) model in mind.  We now support
   LP64 and LLP64, too. We expect:
   sizeof (char)	== 1
   sizeof (short)	== 2
   sizeof (int)		== 4
   sizeof (float)	== 4
   sizeof (long)	== 4/8
   sizeof (*ptr)	== 4/8
   For this, we need the stdint.h or inttypes.h header.
   FIXME: On some platforms, only inttypes.h is available.
   FIXME: Properly replace certain short and int usage
	  with int16_t and int32_t.
 */
#include <stdint.h>


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


/*==========================================================================*/

typedef unsigned char		byte;
#define _DEF_BYTE_

#undef true
#undef false
#ifdef __cplusplus
typedef bool	qboolean;
#else
typedef	enum	{false, true} qboolean;
#endif
#define _DEF_BOOL_


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
#if !defined(_WIN32) && !defined(_WIN64)

#define	APIENTRY
#define	PASCAL
#define	FAR
typedef void *PVOID,*LPVOID;

#endif	/* ! WINDOWS */

/*==========================================================================*/


#endif	/* __QTYPES_H */

