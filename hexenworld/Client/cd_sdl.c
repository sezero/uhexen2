/*
	cd_sdl.c
	$Id: cd_sdl.c,v 1.5 2005-10-25 20:08:41 sezero Exp $

	Copyright (C) 2001  Mark Baker <homer1@together.net>
	Taken from SDLquake with modifications to make it work
	with Linux Hexen II: Hammer of Thyrion

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
		59 Temple Place - Suite 330
		Boston, MA  02111-1307, USA

*/

#include <SDL.h>
#include "quakedef.h"

static qboolean cdValid = false;
static qboolean	initialized = false;
static qboolean	enabled = true;
static qboolean playLooping = false;
static SDL_CD *cd_id;
static float cdvolume = 1.0;

static void CDAudio_Eject(void)
{
	if (!cd_id || !enabled)
		return;

	if (SDL_CDEject(cd_id))
		Con_DPrintf("Unable to eject CD-ROM tray.\n");
}

void CDAudio_Play(byte track, qboolean looping)
{
	CDstatus cd_stat;

	if (!cd_id || !enabled)
		return;
	
	cd_stat=SDL_CDStatus(cd_id);

	if (!cdValid)
	{
		if (!CD_INDRIVE(cd_stat) || (!cd_id->numtracks))
			return;
		cdValid = true;
	}

	if ((track < 1) || (track >= cd_id->numtracks))
	{
		Con_DPrintf("CDAudio: Bad track number %d.\n", track);
		return;
	}

	track--; /* Convert track from person to SDL value */

	if (cd_stat == CD_PLAYING)
	{
		if(cd_id->cur_track == track)
			return;
		CDAudio_Stop();
	}

	if (SDL_CDPlay(cd_id,cd_id->track[track].offset, cd_id->track[track].length))
	{
		Con_DPrintf("CDAudio_Play: Unable to play track: %d\n",track+1);
		return;
	}

	playLooping = looping;
}


void CDAudio_Stop(void)
{
	int cdstate;

	if (!cd_id || !enabled)
		return;

	cdstate = SDL_CDStatus(cd_id);

	if ((cdstate != CD_PLAYING) && (cdstate != CD_PAUSED))
		return;

	if (SDL_CDStop(cd_id))
		Con_DPrintf("CDAudio_Stop: Failed to stop track.\n");
}

void CDAudio_Pause(void)
{
	if (!cd_id || !enabled)
		return;

	if (SDL_CDStatus(cd_id) != CD_PLAYING)
		return;

	if (SDL_CDPause(cd_id))
		Con_DPrintf("CDAudio_Pause: Failed to pause track.\n");
}


void CDAudio_Resume(void)
{
	if (!cd_id || !enabled)
		return;

	if (SDL_CDStatus(cd_id) != CD_PAUSED)
		return;

	if (SDL_CDResume(cd_id))
		Con_DPrintf("CDAudio_Resume: Failed tp resume track.\n");
}

static void CD_f (void)
{
	char	*command;
	int	cdstate;

	if (Cmd_Argc() < 2)
	{
		Con_Printf("commands:");
		Con_Printf("on, off, play, stop, \n");
		Con_Printf("loop, pause, resume\n");
		Con_Printf("eject, close, info\n");
		return;
	}

	command = Cmd_Argv (1);

	if (Q_strcasecmp(command, "on") == 0)
	{
		enabled = true;
		return;
	}

	if (Q_strcasecmp(command, "off") == 0)
	{
		if (!cd_id)
			return;

		cdstate = SDL_CDStatus(cd_id);

		if((cdstate == CD_PLAYING) || (cdstate == CD_PAUSED))
			CDAudio_Stop();

		enabled = false;

		return;
	}

	if (Q_strcasecmp(command, "play") == 0)
	{
		CDAudio_Play((byte)atoi(Cmd_Argv (2)), false);
		return;
	}

	if (Q_strcasecmp(command, "loop") == 0)
	{
		CDAudio_Play((byte)atoi(Cmd_Argv (2)), true);
		return;
	}

	if (Q_strcasecmp(command, "stop") == 0)
	{
		CDAudio_Stop();
		return;
	}

	if (Q_strcasecmp(command, "pause") == 0)
	{
		CDAudio_Pause();
		return;
	}

	if (Q_strcasecmp(command, "resume") == 0)
	{
		CDAudio_Resume();
		return;
	}

	if (Q_strcasecmp(command, "eject") == 0)
	{
		CDAudio_Eject();
		cdValid = false;
		return;
	}

	if (Q_strcasecmp(command, "info") == 0)
	{
		if(!cd_id)
			return;

		cdstate = SDL_CDStatus(cd_id);

		Con_Printf("%d tracks\n", cd_id->numtracks);

		if (cdstate == CD_PLAYING)
		{
			Con_Printf("Currently %s track %d\n",
				playLooping ? "looping" : "playing",
				cd_id->cur_track+1);
		}
		else if (cdstate == CD_PAUSED)
		{
			Con_Printf("Paused %s track %d\n",
				playLooping ? "looping" : "playing",
				cd_id->cur_track+1);
		}

		return;
	}
}

void CDAudio_Update(void)
{
	if (!cd_id || !enabled)
		return;

	// if SDL supports cdrom volume control some day
	// I'll write proper code in here

	if (bgmvolume.value != cdvolume)
	{
		cdvolume = bgmvolume.value;
		if (cdvolume == 0)
		{
			CDAudio_Pause();
		}
		else
		{
			CDAudio_Resume();
		}

		return;
	}

	if (playLooping && (SDL_CDStatus(cd_id) != CD_PLAYING)
			&& (SDL_CDStatus(cd_id) != CD_PAUSED))
		CDAudio_Play(cd_id->cur_track+1,true);
}

int CDAudio_Init(void)
{
	if (COM_CheckParm("-nocdaudio"))
		return -1;

	if (SDL_InitSubSystem(SDL_INIT_CDROM) < 0)
	{
		Con_Printf("Couldn't init SDL cdrom: %s\n", SDL_GetError());
		return -1;
	}

	cd_id = SDL_CDOpen(0);
	if (!cd_id)
	{
		Con_Printf("CDAudio_Init: Unable to open default CD-ROM drive: %s\n",
			SDL_GetError());
		return -1;
	}

	initialized = true;
	enabled = true;
	cdValid = true;

	Con_Printf("CDAudio initialized (using SDL)\n");

	if (!CD_INDRIVE(SDL_CDStatus(cd_id)))
	{
		Con_Printf("CDAudio_Init: No CD in drive\n");
		cdValid = false;
	}

	if (cdValid && !cd_id->numtracks)
	{
		Con_Printf("CDAudio_Init: CD contains no audio tracks.\n");
		cdValid = false;
	}

	Cmd_AddCommand ("cd", CD_f);

	return 0;
}


void CDAudio_Shutdown(void)
{
	if (!cd_id)
		return;
	CDAudio_Stop();
	SDL_CDClose(cd_id);
	cd_id = NULL;
	SDL_QuitSubSystem(SDL_INIT_CDROM);
}

