/*
	snd_sys.h
	Platform specific macros and prototypes for sound

	$Id: snd_sys.h,v 1.22 2008-04-02 20:37:36 sezero Exp $

	Copyright (C) 2007  O.Sezer

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
		Boston, MA  02110-1301  USA
*/

#ifndef __HX2_SND_SYS__
#define __HX2_SND_SYS__

#undef HAVE_SDL_SOUND
#undef HAVE_OSS_SOUND
#undef HAVE_SUN_SOUND
#undef HAVE_ALSA_SOUND
#undef HAVE_DOS_SOUND
#undef HAVE_WIN_SOUND

#undef SOUND_NUMDRIVERS

#if defined(NO_OSS_AUDIO)
#define HAVE_OSS_SOUND	0
/* add more systems with OSS here */
#elif defined(__linux) || defined(__linux__) || defined(__FreeBSD__) || defined(__DragonFly__)
#define HAVE_OSS_SOUND	1
#else
#define HAVE_OSS_SOUND	0
#endif

#if defined(NO_SUN_AUDIO)
#define HAVE_SUN_SOUND	0
/* add more systems with SUN audio here */
#elif defined(__OpenBSD__) || defined(__NetBSD__) || defined(__sun) || defined(sun)
#define HAVE_SUN_SOUND	1
#else
#define HAVE_SUN_SOUND	0
#endif

#if defined(NO_ALSA_AUDIO)
#define HAVE_ALSA_SOUND	0
#elif defined(__linux) || defined(__linux__)
/* add more systems with ALSA here */
#define HAVE_ALSA_SOUND	1
#else
#define HAVE_ALSA_SOUND	0
#endif

#if defined(NO_SDL_AUDIO)
#define HAVE_SDL_SOUND	0
#elif defined(SDLQUAKE)
#define HAVE_SDL_SOUND	1
#else
#define HAVE_SDL_SOUND	0
#endif

#if defined(PLATFORM_WINDOWS)
#define HAVE_WIN_SOUND		1
#else
#define HAVE_WIN_SOUND		0
#endif

#if defined(PLATFORM_DOS)
#define HAVE_DOS_SOUND		1
#else
#define HAVE_DOS_SOUND		0
#endif

#define SOUND_NUMDRIVERS	(HAVE_SDL_SOUND + HAVE_OSS_SOUND + HAVE_SUN_SOUND + HAVE_ALSA_SOUND + HAVE_WIN_SOUND + HAVE_DOS_SOUND)

/* Sound system definitions */
#define	S_SYS_NULL	0
#define	S_SYS_OSS	1
#define	S_SYS_SDL	2
#define	S_SYS_ALSA	3
#define	S_SYS_SUN	4
#define	S_SYS_WIN	5
#define	S_SYS_DOS	6
#define	S_SYS_MAX	7


extern unsigned int	snd_system;

typedef struct snd_driver_s
{
	qboolean (*Init)(dma_t *);	/* initializes the sound dma driver */
	void (*Shutdown)(void);		/* shutdown the DMA xfer and driver */
	int (*GetDMAPos)(void);		/* returns the current dma position */
	void (*LockBuffer)(void);	/* validates & locks the dma buffer */
	void (*Submit)(void);		/* unlocks the dma buffer / sends sound to the device */
	void (*BlockSound)(void);	/* blocks sound output upon window focus loss */
	void (*UnblockSound)(void);	/* unblocks the output upon window focus gain */
	const char *(*DrvName)(void);	/* returns the active driver's name */
} snd_driver_t;

extern void S_InitDrivers(snd_driver_t **sdrv);	/* initializes sound driver function pointers */

#ifdef _SND_LIST_DRIVERS

#if HAVE_WIN_SOUND
extern void S_WIN_LinkFuncs(snd_driver_t *);
#else
#define	S_WIN_LinkFuncs		NULL
#endif

#if HAVE_DOS_SOUND
extern void S_DOS_LinkFuncs(snd_driver_t *);
#else
#define	S_DOS_LinkFuncs		NULL
#endif

#if HAVE_OSS_SOUND
extern void S_OSS_LinkFuncs(snd_driver_t *);
#else
#define	S_OSS_LinkFuncs		NULL
#endif	/* HAVE_OSS_SOUND */

#if HAVE_SUN_SOUND
extern void S_SUN_LinkFuncs(snd_driver_t *);
#else
#define	S_SUN_LinkFuncs		NULL
#endif	/* HAVE_SUN_SOUND */

#if HAVE_ALSA_SOUND
extern void S_ALSA_LinkFuncs(snd_driver_t *);
#else
#define	S_ALSA_LinkFuncs	NULL
#endif	/* HAVE_ALSA_SOUND */

#if HAVE_SDL_SOUND
extern void S_SDL_LinkFuncs(snd_driver_t *);
#else
#define	S_SDL_LinkFuncs		NULL
#endif	/* HAVE_SDL_SOUND */

#endif	/* _SND_LIST_DRIVERS */

#endif	/* __HX2_SND_SYS__ */

