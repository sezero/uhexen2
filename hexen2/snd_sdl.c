/*
	snd_sdl.c
	SDL sound driver for Linux Hexen II,  based on the SDLquake
	code by Sam Lantinga (http://www.libsdl.org/projects/quake/)

	$Id: snd_sdl.c,v 1.14 2005-06-28 17:20:20 sezero Exp $
*/

#include "SDL.h"
#include "quakedef.h"

static int snd_inited;

extern int desired_speed, desired_bits, desired_channels;
extern int tryrates[MAX_TRYRATES];

static void paint_audio(void *unused, Uint8 *stream, int len)
{
	if ( shm ) {
		shm->buffer = stream;
		shm->samplepos += len/(shm->samplebits/8)/2;
		// Check for samplepos overflow?
		S_PaintChannels (shm->samplepos);
	}
}

qboolean S_SDL_Init(void)
{
	SDL_AudioSpec desired, obtained;

	snd_inited = 0;

	if (SDL_InitSubSystem (SDL_INIT_AUDIO)<0) {
		Con_Printf("Couldn't init SDL audio: %s\n", SDL_GetError());
		return 0;
	}

	/* Set up the desired format */
	desired.freq = desired_speed;
	switch (desired_bits) {
		case 8:
			desired.format = AUDIO_U8;
			break;
		case 16:
			if ( SDL_BYTEORDER == SDL_BIG_ENDIAN )
				desired.format = AUDIO_S16MSB;
			else
				desired.format = AUDIO_S16LSB;
			break;
	}
	desired.channels = desired_channels;
	desired.samples  = 1024; // previously 512 S.A.
	desired.callback = paint_audio;

	/* Open the audio device */
	if ( SDL_OpenAudio(&desired, &obtained) < 0 ) {
		Con_Printf("Couldn't open SDL audio: %s\n", SDL_GetError());
		return 0;
	}

	/* Make sure we can support the audio format */
	switch (obtained.format) {
		case AUDIO_U8:
			/* Supported */
			break;
		case AUDIO_S16LSB:
		case AUDIO_S16MSB:
			if ( ((obtained.format == AUDIO_S16LSB) && (SDL_BYTEORDER == SDL_LIL_ENDIAN)) ||
				((obtained.format == AUDIO_S16MSB) && (SDL_BYTEORDER == SDL_BIG_ENDIAN)) )
				break;	/* Supported */
		default:
			/* Not supported -- force SDL to do our bidding */
			Con_Printf ("Warning: sound format / endianness mismatch\n");
			Con_Printf ("Warning: will try forcing sdl audio\n");
			SDL_CloseAudio();
			if ( SDL_OpenAudio(&desired, NULL) < 0 ) {
				Con_Printf("Couldn't open SDL audio: %s\n", SDL_GetError());
				return 0;
			}
			memcpy(&obtained, &desired, sizeof(desired));
			break;
	}

	SDL_PauseAudio(0);

	/* Fill the audio DMA information block */
	shm = &sn;
	shm->splitbuffer = 0;
	shm->samplebits = (obtained.format & 0xFF);
	if (obtained.freq != desired_speed)
		Con_Printf ("Warning: Rate set (%i) didn't match requested rate (%i)!\n", obtained.freq, desired_speed);
	shm->speed = obtained.freq;
	shm->channels = obtained.channels;
	shm->samples = obtained.samples*shm->channels;
	shm->samplepos = 0;
	shm->submission_chunk = 1;
	shm->buffer = NULL;

	snd_inited = 1;
	Con_Printf("Audio Subsystem initialized in SDL mode.\n");
	return 1;
}

int S_SDL_GetDMAPos(void)
{
	return shm->samplepos;
}

void S_SDL_Shutdown(void)
{
	if (snd_inited)
	{
		Con_Printf ("Shutting down SDL sound\n");
//		SDL_PauseAudio (1);
		SDL_CloseAudio();
		snd_inited = 0;
		SDL_QuitSubSystem(SDL_INIT_AUDIO);
	}
}

void S_SDL_Submit(void)
{
}

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.13  2005/06/28 17:01:49  sezero
 * Added warning messages to snd_sdl for endianness-format mismatches
 *
 * Revision 1.12  2005/06/12 07:28:51  sezero
 * clean-up of includes and a fix (hopefully) for endianness detection
 *
 * Revision 1.11  2005/06/06 10:14:05  sezero
 * put my usual requested/obtained sndrate mismatch
 * warning in snd_sdl, as well.
 *
 * Revision 1.10  2005/02/20 12:44:58  sezero
 * - Process all command line options in snd_dma.c, S_Startup() only.
 *   Targets will do to its bidding first. And don't die immediately,
 *   try setting alternative hw parameters. (FWIW, snd_oss.c now applies
 *   all hardware settings before mmaping the buffer)
 * - Check for requested and set rate mismatches and fail (Found in alsa
 *   examples, is it necessary at all? Commented out for now.)
 *
 * Revision 1.9  2005/02/06 15:22:55  sezero
 * log entries cleanup
 *
 * Revision 1.8  2005/02/04 13:40:20  sezero
 * build all all the sound drivers in and choose from command line
 *
 * Revision 1.7  2005/02/04 11:28:59  sezero
 * make sdl_audio actually work (finally)
 *
 * Revision 1.2  2004.12.05.10.52.18  sezero
 * Change desired.samples to 1024 (from Steven)
 *
 * Revision 1.1  2004/11/28 00:50:00  sezero
 * Initial version.
 */
