/*
	q_endian.h
	endianness handling

	$Id: q_endian.h,v 1.1 2007-02-12 16:54:49 sezero Exp $
*/

#ifndef __QENDIAN_H
#define __QENDIAN_H

#include <sys/types.h>

extern short	ShortSwap (short);
extern int	LongSwap (int);
extern float	FloatSwap (float);

/*
 * endianness stuff: <sys/types.h> is supposed
 * to succeed in locating the correct endian.h
 * this BSD style may not work everywhere, eg. on WIN32
 */

#if !defined(BYTE_ORDER) || !defined(LITTLE_ENDIAN) || !defined(BIG_ENDIAN) || (BYTE_ORDER != LITTLE_ENDIAN && BYTE_ORDER != BIG_ENDIAN)

# undef BYTE_ORDER
# undef LITTLE_ENDIAN
# undef BIG_ENDIAN
# define LITTLE_ENDIAN	1234
# define BIG_ENDIAN	4321

#endif

// assumptions in case we don't have endianness info
#ifndef BYTE_ORDER
# if defined(_WIN32)
#    define BYTE_ORDER LITTLE_ENDIAN
# elif defined(SUNOS) /* these bits from darkplaces project */
#    define GUESSED_SUNOS_ENDIANNESS
#    if defined(__i386) || defined(__amd64)
#	define BYTE_ORDER LITTLE_ENDIAN
#    else
#	define BYTE_ORDER BIG_ENDIAN
#    endif	/* end of SUNOS */
# else
#    define ASSUMED_LITTLE_ENDIAN
#    define BYTE_ORDER LITTLE_ENDIAN
# endif
#endif

#if BYTE_ORDER == BIG_ENDIAN

#define BigShort(s) (s)
#define LittleShort(s) ShortSwap((s))
#define BigLong(l) (l)
#define LittleLong(l) LongSwap((l))
#define BigFloat(f) (f)
#define LittleFloat(f) FloatSwap((f))

#else

/* BYTE_ORDER == LITTLE_ENDIAN */
#define BigShort(s) ShortSwap((s))
#define LittleShort(s) (s)
#define BigLong(l) LongSwap((l))
#define LittleLong(l) (l)
#define BigFloat(f) FloatSwap((f))
#define LittleFloat(f) (f)

#endif

#endif	/* __QENDIAN_H */

