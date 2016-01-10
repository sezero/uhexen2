/*
 * snd_sys.c -- pre-Init platform specific sound stuff
 * $Id$
 *
 * Copyright (C) 2007-2012 O.Sezer <sezero@users.sourceforge.net>
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

#include "quakedef.h"
#include "snd_sys.h"

/* drivers' headers: */
#include "snd_alsa.h"
#include "snd_oss.h"
#include "snd_sdl.h"
#include "snd_sun.h"
#include "snd_win.h"
#include "snd_dsound.h"
#include "snd_sb.h"
#include "snd_gus.h"
#include "snd_pci.h"
#include "snd_ahi.h"

static qboolean	snd_sys_inited = false;


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

#define S_NULL_Shutdown		NULL_void_func
#define S_NULL_LockBuffer	NULL_void_func
#define S_NULL_Submit		NULL_void_func
#define S_NULL_BlockSound	NULL_void_func
#define S_NULL_UnblockSound	NULL_void_func
static void NULL_void_func (void)
{
}

static snd_driver_t snddrv_null =
{
/* do register this first so that it
 * stays last in the linked list.  */
	S_NULL_Init,
	S_NULL_Shutdown,
	S_NULL_GetDMAPos,
	S_NULL_LockBuffer,
	S_NULL_Submit,
	S_NULL_BlockSound,
	S_NULL_UnblockSound,
	s_null_driver,
	SNDDRV_ID_NULL,
	false,
	NULL
};

static snd_driver_t	*snd_drivers = &snddrv_null;


static void S_RegisterDriver(snd_driver_t *driver)
{
	driver->next = snd_drivers;
	snd_drivers = driver;
}

void S_DriversInit (void)
{
	if (snd_sys_inited)
		return;
	snd_sys_inited = true;

	snd_drivers = NULL;

	S_RegisterDriver(&snddrv_null);
	if (safemode || COM_CheckParm("-nosound") || COM_CheckParm("-s"))
		snd_drivers->userpreferred = true;
#if HAVE_SUN_SOUND
	S_RegisterDriver(&snddrv_sunaudio);
	if (COM_CheckParm ("-sndsun") || COM_CheckParm ("-sndbsd"))
		snd_drivers->userpreferred = true;
#endif
#if HAVE_ALSA_SOUND
	S_RegisterDriver(&snddrv_alsa);
	if (COM_CheckParm ("-sndalsa"))
		snd_drivers->userpreferred = true;
#endif
#if HAVE_OSS_SOUND
	S_RegisterDriver(&snddrv_oss);
	if (COM_CheckParm ("-sndoss"))
		snd_drivers->userpreferred = true;
#endif
#if HAVE_WIN_SOUND
	S_RegisterDriver(&snddrv_win);
	if (COM_CheckParm ("-wavonly"))
		snd_drivers->userpreferred = true;
#endif
#if HAVE_WIN_DX_SOUND
	S_RegisterDriver(&snddrv_dsound);
#endif
#if HAVE_DOS_SB_SOUND
	S_RegisterDriver(&snddrv_blaster);
#endif
#if HAVE_DOS_GUS_SOUND
	S_RegisterDriver(&snddrv_gus);
#endif
#if HAVE_DOS_PCI_SOUND
	S_RegisterDriver(&snddrv_pci);
#endif
#if HAVE_AHI_SOUND
	S_RegisterDriver(&snddrv_ahi);
	if (COM_CheckParm ("-sndahi"))
		snd_drivers->userpreferred = true;
#endif
/* if sdl audio is compiled for any supported platform, then
 * register sdl audio the last to make it the default choice. */
#if HAVE_SDL_SOUND
	S_RegisterDriver(&snddrv_sdl);
	if (COM_CheckParm ("-sndsdl"))
		snd_drivers->userpreferred = true;
#endif
}

void S_GetDriverList (snd_driver_t **p)
{
	*p = snd_drivers;
}

void S_GetNullDriver (snd_driver_t **p)
{
	*p = &snddrv_null;
}

