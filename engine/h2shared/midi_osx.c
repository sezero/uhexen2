/*
 * $Id$
 * based on an SDL_mixer code:
 * native_midi_macosx:  Native Midi support on Mac OS X for the SDL_mixer library
 * Copyright (C) 2009  Ryan C. Gordon <icculus@icculus.org>
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 * Adaptation to uHexen2:
 * Copyright (C) 2012  O. Sezer <sezero@users.sourceforge.net>
 *
 * Only for Mac OS X using Core MIDI and requiring version
 * 10.2 or later. Use QuickTime with midi_mac.c, otherwise.
 */

#include "quakedef.h"
#include "bgmusic.h"
#include "midi_drv.h"

#include <CoreServices/CoreServices.h>		/* ComponentDescription */
#include <AudioUnit/AudioUnit.h>
#include <AudioToolbox/AudioToolbox.h>
#include <AvailabilityMacros.h>

#if (MAC_OS_X_VERSION_MIN_REQUIRED < 1050)
#define AUGraphNodeInfo_FN(_graph,_node,_desc,_unit)			\
	AUGraphGetNodeInfo((_graph),(_node),(_desc),NULL,NULL,(_unit))
typedef struct ComponentDescription	AudioComponentDesc_t;
/* MusicSequenceLoadSMFData() (avail. in 10.2, no 64 bit) is
 * equivalent to calling MusicSequenceLoadSMFDataWithFlags()
 * with a flags value of 0 (avail. in 10.3, avail. 64 bit).
 * So, we use MusicSequenceLoadSMFData() for powerpc versions
 * but the *WithFlags() on intel which require 10.4 anyway. */
#if defined(__ppc__) || defined(__POWERPC__)
#define MusicSequenceFileLoadData_FN(_seq,_data,_ftid,_flag)	\
	MusicSequenceLoadSMFData((_seq),(_data))
#else	/* 10.4 - intel: */
#define MusicSequenceFileLoadData_FN(_seq,_data,_ftid,_flag)	\
	MusicSequenceLoadSMFDataWithFlags((_seq),(_data),(_flag))
#endif

#else	/* 10.5 OR NEWER: */

#define AUGraphNodeInfo_FN(_graph,_node,_desc,_unit)		\
	AUGraphNodeInfo((_graph),(_node),(_desc),(_unit))
/* AUGraphNodeInfo() is changed to take an AudioComponentDescription*
 * desc parameter instead of a ComponentDescription* in the 10.6 SDK.
 * AudioComponentDescription is in 10.6 or newer SDKs, but is actually
 * the same as ComponentDescription with 20 bytes of size and same
 * offsets and names of all members, therefore is binary compatible. */
#if !defined(AUDIO_UNIT_VERSION) || ((AUDIO_UNIT_VERSION + 0) < 1060)
typedef struct ComponentDescription	AudioComponentDesc_t;
#else	/* SDK 10.6 or newer : */
typedef AudioComponentDescription	AudioComponentDesc_t;
#endif
#define MusicSequenceFileLoadData_FN(_seq,_data,_ftid,_flag)	\
	MusicSequenceFileLoadData((_seq),(_data),(_ftid),(_flag))
#endif	/* -- compat macros */

/* prototypes of functions exported to BGM: */
static void *MIDI_Play (const char *filename);
static void MIDI_Update (void **handle);
static void MIDI_Rewind (void **handle);
static void MIDI_Stop (void **handle);
static void MIDI_Pause (void **handle);
static void MIDI_Resume (void **handle);
static void MIDI_SetVolume (void **handle, float value);

