/*
 * Background music handling for Hexen II: Hammer of Thyrion (uHexen2)
 * Handles streaming music as raw sound samples and runs the midi driver
 *
 * Copyright (C) 1999-2005 Id Software, Inc.
 * Copyright (C) 2010-2012 O.Sezer <sezero@users.sourceforge.net>
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
#include "snd_codec.h"
#include "bgmusic.h"
#include "cdaudio.h"
#include "midi_drv.h"

#define MIDI_DIRNAME	"midi"
#define MUSIC_DIRNAME	"music"

qboolean	bgmloop;
cvar_t		bgm_extmusic = {"bgm_extmusic", "1", CVAR_ARCHIVE};

static qboolean	no_extmusic= false;
static float	old_volume = -1.0f;

static midi_driver_t *midi_drivers = NULL;

typedef enum _bgm_player
{
	BGM_NONE = -1,
	BGM_MIDIDRV = 1,
	BGM_STREAMER
} bgm_player_t;

typedef struct music_handler_s
{
	unsigned int	type;	/* 1U << n (see snd_codec.h)	*/
	bgm_player_t	player;	/* Enumerated bgm player type	*/
	int	is_available;	/* -1 means not present		*/
	const char	*ext;	/* Expected file extension	*/
	const char	*dir;	/* Where to look for music file */
	struct music_handler_s	*next;
} music_handler_t;

static music_handler_t wanted_handlers[] =
{
	{ CODECTYPE_VORBIS,BGM_STREAMER,-1,  "ogg", MUSIC_DIRNAME, NULL },
	{ CODECTYPE_OPUS, BGM_STREAMER, -1, "opus", MUSIC_DIRNAME, NULL },
	{ CODECTYPE_MP3,  BGM_STREAMER, -1,  "mp3", MUSIC_DIRNAME, NULL },
	{ CODECTYPE_FLAC, BGM_STREAMER, -1, "flac", MUSIC_DIRNAME, NULL },
	{ CODECTYPE_WAV,  BGM_STREAMER, -1,  "wav", MUSIC_DIRNAME, NULL },
	{ CODECTYPE_MOD,  BGM_STREAMER, -1,  "it",  MUSIC_DIRNAME, NULL },
	{ CODECTYPE_MOD,  BGM_STREAMER, -1,  "s3m", MUSIC_DIRNAME, NULL },
	{ CODECTYPE_MOD,  BGM_STREAMER, -1,  "xm",  MUSIC_DIRNAME, NULL },
	{ CODECTYPE_MOD,  BGM_STREAMER, -1,  "mod", MUSIC_DIRNAME, NULL },
/* midi must be last before NULL terminator. */
#define MIDIDRIVER_MID (1U << 31) /* special, comes before CODECTYPE_MID */
	{ MIDIDRIVER_MID, BGM_MIDIDRV,  -1,  "mid", MIDI_DIRNAME,  NULL },
	{ CODECTYPE_MID,  BGM_STREAMER, -1,  "mid", MIDI_DIRNAME,  NULL },
	{ CODECTYPE_NONE, BGM_NONE,     -1,   NULL,         NULL,  NULL }
};

static music_handler_t *music_handlers = NULL;

#define ANY_CODECTYPE	0xFFFFFFFF
#define MIDI_TYPES	(CODECTYPE_MID | MIDIDRIVER_MID)
#define MIDITYPE(x)	(((x) & MIDI_TYPES) != 0)
#define CDRIP_TYPES	(CODECTYPE_VORBIS | CODECTYPE_MP3 | CODECTYPE_FLAC | CODECTYPE_WAV)
#define CDRIPTYPE(x)	(((x) & CDRIP_TYPES) != 0)

typedef struct midi_handle_s
{
	void	*handle;
	midi_driver_t	*driver;
} midi_handle_t;

/* only one of bgmstream and mid_handle is supposed to be
 * active, not both. */
