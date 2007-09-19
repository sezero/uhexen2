/*
	snd_sdl.c
	SDL audio driver for Hexen II: Hammer of Thyrion, based on the
	sdlquake-1.0.9 code (http://www.libsdl.org/projects/quake/) by
	Sam Lantinga from 2000.

	NOTES:  With this driver active, the game would randomly quit
	with an error message 'Cache_MakeLRU: active link'.  This was
	not a common error and it wassn't easy to reproduce.  See the
	following help forum threads for more info:
	http://sourceforge.net/forum/forum.php?thread_id=1308186&forum_id=425207
	http://sourceforge.net/forum/forum.php?thread_id=1821052&forum_id=425207

	$Id: snd_sdl.c,v 1.25 2007-09-19 06:19:33 sezero Exp $
*/

#define _SND_SYS_MACROS_ONLY

#include "quakedef.h"
#include "snd_sys.h"

#if HAVE_SDL_SOUND

#include "sdl_inc.h"

static void paint_audio (void *unused, Uint8 *stream, int len)
{
	if (!shm)
		return;

	shm->buffer = stream;
	shm->samplepos += len / (shm->samplebits/8) / 2;
	/* check for samplepos overflow? */
	S_PaintChannels (shm->samplepos);
}

qboolean S_SDL_Init (void)
{
	SDL_AudioSpec desired, obtained;
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
	desired.samples  = 1024; /* previously 512 S.A. */
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

	snd_skippaint = true;	/* our callback already 'paints' audio */
	memset ((void *) &sn, 0, sizeof(sn));
	shm = &sn;

	/* Fill the audio DMA information block */
	shm->samplebits = (obtained.format & 0xFF); /* first byte of format is bits */
	if (obtained.freq != desired_speed)
		Con_Printf ("Warning: Rate set (%d) didn't match requested rate (%d)!\n", obtained.freq, desired_speed);
	shm->speed = obtained.freq;
	shm->channels = obtained.channels;
	shm->samples = obtained.samples * shm->channels;
	shm->samplepos = 0;
	shm->submission_chunk = 1;

	shm->buffer = NULL;

	if (SDL_AudioDriverName(drivername, sizeof(drivername)) == NULL)
		strcpy(drivername, "(UNKNOWN)");

	SDL_PauseAudio(0);

	Con_Printf ("SDL audio driver: %s\n", drivername);

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
//		SDL_PauseAudio(1);
		SDL_CloseAudio();
		SDL_QuitSubSystem(SDL_INIT_AUDIO);
		shm->buffer = NULL;
		shm = NULL;
		snd_skippaint = false;
	}
}

void S_SDL_Submit (void)
{
}

#endif	/* HAVE_SDL_SOUND */

