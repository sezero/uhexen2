/*
 * Background music handling for Hexen II: Hammer of Thyrion (uHexen2)
 * Handle cases when we are configured for no sound BUT with a midi driver
 *
 * Copyright (C) 1999-2005 Id Software, Inc.
 * Copyright (C) 2010 O.Sezer <sezero@users.sourceforge.net>
 *
 * $Id$
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
 *
 */

#include "quakedef.h"
#include "bgmusic.h"
#include "midi_drv.h"

#define MIDI_DIRNAME	"midi"

qboolean	bgmloop;

static float	old_volume = -1.0f;

typedef struct midi_handle_s
{
	void	*handle;
	midi_driver_t	*driver;
} midi_handle_t;

static midi_driver_t *midi_drivers = NULL;
static midi_handle_t midi_handle;

static void BGM_Play_f (void)
{
	if (Cmd_Argc() == 2)
	{
		BGM_Play (Cmd_Argv(1));
	}
	else
	{
		Con_Printf ("music <musicfile>\n");
		return;
	}
}

static void BGM_Pause_f (void)
{
	BGM_Pause ();
}

static void BGM_Resume_f (void)
{
	BGM_Resume ();
}

static void BGM_Loop_f (void)
{
	if (Cmd_Argc() == 2)
	{
		if (q_strcasecmp(Cmd_Argv(1),  "0") == 0 ||
		    q_strcasecmp(Cmd_Argv(1),"off") == 0)
			bgmloop = false;
		else if (q_strcasecmp(Cmd_Argv(1), "1") == 0 ||
		         q_strcasecmp(Cmd_Argv(1),"on") == 0)
			bgmloop = true;
		else if (q_strcasecmp(Cmd_Argv(1),"toggle") == 0)
			bgmloop = !bgmloop;
	}

	if (bgmloop)
		Con_Printf("Music will be looped\n");
	else
		Con_Printf("Music will not be looped\n");
}

static void BGM_Stop_f (void)
{
	BGM_Stop();
}

void BGM_RegisterMidiDRV (void *drv)
{
	midi_driver_t *driver = (midi_driver_t *) drv;
	driver->next = midi_drivers;
	midi_drivers = driver;
}

qboolean BGM_Init (void)
{
	memset (&midi_handle, 0, sizeof(midi_handle_t));

	Cmd_AddCommand("music", BGM_Play_f);
	Cmd_AddCommand("music_pause", BGM_Pause_f);
	Cmd_AddCommand("music_resume", BGM_Resume_f);
	Cmd_AddCommand("music_loop", BGM_Loop_f);
	Cmd_AddCommand("music_stop", BGM_Stop_f);

	bgmloop = true;

	return true;
}

void BGM_Play (const char *filename)
{
	midi_driver_t *drv;
	char tmp[MAX_QPATH];

	if (midi_drivers == NULL)
		return;

	drv = midi_drivers;
	q_snprintf(tmp, sizeof(tmp), "%s/%s%s", MIDI_DIRNAME, filename, ".mid");
	while (drv)
	{
		if (drv->available)
		{
			midi_handle.handle = drv->mididrv_open (filename);
			if (midi_handle.handle != NULL)
			{
				midi_handle.driver = drv;
				return;
			}
		}
		drv = drv->next;
	}

	Con_Printf("Couldn't open music file %s\n", filename);
}

void BGM_Stop (void)
{
	if (midi_handle.handle)
	{
		midi_handle.driver->mididrv_close (& midi_handle.handle);
		midi_handle.handle = NULL;
		midi_handle.driver = NULL;
	}
}

void BGM_Pause (void)
{
	if (midi_handle.handle)
		midi_handle.driver->mididrv_pause (& midi_handle.handle);
}

void BGM_Resume (void)
{
	if (midi_handle.handle)
		midi_handle.driver->mididrv_resume (& midi_handle.handle);
}

void BGM_Update (void)
{
	if (old_volume != bgmvolume.value)
	{
		if (bgmvolume.value < 0)
			Cvar_Set ("bgmvolume", "0");
		else if (bgmvolume.value > 1)
			Cvar_Set ("bgmvolume", "1");
		old_volume = bgmvolume.value;
		if (midi_handle.handle)
		{
			midi_handle.driver->mididrv_setvol (& midi_handle.handle, bgmvolume.value);
			if (bgmvolume.value == 0.0f) /* don't bother advancing */
				midi_handle.driver->mididrv_pause (& midi_handle.handle);
			else
				midi_handle.driver->mididrv_resume(& midi_handle.handle);
		}
	}
	if (midi_handle.handle)
		midi_handle.driver->mididrv_advance (& midi_handle.handle);
}

