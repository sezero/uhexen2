/*
 * midi_win.c -- MIDI module for Windows using midiStream API
 *
 * Originally from Hexen II source (C) Raven Software Corp.
 * based on an old DirectX5 sample code.
 * Few bits from Doom Legacy: Copyright (C) 1998-2000 by DooM Legacy Team.
 * Multiple fixes and cleanups and adaptation into new Hammer of Thyrion
 * (uHexen2) code by O.Sezer:
 * Copyright (C) 2006-2011 O.Sezer <sezero@users.sourceforge.net>
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

#include <windows.h>
#include <commctrl.h>
#include <memory.h>
/*#include <mmreg.h>*/
#include <mmsystem.h>

#include "midifile.h"
#include "mid2strm.h"
#include "quakedef.h"
#include "winquake.h"
#include "bgmusic.h"
#include "midi_drv.h"


/* prototypes of functions exported to BGM: */
static void *MIDI_Play (const char *filename);
static void MIDI_Update (void **handle);
static void MIDI_Rewind (void **handle);
static void MIDI_Stop (void **handle);
static void MIDI_Pause (void **handle);
static void MIDI_Resume (void **handle);
static void MIDI_SetVolume (void **handle, float value);

static midi_driver_t midi_win_ms =
{
	false, /* init success */
	"midiStream for Windows",
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

/* macros to be used with windows MIDIEVENT structure -> dwEvent */
#define MIDIEVENT_CHANNEL(x)	(x & 0x0000000F)
#define MIDIEVENT_TYPE(x)	(x & 0x000000F0)
#define MIDIEVENT_DATA1(x)	((x & 0x0000FF00) >> 8)
#define MIDIEVENT_VOLUME(x)	((x & 0x007F0000) >> 16)

static qboolean	midi_file_open, midi_playing, midi_paused;
static UINT	device_id = MIDI_MAPPER, callback_status;
static int	buf_num, num_empty_bufs;
static DWORD	volume_cache[MIDI_CHANNELS];
static qboolean	hw_vol_capable = false;

static HMIDISTRM	hStream;
static convert_buf_t	stream_bufs[NUM_STREAM_BUFFERS];

static HANDLE		hBufferReturnEvent;

static void FreeBuffers (void);
static int  StreamBufferSetup (const char *filename);
static void CALLBACK MidiProc (HMIDIIN, UINT, DWORD_PTR, DWORD_PTR, DWORD_PTR);
static void SetAllChannelVolumes (DWORD volume_percent);
/*static void SetChannelVolume (DWORD channel_num, DWORD volume_percent);*/


#define CHECK_MIDI_ALIVE()		\
do {					\
	if (!midi_playing)		\
	{				\
		if (handle)		\
			*handle = NULL;	\
		return;			\
	}				\
} while (0)

static void MidiErrorMessageBox (MMRESULT mmr)
{
	char temp[1024];

	midiOutGetErrorText(mmr, temp, sizeof(temp));
	Con_Printf("MIDI_DRV: %s\n", temp);
}

static void MIDI_SetVolume (void **handle, float value)
{
	DWORD val;

	CHECK_MIDI_ALIVE();

	if (hw_vol_capable)
	{
		val = (DWORD)(value * 65535.0f);
		midiOutSetVolume((HMIDIOUT)hStream, (val << 16) + val);
	}
	else
	{
		val = (DWORD)(value * 1000.0f);
		SetAllChannelVolumes (val);
	}
}

static void MIDI_Rewind (void **handle)
{
	CHECK_MIDI_ALIVE();

	/* handled by converter module */
}

static void MIDI_Update (void **handle)
{
	CHECK_MIDI_ALIVE();

	/* handled by callback */
}

qboolean MIDI_Init(void)
{
	MMRESULT mmr;
	MIDIOUTCAPS midi_caps;

	if (midi_win_ms.available)
		return true;

	BGM_RegisterMidiDRV(&midi_win_ms);

	if (safemode || COM_CheckParm("-nomidi"))
		return false;

	hBufferReturnEvent = CreateEvent(NULL, FALSE, FALSE, "uHexen2 Midi: Wait For Buffer Return");

	mmr = midiStreamOpen(&hStream, &device_id, (DWORD)1, (DWORD_PTR)MidiProc, (DWORD_PTR)0, CALLBACK_FUNCTION);
	if (mmr != MMSYSERR_NOERROR)
	{
		MidiErrorMessageBox(mmr);
		return false;
	}

	midi_file_open = false;
	midi_playing = false;
	midi_paused = false;
	callback_status = 0;
	midi_win_ms.available = true;

	Con_Printf("%s initialized.\n", midi_win_ms.desc);

	/* try to see if the MIDI device supports midiOutSetVolume */
	if (midiOutGetDevCaps(device_id, &midi_caps, sizeof(midi_caps)) == MMSYSERR_NOERROR)
	{
		if (midi_caps.dwSupport & MIDICAPS_VOLUME)
		{
			if (COM_CheckParm("-nohwmidivol"))
				Con_Printf("Hardware MIDI volume disabled by user\n");
			else if (WinVista)
			/*
			http://msdn.microsoft.com/en-us/library/dd798480(VS.85).aspx#1
			"This [midiOutSetVolume] function does not set the MIDI device
			volume when using a software synthesizer under Windows Vista
			or Windows 7, but instead alters the application-specific volume
			level in the system mixer. This means that if your application
			also outputs digital audio, the volume level of that audio will
			be reduced or increased by the same amount."
			*/
				Con_Printf("Hardware MIDI volume ignored (Vista/7)\n");
			else
			{
				hw_vol_capable = true;
				Con_Printf("Using hardware MIDI volume adjustment\n");
			}
		}
	}

	return true;
}

static void *MIDI_Play (const char *filename)
{
	MMRESULT mmr;

	if (!midi_win_ms.available)
		return NULL;

	if (!filename || !*filename)
	{
		Con_DPrintf("null music file name\n");
		return NULL;
	}

	if (StreamBufferSetup(filename))
	{
		Con_DPrintf("Couldn't open %s\n", filename);
		return NULL;
	}

	Con_Printf("Started midi music %s\n", filename);
	midi_file_open = true;
	callback_status = 0;

	mmr = midiStreamRestart(hStream);
	if (mmr != MMSYSERR_NOERROR)
	{
		MidiErrorMessageBox(mmr);
		return NULL;
	}

	midi_playing = true;
	midi_paused = false;
	MIDI_SetVolume ((void **) &hStream, bgmvolume.value);

	return hStream;
}

static void MIDI_Pause (void **handle)
{
	CHECK_MIDI_ALIVE();

	if (!midi_paused)
	{
		midi_paused = true;
		midiStreamPause(hStream);
	}
}

static void MIDI_Resume (void **handle)
{
	CHECK_MIDI_ALIVE();

	if (midi_paused)
	{
		midi_paused = false;
		midiStreamRestart(hStream);
	}
}

static void MIDI_Stop (void **handle)
{
	MMRESULT mmr;

	/*CHECK_MIDI_ALIVE();*/
	if (handle)
		*handle = NULL;

	if (midi_file_open || midi_playing)/* || callback_status != STATUS_CALLBACKDEAD)*/
	{
		midi_playing = midi_paused = false;
		if (callback_status != STATUS_CALLBACKDEAD && callback_status != STATUS_WAITINGFOREND)
			callback_status = STATUS_KILLCALLBACK;

		mmr = midiStreamStop(hStream);
		if (mmr != MMSYSERR_NOERROR)
		{
			MidiErrorMessageBox(mmr);
			return;
		}

		mmr = midiOutReset((HMIDIOUT)hStream);
		if (mmr != MMSYSERR_NOERROR)
		{
			MidiErrorMessageBox(mmr);
			return;
		}

		if (WaitForSingleObject(hBufferReturnEvent,DEBUG_CALLBACK_TIMEOUT) == WAIT_TIMEOUT)
		{
			Con_DPrintf("Timed out waiting for MIDI callback\n");
			callback_status = STATUS_CALLBACKDEAD;
		}
	}

	if (callback_status == STATUS_CALLBACKDEAD)
	{
		callback_status = 0;
		if (midi_file_open)
		{
			ConverterCleanup();
			FreeBuffers();
			if (hStream)
			{
				mmr = midiStreamClose(hStream);
				if (mmr != MMSYSERR_NOERROR)
					MidiErrorMessageBox(mmr);
				hStream = NULL;
			}

			midi_file_open = false;
		}
	}
}

void MIDI_Cleanup(void)
{
	MMRESULT mmr;

	if (!midi_win_ms.available)
		return;

	midi_win_ms.available = false;

	CloseHandle(hBufferReturnEvent);

	if (hStream)
	{
		mmr = midiStreamClose(hStream);
		if (mmr != MMSYSERR_NOERROR)
			MidiErrorMessageBox(mmr);
		hStream = NULL;
	}
}

/* FreeBuffers
 *
 * unprepares and frees all our buffers -- something we must do to
 * work around a bug in MMYSYSTEM that prevents a device from playing
 * back properly unless it is closed and reopened after each stop.
 */
static void FreeBuffers(void)
{
	int i;
	MMRESULT mmr;

	for (i = 0; i < NUM_STREAM_BUFFERS; i++)
	{
		if (stream_bufs[i].prepared)
		{
			stream_bufs[i].prepared = FALSE;
			mmr = midiOutUnprepareHeader((HMIDIOUT)hStream, &stream_bufs[i].mh, sizeof(MIDIHDR));
			if (mmr != MMSYSERR_NOERROR)
				MidiErrorMessageBox(mmr);
		}

		if (stream_bufs[i].mh.lpData)
		{
			Z_Free(stream_bufs[i].mh.lpData);
			stream_bufs[i].mh.lpData = NULL;
		}
	}
}

/* StreamBufferSetup
 *
 * Uses the filename to open a MIDI file. Then goes
 * about converting at least the first part of
 * that file into a midiStream buffer for playback.
 */
static int StreamBufferSetup(const char *filename)
{
	int err, i;
	qboolean found_end = false;
	unsigned int flags;
	MMRESULT mmr;
	MIDIPROPTIMEDIV mptd;

	if (!hStream)
	{
		mmr = midiStreamOpen(&hStream, &device_id, (DWORD)1, (DWORD_PTR)MidiProc, (DWORD_PTR)0, CALLBACK_FUNCTION);
		if (mmr != MMSYSERR_NOERROR)
		{
			MidiErrorMessageBox(mmr);
			return 1;
		}
	}

	for (i = 0; i < NUM_STREAM_BUFFERS; i++)
	{
		stream_bufs[i].mh.dwBufferLength = OUT_BUFFER_SIZE;
		stream_bufs[i].mh.lpData = (LPSTR) Z_Malloc(OUT_BUFFER_SIZE, Z_MAINZONE);
	}

	if (ConverterInit(filename))
		return 1;

	for (i = 0; i < MIDI_CHANNELS; i++)
		volume_cache[i] = VOL_CACHE_INIT;

	mptd.cbStruct = sizeof(mptd);
	mptd.dwTimeDiv = mfs.timediv;

	mmr = midiStreamProperty(hStream, (LPBYTE)&mptd, MIDIPROP_SET | MIDIPROP_TIMEDIV);
	if (mmr != MMSYSERR_NOERROR)
	{
		MidiErrorMessageBox(mmr);
		ConverterCleanup();
		return 1;
	}

	num_empty_bufs = 0;
	flags = CONVERTF_RESET;

	for (buf_num = 0; buf_num < NUM_STREAM_BUFFERS; buf_num++)
	{
	/* Tell the converter to convert up to one entire buffer's length of output
	 * data. Also, set a flag so it knows to reset any saved state variables it
	 * may keep from call to call. */
		stream_bufs[buf_num].start_ofs = 0;
		stream_bufs[buf_num].maxlen = OUT_BUFFER_SIZE;
		stream_bufs[buf_num].starttime = 0;
		stream_bufs[buf_num].times_up = FALSE;

		err = ConvertToBuffer(flags, &stream_bufs[buf_num]);
		if (err != CONVERTERR_NOERROR)
		{
			if (err == CONVERTERR_DONE)
			{
				found_end = true;
			}
			else
			{
				DEBUG_Printf("%s: Initial conversion pass failed\n", __thisfunc__);
				ConverterCleanup();
				return 1;
			}
		}
		stream_bufs[buf_num].mh.dwBytesRecorded = stream_bufs[buf_num].bytes_in;

		if (!stream_bufs[buf_num].prepared)
		{
			mmr = midiOutPrepareHeader((HMIDIOUT)hStream, &stream_bufs[buf_num].mh, sizeof(MIDIHDR));
			if (mmr != MMSYSERR_NOERROR)
			{
				MidiErrorMessageBox(mmr);
				ConverterCleanup();
				return 1;
			}
			stream_bufs[buf_num].prepared = TRUE;
		}

		mmr = midiStreamOut(hStream, &stream_bufs[buf_num].mh, sizeof(MIDIHDR));
		if (mmr != MMSYSERR_NOERROR)
		{
			MidiErrorMessageBox(mmr);
			break;
		}
		flags = 0;

		if (found_end)
			break;
	}

	buf_num = 0;

	return 0;
}

/* MidiProc
 *
 * the callback handler which continually refills MIDI data buffers
 * as they're returned to us from the audio subsystem.
 */
static void CALLBACK MidiProc(HMIDIIN hMidi, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
	MIDIEVENT *me;
	MIDIHDR *mh;
	MMRESULT mmr;
	int err;

	switch (uMsg)
	{
	case MOM_DONE:
		if (callback_status == STATUS_CALLBACKDEAD)
			return;

		num_empty_bufs++;

		if (callback_status == STATUS_WAITINGFOREND)
		{
			if (num_empty_bufs < NUM_STREAM_BUFFERS)
			{
				return;
			}
			else
			{
				callback_status = STATUS_CALLBACKDEAD;
				MIDI_Stop((void **)NULL);
				SetEvent(hBufferReturnEvent);
				return;
			}
		}

		/* this flag is set whenever the callback is waiting for all buffers to
		 * come back. */
		if (callback_status == STATUS_KILLCALLBACK)
		{
			/* count NUM_STREAM_BUFFERS-1 being returned for the last time */
			if (num_empty_bufs < NUM_STREAM_BUFFERS)
			{
				return;
			}
			/* .. then send a stop message when we get the last buffer back */
			else
			{
				callback_status = STATUS_CALLBACKDEAD;
				SetEvent(hBufferReturnEvent);
				return;
			}
		}

		/* fill an available buffer with audio data again */
		if (midi_playing && num_empty_bufs)
		{
			stream_bufs[buf_num].start_ofs = 0;
			stream_bufs[buf_num].maxlen = OUT_BUFFER_SIZE;
			stream_bufs[buf_num].starttime = 0;
			stream_bufs[buf_num].bytes_in = 0;
			stream_bufs[buf_num].times_up = FALSE;

			err = ConvertToBuffer(0, &stream_bufs[buf_num]);
			if (err != CONVERTERR_NOERROR)
			{
				if (err == CONVERTERR_DONE)
				{
					callback_status = STATUS_WAITINGFOREND;
					return;
				}
				else
				{
					Con_Printf("MidiProc() conversion pass failed!");
					ConverterCleanup();
					return;
				}
			}

			stream_bufs[buf_num].mh.dwBytesRecorded = stream_bufs[buf_num].bytes_in;

			mmr = midiStreamOut(hStream, &stream_bufs[buf_num].mh, sizeof(MIDIHDR));
			if (mmr != MMSYSERR_NOERROR)
			{
				MidiErrorMessageBox(mmr);
				ConverterCleanup();
				return;
			}
			buf_num = (buf_num + 1) % NUM_STREAM_BUFFERS;
			num_empty_bufs--;
		}
		break;

	case MOM_POSITIONCB:
		mh = (MIDIHDR *)dwParam1;
		me = (MIDIEVENT *)(mh->lpData + mh->dwOffset);
		if (MIDIEVENT_TYPE(me->dwEvent) == MIDICMD_CONTROL)
		{
			if (MIDIEVENT_DATA1(me->dwEvent) != MIDICTL_MSB_MAIN_VOLUME)
				break;

			/* mask off the channel number and cache the volume data byte */
			volume_cache[MIDIEVENT_CHANNEL(me->dwEvent)] = MIDIEVENT_VOLUME(me->dwEvent);
		}
		break;

	default:
		break;
	}
}

/* SetAllChannelVolumes
 *
 * Given a percent in tenths of a percent, sets volume
 * on all channels to reflect the new value.
 */
static void SetAllChannelVolumes(DWORD volume_percent)
{
	int i;
	DWORD event, status, vol;
	MMRESULT mmr;

	if (!midi_playing)
		return;

	for (i = 0, status = MIDICMD_CONTROL; i < MIDI_CHANNELS; i++, status++)
	{
		vol = (volume_cache[i] * volume_percent) / 1000;
		event = status | ((DWORD)MIDICTL_MSB_MAIN_VOLUME << 8) | ((DWORD)vol << 16);
		mmr = midiOutShortMsg((HMIDIOUT)hStream, event);
		if (mmr != MMSYSERR_NOERROR)
		{
			MidiErrorMessageBox(mmr);
			return;
		}
	}
}

/* SetChannelVolume
 *
 * Given a percent in tenths of a percent, sets volume
 * on a specified channel to reflect the new value.
 */
#if 0	/* not used */
static void SetChannelVolume(DWORD channel_num, DWORD volume_percent)
{
	DWORD event, vol;
	MMRESULT mmr;

	if (!midi_playing)
		return;

	vol = (volume_cache[channel_num] * volume_percent) / 1000;
	event = MIDICMD_CONTROL | channel_num | ((DWORD)MIDICTL_MSB_MAIN_VOLUME << 8) | ((DWORD)vol << 16);

	mmr = midiOutShortMsg((HMIDIOUT)hStream, event);
	if (mmr != MMSYSERR_NOERROR)
		MidiErrorMessageBox(mmr);
}
#endif	/* #if 0 */

