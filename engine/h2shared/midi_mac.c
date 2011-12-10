/*
	midi_mac.c
	$Id$

	MIDI module for Mac OS X using QuickTime:
	Based the macglquake project with adjustments to make
	it work with Mac OS X and Hexen II: Hammer of Thyrion.

	Copyright (C) 2002  contributors of the macglquake project
	Copyright (C) 2006  Levent Yavas
	Copyright (C) 2006-2011  O.Sezer

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
#include "bgmusic.h"
#include "midi_drv.h"
#include <Sound.h>
#include <QuickTime/Movies.h>

static Movie	midiTrack = NULL;
static qboolean	midi_paused;

/* prototypes of functions exported to BGM: */
static void *MIDI_Play (const char *filename);
static void MIDI_Update (void **handle);
static void MIDI_Rewind (void **handle);
static void MIDI_Stop (void **handle);
static void MIDI_Pause (void **handle);
static void MIDI_Resume (void **handle);
static void MIDI_SetVolume (void **handle, float value);

static midi_driver_t midi_mac_qt =
{
	false, /* init success */
	"QuickTime midi for Mac",
	MIDI_Init,
	MIDI_Cleanup,
	MIDI_Play,
	MIDI_Update,
	MIDI_Rewind,
	MIDI_Stop,
	MIDI_Pause,
	MIDI_Resume,
	MIDI_SetVolume,
	NULL
};


#define CHECK_MIDI_ALIVE()		\
do {					\
	if (!midiTrack)			\
	{				\
		if (handle)		\
			*handle = NULL;	\
		return;			\
	}				\
} while (0)

static void MIDI_SetVolume (void **handle, float value)
{
	CHECK_MIDI_ALIVE();

	SetMovieVolume (midiTrack, (short)(value * 256.0f));
}

static void MIDI_Rewind (void **handle)
{
	CHECK_MIDI_ALIVE();

	GoToBeginningOfMovie (midiTrack);
	StartMovie (midiTrack);
}

static void MIDI_Update (void **handle)
{
	CHECK_MIDI_ALIVE();

	/* let QuickTime get some time */
	MoviesTask (midiTrack, 0);
	if (IsMovieDone (midiTrack))
	{
		if (bgmloop)
		{
			GoToBeginningOfMovie (midiTrack);
			StartMovie (midiTrack);
		}
		else
		{
			DisposeMovie (midiTrack);
			midiTrack = NULL;
			if (handle)
				*handle = NULL;
		}
	}
}

qboolean MIDI_Init(void)
{
	OSErr	theErr;

	if (midi_mac_qt.available)
		return true;

	BGM_RegisterMidiDRV(&midi_mac_qt);

	if (safemode || COM_CheckParm("-nomidi"))
		return false;

	theErr = EnterMovies ();
	if (theErr != noErr)
	{
		Con_Printf ("Unable to initialize QuickTime.\n");
		return false;
	}

	Con_Printf("%s initialized.\n", midi_mac_qt.desc);

	midi_paused = false;
	midi_mac_qt.available = true;

	return true;
}

static void *MIDI_Play (const char *filename)
{
#define	TEMP_MIDINAME	"tmpmusic.mid"
	FILE	*f;
	char	midipath[MAX_OSPATH];
	OSErr	err;
	FSSpec	midiSpec;
	FSRef	midiRef;
	short	midiRefNum;

	if (!midi_mac_qt.available)
		return NULL;

	if (!filename || !*filename)
	{
		Con_DPrintf("null music file name\n");
		return NULL;
	}

	FS_OpenFile (filename, &f, NULL);
	if (!f)
	{
		Con_DPrintf("Couldn't open %s\n", filename);
		return NULL;
	}

	/* FIXME: is there not an api with which I can send the
	 * midi data from memory and avoid this utter crap?? */
	if (file_from_pak)
	{
		int		ret;

		Con_DPrintf("Extracting %s from pakfile\n", filename);
		q_snprintf (midipath, sizeof(midipath), "%s/%s",
			    host_parms->userdir, TEMP_MIDINAME);
		ret = FS_WriteFileFromHandle (f, midipath, fs_filesize);
		fclose (f);
		if (ret != 0)
		{
			Con_Printf("Error while extracting from pak\n");
			return NULL;
		}
	}
	else	/* use the file directly */
	{
		fclose (f);
		q_snprintf (midipath, sizeof(midipath), "%s/%s",
			    fs_filepath, filename);
	}

	/* converting path to FSSpec. found in CarbonCocoaIntegration.pdf:
	 * page 27, Obtaining an FSSpec Structure */
	err = FSPathMakeRef ((UInt8 *)midipath, &midiRef, NULL);
	if (err != noErr)
	{
		Con_Printf ("MIDI_DRV: FSPathMakeRef error while opening %s\n", midipath);
		return NULL;
	}

	err = FSGetCatalogInfo (&midiRef, kFSCatInfoNone, NULL, NULL, &midiSpec, NULL);
	if (err != noErr)
	{
		Con_Printf ("MIDI_DRV: FSGetCatalogInfo error while opening %s\n", midipath);
		return NULL;
	}

	err = OpenMovieFile (&midiSpec, &midiRefNum, fsRdPerm);
	if (err != noErr)
	{
		Con_Printf ("MIDI_DRV: OpenMovieStream error opening midi file\n");
		return NULL;
	}

	err = NewMovieFromFile (&midiTrack, midiRefNum, NULL, NULL, newMovieActive, NULL);
	if (err != noErr || !midiTrack)
	{
		Con_Printf ("MIDI_DRV: QuickTime error in creating stream.\n");
		return NULL;
	}

	GoToBeginningOfMovie (midiTrack);
	PrerollMovie (midiTrack, 0, 0);

	SetMovieVolume (midiTrack, (short)(bgmvolume.value * 256.0f));

	StartMovie (midiTrack);
	Con_Printf ("Started midi music %s\n", filename);

	return midiTrack;
}

static void MIDI_Pause (void **handle)
{
	CHECK_MIDI_ALIVE();

	if (!midi_paused)
	{
		StopMovie (midiTrack);
		midi_paused = true;
	}
}

static void MIDI_Resume (void **handle)
{
	CHECK_MIDI_ALIVE();

	if (midi_paused)
	{
		StartMovie (midiTrack);
		midi_paused = false;
	}
}

static void MIDI_Stop (void **handle)
{
	CHECK_MIDI_ALIVE();

	StopMovie (midiTrack);
	DisposeMovie (midiTrack);
	midiTrack = NULL;
	midi_paused = false;
}

void MIDI_Cleanup(void)
{
	if (midi_mac_qt.available)
	{
		midi_mac_qt.available = false;
		Con_Printf("%s: closing QuickTime.\n", __thisfunc__);
		ExitMovies ();
	}
}

