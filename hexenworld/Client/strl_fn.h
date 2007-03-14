/*
	strl_fn.h
	header file for BSD strlcat and strlcpy

	$Id: strl_fn.h,v 1.3 2007-03-14 21:03:42 sezero Exp $
*/

#ifndef __STRLFUNCS_H
#define __STRLFUNCS_H

// do we have strlcpy and strlcat as native library functions?

#if defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__MACOSX__) || defined(__DragonFly__)

#define HAVE_STRLCPY	1
#define HAVE_STRLCAT	1

#else

#define HAVE_STRLCPY	0
#define HAVE_STRLCAT	0

#endif

#endif	/* __STRLFUNCS_H */

