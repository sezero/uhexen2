/*
	midi_mac.c
	$Id: midi_mac.c,v 1.21 2009-01-22 08:06:19 sezero Exp $

	MIDI module for Mac OS X using QuickTime:
	Taken from the macglquake project with adjustments to make
	it work with Mac OS X and Hexen II: Hammer of Thyrion.

	Copyright (C) 2002  contributors of the macglquake project
	Copyright (C) 2006  Levent Yavas

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

#include "quakedef.h"
#include <Sound.h>
#include <QuickTime/Movies.h>

static Movie	midiTrack = NULL;
static qboolean	bMidiInited, bPaused, bLooped;
static float	old_volume = -1.0f;

static void MIDI_Play_f (void)
{
	if (Cmd_Argc () == 2)
	{
		MIDI_Play(Cmd_Argv(1));
	}
}

static void MIDI_Stop_f (void)
{
	MIDI_Stop();
}

static void MIDI_Pause_f (void)
{
	MIDI_Pause (MIDI_TOGGLE_PAUSE);
}

static void MIDI_Loop_f (void)
{
	if (Cmd_Argc () == 2)
	{
		if (q_strcasecmp(Cmd_Argv(1),"on") == 0 || q_strcasecmp(Cmd_Argv(1),"1") == 0)
			MIDI_Loop(MIDI_ENABLE_LOOP);
		else if (q_strcasecmp(Cmd_Argv(1),"off") == 0 || q_strcasecmp(Cmd_Argv(1),"0") == 0)
			MIDI_Loop(MIDI_DISABLE_LOOP);
		else if (q_strcasecmp(Cmd_Argv(1),"toggle") == 0)
			MIDI_Loop(MIDI_TOGGLE_LOOP);
	}

	if (bLooped)
		Con_Printf("MIDI music will be looped\n");
	else
		Con_Printf("MIDI music will not be looped\n");
}

static void MIDI_SetVolume (cvar_t *var)
{
	if (!bMidiInited || !midiTrack)
		return;

	if (var->value < 0.0)
		Cvar_SetValue (var->name, 0.0);
	else if (var->value > 1.0)
		Cvar_SetValue (var->name, 1.0);
	old_volume = var->value;
	SetMovieVolume(midiTrack, (short)(var->value * 256.0));
}

//
// MusicEvents
// Called in the event loop to keep track of MIDI music
//
void MIDI_Update(void)
{
	if (midiTrack)
	{
		// pOx - adjust volume if changed
		if (old_volume != bgmvolume.value)
			MIDI_SetVolume (&bgmvolume);

		// Let QuickTime get some time
		MoviesTask (midiTrack, 0);

		// If this song is looping, restart it
		if (IsMovieDone (midiTrack))
		{
			if (bLooped)
			{
				GoToBeginningOfMovie (midiTrack);
				StartMovie (midiTrack);
			}
			else
			{
				DisposeMovie (midiTrack);
				midiTrack = NULL;
			}
		}
	}
}

qboolean MIDI_Init(void)
{
	OSErr		theErr;

	bMidiInited = false;
	Con_Printf("%s: ", __thisfunc__);

	if (safemode || COM_CheckParm("-nomidi") || COM_CheckParm("-nosound") || COM_CheckParm("-s"))
	{
		Con_Printf("disabled by commandline\n");
		return false;
	}

	theErr = EnterMovies ();
	if (theErr != noErr)
	{
		Con_Printf ("Unable to initialize QuickTime.\n");
		return false;
	}

	Con_Printf("Started QuickTime midi.\n");

	Cmd_AddCommand ("midi_play", MIDI_Play_f);
	Cmd_AddCommand ("midi_stop", MIDI_Stop_f);
	Cmd_AddCommand ("midi_pause", MIDI_Pause_f);
	Cmd_AddCommand ("midi_loop", MIDI_Loop_f);

	bLooped = true;
	bPaused = false;
	bMidiInited = true;

	return true;
}


#define	TEMP_MUSICNAME	"tmpmusic"

void MIDI_Play (const char *Name)
{
	FILE		*midiFile;
	char	midiName[MAX_OSPATH], tempName[MAX_QPATH];
	OSErr	err;
	FSSpec	midiSpec;
	FSRef	midiRef;
	short	midiRefNum;

	if (!bMidiInited)	//don't try to play if there is no midi
		return;

	MIDI_Stop();

	if (!Name || !*Name)
	{
		Sys_Printf("no midi music to play\n");
		return;
	}

	q_snprintf (tempName, sizeof(tempName), "%s.%s", Name, "mid");
	FS_OpenFile (va("%s/%s", "midi", tempName), &midiFile, false);
	if (!midiFile)
	{
		Con_Printf("music file %s not found\n", tempName);
		return;
	}
	else
	{
		if (file_from_pak)
		{
			int		ret;

			Con_Printf("Extracting %s from pakfile\n", tempName);
			q_snprintf (midiName, sizeof(midiName), "%s/%s.%s",
							host_parms->userdir,
							TEMP_MUSICNAME,
							"mid" );
			ret = FS_CopyFromFile (midiFile, midiName, fs_filesize);
			fclose (midiFile);
			if (ret != 0)
			{
				Con_Printf("Error while extracting from pak\n");
				return;
			}
		}
		else	/* use the file directly */
		{
			fclose (midiFile);
			q_snprintf (midiName, sizeof(midiName), "%s/%s/%s",
							fs_filepath,
							"midi",
							tempName );
		}
	}

	// converting path to FSSpec. found in CarbonCocoaIntegration.pdf:
	// page 27, Obtaining an FSSpec Structure
	err = FSPathMakeRef ((UInt8*)midiName, &midiRef, NULL);
	if (err != noErr)
	{
		Con_Printf ("MIDI: FSPathMakeRef: error while opening %s\n", midiName);
		return;
	}

	err = FSGetCatalogInfo (&midiRef, kFSCatInfoNone, NULL, NULL, &midiSpec, NULL);
	if (err != noErr)
	{
		Con_Printf ("MIDI: FSGetCatalogInfo: error while opening %s\n", midiName);
		return;
	}

	err = OpenMovieFile (&midiSpec, &midiRefNum, fsRdPerm);
	if (err != noErr)
	{
		Con_Printf ("MIDI: OpenMovieStream: error opening midi file\n");
		return;
	}

	err = NewMovieFromFile (&midiTrack, midiRefNum, NULL, NULL, newMovieActive, NULL);
	if (err != noErr || !midiTrack)
	{
		Con_Printf ("MIDI: QuickTime error in creating stream.\n");
		return;
	}

	GoToBeginningOfMovie (midiTrack);
	PrerollMovie (midiTrack, 0, 0);

	// pOx - set initial volume
	MIDI_SetVolume (&bgmvolume);

	StartMovie (midiTrack);
	Con_Printf ("Started midi music %s\n", tempName);
}

void MIDI_Pause(int mode)
{
	if (!midiTrack)
		return;

	if ((mode == MIDI_TOGGLE_PAUSE && bPaused) || mode == MIDI_ALWAYS_RESUME)
	{
		StartMovie (midiTrack);
		bPaused = false;
	}
	else
	{
		StopMovie (midiTrack);
		bPaused = true;
	}
}

void MIDI_Loop(int mode)
{
	switch (mode)
	{
	case MIDI_TOGGLE_LOOP:
		bLooped = !bLooped;
		break;
	case MIDI_DISABLE_LOOP:
		bLooped = false;
		break;
	case MIDI_ENABLE_LOOP:
	default:
		bLooped = true;
		break;
	}
}

void MIDI_Stop(void)
{
	if (!bMidiInited)	//Just to be safe
		return;

	if (midiTrack)
	{
		StopMovie (midiTrack);
		DisposeMovie (midiTrack);
		midiTrack = NULL;
		bPaused = false;
	}
}

void MIDI_Cleanup(void)
{
	if (bMidiInited)
	{
		MIDI_Stop();
		bMidiInited = false;
		Con_Printf("%s: closing QuickTime.\n", __thisfunc__);
		ExitMovies ();
	}
}

