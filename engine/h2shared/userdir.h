/*
	userdir.h
	arch specific user directory definitions

	$Id$
*/

#ifndef __USERDIR_H
#define __USERDIR_H

#if defined(DEMOBUILD)
/* user a different user directory for the demo version
 * so that the demo and retail versions can co-exist on
 * the same machine peacefully */
#define	SYS_USERDIR_OSX		"Library/Application Support/Hexen2 Demo"
#define	SYS_USERDIR_UNIX	".hexen2demo"

#else	/* for retail version: */

#define	SYS_USERDIR_OSX		"Library/Application Support/Hexen2"
#define	SYS_USERDIR_UNIX	".hexen2"

#endif


#if defined(__MACOSX__)
#define	AOT_USERDIR		SYS_USERDIR_OSX
#else	/* unix: */
#define	AOT_USERDIR		SYS_USERDIR_UNIX
#endif

#endif	/* __USERDIR_H */

