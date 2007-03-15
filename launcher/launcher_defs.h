/*
	launcher_defs.h
	hexen2 launcher: common definitions

	$Id: launcher_defs.h,v 1.24 2007-03-15 15:08:42 sezero Exp $
*/

#ifndef	LAUNCHER_COMMONDEFS_H
#define	LAUNCHER_COMMONDEFS_H

// definitions common to all of the launcher source files.

#define	__STRINGIFY(x) #x
#define	STRINGIFY(x) __STRINGIFY(x)

// Hammer of Thyrion version num.
#define HOT_VERSION_MAJ		1
#define HOT_VERSION_MID		4
#define HOT_VERSION_MIN		2
#define HOT_VERSION_STR		STRINGIFY(HOT_VERSION_MAJ) "." STRINGIFY(HOT_VERSION_MID) "." STRINGIFY(HOT_VERSION_MIN)

// Launcher version num.
#define LAUNCHER_VERSION_MAJ	0
#define LAUNCHER_VERSION_MID	9
#define LAUNCHER_VERSION_MIN	0
#define LAUNCHER_VERSION_STR	STRINGIFY(LAUNCHER_VERSION_MAJ) "." STRINGIFY(LAUNCHER_VERSION_MID) "." STRINGIFY(LAUNCHER_VERSION_MIN)

#ifndef DEMOBUILD
#define	AOT_USERDIR	".hexen2"
#else
#define	AOT_USERDIR	".hexen2demo"
#endif

#define	DEST_H2		0
#define	DEST_HW		1

#define	H2_BINARY_NAME	"hexen2"
#define	HW_BINARY_NAME	"hwcl"

#define	RES_320		0
#define	RES_400		1
#define	RES_512		2
#define	RES_640		3
#define	RES_800		4
#define	RES_1024	5
#define	RES_1280	6
#define	RES_1600	7
#define	RES_MAX		8

#define	MAX_H2GAMES	3	// max entries in the h2game_names table
#define	MAX_HWGAMES	6	// max entries in the hwgame_names table

#if defined(__linux__)
#define	HAVE_ALSA_SOUND	1
#else
#define	HAVE_ALSA_SOUND	0
#endif
#if defined(__linux__) || defined(__FreeBSD__) || defined(__DragonFly__)
#define	HAVE_OSS_SOUND	1
#else
#define	HAVE_OSS_SOUND	0
#endif
#if defined(__OpenBSD__) || defined(__NetBSD__) || defined(SUNOS)
#define	HAVE_SUN_SOUND	1
#else
#define	HAVE_SUN_SOUND	0
#endif

#define	MAX_SOUND	(2 + HAVE_ALSA_SOUND + HAVE_OSS_SOUND + HAVE_SUN_SOUND)
			// max entries in the snddrv_names table

#define	MAX_RATES	8	// max entries in the snd_rates table

#define HEAP_MINSIZE	16384	// minimum heap memory size in KB
#define HEAP_DEFAULT	32768	// default heap memory size in KB
#define HEAP_MAXSIZE	98304	// maximum heap memory size in KB
#define ZONE_MINSIZE	256	// minimum zone memory size in KB
#define ZONE_DEFAULT	256	// default zone memory size in KB
#define ZONE_MAXSIZE	1024	// maximum zone memory size in KB

#endif	// LAUNCHER_COMMONDEFS_H

