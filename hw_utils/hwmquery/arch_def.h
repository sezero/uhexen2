/*
	arch_def.h
	platform specific definitions

	$Id: arch_def.h,v 1.6 2007-10-21 15:25:23 sezero Exp $
*/

#ifndef __HX2_ARCHDEFS__
#define __HX2_ARCHDEFS__


#if defined(__APPLE__) && defined(__MACH__)
#   if !defined(__MACOSX__)
#	define	__MACOSX__		1
#   endif
#elif defined(macintosh) /* MacOS Classic */
#   if !defined(__MACOS__)
#	define	__MACOS__		1
#   endif
#elif defined(__sun) && (defined(__svr4__) || defined(__SVR4))
#   if !defined(__SOLARIS__)
#	define	__SOLARIS__		1
#   endif
#elif defined(__QNXNTO__)
#   if !defined(__QNX__)
#	define	__QNX__			1
#   endif
#elif defined(__amigados__) || defined(__amigaos4__) || \
		defined(__AMIGA) || defined(__amigaos__)
#   if !defined(__AMIGA__)
#	define	__AMIGA__		1
#   endif
#endif	/* end of custom definitions	*/


#if defined(__DJGPP__) || defined(MSDOS) || defined(__MSDOS__)

#   if !defined(PLATFORM_DOS)
#	define	PLATFORM_DOS		1
#   endif

#elif defined(_WIN32) || defined(_WIN64)

#   if !defined(PLATFORM_WINDOWS)
#	define	PLATFORM_WINDOWS	1
#   endif

#elif defined(__MACOS__) || defined(__MACOSX__)

#   if !defined(PLATFORM_MAC)
#	define	PLATFORM_MAC		1
#   endif

#elif defined(__MORPHOS__) || defined(__AMIGA__)
/* FIXME: Since these aren't classified as PLATFORM_UNIX,
   several things may well be broken in all of the uhexen2
   tree. Particularly, search for __MORPHOS__ ifdefs... */
#   if !defined(PLATFORM_AMIGA)
#	define	PLATFORM_AMIGA		1
#   endif

#else	/* here goes the unix platforms */

#if defined(__unix) || defined(__unix__) || defined(unix)	|| \
    defined(__linux__) || defined(__linux)			|| \
    defined(__FreeBSD__) || defined(__DragonFly__)		|| \
    defined(__OpenBSD__) || defined(__NetBSD__)			|| \
    defined(__hpux) || defined(__hpux__)			|| \
    defined(__SOLARIS__) || defined(__QNX__)
#   if !defined(PLATFORM_UNIX)
#	define	PLATFORM_UNIX		1
#   endif
#endif

#endif	/* end of PLATFORM_ definitions */


/* Hack section: Platforms that are mostly fine when
 * classified under PLATFORM_UNIX :
 */
#if defined(__MACOSX__)

#   if !defined(PLATFORM_UNIX)
#	define	PLATFORM_UNIX		2
#   endif

#endif	/* end of PLATFORM_UNIX hacks */


#if defined(_WIN64)
#	define	PLATFORM_STRING	"Win64"
#elif defined(_WIN32)
#	define	PLATFORM_STRING	"Windows"
#elif defined(PLATFORM_DOS)
#	define	PLATFORM_STRING	"DOS"
#elif defined(__linux__) || defined(__linux)
#	define	PLATFORM_STRING	"Linux"
#elif defined(__DragonFly__)
#	define	PLATFORM_STRING "DragonFly"
#elif defined(__FreeBSD__)
#	define	PLATFORM_STRING	"FreeBSD"
#elif defined(__NetBSD__)
#	define	PLATFORM_STRING	"NetBSD"
#elif defined(__OpenBSD__)
#	define	PLATFORM_STRING	"OpenBSD"
#elif defined(__MORPHOS__)
#	define	PLATFORM_STRING	"MorphOS"
#elif defined(__AMIGA__)
#	define	PLATFORM_STRING	"AmigaOS"
#elif defined(__QNX__)
#	define	PLATFORM_STRING	"QNX"
#elif defined(__MACOSX__)
#	define	PLATFORM_STRING	"MacOSX"
#elif defined(__MACOS__)
#	define	PLATFORM_STRING	"MacOS"
#elif defined(__hpux) || defined(__hpux__)
#	define	PLATFORM_STRING	"HP-UX"
#elif defined(__SOLARIS__)
#	define	PLATFORM_STRING	"Solaris"
#elif defined(PLATFORM_UNIX)
#	define	PLATFORM_STRING	"Unix"
#else
#	define	PLATFORM_STRING	"Unknown"
#	warning "Platform is UNKNOWN."
#endif	/* end of PLATFORM_STRING definitions */

#endif	/* __HX2_ARCHDEFS__ */

