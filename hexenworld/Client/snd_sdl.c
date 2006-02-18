/*
	snd_sdl.c
	SDL sound driver for Linux Hexen II,  based on the SDLquake
	code by Sam Lantinga (http://www.libsdl.org/projects/quake/)
	Additional bits taken from QuakeForge and Quake3 projects.

	$Id: snd_sdl.c,v 1.15 2006-02-18 09:15:03 sezero Exp $
*/

#include "sdl_inc.h"
#include "quakedef.h"

static int snd_inited;

extern int desired_speed, desired_bits, desired_channels;
extern int tryrates[MAX_TRYRATES];

static void paint_audio(void *unused, Uint8 *stream, int len)
{
	if ( !shm )
		return;

	shm->buffer = stream;
	shm->samplepos += len/(shm->samplebits/8)/2;
	// Check for samplepos overflow?
	S_PaintChannels (shm->samplepos);
}

qboolean S_SDL_Init(void)
{
	SDL_AudioSpec desired, obtained;
	char	drivername[128];

	snd_inited = 0;

	if (SDL_InitSubSystem (SDL_INIT_AUDIO) < 0)
	{
		Con_Printf("Couldn't init SDL audio: %s\n", SDL_GetError());
		return 0;
	}

	/* Set up the desired format */
	desired.freq = desired_speed;
	switch (desired_bits)
	{
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
	desired.userdata = NULL;

	/* Open the audio device */
	if ( SDL_OpenAudio(&desired, &obtained) < 0 )
	{
		Con_Printf("Couldn't open SDL audio: %s\n", SDL_GetError());
		return 0;
	}

	/* Make sure we can support the audio format */
	switch (obtained.format)
	{
		case AUDIO_U8:
			/* Supported */
			break;
		case AUDIO_S16LSB:
		case AUDIO_S16MSB:
			if ( ((obtained.format == AUDIO_S16LSB) && (SDL_BYTEORDER == SDL_LIL_ENDIAN)) ||
				((obtained.format == AUDIO_S16MSB) && (SDL_BYTEORDER == SDL_BIG_ENDIAN)) )
				break;	/* Supported */
			else
				Con_Printf ("Warning: sound format / endianness mismatch\n");
		default:
			/* Not supported -- force SDL to do our bidding */
			Con_Printf ("Warning: unsupported audio format received\n");
			Con_Printf ("Warning: will try forcing sdl audio\n");
			SDL_CloseAudio();
			if ( SDL_OpenAudio(&desired, NULL) < 0 )
			{
				Con_Printf("Couldn't open SDL audio: %s\n", SDL_GetError());
				return 0;
			}
			memcpy(&obtained, &desired, sizeof(desired));
			break;
	}

	/* Fill the audio DMA information block */
	shm = &sn;
	shm->splitbuffer = 0;
	shm->samplebits = (obtained.format & 0xFF); // first byte of format is bits
	if (obtained.freq != desired_speed)
		Con_Printf ("Warning: Rate set (%i) didn't match requested rate (%i)!\n", obtained.freq, desired_speed);
	shm->speed = obtained.freq;
	shm->channels = obtained.channels;
	shm->samples = obtained.samples*shm->channels;
	shm->samplepos = 0;
	shm->submission_chunk = 1;

	shm->buffer = NULL;

	if (SDL_AudioDriverName(drivername, sizeof (drivername)) == NULL)
		strcpy(drivername, "(UNKNOWN)");

	snd_inited = 1;
	SDL_PauseAudio(0);

	Con_Printf("Audio Subsystem initialized in SDL mode.\n");
	Con_Printf ("SDL audio driver: %s\n", drivername);
	Con_Printf ("%5d stereo\n", shm->channels - 1);
	Con_Printf ("%5d samples\n", shm->samples);
	Con_Printf ("%5d samplepos\n", shm->samplepos);
	Con_Printf ("%5d samplebits\n", shm->samplebits);
	Con_Printf ("%5d submission_chunk\n", shm->submission_chunk);
	Con_Printf ("%5d speed\n", shm->speed);
	Con_Printf ("0x%x dma buffer address\n", (int) shm->buffer);
	Con_Printf ("%5d total_channels\n", total_channels);
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
		snd_inited = 0;
//		SDL_PauseAudio (1);
		SDL_CloseAudio();
		SDL_QuitSubSystem(SDL_INIT_AUDIO);
	}
}

void S_SDL_Submit(void)
{
}

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.14  2006/01/12 13:08:47  sezero
 * small update to the sdl sound driver
 *
 * Revision 1.13  2006/01/12 13:07:13  sezero
 * sound whitespace cleanup #1
 *
 * Revision 1.12  2006/01/12 12:43:49  sezero
 * Created an sdl_inc.h with all sdl version requirements and replaced all
 * SDL.h and SDL_mixer.h includes with it. Made the source to compile against
 * SDL versions older than 1.2.6 without disabling multisampling. Multisampling
 * (fsaa) option is now decided at runtime. Minimum required SDL and SDL_mixer
 * versions are now 1.2.4. If compiled without midi, minimum SDL required is
 * 1.2.0. Added SDL_mixer version checking to sdl-midi with measures to prevent
 * relocation errors.
 *
 * Revision 1.11  2005/07/05 17:16:53  sezero
 * Updated sdl sound (added soundinfo to init, various insignificant things)
 *
 * Revision 1.10  2005/06/28 17:20:32  sezero
 * Tiny cosmetic clean-up
 *
 * Revision 1.9  2005/06/28 17:01:52  sezero
 * Added warning messages to snd_sdl for endianness-format mismatches
 *
 * Revision 1.8  2005/06/12 07:28:54  sezero
 * clean-up of includes and a fix (hopefully) for endianness detection
 *
 * Revision 1.7  2005/06/06 10:14:18  sezero
 * put my usual requested/obtained sndrate mismatch
 * warning in snd_sdl, as well.
 *
 * Revision 1.6  2005/02/20 12:46:43  sezero
 * - Process all command line options in snd_dma.c, S_Startup() only.
 *   Targets will do to its bidding first. And don't die immediately,
 *   try setting alternative hw parameters. (FWIW, snd_oss.c now applies
 *   all hardware settings before mmaping the buffer)
 * - Check for requested and set rate mismatches and fail (Found in alsa
 *   examples, is it necessary at all? Commented out for now.)
 *
 * Revision 1.5  2005/02/06 15:22:56  sezero
 * log entries cleanup
 *
 * Revision 1.4  2005/02/04 13:40:52  sezero
 * build all all the sound drivers in and choose from command line
 *
 * Revision 1.3  2005/02/04 11:29:38  sezero
 * make sdl_audio actually work (finally)
 *
 * Revision 1.2  2004.12.05.10.52.18  sezero
 * Change desired.samples to 1024 (from Steven)
 *
 * Revision 1.1  2004/11/28 00:50:00  sezero
 * Initial version.
 */
