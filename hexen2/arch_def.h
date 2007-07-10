/*
	arch_def.h
	platform specific definitions

	$Id: arch_def.h,v 1.6 2007-07-10 18:48:17 sezero Exp $
*/

#ifndef __HX2_ARCHDEFS__
#define __HX2_ARCHDEFS__

#if defined (__APPLE__)
#	undef __MACOSX__
#	define __MACOSX__	1
#elif defined (macintosh)
#	undef __MACOS__
#	define __MACOS__	1
#elif defined (__QNX__) || defined (__QNXNTO__)
#	undef __QNX__
#	define __QNX__		1
#endif

#if defined (__sun) && !defined(__SUNOS__)
#	define __SUNOS__	1
#endif

#if defined (PLATFORM_UNIX)
#	if defined (__linux__)
#		define VERSION_PLATFORM "Linux"
#	elif defined (__DragonFly__)
#		define VERSION_PLATFORM "DragonFly"
#	elif defined (__FreeBSD__)
#		define VERSION_PLATFORM "FreeBSD"
#	elif defined (__NetBSD__)
#		define VERSION_PLATFORM "NetBSD"
#	elif defined (__OpenBSD__)
#		define VERSION_PLATFORM "OpenBSD"
#	elif defined (__MORPHOS__)
#		define VERSION_PLATFORM "MorphOS"
#	elif defined (__QNX__)
#		define VERSION_PLATFORM "QNX"
#	elif defined (__MACOSX__)
#		define VERSION_PLATFORM "MacOSX"
#	elif defined (__MACOS__)
#		define VERSION_PLATFORM "MacOS"
#	else
#		define VERSION_PLATFORM "Unix"
#	endif
#elif defined (_WIN32)
#	define VERSION_PLATFORM "Windows"
#else
#	define VERSION_PLATFORM "Unknown"
#	warning "Platform is UNKNOWN"
#endif

#endif	/* __HX2_ARCHDEFS__ */

