/*
 * launcher_defs.h
 * hexen2 launcher: common definitions
 *
 * $Id$
 *
 * Copyright (C) 2001 contributors of the Anvil of Thyrion project
 * Copyright (C) 2005-2012  O.Sezer <sezero@users.sourceforge.net>
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

#ifndef	LAUNCHER_COMMONDEFS_H
#define	LAUNCHER_COMMONDEFS_H

/* definitions common to all of the launcher source files. */

#define	__STRINGIFY(x) #x
#define	STRINGIFY(x) __STRINGIFY(x)

/* Hammer of Thyrion version num. */
#define HOT_VERSION_MAJ		1
#define HOT_VERSION_MID		5
#define HOT_VERSION_MIN		6
#define HOT_VERSION_STR		STRINGIFY(HOT_VERSION_MAJ) "." STRINGIFY(HOT_VERSION_MID) "." STRINGIFY(HOT_VERSION_MIN)

/* Launcher version num. */
#define LAUNCHER_VERSION_MAJ	1
#define LAUNCHER_VERSION_MID	0
#define LAUNCHER_VERSION_MIN	8
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
#define	RES_MINGL	RES_512		/* minimum resolution for OpenGL */

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

/* sound driver ID nums */
#define	SNDDRV_DEFAULT	-1	/* no cmdline option */
#define	SNDDRV_ID_NULL	0		/* must be 0 */
#define	SNDDRV_ID_OSS	1
#define	SNDDRV_ID_ALSA	2
#define	SNDDRV_ID_SDL	3
#define	SNDDRV_ID_SUN	4

typedef struct _launcher_snddrv_t
{
	int		id;	/* as above. */
	const char	*cmd;	/* cmdline switch */
	const char	*name;
} launcher_snddrv_t;
extern launcher_snddrv_t snd_drivers[];		/* in launch_bin.c */

#define	MAX_RATES	8
extern const char	*snd_rates[MAX_RATES];	/* in launch_bin.c */

#define HEAP_MINSIZE	16384	/* minimum heap memory size in KB */
#define HEAP_DEFAULT	32768	/* default heap memory size in KB */
#define HEAP_MAXSIZE	98304	/* maximum heap memory size in KB */
#define ZONE_MINSIZE	256	/* minimum zone memory size in KB */
#define ZONE_DEFAULT	256	/* default zone memory size in KB */
#define ZONE_MAXSIZE	1024	/* maximum zone memory size in KB */

#define MAX_EXTARGS	80	/* max array size for extra user args */

#define MAX_ARGS	40	/* max number of arguments for exec() */

#endif	/* LAUNCHER_COMMONDEFS_H */

