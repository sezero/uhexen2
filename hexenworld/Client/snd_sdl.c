/*
	snd_sdl.c
	SDL sound driver for Linux Hexen II,  based on the SDLquake
	code by Sam Lantinga (http://www.libsdl.org/projects/quake/)

	$Header: /home/ozzie/Download/0000/uhexen2/hexenworld/Client/snd_sdl.c,v 1.4 2005-02-04 13:40:52 sezero Exp $
*/

#include <stdio.h>
#include "SDL.h"
//#include "SDL_byteorder.h"
#include "quakedef.h"

static int snd_inited;

extern int desired_speed;
extern int desired_bits;

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
		default:
        		Con_Printf("Unknown number of audio bits: %d\n",
								desired_bits);
			return 0;
	}
	desired.channels = 2;
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
			if ( ((obtained.format == AUDIO_S16LSB) &&
			     (SDL_BYTEORDER == SDL_LIL_ENDIAN)) ||
			     ((obtained.format == AUDIO_S16MSB) &&
			     (SDL_BYTEORDER == SDL_BIG_ENDIAN)) ) {
				/* Supported */
				break;
			}
			/* Unsupported, fall through */;
		default:
			/* Not supported -- force SDL to do our bidding */
			SDL_CloseAudio();
			if ( SDL_OpenAudio(&desired, NULL) < 0 ) {
        			Con_Printf("Couldn't open SDL audio: %s\n",
							SDL_GetError());
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
 * Revision 1.3  2005/02/04 11:29:38  sezero
 * make sdl_audio actually work (finally)
 *
 * Revision 1.2  2004/12/21 17:35:19  sezero
 * revert two commits (obsolete experimentals)
 *
 * Revision 1.5  2004/12/12 14:40:56  sezero
 * sync with steven
 *
 * Revision 1.4  2004/12/12 14:14:42  sezero
 * style changes to our liking
 *
 * Revision 1.3  2004/12/05 10:52:18  sezero
 * Sync with Steven, 2004-12-04 :
 *  Fix the "Old Mission" menu PoP
 *  Also release the windowed mouse on pause
 *  Heapsize is now 32768 default
 *  The final splash screens now centre the messages properly
 *  Add more mods to the video mods table
 *  Add the docs folder and update it
 *
 * Revision 1.2  2004/11/28 01:45:25  sezero
 * Log entries.
 *
 * Revision 1.1.1.1  2004/11/28 00:50:00  sezero
 */
