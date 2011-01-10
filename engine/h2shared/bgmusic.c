/*
 * Background music handling for Hexen II: Hammer of Thyrion (uHexen2)
 * Handles streaming music as raw sound samples and runs the midi driver
 *
 * Copyright (C) 1999-2005 Id Software, Inc.
 * Copyright (C) 2010-2011 O.Sezer <sezero@users.sourceforge.net>
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
#include "midi_drv.h"

/*
 * MUSIC FILE SEARCH METHOD:
 *
 * Unlike quake, hexen2 server sends a svc_midi_name message after
 * svc_cdtrack, therefore we do have a filename for the background
 * music. In quake, snd_codec functionality would have been simply
 * suffice for all purposes. However, in order to honor the music
 * name and to keep the midi functionality, we need this interface.
 *
 * Therefore, we first search for an alternative to midi, like ogg,
 * mp3, wav and so on. If we can't find it, then we try to be clever
 * and map the cdtrack number to the midi name and look for a cdrip
 * like track<nn>.ogg, but that does not always work as expected.
 * When that fails too, we go for plain old midi.
 *
 * MUSIC FILE DIRECTORIES:
 *
 * Midi music file have always lived under "midi" directory, we are
 * not changing that.  Any other music files, including cd-rips, are
 * expected under the "music" directory.
 */

#define MIDI_DIRNAME	"midi"
#define MUSIC_DIRNAME	"music"

qboolean	bgmloop;

static float	old_volume = -1.0f;

static midi_driver_t *midi_drivers = NULL;

typedef enum _bgm_player
{
	BGM_NONE = -1,
	BGM_MIDIDRV,
	BGM_STREAMER,
} bgm_player_t;

typedef struct music_handler_s
{
	unsigned int	type;	/* power of two (snd_codec.h)	*/
	bgm_player_t	player;	/* Enumerated bgm player type	*/
	int	is_available;	/* -1 means not present		*/
	const char	*ext;	/* Expected file extension	*/
	const char	*dir;	/* Where to look for music file */
	struct music_handler_s	*next;
} music_handler_t;

static music_handler_t wanted_handlers[] =
{
	{ CODECTYPE_OGG,  BGM_STREAMER, -1,  ".ogg", MUSIC_DIRNAME, NULL },
	{ CODECTYPE_MP3,  BGM_STREAMER, -1,  ".mp3", MUSIC_DIRNAME, NULL },
	{ CODECTYPE_WAV,  BGM_STREAMER, -1,  ".wav", MUSIC_DIRNAME, NULL },
	{ CODECTYPE_FLAC, BGM_STREAMER, -1, ".flac", MUSIC_DIRNAME, NULL },
	{ CODECTYPE_MOD,  BGM_STREAMER, -1,  ".mod", MUSIC_DIRNAME, NULL },
	{ CODECTYPE_MOD,  BGM_STREAMER, -1,  ".it",  MUSIC_DIRNAME, NULL },
	{ CODECTYPE_MOD,  BGM_STREAMER, -1,  ".s3m", MUSIC_DIRNAME, NULL },
/* midi must be last before NULL terminator. */
#define MIDIDRIVER_MID (1 << 31) /* special. */
	{ MIDIDRIVER_MID, BGM_MIDIDRV,  -1,  ".mid", MIDI_DIRNAME,  NULL },
	{ CODECTYPE_MID,  BGM_STREAMER, -1,  ".mid", MIDI_DIRNAME,  NULL },
	{ CODECTYPE_NONE, BGM_NONE,     -1,    NULL,         NULL,  NULL }
};

static music_handler_t *music_handlers = NULL;

#define ANY_CODECTYPE	0xFFFFFFFF
#define MIDI_TYPES	(CODECTYPE_MID | MIDIDRIVER_MID)
#define MIDITYPE(x)	(((x) & MIDI_TYPES) != 0)
#define CDRIP_TYPES	(CODECTYPE_OGG | CODECTYPE_MP3 | CODECTYPE_WAV)
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

/* These mappings apply only to original Hexen II and its
 * expansion pack Portal of Praevus:  community maps need
 * not, and in many examples do not, follow this table.
 * Even some of the maps from Raven itself are quirky:
 * - tower.bsp of the original game wants "casb2" as midi
 *   but wants cd track 3 instead of 16.
 * - keep5.bsp of the expansion pack wants "casb1" as midi
 *   but wants cd track 6 instead of 15.
 */
static const char *data1_tracks[] =
{
	"", /* track01 is data */
	"casa1",	/* 02 */
	"casa2",	/* 03 */
	"casa3",	/* 04 */
	"casa4",	/* 05 */
	"egyp1",	/* 06 */
	"egyp2",	/* 07 */
	"egyp3",	/* 08 */
	"meso1",	/* 09 */
	"meso2",	/* 10 */
	"meso3",	/* 11 */
	"roma1",	/* 12 */
	"roma2",	/* 13 */
	"roma3",	/* 14 */
	"casb1",	/* 15 */
	"casb2",	/* 16 */
	"casb3"		/* track 17 is last */
};

