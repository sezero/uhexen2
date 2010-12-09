/*
	userdir.h
	arch specific user directory definitions

	$Id: userdir.h,v 1.1 2007-03-18 10:20:03 sezero Exp $
*/

#ifndef __USERDIR_H
#define __USERDIR_H

#if defined(__MACOSX__)
#define	SYS_USERDIR		"Library/Application Support/Hexen2"
#define	SYS_DEMODIR		"Library/Application Support/Hexen2 Demo"
#else
#define	SYS_USERDIR		".hexen2"
#define	SYS_DEMODIR		".hexen2demo"
#endif

#if defined(DEMOBUILD)
#define	AOT_USERDIR		SYS_DEMODIR
#else
#define	AOT_USERDIR		SYS_USERDIR
#endif

#endif	/* __USERDIR_H */

