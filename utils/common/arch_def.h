/*
	arch_def.h
	platform specific definitions

	$Id: arch_def.h,v 1.1 2007-07-11 16:47:20 sezero Exp $
*/

#ifndef __HX2_ARCHDEFS__
#define __HX2_ARCHDEFS__


#if defined(__APPLE__)
#	undef __MACOSX__
#	define __MACOSX__		1
#elif defined(macintosh)
#	undef __MACOS__
#	define __MACOS__		1
#elif defined(__sun) && !defined(__SUNOS__)
#	define __SUNOS__		1
#elif defined(__QNXNTO__) && !defined(__QNX__)
#	define __QNX__			1
#endif	/* end of custom definitions	*/


#if defined(__DOS__) || defined(__DJGPP__)

#   if !defined(PLATFORM_DOS)
#	define	PLATFORM_DOS		1
#   endif

#elif defined(_WIN32) || defined(_WIN64)

#   if !defined(PLATFORM_WINDOWS)
#	define	PLATFORM_WINDOWS	1
#   endif

#elif defined(__MACOS__)
/* MacOS X is mostly fine as PLATFORM_UNIX
   but I'm not sure about MacOS Classic. */
#   if !defined(PLATFORM_MAC)
#	define	PLATFORM_MAC		1
#   endif

#else	/* here goes the unix platforms */

#if defined(__unix) || defined(__unix__) || defined(unix)	|| \
    defined(__linux__) || defined(__linux)			|| \
    defined(__FreeBSD__) || defined(__DragonFly__)		|| \
    defined(__OpenBSD__) || defined(__NetBSD__)			|| \
    defined(__MORPHOS__) || defined(__QNX__)			|| \
    defined(__hpux) || defined(__hpux__)			|| \
    defined(__svr4) || defined(__SVR4) || defined(__SUNOS__)	|| \
    defined(__MACOSX__)
#   if !defined(PLATFORM_UNIX)
#	define	PLATFORM_UNIX		1
#   endif
#endif

#endif	/* end of PLATFORM_ definitions */


#if defined(_WIN64)
#	define	PLATFORM_STRING	"Windows64"
#elif defined(_WIN32)
#	define	PLATFORM_STRING	"Windows"
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
#elif defined(__QNX__)
#	define	PLATFORM_STRING	"QNX"
#elif defined(__MACOSX__)
#	define	PLATFORM_STRING	"MacOSX"
#elif defined(__MACOS__)
#	define	PLATFORM_STRING	"MacOS"
#elif defined(__hpux) || defined(__hpux__)
#	define	PLATFORM_STRING	"HP-UX"
#elif defined(PLATFORM_UNIX)
#	define	PLATFORM_STRING	"Unix"
#else
#	define	PLATFORM_STRING	"Unknown"
#	warning "Platform is UNKNOWN."
#endif	/* end of PLATFORM_STRING definitions */

#endif	/* __HX2_ARCHDEFS__ */

