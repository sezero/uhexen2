/*
 * MIDI streaming music support using WildMIDI library.
 *
 * wildmidi at least v0.2.3.4 is required at both compile and runtime:
 * wildmidi-0.2.2 has horrific mistakes like free()ing the buffer that
 * you pass with WildMidi_OpenBuffer() when you do WildMidi_Close().
 *
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

#if defined(USE_CODEC_WILDMIDI)
#include "snd_codec.h"
#include "snd_codeci.h"
#include "snd_wildmidi.h"
#include <wildmidi_lib.h>
#include "filenames.h"

#if !defined(WM_MO_ENHANCED_RESAMPLING)
#error wildmidi version 0.2.3.4 or newer is required.
#endif	/* WM_MO_ENHANCED_RESAMPLING */

#define CACHEBUFFER_SIZE 4096

typedef struct _midi_buf_t
{
	midi *song;
	char midi_buffer[CACHEBUFFER_SIZE];
	int pos, last;
} midi_buf_t;

static unsigned short wildmidi_rate;
static unsigned short wildmidi_opts;
static const char *cfgfile[] = {
#if defined(_WIN32) || defined(__OS2__) || defined(__EMX__)
	"C:\\TIMIDITY",
#elif defined(__DJGPP__)
	"C:/TIMIDITY",
#elif defined(MSDOS) || defined(__MSDOS__) || defined(__DOS__)
	"C:\\TIMIDITY",
#elif defined(__MORPHOS__)
	"LIBS:GerontoPlayer",
#elif defined(__AROS__)
	"Timidity:",
#else /* unix, osx, riscos, ... */
	"/etc",
	"/etc/wildmidi",
	"/etc/timidity",
	"/usr/share/timidity",
	"/usr/local/share/wildmidi",
	"/usr/local/share/timidity",
	"/usr/local/lib/timidity",
#endif
	NULL
};

static int WILDMIDI_InitHelper (const char *cfgdir)
{
	char path[MAX_OSPATH];
	int len;

	len = q_strlcpy(path, cfgdir, sizeof(path));
	if (len >= (int)sizeof(path) - 1)
		return -1;
	if (len && !IS_DIR_SEPARATOR(path[len - 1]))
		path[len++] = DIR_SEPARATOR_CHAR;

	path[len] = '\0';
	q_strlcat(path, "wildmidi.cfg", sizeof(path));
	Con_DPrintf("WildMIDI: trying %s\n", path);
	if (WildMidi_Init(path, wildmidi_rate, wildmidi_opts) == 0)
		return 0;

	path[len] = '\0';
	q_strlcat(path, "timidity.cfg", sizeof(path));
	Con_DPrintf("WildMIDI: trying %s\n", path);
	return WildMidi_Init(path, wildmidi_rate, wildmidi_opts);
}

static qboolean S_WILDMIDI_CodecInitialize (void)
{
	const char *timi_env;
	int i, err;

	if (wildmidi_codec.initialized)
		return true;

	wildmidi_opts = WM_MO_ENHANCED_RESAMPLING;
	if (shm->speed < 11025)
		wildmidi_rate = 11025;
	else if (shm->speed > 48000)
		wildmidi_rate = 44100;
	else	wildmidi_rate = shm->speed;

	err = -1;
	timi_env = getenv("WILDMIDI_CFG");
	if (timi_env == NULL)
		timi_env = getenv("TIMIDITY_CFG");
	if (timi_env)
	{
		Con_DPrintf("WildMIDI: trying %s\n", timi_env);
		/* env is an override: if it fails, we
		 * don't bother trying anything else. */
		err = WildMidi_Init(timi_env, wildmidi_rate, wildmidi_opts);
		goto _finish;
	}
#if DO_USERDIRS
	/* check under the user's directory first: */
	err = WILDMIDI_InitHelper(FS_GetUserbase());
#endif
	/* then, check under the installation dir: */
	if (err != 0)
		err = WILDMIDI_InitHelper(FS_GetBasedir());
	/* lastly, check with the system locations: */
	for (i = 0; err != 0 && cfgfile[i] != NULL; ++i)
		err = WILDMIDI_InitHelper(cfgfile[i]);

	_finish:
	if (err != 0)
	{
		Con_Printf ("Could not initialize WildMIDI\n");
		return false;
	}

	Con_Printf ("WildMIDI initialized\n");
	wildmidi_codec.initialized = true;

	return true;
}

static void S_WILDMIDI_CodecShutdown (void)
{
	if (!wildmidi_codec.initialized)
		return;
	wildmidi_codec.initialized = false;
	Con_Printf("Shutting down WildMIDI.\n");
	WildMidi_Shutdown();
}

static qboolean S_WILDMIDI_CodecOpenStream (snd_stream_t *stream)
{
	midi_buf_t *data;
	unsigned char *temp;
	long len;
	int mark;

	if (!wildmidi_codec.initialized)
		return false;

	len = FS_filelength(&stream->fh);
	mark = Hunk_LowMark();
	temp = (unsigned char *) Hunk_Alloc(len);
	FS_fread(temp, 1, len, &stream->fh);

	data = (midi_buf_t *) Z_Malloc(sizeof(midi_buf_t), Z_MAINZONE);
	data->song = WildMidi_OpenBuffer (temp, len);
	Hunk_FreeToLowMark(mark); /* free original file data */
	if (data->song == NULL)
	{
		Con_Printf ("%s is not a valid MIDI file\n", stream->name);
		Z_Free(data);
		return false;
	}
	stream->info.rate = wildmidi_rate;
	stream->info.width = 2; /* WildMIDI does 16 bit signed */
	stream->info.bits = 16;
	stream->info.channels = 2; /* WildMIDI does stereo */
	stream->priv = data;
	WildMidi_MasterVolume (100);

	return true;
}

static int S_WILDMIDI_CodecReadStream (snd_stream_t *stream, int bytes, void *buffer)
{
	midi_buf_t *data = (midi_buf_t *) stream->priv;
	if (data->pos == 0)
	{
		data->last = WildMidi_GetOutput (data->song, data->midi_buffer,
							CACHEBUFFER_SIZE);
		if (data->last == 0)
			return 0;
		if (bytes > data->last)
			bytes = data->last;
	}
	else if (data->pos + bytes > data->last)
	{
		bytes = data->last - data->pos;
	}
	memcpy (buffer, & data->midi_buffer[data->pos], bytes);
	/* WildMIDI outputs host-endian data, no byte swap needed. */
	data->pos += bytes;
	if (data->pos == data->last)
		data->pos = 0;
	return bytes;
}

static void S_WILDMIDI_CodecCloseStream (snd_stream_t *stream)
{
	WildMidi_Close (((midi_buf_t *)stream->priv)->song);
	Z_Free(stream->priv);
	S_CodecUtilClose(&stream);
}

static int S_WILDMIDI_CodecRewindStream (snd_stream_t *stream)
{
	unsigned long pos = 0;
	return WildMidi_FastSeek (((midi_buf_t *)stream->priv)->song, &pos);
}

snd_codec_t wildmidi_codec =
{
	CODECTYPE_MIDI,
	false,
	"mid",
	S_WILDMIDI_CodecInitialize,
	S_WILDMIDI_CodecShutdown,
	S_WILDMIDI_CodecOpenStream,
	S_WILDMIDI_CodecReadStream,
	S_WILDMIDI_CodecRewindStream,
	S_WILDMIDI_CodecCloseStream,
	NULL
};

#endif	/* USE_CODEC_WILDMIDI */