static midi_handle_t midi_handle;
static snd_stream_t *bgmstream = NULL;

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
	music_handler_t *handlers = NULL;
	int i;

	memset (&midi_handle, 0, sizeof(midi_handle_t));

	Cvar_RegisterVariable(&bgm_extmusic);
	Cmd_AddCommand("music", BGM_Play_f);
	Cmd_AddCommand("music_pause", BGM_Pause_f);
	Cmd_AddCommand("music_resume", BGM_Resume_f);
	Cmd_AddCommand("music_loop", BGM_Loop_f);
	Cmd_AddCommand("music_stop", BGM_Stop_f);

	if (COM_CheckParm("-noextmusic") != 0)
		no_extmusic = true;

	bgmloop = true;

	for (i = 0; wanted_handlers[i].type != CODECTYPE_NONE; i++)
	{
		switch (wanted_handlers[i].player)
		{
		case BGM_MIDIDRV:
			wanted_handlers[i].is_available =
				(midi_drivers == NULL) ? -1 : 1;
			break;
		case BGM_STREAMER:
			wanted_handlers[i].is_available =
				S_CodecIsAvailable(wanted_handlers[i].type);
			break;
		case BGM_NONE:
		default:
			break;
		}
		if (wanted_handlers[i].is_available != -1)
		{
			if (handlers)
			{
				handlers->next = &wanted_handlers[i];
				handlers = handlers->next;
			}
			else
			{
				music_handlers = &wanted_handlers[i];
				handlers = music_handlers;
			}
		}
	}

	return true;
}

void BGM_Shutdown (void)
{
	BGM_Stop();
/* sever our connections to
 * midi_drv and snd_codec */
	music_handlers = NULL;
	midi_drivers = NULL;
}

static int BGM_Play_mididrv (const char *filename)
{
	midi_driver_t *drv = midi_drivers;
	while (drv)
	{
		if (drv->available)
		{
			midi_handle.handle = drv->mididrv_open (filename);
			if (midi_handle.handle != NULL)
			{
				midi_handle.driver = drv;
				if (bgmvolume.value == 0.0f) /* don't bother advancing */
					drv->mididrv_pause(&midi_handle.handle);
				return 0;
			}
		}
		drv = drv->next;
	}
	return -1;
}

static void BGM_Play_noext (const char *filename, unsigned int allowed_types)
{
	char tmp[MAX_QPATH];
	music_handler_t *handler;

	handler = music_handlers;
	while (handler)
	{
		if (! (handler->type & allowed_types))
		{
			handler = handler->next;
			continue;
		}
		if (!handler->is_available)
		{
			handler = handler->next;
			continue;
		}
		q_snprintf(tmp, sizeof(tmp), "%s/%s.%s",
			   handler->dir, filename, handler->ext);
		switch (handler->player)
		{
		case BGM_MIDIDRV:
			if (BGM_Play_mididrv(tmp) == 0)
				return;		/* success */
			/* BGM_MIDIDRV is followed by CODECTYPE_MID streamer.
			 * Even if the midi driver failed, we may still have
			 * a chance with the streamer if it's available... */
			if (! (handler->next && handler->next->is_available))
				break;
			handler = handler->next;
		case BGM_STREAMER:
			bgmstream = S_CodecOpenStreamType(tmp, handler->type);
			if (bgmstream)
				return;		/* success */
			break;
		case BGM_NONE:
		default:
			break;
		}
		handler = handler->next;
	}

	Con_Printf("Couldn't handle music file %s\n", filename);
}

void BGM_Play (const char *filename)
{
	char tmp[MAX_QPATH];
	const char *ext;
	music_handler_t *handler;

	BGM_Stop();

	if (music_handlers == NULL)
		return;

	if (!filename || !*filename)
	{
		Con_DPrintf("null music file name\n");
		return;
	}

	ext = COM_FileGetExtension(filename);
	if (! *ext)	/* try all things */
	{
		BGM_Play_noext(filename, ANY_CODECTYPE);
		return;
	}

	/* use the filename as is */
	handler = music_handlers;
	while (handler)
	{
		if (handler->is_available &&
		    !q_strcasecmp(ext, handler->ext))
			break;
		handler = handler->next;
	}
	if (!handler)
	{
		Con_Printf("Unhandled extension for %s\n", filename);
		return;
	}
	q_snprintf(tmp, sizeof(tmp), "%s/%s", handler->dir, filename);
	switch (handler->player)
	{
	case BGM_MIDIDRV:
		if (BGM_Play_mididrv(tmp) == 0)
			return;		/* success */
		/* BGM_MIDIDRV is followed by CODECTYPE_MID streamer.
		 * Even if the midi driver failed, we may still have
		 * a chance with the streamer if it's available... */
		if (! (handler->next && handler->next->is_available))
			break;
		handler = handler->next;
	case BGM_STREAMER:
		bgmstream = S_CodecOpenStreamType(tmp, handler->type);
		if (bgmstream)
			return;		/* success */
		break;
	case BGM_NONE:
	default:
		break;
	}

	Con_Printf("Couldn't handle music file %s\n", filename);
}

