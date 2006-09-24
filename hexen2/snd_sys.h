/*
	snd_sys.h
	Platform specific macros and prototypes for sound

	$Id: snd_sys.h,v 1.6 2006-09-24 17:28:42 sezero Exp $
*/

#ifndef __HX2_SND_SYS__
#define __HX2_SND_SYS__

#undef HAVE_OSS_SOUND
#undef HAVE_SUN_SOUND
#undef HAVE_ALSA_SOUND

// add more systems with OSS here
#if defined(__linux__) || defined(__DragonFly__) || defined(__FreeBSD__)
#define HAVE_OSS_SOUND
#endif

// add more systems with SUN audio here
#if defined(__OpenBSD__) || defined(__NetBSD__) || defined(SUNOS)
#define HAVE_SUN_SOUND
#endif

#if !defined(NO_ALSA)
#if defined(__linux__)
// add more systems with ALSA here
#define HAVE_ALSA_SOUND
#endif
#endif	// NO_ALSA

// Sound system definitions
#define	S_SYS_NULL	0
#define	S_SYS_OSS	1
#define	S_SYS_SDL	2
#define	S_SYS_ALSA	3
#define	S_SYS_SUN	4
#define	S_SYS_WIN32	5
#define	S_SYS_MAX	6

// this prevents running S_Update_() with the sdl sound driver
// if the snd_sdl implementation already calls S_PaintChannels.
#define SDLSOUND_PAINTS_CHANNELS	1

#if defined(_WIN32)
// for the windows crap used in snd_dma.c
#include "winquake.h"
#endif	//  _WIN32


#ifndef _SND_SYS_MACROS_ONLY

extern unsigned int	snd_system;

// chooses functions to call depending on audio subsystem
extern void S_InitPointers(void);

// initializes driver and cycling through a DMA buffer
extern qboolean (*SNDDMA_Init)(void);

// gets the current DMA position
extern int (*SNDDMA_GetDMAPos)(void);

// shutdown the DMA xfer and driver
extern void (*SNDDMA_Shutdown)(void);

// sends sound to the device
extern void (*SNDDMA_Submit)(void);


#ifdef _SND_LIST_DRIVERS

#ifdef _WIN32
// WIN32 versions of the above
extern qboolean S_WIN32_Init(void);
extern int S_WIN32_GetDMAPos(void);
extern void S_WIN32_Shutdown(void);
extern void S_WIN32_Submit(void);
#endif

#if defined(HAVE_OSS_SOUND)
// OSS versions of the above
extern qboolean S_OSS_Init(void);
extern int S_OSS_GetDMAPos(void);
extern void S_OSS_Shutdown(void);
extern void S_OSS_Submit(void);
#endif	// HAVE_OSS_SOUND

#if defined(HAVE_SUN_SOUND)
// SUN Audio versions of the above
extern qboolean S_SUN_Init(void);
extern int S_SUN_GetDMAPos(void);
extern void S_SUN_Shutdown(void);
extern void S_SUN_Submit(void);
#endif	// HAVE_SUN_SOUND

#if defined(HAVE_ALSA_SOUND)
// ALSA versions of the above
extern qboolean S_ALSA_Init(void);
extern int S_ALSA_GetDMAPos(void);
extern void S_ALSA_Submit(void);
extern void S_ALSA_Shutdown(void);
#endif	// HAVE_ALSA_SOUND

#ifdef PLATFORM_UNIX
// SDL versions of the above
extern qboolean S_SDL_Init(void);
extern int S_SDL_GetDMAPos(void);
extern void S_SDL_Shutdown(void);
extern void S_SDL_Submit(void);
#endif

#endif	// _SND_LIST_DRIVERS

#endif	// !(_SND_SYS_MACROS_ONLY)

#endif	// __HX2_SND_SYS__