static midi_driver_t midi_mac_core =
{
	false, /* init success */
	"Core MIDI for Mac OS X",
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

typedef struct _CoreMidiSong
{
	MusicPlayer	player;
	MusicSequence	sequence;
	MusicTimeStamp	endTime;
	MusicTimeStamp	pauseTime;
	AudioUnit	audiounit;
} CoreMidiSong;

static CoreMidiSong	*currentsong = NULL;
static qboolean	midi_paused;

#define CHECK_MIDI_ALIVE()		\
do {					\
	if (!currentsong)		\
	{				\
		if (handle)		\
			*handle = NULL;	\
		return;			\
	}				\
} while (0)

static void MIDI_SetVolume (void **handle, float value)
{
	CHECK_MIDI_ALIVE();

	if (currentsong->audiounit)
	{
		AudioUnitSetParameter(currentsong->audiounit, kHALOutputParam_Volume,
				      kAudioUnitScope_Global, 0, value, 0);
	}
}

static void MIDI_Rewind (void **handle)
{
	CHECK_MIDI_ALIVE();
	MusicPlayerSetTime(currentsong->player, 0);
}

static void MIDI_Update (void **handle)
{
	MusicTimeStamp currentTime;
	CHECK_MIDI_ALIVE();

	currentTime = 0;
	MusicPlayerGetTime(currentsong->player, &currentTime);
	if (currentTime < currentsong->endTime ||
	    currentTime >= kMusicTimeStamp_EndOfTrack)
	{
		return;
	}
	if (bgmloop)
	{
		MusicPlayerSetTime(currentsong->player, 0);
	}
	else
	{
		MIDI_Stop(NULL);
		if (handle)
			*handle = NULL;
	}
}

qboolean MIDI_Init(void)
{
	if (midi_mac_core.available)
		return true;

	BGM_RegisterMidiDRV(&midi_mac_core);

	if (safemode || COM_CheckParm("-nomidi"))
		return false;

	midi_mac_core.available = true;		/* always available. */
	return true;
}

static OSStatus GetSequenceLength(MusicSequence sequence, MusicTimeStamp *_sequenceLength)
{
/* http://lists.apple.com/archives/Coreaudio-api/2003/Jul/msg00370.html
 * figure out sequence length  */
	static qboolean old_osx = false;
	UInt32 ntracks, i;
	MusicTimeStamp sequenceLength = 0;
	OSStatus err;

	err = MusicSequenceGetTrackCount(sequence, &ntracks);
	if (err != noErr)
		return err;

	for (i = 0; i < ntracks; ++i)
	{
		MusicTrack track;
		MusicEventIterator iter = NULL;
		MusicTimeStamp tracklen = 0;
		UInt32 tracklenlen = sizeof (tracklen);

		err = MusicSequenceGetIndTrack(sequence, i, &track);
		if (err != noErr)
			return err;
		if (!old_osx)
		{
			/* kSequenceTrackProperty_TrackLength (5) needs 10.3 and newer.
			 * so, the following returns error when run on 10.2 or older. */
			err = MusicTrackGetProperty(track, 5, &tracklen, &tracklenlen);
			if (err != noErr)
			{
				if (i != 0) return err;
				old_osx = true;
				goto	altern;
			}
			if (sequenceLength < tracklen)
				sequenceLength = tracklen;
		}
		else {
		altern:
			if ((err = NewMusicEventIterator(track, &iter)) != noErr) goto fail;
			if ((err = MusicEventIteratorSeek(iter, kMusicTimeStamp_EndOfTrack))
									!= noErr) goto fail;
			if (!MusicEventIteratorPreviousEvent(iter))
			{
				if (!MusicEventIteratorGetEventInfo(iter, &tracklen,
								    NULL, NULL, NULL))
				{
					tracklen += 4; /* add 4 beats - nice and arbitrary!!! */
					if (tracklen > sequenceLength)
						sequenceLength = tracklen;
				}
			}
		fail:
			if (iter) DisposeMusicEventIterator (iter);
			if (err != noErr) return err;
		}
	}

	*_sequenceLength = sequenceLength;
	Con_DPrintf("MIDI sequence len: %f\n", sequenceLength);

	return noErr;
}

/* we're looking for the sequence output audiounit. */
static OSStatus GetSequenceAudioUnit(MusicSequence sequence, AudioUnit *aunit)
{
	AUGraph graph;
	UInt32 nodecount, i;
	OSStatus err;

	err = MusicSequenceGetAUGraph(sequence, &graph);
	if (err != noErr)
		return err;

	err = AUGraphGetNodeCount(graph, &nodecount);
	if (err != noErr)
		return err;

	for (i = 0; i < nodecount; i++)
	{
		AUNode node;
		AudioComponentDesc_t desc;

		if (AUGraphGetIndNode(graph, i, &node) != noErr)
			continue;

		if (AUGraphNodeInfo_FN(graph, node, &desc, aunit) != noErr)
			continue;
		if (desc.componentType != kAudioUnitType_Output)
			continue;
		if (desc.componentSubType != kAudioUnitSubType_DefaultOutput)
			continue;

		return noErr;	/* found it! */
	}

	return kAUGraphErr_NodeNotFound;
}

static void *MIDI_Play (const char *filename)
{
	byte *buf;
	size_t len;
	CoreMidiSong *song = NULL;
	CFDataRef data = NULL;

	if (!midi_mac_core.available)
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

	song = (CoreMidiSong *) Z_Malloc(sizeof(CoreMidiSong), Z_MAINZONE);

	if (NewMusicPlayer(&song->player) != noErr)
		goto fail;
	if (NewMusicSequence(&song->sequence) != noErr)
		goto fail;

	data = CFDataCreate(NULL, (const UInt8 *) buf, len);
	if (data == NULL)
		goto fail;

	if (MusicSequenceFileLoadData_FN(song->sequence, data, 0, 0) != noErr)
		goto fail;

	CFRelease(data);
	data = NULL;

	if (GetSequenceLength(song->sequence, &song->endTime) != noErr)
		goto fail;
	if (MusicPlayerSetSequence(song->player, song->sequence) != noErr)
		goto fail;

	MusicPlayerPreroll(song->player);
	MusicPlayerSetTime(song->player, 0);
	MusicPlayerStart(song->player);

	currentsong = song;
	midi_paused = false;

	GetSequenceAudioUnit(song->sequence, &song->audiounit);
	if (currentsong->audiounit)
	{
		AudioUnitSetParameter(currentsong->audiounit, kHALOutputParam_Volume,
				      kAudioUnitScope_Global, 0, bgmvolume.value, 0);
	}

	return song;

fail:
	currentsong = NULL;
	if (song) {
		if (song->sequence)
			DisposeMusicSequence(song->sequence);
		if (song->player)
			DisposeMusicPlayer(song->player);
		Z_Free(song);
	}
	if (data)
		CFRelease(data);

	return NULL;
}

static void MIDI_Pause (void **handle)
{
	CHECK_MIDI_ALIVE();
	if (!midi_paused)
	{
		MusicPlayerGetTime(currentsong->player, &currentsong->pauseTime);
		MusicPlayerStop(currentsong->player);
		midi_paused = true;
	}
}

static void MIDI_Resume (void **handle)
{
	CHECK_MIDI_ALIVE();
	if (midi_paused)
	{
		MusicPlayerSetTime(currentsong->player, currentsong->pauseTime);
		MusicPlayerStart(currentsong->player);
		midi_paused = false;
	}
}

static void MIDI_Stop (void **handle)
{
	CHECK_MIDI_ALIVE();

	MusicPlayerStop(currentsong->player);
	DisposeMusicSequence(currentsong->sequence);
	DisposeMusicPlayer(currentsong->player);
	Z_Free(currentsong);
	currentsong = NULL;
}

void MIDI_Cleanup(void)
{
	if (midi_mac_core.available)
	{
		Con_Printf("%s: closing Core MIDI.\n", __thisfunc__);
		MIDI_Stop (NULL);
		midi_mac_core.available = false;
	}
}

