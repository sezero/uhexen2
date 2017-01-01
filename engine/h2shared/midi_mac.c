/* midi_mac.c -- MIDI module for Mac OS using QuickTime.
 * Based on the macglquake project with adjustments to make
 * it work with Mac OS X and Hexen II: Hammer of Thyrion.
 * (FIXME: Maybe update this to use QuickTime TunePlayer,
 *	   as in native_midi_mac of SDL_mixer, sometime?)
 *
 * Copyright (C) 2002  contributors of the macglquake project
 * Copyright (C) 2006  Levent Yavas
 * Copyright (C) 2006-2011  O.Sezer
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

#ifdef __LP64__
#error QuickTime midi is not for 64 bit
#endif

#include "quakedef.h"
#include "bgmusic.h"
#include "midi_drv.h"
#include <Carbon/Carbon.h>	/* <Sound.h> */
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
	char	midipath[MAX_OSPATH];
	byte	*buf;
	size_t	len;
	int	err = 0;
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

	/* midi files are small: safe to load onto hunk */
	buf = FS_LoadTempFile (filename, NULL);
	if (!buf)
	{
		Con_DPrintf("Couldn't open %s\n", filename);
		return NULL;
	}
	len = fs_filesize;

	/* Using NewMovieFromDataRef() to import midi data to QT
	 * movies would return -2048 if done wrong; using a Data
	 * Reference Extension is advised (TechNote 1195).  Lazy
	 * and ugly workaround is simply extracting the file.  */
	Con_DPrintf("Extracting %s from pakfile\n", filename);
	FS_MakePath_BUF(FS_USERBASE, &err, midipath, sizeof(midipath), TEMP_MIDINAME);
	if (err == 0)
	{
		FILE *f = fopen (midipath, "wb");
		if (!f) err = 1;
		else {
			err = (fwrite(buf, 1, len, f) != len);
			fclose (f);
		}
	}
	if (err != 0)
	{
		Con_Printf("Error extracting %s from pak\n", filename);
		return NULL;
	}
	err = FSPathMakeRef ((UInt8 *)midipath, &midiRef, NULL);
	if (err != noErr)
	{
		Con_Printf ("Error getting FSRef for %s\n", midipath);
		return NULL;
	}
	err = FSGetCatalogInfo (&midiRef, kFSCatInfoNone, NULL, NULL, &midiSpec, NULL);
	if (err != noErr)
	{
		Con_Printf ("Error getting FSSpec for %s\n", midipath);
		return NULL;
	}

	err = OpenMovieFile (&midiSpec, &midiRefNum, fsRdPerm);
	if (err != noErr)
	{
		Con_Printf ("OpenMovieStream error opening midi file\n");
		return NULL;
	}
	err = NewMovieFromFile (&midiTrack, midiRefNum, NULL, NULL, newMovieActive, NULL);
	if (err != noErr || !midiTrack)
	{
		Con_Printf ("QuickTime error in creating midi stream\n");
		return NULL;
	}

	CloseMovieFile (midiRefNum);	/* data now in memory */
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
