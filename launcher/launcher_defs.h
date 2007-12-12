/*
	launcher_defs.h
	hexen2 launcher: common definitions

	$Id: launcher_defs.h,v 1.32 2007-12-12 10:51:08 sezero Exp $

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

	See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to:

		Free Software Foundation, Inc.
		51 Franklin St, Fifth Floor,
		Boston, MA  02110-1301, USA
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
#define LAUNCHER_VERSION_MAJ	1
#define LAUNCHER_VERSION_MID	0
#define LAUNCHER_VERSION_MIN	3
#define LAUNCHER_VERSION_STR	STRINGIFY(LAUNCHER_VERSION_MAJ) "." STRINGIFY(LAUNCHER_VERSION_MID) "." STRINGIFY(LAUNCHER_VERSION_MIN)

#ifndef DEMOBUILD
#define	AOT_USERDIR	".hexen2"
#else
#define	AOT_USERDIR	".hexen2demo"
#endif

/* don't change these two */
#define	DEST_H2		0
#define	DEST_HW		1

/* index for supported resolutions */
#define	RES_320		0
#define	RES_400		1
#define	RES_512		2
#define	RES_640		3
#define	RES_800		4
#define	RES_1024	5
#define	RES_1280	6
#define	RES_1600	7
#define	RES_MAX		8

/* which sound drivers are available: */
#if defined(__linux) || defined(__linux__)
#define	HAVE_ALSA_SOUND	1
#else
#define	HAVE_ALSA_SOUND	0
#endif
#if defined(__linux) || defined(__linux__) || defined(__FreeBSD__) || defined(__DragonFly__)
#define	HAVE_OSS_SOUND	1
#else
#define	HAVE_OSS_SOUND	0
#endif
#if defined(__OpenBSD__) || defined(__NetBSD__) || defined(__sun) || defined(sun)
#define	HAVE_SUN_SOUND	1
#else
#define	HAVE_SUN_SOUND	0
#endif

#define	MAX_SOUND	(2 + HAVE_ALSA_SOUND + HAVE_OSS_SOUND + HAVE_SUN_SOUND)
				/* max entries in the snddrv_names table */

#define	MAX_RATES	8	/* max entries in the snd_rates table */

#define HEAP_MINSIZE	16384	/* minimum heap memory size in KB */
#define HEAP_DEFAULT	32768	/* default heap memory size in KB */
#define HEAP_MAXSIZE	98304	/* maximum heap memory size in KB */
#define ZONE_MINSIZE	256	/* minimum zone memory size in KB */
#define ZONE_DEFAULT	256	/* default zone memory size in KB */
#define ZONE_MAXSIZE	1024	/* maximum zone memory size in KB */

#define MAX_EXTARGS	80U	/* max array size for extra user args */

#define MAX_ARGS	40U	/* max number of arguments for exec() */

#endif	/* LAUNCHER_COMMONDEFS_H */

