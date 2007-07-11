/*
	snd_sys.c
	pre-Init platform specific sound stuff

	$Id: snd_sys.c,v 1.10 2007-07-11 16:47:16 sezero Exp $
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

// dummy SNDDMA functions, just in case
static qboolean S_NULL_Init(void)
{
#if SOUND_NUMDRIVERS
	Con_Printf ("No sound\n");
#else
	Con_Printf ("SOUND: no drivers available\n");
#endif
	return 0;
}

static int S_NULL_GetDMAPos(void)
{
	return 0;
}

static void S_NULL_Shutdown(void)
{
}

static void S_NULL_Submit(void)
{
}


static void S_InitSys (void)
{
	if (safemode || COM_CheckParm("-nosound") || COM_CheckParm("-s"))
		snd_system = S_SYS_NULL;

#if HAVE_SDL_SOUND
	else if (COM_CheckParm ("-sndsdl"))
		snd_system = S_SYS_SDL;
#endif

#if HAVE_ALSA_SOUND
	else if (COM_CheckParm ("-sndalsa"))
		snd_system = S_SYS_ALSA;
#endif

#if HAVE_OSS_SOUND
	else if (COM_CheckParm ("-sndoss"))
		snd_system = S_SYS_OSS;
#endif

#if HAVE_SUN_SOUND
	else if (COM_CheckParm ("-sndsun") || COM_CheckParm ("-sndbsd"))
		snd_system = S_SYS_SUN;
#endif

	else
#if HAVE_WIN_SOUND
		snd_system = S_SYS_WIN;
#elif HAVE_OSS_SOUND
		snd_system = S_SYS_OSS;
#elif HAVE_SUN_SOUND
		snd_system = S_SYS_SUN;
#elif HAVE_ALSA_SOUND
		snd_system = S_SYS_ALSA;
#elif HAVE_SDL_SOUND
		snd_system = S_SYS_SDL;
#else
		snd_system = S_SYS_NULL;
#endif
}


void S_InitDrivers (void)
{
	if (!snd_sys_inited)
	{
		S_InitSys();
		snd_sys_inited = true;
	}

	switch (snd_system)
	{
#if HAVE_WIN_SOUND
	case S_SYS_WIN:
		SNDDMA_Init	 = S_WIN_Init;
		SNDDMA_GetDMAPos = S_WIN_GetDMAPos;
		SNDDMA_Shutdown	 = S_WIN_Shutdown;
		SNDDMA_Submit	 = S_WIN_Submit;
		break;
#endif
#if HAVE_SDL_SOUND
	case S_SYS_SDL:
		SNDDMA_Init	 = S_SDL_Init;
		SNDDMA_GetDMAPos = S_SDL_GetDMAPos;
		SNDDMA_Shutdown	 = S_SDL_Shutdown;
		SNDDMA_Submit	 = S_SDL_Submit;
		break;
#endif
#if HAVE_ALSA_SOUND
	case S_SYS_ALSA:
		SNDDMA_Init	 = S_ALSA_Init;
		SNDDMA_GetDMAPos = S_ALSA_GetDMAPos;
		SNDDMA_Shutdown	 = S_ALSA_Shutdown;
		SNDDMA_Submit	 = S_ALSA_Submit;
		break;
#endif
#if HAVE_OSS_SOUND
	case S_SYS_OSS:
		SNDDMA_Init	 = S_OSS_Init;
		SNDDMA_GetDMAPos = S_OSS_GetDMAPos;
		SNDDMA_Shutdown	 = S_OSS_Shutdown;
		SNDDMA_Submit	 = S_OSS_Submit;
		break;
#endif
#if HAVE_SUN_SOUND
	case S_SYS_SUN:
		SNDDMA_Init	 = S_SUN_Init;
		SNDDMA_GetDMAPos = S_SUN_GetDMAPos;
		SNDDMA_Shutdown	 = S_SUN_Shutdown;
		SNDDMA_Submit	 = S_SUN_Submit;
		break;
#endif
	case S_SYS_NULL:
	default:
		SNDDMA_Init	 = S_NULL_Init;
		SNDDMA_GetDMAPos = S_NULL_GetDMAPos;
		SNDDMA_Shutdown	 = S_NULL_Shutdown;
		SNDDMA_Submit	 = S_NULL_Submit;
		break;
	}
}

