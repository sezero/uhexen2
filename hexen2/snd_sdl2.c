/*
	snd_sdl2.c
	SDL audio driver for Hexen II: Hammer of Thyrion, based on the
	implementations found in the quakeforge and quake3-icculus.org
	projects.

	$Id: snd_sdl2.c,v 1.2 2007-09-20 16:15:16 sezero Exp $
*/

#define _SND_SYS_MACROS_ONLY

#include "quakedef.h"
#include "snd_sys.h"

#if HAVE_SDL_SOUND

#include "sdl_inc.h"

// whether to use hunk for allocating dma
// buffer memory. either 1, or 0.
#define USE_HUNK_ALLOC		0

static int	buffersize;


static void paint_audio (void *unused, Uint8 *stream, int len)
{
	int	pos, tobufend;
	int	len1, len2;

	if (!shm)
	{	/* shouldn't happen, but just in case */
		memset(stream, 0, len);
		return;
	}

	pos = (shm->samplepos * (shm->samplebits/8));
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
		shm->samplepos += (len1 / (shm->samplebits/8));
	}
	else
	{	/* wraparound? */
		memcpy(stream+len1, shm->buffer, len2);
		shm->samplepos = (len2 / (shm->samplebits/8));
	}

	if (shm->samplepos >= buffersize)
		shm->samplepos = 0;
}

qboolean S_SDL_Init (void)
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
	else
		desired.samples = 2048;	/* shrug */
	desired.callback = paint_audio;
	desired.userdata = NULL;

	/* Open the audio device */
	if (SDL_OpenAudio(&desired, &obtained) < 0)
	{
		Con_Printf("Couldn't open SDL audio: %s\n", SDL_GetError());
		SDL_QuitSubSystem(SDL_INIT_AUDIO);
		return false;
	}

	/* Make sure we can support the audio format */
	switch (obtained.format)
	{
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

	SDL_LockAudio();

	memset ((void *) &sn, 0, sizeof(sn));
	shm = &sn;

	/* Fill the audio DMA information block */
	shm->samplebits = (obtained.format & 0xFF); /* first byte of format is bits */
	if (obtained.freq != desired_speed)
		Con_Printf ("Warning: Rate set (%d) didn't match requested rate (%d)!\n", obtained.freq, desired_speed);
	shm->speed = obtained.freq;
	shm->channels = obtained.channels;
	tmp = (obtained.samples * obtained.channels) * 8;
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

	buffersize = shm->samples * (shm->samplebits / 8);
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

	if (SDL_AudioDriverName(drivername, sizeof(drivername)) == NULL)
		strcpy(drivername, "(UNKNOWN)");

	SDL_UnlockAudio();
	SDL_PauseAudio(0);

	Con_Printf ("SDL audio driver: %s, buffer size: %d\n", drivername, buffersize);

	return true;
}

int S_SDL_GetDMAPos (void)
{
	return shm->samplepos;
}

void S_SDL_Shutdown (void)
{
	if (shm)
	{
		Con_Printf ("Shutting down SDL sound\n");
		SDL_PauseAudio(1);
		SDL_LockAudio ();
		SDL_CloseAudio();
#if !USE_HUNK_ALLOC
		if (shm->buffer)
			free (shm->buffer);
#endif
		SDL_QuitSubSystem(SDL_INIT_AUDIO);
		shm->buffer = NULL;
		shm = NULL;
	}
}

void S_SDL_Submit (void)
{
}

#endif	/* HAVE_SDL_SOUND */

