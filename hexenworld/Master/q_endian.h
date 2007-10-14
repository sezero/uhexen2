/*
	q_endian.h
	endianness handling

	$Id: q_endian.h,v 1.6 2007-10-14 14:01:45 sezero Exp $
*/

#ifndef __QENDIAN_H
#define __QENDIAN_H

#include <sys/types.h>

extern int DetectByteorder (void);

extern short	ShortSwap (short);
extern int	LongSwap (int);
extern float	FloatSwap (float);

extern int	LongSwapPDP2BE (int);
extern int	LongSwapPDP2LE (int);
extern float	FloatSwapPDP2BE (float);
extern float	FloatSwapPDP2LE (float);

/*
 * endianness stuff: <sys/types.h> is supposed
 * to succeed in locating the correct endian.h
 * this BSD style may not work everywhere.
 */

#undef ASSUMED_BIG_ENDIAN
#undef ASSUMED_LITTLE_ENDIAN
#undef ASSUMED_PDP_ENDIAN

#if !defined(PDP_ENDIAN)
#if defined(__PDP_ENDIAN)
#define	PDP_ENDIAN	__PDP_ENDIAN
#else
#define	PDP_ENDIAN	3412
#endif
#endif	/* the NUXI endian, not supported actually.. */

#if defined(__BYTE_ORDER) && !defined(BYTE_ORDER)
#define	BYTE_ORDER	__BYTE_ORDER
#endif	/* __BYTE_ORDER */

#if defined(__LITTLE_ENDIAN) && !defined(LITTLE_ENDIAN)
#define	LITTLE_ENDIAN	__LITTLE_ENDIAN
#endif	/* __LITTLE_ENDIAN */

#if defined(__BIG_ENDIAN) && !defined(BIG_ENDIAN)
#define	BIG_ENDIAN	__BIG_ENDIAN
#endif	/* __LITTLE_ENDIAN */

#if defined(BYTE_ORDER) && defined(LITTLE_ENDIAN) && defined(BIG_ENDIAN)

# if (BYTE_ORDER != LITTLE_ENDIAN) && (BYTE_ORDER != BIG_ENDIAN) && (BYTE_ORDER != PDP_ENDIAN)
# error "Unsupported endianness."
# endif

#else	/* one of the definitions is mising. */

# undef BYTE_ORDER
# undef LITTLE_ENDIAN
# undef BIG_ENDIAN
# undef PDP_ENDIAN
# define LITTLE_ENDIAN	1234
# define BIG_ENDIAN	4321
# define PDP_ENDIAN	3412

#endif	/* byte order defs */

#if !defined(BYTE_ORDER)
/* assumptions in case we have no endianness
   info. partially from older SDL headers. */
# define FALLBACK_ORDER	LITTLE_ENDIAN

# if defined(__hppa__) || defined(__sparc__) || defined (__ppc__) || defined(__POWERPC__) || defined(_M_PPC)
#	define	BYTE_ORDER	BIG_ENDIAN
# elif (defined(__i386) || defined(__i386__)) || defined(__amd64) || defined(__ia64__) || defined(__x86_64__)
#	define	BYTE_ORDER	LITTLE_ENDIAN
# elif (defined(__alpha__) || defined(__alpha))
#	define	BYTE_ORDER	LITTLE_ENDIAN
# elif defined(_WIN32) || defined(_WIN64) || defined(__DJGPP__) || defined(__DOS__)
#	define	BYTE_ORDER	LITTLE_ENDIAN
# else
#	/* fallback: caution recommended!! */
#	define	BYTE_ORDER	FALLBACK_ORDER
#	if  (FALLBACK_ORDER == BIG_ENDIAN)
#		define	ASSUMED_BIG_ENDIAN
#	elif (FALLBACK_ORDER == PDP_ENDIAN)
#		define	ASSUMED_PDP_ENDIAN
#	else
#		define	ASSUMED_LITTLE_ENDIAN
#	endif
# endif
#endif	/* BYTE_ORDER */

#if (BYTE_ORDER == BIG_ENDIAN)

#define BigShort(s)	(s)
#define LittleShort(s)	ShortSwap((s))
#define BigLong(l)	(l)
#define LittleLong(l)	LongSwap((l))
#define BigFloat(f)	(f)
#define LittleFloat(f)	FloatSwap((f))

#elif (BYTE_ORDER == PDP_ENDIAN)

#define BigShort(s)	ShortSwap((s))
#define BigLong(l)	LongSwapPDP2BE((l))
#define BigFloat(f)	FloatSwapPDP2BE((f))
#define LittleShort(s)	(s)
#define LittleLong(l)	LongSwapPDP2LE((l))
#define LittleFloat(f)	FloatSwapPDP2LE((f))

#else /* BYTE_ORDER == LITTLE_ENDIAN */

#define BigShort(s)	ShortSwap((s))
#define LittleShort(s)	(s)
#define BigLong(l)	LongSwap((l))
#define LittleLong(l)	(l)
#define BigFloat(f)	FloatSwap((f))
#define LittleFloat(f)	(f)

#endif

#endif	/* __QENDIAN_H */

