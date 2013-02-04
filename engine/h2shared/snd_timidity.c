/*
 * MIDI streaming music support using Timidity library.  Compile against
 * the modified timidity library included here.  Standart libtimidity is
 * not recommended.
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

#if defined(USE_CODEC_TIMIDITY)
#include "snd_codec.h"
#include "snd_codeci.h"
#include "snd_timidity.h"
#include "timidity/timidity.h"
#include "filenames.h"

#define CACHEBUFFER_SIZE 4096

typedef struct _midi_buf_t
{
	MidSong *song;
	sint8 midi_buffer[CACHEBUFFER_SIZE];
	int pos, last;
} midi_buf_t;


static size_t timidity_fread (void *ctx, void *ptr, size_t size, size_t nmemb)
{
	return FS_fread (ptr, size, nmemb, (fshandle_t *) ctx);
}

static int timidity_fseek (void *ctx, long offset, int whence)
{
	return FS_fseek ((fshandle_t *) ctx, offset, whence);
}

static long timidity_ftell (void *ctx)
{
	return FS_ftell ((fshandle_t *) ctx);
}

static int timidity_fclose (void *ctx)
{
	return 0;		/* we fclose() elsewhere. */
}

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
	"/etc/timidity",
	"/usr/share/timidity",
	"/usr/local/share/timidity",
	"/usr/local/lib/timidity",
#endif
	NULL
};

static int TIMIDITY_InitHelper (const char *cfgdir)
{
	char path[MAX_OSPATH];
	int len;

	len = q_strlcpy(path, cfgdir, sizeof(path));
	if (len >= (int)sizeof(path) - 1)
		return -1;
	if (len && !IS_DIR_SEPARATOR(path[len - 1]))
		path[len++] = DIR_SEPARATOR_CHAR;

	path[len] = '\0';
	q_strlcat(path, "timidity.cfg", sizeof(path));
	Con_DPrintf("Timidity: trying %s\n", path);
	return mid_init(path);
}

static qboolean S_TIMIDITY_CodecInitialize (void)
{
	const char *timi_env;
	int i, err;

	if (timidity_codec.initialized)
		return true;

	err = -1;
	timi_env = getenv("TIMIDITY_CFG");
	if (timi_env)
	{
		Con_DPrintf("Timidity: trying %s\n", timi_env);
		/* env is an override: if it fails, we
		 * don't bother trying anything else. */
		err = mid_init(timi_env);
		goto _finish;
	}
#if DO_USERDIRS
	/* check under the user's directory first: */
	err = TIMIDITY_InitHelper(FS_GetUserbase());
#endif
	/* then, check under the installation dir: */
	if (err != 0)
		err = TIMIDITY_InitHelper(FS_GetBasedir());
	/* lastly, check with the system locations: */
	for (i = 0; err != 0 && cfgfile[i] != NULL; ++i)
		err = TIMIDITY_InitHelper(cfgfile[i]);

	_finish:
	if (err != 0)
	{
		Con_Printf ("Could not initialize Timidity\n");
		return false;
	}

	Con_Printf ("Timidity initialized\n");
	timidity_codec.initialized = true;

	return true;
}

static void S_TIMIDITY_CodecShutdown (void)
{
	if (!timidity_codec.initialized)
		return;
	timidity_codec.initialized = false;
	Con_Printf("Shutting down Timidity.\n");
	mid_exit ();
}

static snd_stream_t *S_TIMIDITY_CodecOpenStream (const char *filename)
{
	snd_stream_t *stream;
	midi_buf_t *data;
	MidSongOptions options;
	MidIStream *midistream;
	int width;

	if (!timidity_codec.initialized)
		return NULL;

	stream = S_CodecUtilOpen(filename, &timidity_codec);
	if (!stream)
		return NULL;

	options.rate = shm->speed;
	width = shm->samplebits / 8;
	options.channels = shm->channels;
	if (width == 1)
		options.format = MID_AUDIO_U8;
	else if (host_byteorder == BIG_ENDIAN)
		options.format = MID_AUDIO_S16MSB;
	else /* assumed LITTLE_ENDIAN. */
		options.format = MID_AUDIO_S16LSB;
	options.buffer_size = CACHEBUFFER_SIZE / (width * options.channels);
	options.discard_meta = 1;		/* don't need the metadata */

	midistream = mid_istream_open_callbacks (timidity_fread, timidity_fseek,
						 timidity_ftell, timidity_fclose,
						 & stream->fh);
	if (!midistream)
	{
		Con_Printf ("Couldn't create Timidity stream for %s\n", filename);
		S_CodecUtilClose(&stream);
		return NULL;
	}
	data = (midi_buf_t *) Z_Malloc(sizeof(midi_buf_t), Z_MAINZONE);
	data->song = mid_song_load (midistream, &options);
	mid_istream_close (midistream);
	if (data->song == NULL)
	{
		Con_Printf ("%s is not a valid MIDI file\n", filename);
		Z_Free(data);
		S_CodecUtilClose(&stream);
		return NULL;
	}

	stream->info.rate = options.rate;
	stream->info.width = width;
	stream->info.channels = options.channels;
	stream->priv = data;

	mid_song_set_volume (data->song, 100);
	mid_song_start (data->song);

	return stream;
}

static int S_TIMIDITY_CodecReadStream (snd_stream_t *stream, int bytes, void *buffer)
{
	midi_buf_t *data = (midi_buf_t *) stream->priv;
	if (data->pos == 0)
	{
		data->last = mid_song_read_wave (data->song, data->midi_buffer,
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
	/* Timidity byte swaps according to the format with which
	 * the stream is opened, therefore no swap needed here. */
	data->pos += bytes;
	if (data->pos == data->last)
		data->pos = 0;
	return bytes;
}

static void S_TIMIDITY_CodecCloseStream (snd_stream_t *stream)
{
	mid_song_free (((midi_buf_t *)stream->priv)->song);
	Z_Free(stream->priv);
	S_CodecUtilClose(&stream);
}

static int S_TIMIDITY_CodecRewindStream (snd_stream_t *stream)
{
	mid_song_start (((midi_buf_t *)stream->priv)->song);
	return 0;
}

snd_codec_t timidity_codec =
{
	CODECTYPE_MIDI,
	false,
	"mid",
	S_TIMIDITY_CodecInitialize,
	S_TIMIDITY_CodecShutdown,
	S_TIMIDITY_CodecOpenStream,
	S_TIMIDITY_CodecReadStream,
	S_TIMIDITY_CodecRewindStream,
	S_TIMIDITY_CodecCloseStream,
	NULL
};

#endif	/* USE_CODEC_TIMIDITY */

