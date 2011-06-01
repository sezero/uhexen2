/*
	strl_fn.h
	header file for BSD strlcat and strlcpy

	$Id$
*/

#ifndef __STRLFUNCS_H
#define __STRLFUNCS_H

/* do we have strlcpy and strlcat as native library functions? */
#undef	HAVE_STRLCPY
#undef	HAVE_STRLCAT

#if defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__MACOSX__) || defined(__DragonFly__)

#define HAVE_STRLCPY	1
#define HAVE_STRLCAT	1

#else

#define HAVE_STRLCPY	0
#define HAVE_STRLCAT	0

#endif

#endif	/* __STRLFUNCS_H */

