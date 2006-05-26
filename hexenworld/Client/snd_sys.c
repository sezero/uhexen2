/*
	snd_sys.c
	pre-Init platform specific sound stuff

	$Id: snd_sys.c,v 1.2 2006-05-26 08:21:25 sezero Exp $
*/


#define _SND_LIST_DRIVERS

#include "quakedef.h"
#include "snd_sys.h"

unsigned int	snd_system = S_SYS_NULL;
static qboolean	snd_sys_inited = false;

// pointers for SNDDMA_ functions
qboolean (*SNDDMA_Init)(void);
int (*SNDDMA_GetDMAPos)(void);
void (*SNDDMA_Shutdown)(void);
void (*SNDDMA_Submit)(void);


static void S_InitSys (void)
{
	if (COM_CheckParm("-nosound") || COM_CheckParm("--nosound") || COM_CheckParm("-s"))
		snd_system = S_SYS_NULL;

#if defined(PLATFORM_UNIX)
	else if (COM_CheckParm ("-sndsdl"))
		snd_system = S_SYS_SDL;
#endif

#if defined(__linux__) && !defined(NO_ALSA)
	else if (COM_CheckParm ("-sndalsa"))
		snd_system = S_SYS_ALSA;
#endif

#if defined(HAVE_OSS_SOUND)
	else if (COM_CheckParm ("-sndoss"))
		snd_system = S_SYS_OSS;
#endif

	else
#if defined(_WIN32)
		snd_system = S_SYS_WIN32;
#else	/* unix	*/
#  if defined(HAVE_OSS_SOUND)
		snd_system = S_SYS_OSS;
#  else
		snd_system = S_SYS_SDL;
#  endif
#endif
}


void S_InitPointers (void)
{
	if (!snd_sys_inited)
	{
		 S_InitSys();
		 snd_sys_inited = true;
	}

	switch (snd_system)
	{
#if defined(_WIN32)
	case S_SYS_WIN32:
		SNDDMA_Init	 = S_WIN32_Init;
		SNDDMA_GetDMAPos = S_WIN32_GetDMAPos;
		SNDDMA_Shutdown	 = S_WIN32_Shutdown;
		SNDDMA_Submit	 = S_WIN32_Submit;
		break;
#endif
#if defined(PLATFORM_UNIX)
	case S_SYS_SDL:
		SNDDMA_Init	 = S_SDL_Init;
		SNDDMA_GetDMAPos = S_SDL_GetDMAPos;
		SNDDMA_Shutdown	 = S_SDL_Shutdown;
		SNDDMA_Submit	 = S_SDL_Submit;
		break;
#endif
#if defined(__linux__) && !defined(NO_ALSA)
	case S_SYS_ALSA:
		SNDDMA_Init	 = S_ALSA_Init;
		SNDDMA_GetDMAPos = S_ALSA_GetDMAPos;
		SNDDMA_Shutdown	 = S_ALSA_Shutdown;
		SNDDMA_Submit	 = S_ALSA_Submit;
		break;
#endif
#if defined(HAVE_OSS_SOUND)
	case S_SYS_OSS:
		SNDDMA_Init	 = S_OSS_Init;
		SNDDMA_GetDMAPos = S_OSS_GetDMAPos;
		SNDDMA_Shutdown	 = S_OSS_Shutdown;
		SNDDMA_Submit	 = S_OSS_Submit;
		break;
#endif
	case S_SYS_NULL:
	default:
	// Paranoia: We should never have come this far!..
	// No function to point at, set snd_initialized to false.
		snd_initialized = false;
		break;
	}
}

