/*
 * snd_sys.h --
 * $Id$
 *
 * Copyright (C) 2007-2012  O.Sezer
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

#ifndef __HX2_SND_SYS__
#define __HX2_SND_SYS__

#undef HAVE_SDL_SOUND
#undef HAVE_OSS_SOUND
#undef HAVE_SUN_SOUND
#undef HAVE_ALSA_SOUND
#undef HAVE_DOS_GUS_SOUND
#undef HAVE_DOS_SB_SOUND
#undef HAVE_DOS_PCI_SOUND
#undef HAVE_WIN_SOUND
#undef HAVE_WIN_DX_SOUND
#undef HAVE_AHI_SOUND

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
#define HAVE_WIN_DX_SOUND	1
#else
#define HAVE_WIN_SOUND		0
#define HAVE_WIN_DX_SOUND	0
#endif

#if defined(PLATFORM_DOS)
#define HAVE_DOS_GUS_SOUND	1
#define HAVE_DOS_SB_SOUND	1
#if defined(NO_PCI_AUDIO)
#define HAVE_DOS_PCI_SOUND	0
#else
#define HAVE_DOS_PCI_SOUND	1
#endif
#else
#define HAVE_DOS_GUS_SOUND	0
#define HAVE_DOS_SB_SOUND	0
#define HAVE_DOS_PCI_SOUND	0
#endif

#if defined(PLATFORM_AMIGA)
#define HAVE_AHI_SOUND		1
#else
#define HAVE_AHI_SOUND		0
#endif

#define SOUND_NUMDRIVERS	(HAVE_SDL_SOUND + HAVE_OSS_SOUND + HAVE_SUN_SOUND + HAVE_ALSA_SOUND + HAVE_WIN_SOUND + HAVE_WIN_DX_SOUND + HAVE_DOS_SB_SOUND + HAVE_DOS_GUS_SOUND + HAVE_AHI_SOUND + HAVE_DOS_PCI_SOUND)

enum snddrv_id_t
{
	SNDDRV_ID_NULL	= 0,
	SNDDRV_ID_OSS,
	SNDDRV_ID_ALSA,
	SNDDRV_ID_SDL,
	SNDDRV_ID_SUN,
	SNDDRV_ID_WIN,
	SNDDRV_ID_DSOUND,
	SNDDRV_ID_GUS_DOS,
	SNDDRV_ID_SB_DOS,
	SNDDRV_ID_PCI_DOS,
	SNDDRV_ID_AHI,
	SNDDRV_ID_MAX
};

typedef struct snd_driver_s
{
	qboolean (*Init)(dma_t *);	/* initializes the sound dma driver */
	void (*Shutdown)(void);		/* shutdown the DMA xfer and driver */
	int (*GetDMAPos)(void);		/* returns the current dma position */
	void (*LockBuffer)(void);	/* validates & locks the dma buffer */
	void (*Submit)(void);		/* unlocks the dma buffer / sends sound to the device */
	void (*BlockSound)(void);	/* blocks sound output upon window focus loss */
	void (*UnblockSound)(void);	/* unblocks the output upon window focus gain */

	const char *snddrv_name;	/* active driver's name */
	enum snddrv_id_t snddrv_id;	/* enumerated ID number */
	qboolean userpreferred;		/* preferred by command line arguments */

	struct snd_driver_s *next;	/* next to try if previous failed */
} snd_driver_t;

extern void S_DriversInit (void);
extern void S_GetDriverList (snd_driver_t **);
extern void S_GetNullDriver (snd_driver_t **);

#endif	/* __HX2_SND_SYS__ */