static const char *portals_tracks[] =
{
	"", /* track01 is data */
	"tulku7",	/* 02 */
	"tulku1",	/* 03 */
	"tulku4",	/* 04 */
	"tulku2",	/* 05 */
	"tulku9",	/* 06 */
	"tulku10",	/* 07 */
	"tulku6",	/* 08 */
	"tulku5",	/* 09 */
	"tulku8",	/* 10 */
	"tulku3"	/* 11, last. */
	/* track12 is last but is not associated
	 * with a midi/music name. it is only used
	 * by the menu during credits display.
	 * therefore, it is not included here. */
};

#define num_data1_tracks	(sizeof(data1_tracks) / sizeof(data1_tracks[0]))
#define num_portals_tracks	(sizeof(portals_tracks) / sizeof(portals_tracks[0]))

static int map_cdtrack (const char *midiname) /* crapola: see notes above */
{
	if (cls.state <  ca_connected)	/* no have a cd track number yet. */
		return 0;

#if !defined(H2W)
	if ((cls.demoplayback || cls.demorecording) && cls.forcetrack != -1)
	{
		/*
		if (cls.forcetrack < 2)
			return 0;
		else if (gameflags & GAME_PORTALS &&
			 cls.forcetrack <= num_portals_tracks)
			return cls.forcetrack;
		else if (cls.forcetrack <= num_data1_tracks)
			return cls.forcetrack;
		else
			return 0;
		*/
		/* There is forcetrack for cdaudio, but there is
		 * no corresponding "forcemidi", therefore there
		 * is no meaning in mapping forcetrack to a name.
		 * If you really want your forced track, you use
		 * cdaudio.
		 */
		if (cl.cdtrack != cls.forcetrack)
			return 0;
	}
#endif	/* H2W */

	if (cl.cdtrack < 2)
		return 0;	/* track01 is always data */

	if (cl.cdtrack <= num_portals_tracks &&
	    q_strcasecmp(midiname, portals_tracks[cl.cdtrack - 1]) == 0)
	{
		return cl.cdtrack;
	}

	if (cl.cdtrack <= num_data1_tracks &&
	    q_strcasecmp(midiname, data1_tracks[cl.cdtrack - 1]) == 0)
	{
		return cl.cdtrack;
	}

	return 0;
}

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

	Cmd_AddCommand("music", BGM_Play_f);
	Cmd_AddCommand("music_pause", BGM_Pause_f);
	Cmd_AddCommand("music_resume", BGM_Resume_f);
	Cmd_AddCommand("music_loop", BGM_Loop_f);
	Cmd_AddCommand("music_stop", BGM_Stop_f);

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
	int cdtrack = 0;
	music_handler_t *handler;

 search_cdrip:
	handler = music_handlers;
	if (cdtrack != 0)
		Con_Printf ("trying a cd-rip for track %d\n", cdtrack);

	while (handler)
	{
		if (! (handler->type & allowed_types))
		{
			handler = handler->next;
			continue;
		}
		if (MIDITYPE(handler->type))
		{
			if (cdtrack == 0)
			{
				cdtrack = map_cdtrack(filename);
				if (cdtrack != 0)
					goto search_cdrip;
			}
			else
			{
				cdtrack = 0;
			}
		}
		if (!handler->is_available)
		{
		/* skip handlers which failed to initialize */
		/* TODO: implement re-init, make BGM aware of it */
			handler = handler->next;
			continue;
		}
		if (cdtrack != 0)
		{
			if (!CDRIPTYPE(handler->type))
			{
				handler = handler->next;
				continue;
			}
			q_snprintf(tmp, sizeof(tmp), "%s/track%02d%s",
				   handler->dir, cdtrack, handler->ext);
		}
		else
		{
			if (MIDITYPE(handler->type))
				Con_DPrintf("trying a midi file\n");
			q_snprintf(tmp, sizeof(tmp), "%s/%s%s",
				   handler->dir, filename, handler->ext);
		}
		switch (handler->player)
		{
		case BGM_MIDIDRV:
			if (BGM_Play_mididrv(tmp) == 0)
				return;		/* success */
			break;
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

	if (!filename || !*filename)
	{
		Con_DPrintf("null music file name\n");
		return;
	}

	BGM_Stop();

	ext = S_FileExtension(filename);
	if (!ext)	/* try all things */
	{
		BGM_Play_noext(filename, ANY_CODECTYPE);
		return;
	}

	/* use the filename as is, no cdrip searching */
	handler = music_handlers;
	while (handler)
	{
		/* skip handlers which failed to initialize */
		/* TODO: implement re-init, make BGM aware of it */
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
		break;
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
		if (fileBytes > sizeof(raw))
		{
			fileBytes = sizeof(raw);
			fileSamples = fileBytes / (bgmstream->info.width * bgmstream->info.channels);
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
				bgmstream->info.width, bgmstream->info.channels, raw, bgmvolume.value);
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
	if (bgmstream)
		BGM_UpdateStream ();
}