void BGM_PlayMIDIorMusic (const char *filename)
{
/* instead of searching by the order of music_handlers, do so by
 * the order of searchpath priority: the file from the searchpath
 * with the highest path_id is most likely from our own gamedir
 * itself.  this way, if a mod has egyp1 as a mp3 or a midi, which
 * is below *.ogg in the music_handler order, the mp3 or midi will
 * still have priority over egyp1.ogg from, say, data1.
 */
	char tmp[MAX_QPATH];
	const char *ext, *dir;
	unsigned int path_id, prev_id, type;
	qboolean try_midi_stream;
	music_handler_t *handler;

	if (music_handlers == NULL)
		return;

	BGM_Stop();

	if (!filename || !*filename)
	{
		Con_DPrintf("null music file name\n");
		return;
	}

	ext = COM_FileGetExtension(filename);
	if (*ext != '\0')
	{
		BGM_Play(filename);
		return;
	}

	prev_id = 0;
	type = 0;
	dir = ext = NULL;
	handler = music_handlers;
	try_midi_stream = false;
	while (handler)
	{
		if (! handler->is_available)
			goto _next;
		if (! MIDITYPE(handler->type) &&
		     (no_extmusic || !bgm_extmusic.value))
			goto _next;
		q_snprintf(tmp, sizeof(tmp), "%s/%s.%s",
			   handler->dir, filename, handler->ext);
		if (! FS_FileExists(tmp, &path_id))
		{
			if (handler->type == MIDIDRIVER_MID)
				break;
			goto _next;
		}
		if (path_id > prev_id)
		{
			prev_id = path_id;
			type = handler->type;
			ext = handler->ext;
			dir = handler->dir;
			if (handler->type == MIDIDRIVER_MID)
			{
				if (handler->next &&
				    handler->next->is_available)
					try_midi_stream = true;
				break;
			}
		}
	_next:
		handler = handler->next;
	}
	if (ext == NULL)
		Con_Printf("Couldn't handle music file %s\n", filename);
	else
	{
		q_snprintf(tmp, sizeof(tmp), "%s/%s.%s", dir, filename, ext);
		switch (type)
		{
		case MIDIDRIVER_MID:
			if (BGM_Play_mididrv(tmp) == 0)
				return;		/* success */
			/* BGM_MIDIDRV is followed by CODECTYPE_MID streamer.
			 * Even if the midi driver failed, we may still have
			 * a chance with the streamer if it's available... */
			if (!try_midi_stream)
				break;
			type = CODECTYPE_MID;
		default:
			bgmstream = S_CodecOpenStreamType(tmp, type);
			if (bgmstream)
				return;		/* success */
		}
		Con_Printf("Couldn't handle music file %s\n", tmp);
	}
}

void BGM_PlayCDtrack (byte track, qboolean looping)
{
/* instead of searching by the order of music_handlers, do so by
 * the order of searchpath priority: the file from the searchpath
 * with the highest path_id is most likely from our own gamedir
 * itself. This way, if a mod has track02 as a *.mp3 file, which
 * is below *.ogg in the music_handler order, the mp3 will still
 * have priority over track02.ogg from, say, data1.
 */
	char tmp[MAX_QPATH];
	const char *ext;
	unsigned int path_id, prev_id, type;
	music_handler_t *handler;

	BGM_Stop();
	if (CDAudio_Play(track, looping) == 0)
		return;			/* success */

	if (music_handlers == NULL)
		return;

	if (no_extmusic || !bgm_extmusic.value)
		return;

	prev_id = 0;
	type = 0;
	ext  = NULL;
	handler = music_handlers;
	while (handler)
	{
		if (! handler->is_available)
			goto _next;
		if (! CDRIPTYPE(handler->type))
			goto _next;
		q_snprintf(tmp, sizeof(tmp), "%s/track%02d.%s",
				MUSIC_DIRNAME, (int)track, handler->ext);
		if (! FS_FileExists(tmp, &path_id))
			goto _next;
		if (path_id > prev_id)
		{
			prev_id = path_id;
			type = handler->type;
			ext = handler->ext;
		}
	_next:
		handler = handler->next;
	}
	if (ext == NULL)
		Con_Printf("Couldn't find a cdrip for track %d\n", (int)track);
	else
	{
		q_snprintf(tmp, sizeof(tmp), "%s/track%02d.%s",
				MUSIC_DIRNAME, (int)track, ext);
		bgmstream = S_CodecOpenStreamType(tmp, type);
		if (! bgmstream)
			Con_Printf("Couldn't handle music file %s\n", tmp);
	}
}

