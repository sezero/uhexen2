/*
	q_types.h
	common type definitions

	$Id: q_types.h,v 1.3 2007-07-08 11:55:38 sezero Exp $
*/

#ifndef __QTYPES_H
#define __QTYPES_H

#include <sys/types.h>
#include <stdint.h>


/*==========================================================================*/

#ifndef NULL
#define NULL ((void *)0)
#endif

#define Q_MAXCHAR ((char)0x7f)
#define Q_MAXSHORT ((short)0x7fff)
#define Q_MAXINT	((int)0x7fffffff)
#define Q_MAXLONG ((int)0x7fffffff)
#define Q_MAXFLOAT ((int)0x7fffffff)

#define Q_MINCHAR ((char)0x80)
#define Q_MINSHORT ((short)0x8000)
#define Q_MININT 	((int)0x80000000)
#define Q_MINLONG ((int)0x80000000)
#define Q_MINFLOAT ((int)0x7fffffff)


/*==========================================================================*/

typedef unsigned char byte;
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

// math
typedef float	vec_t;
typedef vec_t	vec3_t[3];
typedef vec_t	vec5_t[5];
typedef	int	fixed4_t;
typedef int	fixed8_t;
typedef	int	fixed16_t;


/*==========================================================================*/

#if !defined(_WIN32)
/* compatibility with M$ types */
//#define _inline inline
//#define __inline static inline
#define HANDLE int
#define HINST int
#define HWND int
#define APIENTRY
#define PASCAL
#define FAR
#define SOCKET int
#define LPWSADATA int
#define BOOL int
#define UINT unsigned int
#define WORD unsigned short
#define DWORD unsigned long
#define LONG long
#define LONGLONG long long

typedef void *PVOID,*LPVOID;

typedef struct RECT_s {
	int left;
	int right;
	int top;
	int bottom;
} RECT;

typedef union _LARGE_INTEGER {
	struct {
		DWORD LowPart;
		LONG  HighPart;
	} part;
	LONGLONG QuadPart;
} LARGE_INTEGER, *PLARGE_INTEGER;

#endif	/* !_WIN32 */

/*==========================================================================*/


#endif	/* __QTYPES_H */

