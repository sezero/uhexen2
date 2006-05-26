/*
	snd_sys.h
	Platform specific macros and prototypes for sound

	$Id: snd_sys.h,v 1.3 2006-05-26 08:21:25 sezero Exp $
*/

// add more systems with OSS here
#if defined(__linux__) || defined(__FreeBSD__)
#define HAVE_OSS_SOUND
#endif

// Sound system definitions
#define	S_SYS_NULL	0
#define	S_SYS_OSS	1
#define	S_SYS_SDL	2
#define	S_SYS_ALSA	3
#define	S_SYS_WIN32	4
#define S_SYS_MAX	5

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

#if defined(__linux__) && !defined(NO_ALSA)
// ALSA versions of the above
extern qboolean S_ALSA_Init(void);
extern int S_ALSA_GetDMAPos(void);
extern void S_ALSA_Submit(void);
extern void S_ALSA_Shutdown(void);
#endif	// NO_ALSA

#ifdef PLATFORM_UNIX
// SDL versions of the above
extern qboolean S_SDL_Init(void);
extern int S_SDL_GetDMAPos(void);
extern void S_SDL_Shutdown(void);
extern void S_SDL_Submit(void);
#endif

#endif	// _SND_LIST_DRIVERS

#endif	// !(_SND_SYS_MACROS_ONLY)

