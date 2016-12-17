/*
 * snd_sdl.c - SDL audio driver for Hexen II: Hammer of Thyrion (uHexen2)
 * based on implementations found in the quakeforge and ioquake3 projects.
 * $Id$
 *
 * Copyright (C) 1999-2005 Id Software, Inc.
 * Copyright (C) 2005-2012 O.Sezer <sezero@users.sourceforge.net>
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

#include "quakedef.h"
#include "snd_sys.h"

#if HAVE_SDL_SOUND

#include "snd_sdl.h"
#include "sdl_inc.h"

/* whether to use hunk for dma buffer memory, either 1 or 0  */
#define USE_HUNK_ALLOC		0

static char s_sdl_driver[] = "SDLAudio";

static int	buffersize;


static void SDLCALL paint_audio (void *unused, Uint8 *stream, int len)
{
	int	pos, tobufend;
	int	len1, len2;

	if (!shm)
	{	/* shouldn't happen, but just in case */
		memset(stream, 0, len);
		return;
	}

	pos = (shm->samplepos * (shm->samplebits / 8));
	if (pos >= buffersize)
		shm->samplepos = pos = 0;

	tobufend = buffersize - pos;  /* bytes to buffer's end. */
	len1 = len;
	len2 = 0;

	if (len1 > tobufend)
	{
		len1 = tobufend;
		len2 = len - len1;
	}

	memcpy(stream, shm->buffer + pos, len1);

	if (len2 <= 0)
	{
		shm->samplepos += (len1 / (shm->samplebits / 8));
	}
	else
	{	/* wraparound? */
		memcpy(stream + len1, shm->buffer, len2);
		shm->samplepos = (len2 / (shm->samplebits / 8));
	}

	if (shm->samplepos >= buffersize)
		shm->samplepos = 0;
}

static qboolean S_SDL_Init (dma_t *dma)
{
	SDL_AudioSpec desired, obtained;
	int		tmp, val;
	char	drivername[128];

	if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0)
	{
		Con_Printf("Couldn't init SDL audio: %s\n", SDL_GetError());
		return false;
	}

	/* Set up the desired format */
	desired.freq = desired_speed;
	desired.format = (desired_bits == 16) ? AUDIO_S16SYS : AUDIO_U8;
	desired.channels = desired_channels;
	if (desired.freq <= 11025)
		desired.samples = 256;
	else if (desired.freq <= 22050)
		desired.samples = 512;
	else if (desired.freq <= 44100)
		desired.samples = 1024;
	else if (desired.freq <= 56000)
		desired.samples = 2048; /* for 48 kHz */
	else
		desired.samples = 4096; /* for 96 kHz */
	desired.callback = paint_audio;
	desired.userdata = NULL;

	/* Open the audio device */
	if (SDL_OpenAudio(&desired, &obtained) == -1)
	{
		Con_Printf("Couldn't open SDL audio: %s\n", SDL_GetError());
		SDL_QuitSubSystem(SDL_INIT_AUDIO);
		return false;
	}

	/* Make sure we can support the audio format */
	switch (obtained.format)
	{
	case AUDIO_S8:		/* maybe needed by AHI */
	case AUDIO_U8:
	case AUDIO_S16SYS:
		/* Supported */
		break;
	default:
		Con_Printf ("Unsupported audio format received (%u)\n", obtained.format);
		SDL_CloseAudio();
		SDL_QuitSubSystem(SDL_INIT_AUDIO);
		return false;
	}

	memset ((void *) dma, 0, sizeof(dma_t));
	shm = dma;

	/* Fill the audio DMA information block */
	shm->samplebits = (obtained.format & 0xFF); /* first byte of format is bits */
	shm->signed8 = (obtained.format == AUDIO_S8);
	if (obtained.freq != desired_speed)
		Con_Printf ("Warning: Rate set (%d) didn't match requested rate (%d)!\n", obtained.freq, desired_speed);
	shm->speed = obtained.freq;
	shm->channels = obtained.channels;
	tmp = (obtained.samples * obtained.channels) * 10;
	if (tmp & (tmp - 1))
	{	/* make it a power of two */
		val = 1;
		while (val < tmp)
			val <<= 1;

		tmp = val;
	}
	shm->samples = tmp;
	shm->samplepos = 0;
	shm->submission_chunk = 1;

	Con_Printf ("SDL audio spec  : %d Hz, %d samples, %d channels\n",
			obtained.freq, obtained.samples, obtained.channels);
	if (SDL_AudioDriverName(drivername, sizeof(drivername)) == NULL)
		strcpy(drivername, "(UNKNOWN)");
	buffersize = shm->samples * (shm->samplebits / 8);
	Con_Printf ("SDL audio driver: %s, %d bytes buffer\n", drivername, buffersize);

#if USE_HUNK_ALLOC
	shm->buffer = (unsigned char *) Hunk_AllocName(buffersize, "sdl_audio");
#else
	shm->buffer = (unsigned char *) calloc (1, buffersize);
	if (!shm->buffer)
	{
		SDL_CloseAudio();
		SDL_QuitSubSystem(SDL_INIT_AUDIO);
		shm = NULL;
		Con_Printf ("Failed allocating memory for SDL audio\n");
		return false;
	}
#endif

	SDL_PauseAudio(0);

	return true;
}

static int S_SDL_GetDMAPos (void)
{
	return shm->samplepos;
}

static void S_SDL_Shutdown (void)
{
	if (shm)
	{
		Con_Printf ("Shutting down SDL sound\n");
		SDL_CloseAudio();
		SDL_QuitSubSystem(SDL_INIT_AUDIO);
#if !USE_HUNK_ALLOC
		if (shm->buffer)
			free (shm->buffer);
#endif
		shm->buffer = NULL;
		shm = NULL;
	}
}

static void S_SDL_LockBuffer (void)
{
	SDL_LockAudio ();
}

static void S_SDL_Submit (void)
{
	SDL_UnlockAudio();
}

static void S_SDL_BlockSound (void)
{
	SDL_PauseAudio(1);
}

static void S_SDL_UnblockSound (void)
{
	SDL_PauseAudio(0);
}

snd_driver_t snddrv_sdl =
{
	S_SDL_Init,
	S_SDL_Shutdown,
	S_SDL_GetDMAPos,
	S_SDL_LockBuffer,
	S_SDL_Submit,
	S_SDL_BlockSound,
	S_SDL_UnblockSound,
	s_sdl_driver,
	SNDDRV_ID_SDL,
	false,
	NULL
};

#endif	/* HAVE_SDL_SOUND */