void BGM_Stop (void)
{
	if (midi_handle.handle)
	{
		midi_handle.driver->mididrv_close (& midi_handle.handle);
		midi_handle.handle = NULL;
		midi_handle.driver = NULL;
	}
	if (bgmstream)
	{
		bgmstream->status = STREAM_NONE;
		S_CodecCloseStream(bgmstream);
		bgmstream = NULL;
		s_rawend = 0;
	}
}

void BGM_Pause (void)
{
	if (midi_handle.handle)
		midi_handle.driver->mididrv_pause (& midi_handle.handle);
	if (bgmstream)
	{
		if (bgmstream->status == STREAM_PLAY)
			bgmstream->status = STREAM_PAUSE;
	}
}

void BGM_Resume (void)
{
	if (midi_handle.handle)
		midi_handle.driver->mididrv_resume (& midi_handle.handle);
	if (bgmstream)
	{
		if (bgmstream->status == STREAM_PAUSE)
			bgmstream->status = STREAM_PLAY;
	}
}

static void BGM_UpdateStream (void)
{
	int	res;	/* Number of bytes read. */
	int	bufferSamples;
	int	fileSamples;
	int	fileBytes;
	byte	raw[16384];

	if (bgmstream->status != STREAM_PLAY)
		return;

	/* don't bother playing anything if musicvolume is 0 */
	if (bgmvolume.value <= 0)
		return;

	/* see how many samples should be copied into the raw buffer */
	if (s_rawend < paintedtime)
		s_rawend = paintedtime;

	while (s_rawend < paintedtime + MAX_RAW_SAMPLES)
	{
		bufferSamples = MAX_RAW_SAMPLES - (s_rawend - paintedtime);

		/* decide how much data needs to be read from the file */
		fileSamples = bufferSamples * bgmstream->info.rate / shm->speed;
		if (!fileSamples)
			return;

		/* our max buffer size */
		fileBytes = fileSamples * (bgmstream->info.width * bgmstream->info.channels);
		if (fileBytes > (int) sizeof(raw))
		{
			fileBytes = (int) sizeof(raw);
			fileSamples = fileBytes /
					  (bgmstream->info.width * bgmstream->info.channels);
		}

		/* Read */
		res = S_CodecReadStream(bgmstream, fileBytes, raw);
		if (res < fileBytes)
		{
			fileBytes = res;
			fileSamples = res / (bgmstream->info.width * bgmstream->info.channels);
		}

		if (res > 0)	/* data: add to raw buffer */
		{
			S_RawSamples(fileSamples, bgmstream->info.rate,
							bgmstream->info.width,
							bgmstream->info.channels,
							raw, bgmvolume.value);
		}
		else if (res == 0)	/* EOF */
		{
			if (bgmloop)
			{
				if (S_CodecRewindStream(bgmstream) < 0)
				{
					BGM_Stop();
					return;
				}
			}
			else
			{
				BGM_Stop();
				return;
			}
		}
		else	/* res < 0: some read error */
		{
			Con_Printf("Stream read error (%i), stopping.\n", res);
			BGM_Stop();
			return;
		}
	}
}

void BGM_Update (void)
{
	if (old_volume != bgmvolume.value)
	{
		if (bgmvolume.value < 0)
			Cvar_SetQuick (&bgmvolume, "0");
		else if (bgmvolume.value > 1)
			Cvar_SetQuick (&bgmvolume, "1");
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
	if (bgmstream)
		BGM_UpdateStream ();
}

