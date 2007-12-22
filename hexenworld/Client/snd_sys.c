/*
	snd_sys.c
	pre-Init platform specific sound stuff

	$Id: snd_sys.c,v 1.15 2007-12-22 13:03:24 sezero Exp $

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


#define _SND_LIST_DRIVERS

#include "quakedef.h"
#include "snd_sys.h"

unsigned int	snd_system = S_SYS_NULL;
static qboolean	snd_sys_inited = false;

static snd_driver_t	snd_driver;


/* dummy SNDDMA functions, just in case */
static char s_null_driver[] = "NULL";

static qboolean S_NULL_Init (dma_t *dma)
{
#if SOUND_NUMDRIVERS
	Con_Printf ("No sound\n");
#else
	Con_Printf ("SOUND: no drivers available\n");
#endif
	shm = NULL;
	return false;
}

static int S_NULL_GetDMAPos (void)
{
	return 0;
}

static const char *S_NULL_DrvName (void)
{
	return s_null_driver;
}

#define S_NULL_Shutdown		NULL_void_func
#define S_NULL_LockBuffer	NULL_void_func
#define S_NULL_Submit		NULL_void_func
static void NULL_void_func (void)
{
}

static void S_NULL_LinkFuncs (snd_driver_t *p)
{
	p->Init		= S_NULL_Init;
	p->Shutdown	= S_NULL_Shutdown;
	p->GetDMAPos	= S_NULL_GetDMAPos;
	p->LockBuffer	= S_NULL_LockBuffer;
	p->Submit	= S_NULL_Submit;
	p->DrvName	= S_NULL_DrvName;
}


static struct
{
	void (*LinkFunc)(snd_driver_t *);
} snd_linkfunc[S_SYS_MAX] =
{
	{ S_NULL_LinkFuncs },
	{ S_OSS_LinkFuncs  },
	{ S_SDL_LinkFuncs  },
	{ S_ALSA_LinkFuncs },
	{ S_SUN_LinkFuncs  },
	{ S_WIN_LinkFuncs  },
	{ S_DOS_LinkFuncs  },
};

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
#elif HAVE_DOS_SOUND
		snd_system = S_SYS_DOS;
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


void S_InitDrivers (snd_driver_t **p)
{
	if (!snd_sys_inited)
	{
		S_InitSys();
		snd_sys_inited = true;
	}

	if (snd_system < 0 || snd_system >= S_SYS_MAX)
		Sys_Error ("%s: Bad index %d", __thisfunc__, snd_system);
	if (snd_linkfunc[snd_system].LinkFunc == NULL)
		Sys_Error ("%s: NULL function pointer for %d", __thisfunc__, snd_system);

	snd_linkfunc[snd_system].LinkFunc(&snd_driver);
	*p = &snd_driver;
}

