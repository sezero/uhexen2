/*
	snd_alsa.c
	$Id: snd_alsa.c,v 1.7 2005-02-20 12:44:58 sezero Exp $

	ALSA 1.0 sound driver for Linux Hexen II

	Copyright (C) 1999,2004  contributors of the QuakeForge project

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

	See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to:

		Free Software Foundation, Inc.
		59 Temple Place - Suite 330
		Boston, MA  02111-1307, USA
*/

#ifndef NO_ALSA

#include <stdio.h>
#include <dlfcn.h>
#include <alsa/asoundlib.h>
#include "quakedef.h"

static void *alsa_handle;
static char *pcmname = "default";
static snd_pcm_t *pcm;
static int snd_inited;
static snd_pcm_uframes_t buffer_size;
extern int desired_bits, desired_speed, desired_channels;
extern int tryrates[MAX_TRYRATES];
extern int soundtime;

#define HX2_ALSA(ret, func, params) \
static ret (*hx2##func) params;
#include "alsa_funcs.h"
#undef HX2_ALSA

static qboolean load_libasound (void)
{
	if (!(alsa_handle = dlopen ("libasound.so.2", RTLD_GLOBAL | RTLD_NOW))) {
		Con_Printf ("Couldn't load libasound.so.2: %s\n", dlerror ());
		return false;
	}

#define HX2_ALSA(ret, func, params) \
	if (!(hx2##func = dlsym (alsa_handle, #func))) { \
		Con_Printf ("Couldn't load ALSA function %s\n", #func); \
		dlclose (alsa_handle); \
		alsa_handle = 0; \
		return false; \
}

#include "alsa_funcs.h"
#undef HX2_ALSA
	return true;
}

#define snd_pcm_hw_params_sizeof hx2snd_pcm_hw_params_sizeof
#define snd_pcm_sw_params_sizeof hx2snd_pcm_sw_params_sizeof

qboolean S_ALSA_Init (void)
{
	int			i, err;
	unsigned int		rate;
	snd_pcm_uframes_t	frag_size;
	snd_pcm_hw_params_t	*hw;
	snd_pcm_sw_params_t	*sw;

	if (!load_libasound ())
		return false;

	snd_pcm_hw_params_alloca (&hw);
	snd_pcm_sw_params_alloca (&sw);

	if ((err = COM_CheckParm("-alsadev")) != 0)
		pcmname = com_argv[err+1];

	err = hx2snd_pcm_open (&pcm, pcmname, SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK);
	if (err < 0) {
		Con_Printf ("Error: audio open error: %s\n", hx2snd_strerror (err));
		return 0;
	}
	Con_Printf ("Using PCM %s.\n", pcmname);

	err = hx2snd_pcm_hw_params_any (pcm, hw);
	if (err < 0) {
		Con_Printf ("ALSA: error setting hw_params_any. %s\n", hx2snd_strerror (err));
		goto error;
	}

	err = hx2snd_pcm_hw_params_set_access (pcm, hw, SND_PCM_ACCESS_MMAP_INTERLEAVED);
	if (err < 0) {
		Con_Printf ("ALSA: interleaved is not supported. %s\n", hx2snd_strerror (err));
		goto error;
	}

	err = hx2snd_pcm_hw_params_set_format (pcm, hw, desired_bits == 8 ? SND_PCM_FORMAT_U8 : SND_PCM_FORMAT_S16);
	if (err < 0) {
		Con_Printf ("Problems setting sndformat, retrying...\n");
		desired_bits = (desired_bits == 8) ? 16 : 8;
		err = hx2snd_pcm_hw_params_set_format (pcm, hw, desired_bits == 8 ? SND_PCM_FORMAT_U8 : SND_PCM_FORMAT_S16);
		if (err < 0) {
			Con_Printf ("ALSA: Neither 8 nor 16 bit format supported. %s\n", hx2snd_strerror (err));
			goto error;
		}
	}

	err = hx2snd_pcm_hw_params_set_channels (pcm, hw, desired_channels);
	if (err < 0) {
		Con_Printf ("Problems setting mono/stereo, retrying..\n");
		desired_channels = (desired_channels == 2) ? 1 : 2;
		err = hx2snd_pcm_hw_params_set_channels (pcm, hw, desired_channels);
		if (err < 0) {
			Con_Printf ("ALSA: could not set desired channels. %s\n", hx2snd_strerror (err));
			goto error;
		}
	}

	rate = desired_speed;
	err = hx2snd_pcm_hw_params_set_rate_near (pcm, hw, &rate, 0);
	if (err < 0) {
		Con_Printf("Problems setting dsp speed, trying alternatives..\n");
		desired_speed = 0;
		for (i=0 ; i<MAX_TRYRATES ; i++) {
			rate = tryrates[i];
			err = hx2snd_pcm_hw_params_set_rate_near (pcm, hw, &rate, 0);
			if (err < 0) {
				Con_DPrintf ("Could not set dsp to speed %d\n", tryrates[i]);
			} else {
			/* From alsa examples: is this necessary?
				if (rate != tryrates[i]) {
					Con_Printf ("Failure: Rate set (%d) didn't match requested rate (%d)!\n", rate, tryrates[i]);
					goto error;
				}
			*/	desired_speed = rate;
				break;
			}
		}
		if (desired_speed == 0) {
			Con_Printf ("ALSA: Could not set dsp to any speed!..\n");
			goto error;
		}
	}
/*	else {	// From alsa examples: is this necessary?
		if (rate != desired_speed) {
			Con_Printf ("Failure: Rate set (%d) didn't match requested rate (%d)!\n", rate, desired_speed);
			goto error;
		}
	}
*/	frag_size = 8 * desired_bits * desired_speed / 11025;

	err = hx2snd_pcm_hw_params_set_period_size_near (pcm, hw, &frag_size, 0);
	if (err < 0) {
		Con_Printf ("ALSA: unable to set period size near %i. %s\n",
			    (int) frag_size, hx2snd_strerror (err));
		goto error;
	}

	err = hx2snd_pcm_hw_params (pcm, hw);
	if (err < 0) {
		Con_Printf ("ALSA: unable to install hw params. %s\n", hx2snd_strerror (err));
		goto error;
	}

	err = hx2snd_pcm_sw_params_current (pcm, sw);
	if (err < 0) {
		Con_Printf ("ALSA: unable to determine current sw params. %s\n", hx2snd_strerror (err));
		goto error;
	}

	err = hx2snd_pcm_sw_params_set_start_threshold (pcm, sw, ~0U);
	if (err < 0) {
		Con_Printf ("ALSA: unable to set playback threshold. %s\n", hx2snd_strerror (err));
		goto error;
	}

	err = hx2snd_pcm_sw_params_set_stop_threshold (pcm, sw, ~0U);
	if (err < 0) {
		Con_Printf ("ALSA: unable to set playback stop threshold. %s\n", hx2snd_strerror (err));
		goto error;
	}

	err = hx2snd_pcm_sw_params (pcm, sw);
	if (err < 0) {
		Con_Printf ("ALSA: unable to install sw params. %s\n", hx2snd_strerror (err));
		goto error;
	}

	shm = &sn;
	memset ((dma_t *) shm, 0, sizeof (*shm));
	shm->splitbuffer = 0;
	shm->channels = desired_channels;
	err = hx2snd_pcm_hw_params_get_period_size (hw, 
			(snd_pcm_uframes_t *) (&shm->submission_chunk), 0);
			// don't mix less than this
	if (err < 0) {
		Con_Printf ("ALSA: unable to get period size. %s\n", hx2snd_strerror (err));
		goto error;
	}
	shm->samplepos = 0; // in mono samples
	shm->samplebits = desired_bits;
	err = hx2snd_pcm_hw_params_get_buffer_size (hw, &buffer_size);
	if (err < 0) {
		Con_Printf ("ALSA: unable to get buffer size. %s\n", hx2snd_strerror(err));
		goto error;
	}

	shm->samples = buffer_size * shm->channels; // mono samples in buffer
	shm->speed = desired_speed;

	snd_inited = 1;
	SNDDMA_GetDMAPos ();	// sets shm->buffer
	Con_Printf("Audio Subsystem initialized in ALSA mode.\n");

	Con_Printf ("%5d stereo\n", shm->channels - 1);
	Con_Printf ("%5d samples\n", shm->samples);
	Con_Printf ("%5d samplepos\n", shm->samplepos);
	Con_Printf ("%5d samplebits\n", shm->samplebits);
	Con_Printf ("%5d submission_chunk\n", shm->submission_chunk);
	Con_Printf ("%5d speed\n", shm->speed);
	Con_Printf ("0x%x dma buffer\n", (int) shm->buffer);
	Con_Printf ("%5d total_channels\n", total_channels);

	return 1;
error:
	hx2snd_pcm_close (pcm);
	return 0;
}

int S_ALSA_GetDMAPos (void)
{
	snd_pcm_uframes_t offset;
	snd_pcm_uframes_t nframes = shm->samples/shm->channels;
	const snd_pcm_channel_area_t *areas;

	if (!snd_inited) {
		Con_Printf ("Sound not initialized\n");
		return 0;
	}

	hx2snd_pcm_avail_update (pcm);
	hx2snd_pcm_mmap_begin (pcm, &areas, &offset, &nframes);
	// The following commit was absent in QF, causing the
	// very first sound to be corrupted
	hx2snd_pcm_mmap_commit (pcm, offset, nframes);
	offset *= shm->channels;
	nframes *= shm->channels;
	shm->samplepos = offset;
	shm->buffer = areas->addr; //XXX FIXME there's an area per channel
	return shm->samplepos;
}

void S_ALSA_Shutdown (void)
{
	if (snd_inited) {
		Con_Printf ("Shutting down ALSA sound\n");
		hx2snd_pcm_close (pcm);
		snd_inited = 0;
	}
}

/*
	SNDDMA_Submit
	Send sound to device if buffer isn't really the dma buffer
*/
void S_ALSA_Submit (void)
{
	snd_pcm_uframes_t offset;
	snd_pcm_uframes_t nframes;
	const snd_pcm_channel_area_t *areas;
	int         state;
	int         count = paintedtime - soundtime;

	nframes = count / shm->channels;
	hx2snd_pcm_avail_update (pcm);
	hx2snd_pcm_mmap_begin (pcm, &areas, &offset, &nframes);
	state = hx2snd_pcm_state (pcm);

	switch (state) {
		case SND_PCM_STATE_PREPARED:
			hx2snd_pcm_mmap_commit (pcm, offset, nframes);
			hx2snd_pcm_start (pcm);
			break;
		case SND_PCM_STATE_RUNNING:
			hx2snd_pcm_mmap_commit (pcm, offset, nframes);
			break;
		default:
			break;
	}
}

#endif	// NO_ALSA

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.6  2005/02/14 15:12:32  sezero
 * added ability to disable ALSA support at compile time
 *
 * Revision 1.5  2005/02/14 10:08:00  sezero
 * alsa sound:
 * - replicate the order in quakeforge as much as possible
 *
 * Revision 1.4  2005/02/14 10:07:03  sezero
 * alsa sound improvements:
 * - more error checking and detailed error reporting
 *
 * Revision 1.3  2005/02/11 23:44:22  sezero
 * add 48000 to the alsa rate switch
 *
 * Revision 1.2  2005/02/05 16:38:34  sezero
 * fix silly copy+paste error in snd_alsa.c
 *
 * Revision 1.1  2005/02/04 11:59:16  sezero
 * add ALSA sound driver (from the quakeforge project)
 *
 */
