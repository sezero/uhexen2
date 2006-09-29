/*
	snd_sdl.c
	SDL sound driver for Linux Hexen II,  based on the SDLquake
	code by Sam Lantinga (http://www.libsdl.org/projects/quake/)
	Additional bits taken from QuakeForge and Quake3 projects.

	$Id: snd_sdl.c,v 1.23 2006-09-29 18:00:35 sezero Exp $
*/

#include "sdl_inc.h"
#include "quakedef.h"


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

	if (SDL_InitSubSystem (SDL_INIT_AUDIO) < 0)
	{
		Con_Printf("Couldn't init SDL audio: %s\n", SDL_GetError());
		return 0;
	}

	/* Set up the desired format */
	desired.freq = desired_speed;
	desired.format = (desired_bits == 16) ? AUDIO_S16SYS : AUDIO_U8;
	desired.channels = desired_channels;
	desired.samples  = 1024; // previously 512 S.A.
	desired.callback = paint_audio;
	desired.userdata = NULL;

	/* Open the audio device */
	if ( SDL_OpenAudio(&desired, &obtained) < 0 )
	{
		Con_Printf("Couldn't open SDL audio: %s\n", SDL_GetError());
		SDL_QuitSubSystem(SDL_INIT_AUDIO);
		return 0;
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
		return 0;
	}

	memset ((dma_t *) &sn, 0, sizeof(sn));
	shm = &sn;

	/* Fill the audio DMA information block */
	shm->splitbuffer = 0;
	shm->samplebits = (obtained.format & 0xFF); // first byte of format is bits
	if (obtained.freq != desired_speed)
		Con_Printf ("Warning: Rate set (%d) didn't match requested rate (%d)!\n", obtained.freq, desired_speed);
	shm->speed = obtained.freq;
	shm->channels = obtained.channels;
	shm->samples = obtained.samples*shm->channels;
	shm->samplepos = 0;
	shm->submission_chunk = 1;

	shm->buffer = NULL;

	if (SDL_AudioDriverName(drivername, sizeof (drivername)) == NULL)
		strcpy(drivername, "(UNKNOWN)");

	SDL_PauseAudio(0);

	Con_Printf ("SDL audio driver: %s\n", drivername);

	return 1;
}

int S_SDL_GetDMAPos(void)
{
	return shm->samplepos;
}

void S_SDL_Shutdown(void)
{
	if (shm)
	{
		Con_Printf ("Shutting down SDL sound\n");
//		SDL_PauseAudio(1);
		SDL_CloseAudio();
		SDL_QuitSubSystem(SDL_INIT_AUDIO);
		shm->buffer = NULL;
		shm = NULL;
	}
}

void S_SDL_Submit(void)
{
}

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.22  2006/09/29 11:17:51  sezero
 * more sound clean up
 *
 * Revision 1.21  2006/09/27 17:17:30  sezero
 * a lot of clean-ups in sound and midi files.
 *
 * Revision 1.20  2006/05/20 12:38:01  sezero
 * cleaned up sound tryrates, etc. changed tryrates array to include
 * 48000, 24000, and 16000 speeds (this should help 48khz AC97 chips,
 * from darkplaces).
 *
 * Revision 1.19  2006/02/18 09:15:03  sezero
 * updated some snd_sdl comments
 *
 * Revision 1.18  2006/01/12 13:08:47  sezero
 * small update to the sdl sound driver
 *
 * Revision 1.17  2006/01/12 13:07:09  sezero
 * sound whitespace cleanup #1
 *
 * Revision 1.16  2006/01/12 12:43:49  sezero
 * Created an sdl_inc.h with all sdl version requirements and replaced all
 * SDL.h and SDL_mixer.h includes with it. Made the source to compile against
 * SDL versions older than 1.2.6 without disabling multisampling. Multisampling
 * (fsaa) option is now decided at runtime. Minimum required SDL and SDL_mixer
 * versions are now 1.2.4. If compiled without midi, minimum SDL required is
 * 1.2.0. Added SDL_mixer version checking to sdl-midi with measures to prevent
 * relocation errors.
 *
 * Revision 1.15  2005/07/05 17:16:52  sezero
 * Updated sdl sound (added soundinfo to init, various insignificant things)
 *
 * Revision 1.14  2005/06/28 17:20:20  sezero
 * Tiny cosmetic clean-up
 *
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
