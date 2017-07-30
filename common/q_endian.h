/* q_endian.h -- endianness detection / handling
 * $Id$
 *
 * Copyright (C) 1996-1997  Id Software, Inc.
 * Copyright (C) 2007-2016  O.Sezer <sezero@users.sourceforge.net>
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

#ifndef QENDIAN_H
#define QENDIAN_H

/* to detect byte order at runtime instead of compile time, define as 1 */
#undef ENDIAN_RUNTIME_DETECT
#define ENDIAN_RUNTIME_DETECT  0

/* try the system headers first: with BSD and derivatives,
 * sys/types.h can pull in the correct header and defs.  */
#if !defined __VBCC__
#include <sys/types.h>
#endif

#if !defined(BYTE_ORDER) /* include more if it didn't work */

# if defined(__linux__) || defined(__linux)
#  include <endian.h>
# elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || \
       defined(__FreeBSD_kernel__) /* Debian GNU/kFreeBSD */               || \
       (defined(__APPLE__) && defined(__MACH__)) /* Mac OS X */            || \
       defined(__DragonFly__)
#  include <machine/endian.h>
# elif defined(__sun) || defined(__svr4__)
#  include <sys/byteorder.h>
# elif defined(_AIX)
#  include <sys/machine.h>
# elif defined(sgi)
#  include <sys/endian.h>
# elif defined(__MINGW32__)
#  include <sys/param.h>
# elif defined(__DJGPP__)
#  include <machine/endian.h>
# elif defined(__EMX__)
#  include <machine/endian.h>
# elif defined(__OS2__) && defined(__WATCOMC__)
#  include <machine/endian.h>
# elif defined(__amigaos__) && defined(__NEWLIB__)
#  include <machine/endian.h>
# elif defined(__amigaos__) && defined(__CLIB2__)
#  include <unistd.h>
# endif

#endif /* endian includes */


/* alternative macro names: */
#if defined(__BYTE_ORDER) && !defined(BYTE_ORDER)
#define BYTE_ORDER  __BYTE_ORDER
#endif
#if defined(__LITTLE_ENDIAN) && !defined(LITTLE_ENDIAN)
#define LITTLE_ENDIAN __LITTLE_ENDIAN
#endif
#if defined(__BIG_ENDIAN) && !defined(BIG_ENDIAN)
#define BIG_ENDIAN __BIG_ENDIAN
#endif

/* new gcc and clang versions pre-define the following: */
#if defined(__BYTE_ORDER__) && !defined(BYTE_ORDER)
#define BYTE_ORDER  __BYTE_ORDER__
#endif
#if defined(__ORDER_LITTLE_ENDIAN__) && !defined(LITTLE_ENDIAN)
#define LITTLE_ENDIAN __ORDER_LITTLE_ENDIAN__
#endif
#if defined(__ORDER_BIG_ENDIAN__) && !defined(BIG_ENDIAN)
#define BIG_ENDIAN __ORDER_BIG_ENDIAN__
#endif

#if !defined(PDP_ENDIAN)
#if defined(__ORDER_PDP_ENDIAN__)
#define PDP_ENDIAN __ORDER_PDP_ENDIAN__
#elif defined(__PDP_ENDIAN)
#define PDP_ENDIAN __PDP_ENDIAN
#else
#define PDP_ENDIAN 3412
#endif
#endif /* NUXI endian -- unsupported */

#if defined(BYTE_ORDER) && defined(LITTLE_ENDIAN) && defined(BIG_ENDIAN)

# if (BYTE_ORDER != LITTLE_ENDIAN) && (BYTE_ORDER != BIG_ENDIAN)
# error "Unsupported endianness."
# endif

#else /* one of the definitions is mising. */

# undef BYTE_ORDER
# undef LITTLE_ENDIAN
# undef BIG_ENDIAN
# define LITTLE_ENDIAN 1234
# define BIG_ENDIAN 4321

#endif /* byte order defs */


#if !defined(BYTE_ORDER)	/* good assumptions: */

/* try the compiler-predefined endianness macros.
 * Ref.: https://sf.net/p/predef/wiki/Endianness/
 */
# if defined(__BIG_ENDIAN__) || defined(_BIG_ENDIAN)
#  define BYTE_ORDER BIG_ENDIAN

# elif defined(__ARMEB__) || defined(__THUMBEB__) || defined(__AARCH64EB__)	|| \
       defined(_MIPSEB) || defined(__MIPSEB) || defined(__MIPSEB__)
#  define BYTE_ORDER BIG_ENDIAN

# elif defined(__LITTLE_ENDIAN__) || defined(_LITTLE_ENDIAN)
#  define BYTE_ORDER LITTLE_ENDIAN

# elif defined(__ARMEL__) || defined(__THUMBEL__) || defined(__AARCH64EL__)	|| \
       defined(_MIPSEL) || defined(__MIPSEL) || defined(__MIPSEL__)
#  define BYTE_ORDER LITTLE_ENDIAN

/* assumptions based on OS and/or architecture macros. Some refs:
 * predef/other/endian.h from the boost library
 * http://labs.hoffmanlabs.com/node/544
 * https://blogs.msdn.com/larryosterman/archive/2005/06/07/426334.aspx
 */
# elif defined(__i386) || defined(__i386__) || defined(__386__) || defined(_M_IX86) || defined(__I386__)
#  define BYTE_ORDER LITTLE_ENDIAN /* any x86 */

