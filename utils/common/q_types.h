/*
	q_types.h
	common type definitions

	$Id: q_types.h,v 1.3 2007-07-11 16:47:20 sezero Exp $
*/

#ifndef __QTYPES_H
#define __QTYPES_H

#include <sys/types.h>
#include <stdint.h>
/* FIXME: on some OSs, only inttypes.h is available. */


/*==========================================================================*/

#ifndef NULL
#define	NULL		((void *)0)
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

