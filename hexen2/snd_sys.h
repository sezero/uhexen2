/*
	snd_sys.h
	Platform specific macros and prototypes for sound

	$Id: snd_sys.h,v 1.13 2007-07-11 21:00:12 sezero Exp $
*/

#ifndef __HX2_SND_SYS__
#define __HX2_SND_SYS__

#undef HAVE_SDL_SOUND
#undef HAVE_OSS_SOUND
#undef HAVE_SUN_SOUND
#undef HAVE_ALSA_SOUND
#undef HAVE_WIN_SOUND

#undef SOUND_NUMDRIVERS

#if defined(NO_OSS_AUDIO)
#define HAVE_OSS_SOUND	0
// add more systems with OSS here
#elif defined(__linux__) || defined(__FreeBSD__) || defined(__DragonFly__)
#define HAVE_OSS_SOUND	1
#else
#define HAVE_OSS_SOUND	0
#endif

#if defined(NO_SUN_AUDIO)
#define HAVE_SUN_SOUND	0
// add more systems with SUN audio here
#elif defined(__OpenBSD__) || defined(__NetBSD__) || defined(__SOLARIS__)
#define HAVE_SUN_SOUND	1
#else
#define HAVE_SUN_SOUND	0
#endif

#if defined(NO_ALSA_AUDIO)
#define HAVE_ALSA_SOUND	0
#elif defined(__linux__)
// add more systems with ALSA here
#define HAVE_ALSA_SOUND	1
#else
#define HAVE_ALSA_SOUND	0
#endif

#if defined(NO_SDL_AUDIO)
#define HAVE_SDL_SOUND	0
#elif defined(PLATFORM_UNIX)
#define HAVE_SDL_SOUND	1
#else
#define HAVE_SDL_SOUND	0
#endif

#if defined(PLATFORM_WINDOWS)
#define HAVE_WIN_SOUND		1
#else
#define HAVE_WIN_SOUND		0
#endif

#define SOUND_NUMDRIVERS	(HAVE_SDL_SOUND + HAVE_OSS_SOUND + HAVE_SUN_SOUND + HAVE_ALSA_SOUND + HAVE_WIN_SOUND)

// Sound system definitions
#define	S_SYS_NULL	0
#define	S_SYS_OSS	1
#define	S_SYS_SDL	2
#define	S_SYS_ALSA	3
#define	S_SYS_SUN	4
#define	S_SYS_WIN	5
#define	S_SYS_MAX	6

// this prevents running S_Update_() with the sdl sound driver
// if the snd_sdl implementation already calls S_PaintChannels.
#define SDLSOUND_PAINTS_CHANNELS	1

#if defined(PLATFORM_WINDOWS)
// for the windows crap used in snd_dma.c
#include "winquake.h"
#endif	/* PLATFORM_WINDOWS */


extern unsigned int	snd_system;

#ifndef _SND_SYS_MACROS_ONLY

// chooses functions to call depending on audio subsystem
extern void S_InitDrivers(void);

// initializes driver and cycling through a DMA buffer
extern qboolean (*SNDDMA_Init)(void);

// gets the current DMA position
extern int (*SNDDMA_GetDMAPos)(void);

// shutdown the DMA xfer and driver
extern void (*SNDDMA_Shutdown)(void);

// sends sound to the device
extern void (*SNDDMA_Submit)(void);


#ifdef _SND_LIST_DRIVERS

#if HAVE_WIN_SOUND
// Windows versions of the above
extern qboolean S_WIN_Init(void);
extern int S_WIN_GetDMAPos(void);
extern void S_WIN_Shutdown(void);
extern void S_WIN_Submit(void);
#endif

#if HAVE_OSS_SOUND
// OSS versions of the above
extern qboolean S_OSS_Init(void);
extern int S_OSS_GetDMAPos(void);
extern void S_OSS_Shutdown(void);
extern void S_OSS_Submit(void);
#endif	// HAVE_OSS_SOUND

#if HAVE_SUN_SOUND
// SUN Audio versions of the above
extern qboolean S_SUN_Init(void);
extern int S_SUN_GetDMAPos(void);
extern void S_SUN_Shutdown(void);
extern void S_SUN_Submit(void);
#endif	// HAVE_SUN_SOUND

#if HAVE_ALSA_SOUND
// ALSA versions of the above
extern qboolean S_ALSA_Init(void);
extern int S_ALSA_GetDMAPos(void);
extern void S_ALSA_Submit(void);
extern void S_ALSA_Shutdown(void);
#endif	// HAVE_ALSA_SOUND

#if HAVE_SDL_SOUND
// SDL versions of the above
extern qboolean S_SDL_Init(void);
extern int S_SDL_GetDMAPos(void);
extern void S_SDL_Shutdown(void);
extern void S_SDL_Submit(void);
#endif	// HAVE_SDL_SOUND

#endif	// _SND_LIST_DRIVERS

#endif	// !(_SND_SYS_MACROS_ONLY)

#endif	// __HX2_SND_SYS__