# elif defined(__amd64) || defined(__x86_64__) || defined(_M_X64)
#  define BYTE_ORDER LITTLE_ENDIAN /* any x64 */

# elif defined(__mc68000__) || defined(__M68K__) || defined(__m68k__) || defined(__MC68K__)
#  define BYTE_ORDER BIG_ENDIAN  /* Motorola 68k */

# elif defined(__sun) || defined(__svr4__) /* solaris */
#   if defined(_LITTLE_ENDIAN)     /* x86 */
#     define BYTE_ORDER LITTLE_ENDIAN
#   elif defined(_BIG_ENDIAN)      /* sparc */
#     define BYTE_ORDER BIG_ENDIAN
#   endif

# elif defined(__hppa) || defined(__hppa__)
#  define BYTE_ORDER BIG_ENDIAN /* PARISC */

# elif defined(__sparc) || defined(__sparc__)
#  define BYTE_ORDER BIG_ENDIAN  /* SPARC -- cf. boost/predef/other/endian.h */

# elif defined(__DJGPP__) || defined(__MSDOS__) || defined(__DOS__) || defined(_DOS)
#  define BYTE_ORDER LITTLE_ENDIAN /* DOS */

# elif defined(__OS2__) || defined(__EMX__)
#  define BYTE_ORDER LITTLE_ENDIAN /* OS2 */

# elif defined(_WIN32) || defined(__WIN32__) || defined(_WIN64) || defined(__NT__) || defined(_Windows)
#  define BYTE_ORDER LITTLE_ENDIAN /* Windows */

# endif

#endif /* good assumptions */


#if !defined(BYTE_ORDER)	/* [un]safe assumptions: */

/* assumptions based on OS and/or architecture macros.
 * proceed carefully: many of these are bi-endian CPUs.
 */
# if defined (__ppc__) || defined(__POWERPC__) || defined(__PPC__)
#  define BYTE_ORDER BIG_ENDIAN

# elif (defined(__alpha__) || defined(__alpha))
#  define BYTE_ORDER LITTLE_ENDIAN

# elif defined(__mips__) || defined(__MIPS__)
#  define BYTE_ORDER BIG_ENDIAN

# endif

# if defined(BYTE_ORDER) && 0 /* change to 1 to for warnings */
#  if (BYTE_ORDER == LITTLE_ENDIAN)
#   warning "Using LIL endian as a SAFE default."
#  else /*(BYTE_ORDER == BIG_ENDIAN)*/
#   warning "Using BIG endian as a SAFE default."
#  endif
# endif

#endif /* [un]safe assumptions */


#if !ENDIAN_RUNTIME_DETECT

#if !defined(BYTE_ORDER)
# error Could not determine BYTE_ORDER
#endif

/* for autotools compatibility */
#if (BYTE_ORDER == BIG_ENDIAN) && !defined(WORDS_BIGENDIAN)
# define WORDS_BIGENDIAN 1
#endif
#if (BYTE_ORDER != BIG_ENDIAN) && defined(WORDS_BIGENDIAN)
#error WORDS_BIGENDIAN defined for non-BIG_ENDIAN
#endif

#endif /* ! ENDIAN_RUNTIME_DETECT */

#ifdef __cplusplus
extern "C" {
#endif

extern int host_byteorder;
extern int host_bigendian; /* bool */

extern int DetectByteorder (void);
extern void ByteOrder_Init (void);

/* byte swapping. most times we want to convert to
 * little endian: our data files are written in LE
 * format.  sometimes, such as when writing to net,
 * we also convert to big endian.
 */
#if ENDIAN_RUNTIME_DETECT

extern short (*BigShort) (short);
extern short (*LittleShort) (short);
extern int   (*BigLong) (int);
extern int   (*LittleLong) (int);
extern float (*BigFloat) (float);
extern float (*LittleFloat) (float);

#else /* ! ENDIAN_RUNTIME_DETECT */

extern short   ShortSwap (short);
extern int     LongSwap (int);
extern float   FloatSwap (float);

#if defined(__VBCC__) && defined(__M68K__)
int m68k_swap32(__reg("d0") int) =
    "\trol.w\t#8,d0\n"
    "\tswap\td0\n"
    "\trol.w\t#8,d0";
short m68k_swap16(__reg("d0") short) =
    "\trol.w\t#8,d0";
#define LongSwap(s) m68k_swap32((s))
#define ShortSwap(s) m68k_swap16((s))
float m68k_swap32f(__reg("fp0") float) =
    "\tfmove.s\tfp0,d0\n"
    "\trol.w\t#8,d0\n"
    "\tswap\td0\n"
    "\trol.w\t#8,d0\n"
    "\tfmove.s\td0,fp0";
#define FloatSwap(s) m68k_swap32f((s))
#endif

#if (BYTE_ORDER == BIG_ENDIAN)

#define BigShort(s)    (s)
#define LittleShort(s) ShortSwap((s))
#define BigLong(l)     (l)
#define LittleLong(l)  LongSwap((l))
#define BigFloat(f)    (f)
#define LittleFloat(f) FloatSwap((f))

#else /* BYTE_ORDER == LITTLE_ENDIAN */

#define BigShort(s)    ShortSwap((s))
#define LittleShort(s) (s)
#define BigLong(l)     LongSwap((l))
#define LittleLong(l)  (l)
#define BigFloat(f)    FloatSwap((f))
#define LittleFloat(f) (f)

#endif /* swap macros */

#endif /* runtime det */

#ifdef __cplusplus
}
#endif /* */

#endif /* QENDIAN_H */
