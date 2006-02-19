/*
	snd_alsa.c
	$Id: snd_alsa.c,v 1.15 2006-02-19 12:33:24 sezero Exp $

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

#if defined(__linux__) && !defined(NO_ALSA)

#include "quakedef.h"
#include <dlfcn.h>
#include <alsa/asoundlib.h>

static void *alsa_handle;
//static char *pcmname = "hw:0,0";
static char *pcmname = "default";
static snd_pcm_t *pcm;
static int snd_inited;
static snd_pcm_uframes_t buffer_size;
static snd_pcm_hw_params_t *hw;
static snd_pcm_sw_params_t *sw;
extern int desired_bits, desired_speed, desired_channels;
extern int tryrates[MAX_TRYRATES];
extern int soundtime;

int S_ALSA_GetDMAPos (void);

#define HX2_ALSA(ret, func, params) \
static ret (*hx2##func) params;
#include "alsa_funcs.h"
#undef HX2_ALSA

static qboolean load_libasound (void)
{
	alsa_handle = dlopen ("libasound.so.2", RTLD_GLOBAL | RTLD_NOW);
	if (!alsa_handle)
	{
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

#define ALSA_CHECK_ERR(check, fmt, args...) {\
	if (check < 0) {\
		Con_Printf ("ALSA: " fmt, ##args);\
		goto error;\
	}\
}

static snd_pcm_uframes_t round_buffer_size (snd_pcm_uframes_t sz)
{
	snd_pcm_uframes_t mask = ~0;

	while (sz & mask)
	{
		sz &= mask;
		mask <<= 1;
	}
	return sz;
}

qboolean S_ALSA_Init (void)
{
	int			i, err;
	unsigned int		rate;
	int			tmp_bits, tmp_chan;
	snd_pcm_uframes_t	frag_size;

	if (!load_libasound ())
		return false;

	if ((err = COM_CheckParm("-alsadev")) != 0)
		pcmname = com_argv[err+1];

	err = hx2snd_pcm_open (&pcm, pcmname, SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK);
	if (err < 0)
	{
		Con_Printf ("ALSA: audio open error: %s\n", hx2snd_strerror (err));
		return 0;
	}
	Con_Printf ("Using PCM %s.\n", pcmname);

	err = hx2snd_pcm_hw_params_malloc (&hw);
	ALSA_CHECK_ERR(err, "unable to allocate hardware params. %s\n", hx2snd_strerror (err));

	err = hx2snd_pcm_hw_params_any (pcm, hw);
	ALSA_CHECK_ERR(err, "unable to init hardware params. %s\n", hx2snd_strerror (err));

	err = hx2snd_pcm_hw_params_set_access (pcm, hw, SND_PCM_ACCESS_MMAP_INTERLEAVED);
	ALSA_CHECK_ERR(err, "unable to set interleaved access. %s\n", hx2snd_strerror (err));

	tmp_bits = (desired_bits == 8) ? SND_PCM_FORMAT_U8 : SND_PCM_FORMAT_S16;
	err = hx2snd_pcm_hw_params_set_format (pcm, hw, tmp_bits);
	if (err < 0)
	{
		tmp_bits = (desired_bits == 8) ? 16 : 8;
		Con_Printf ("Problems setting %d bit format, retrying for %d bit\n", desired_bits, tmp_bits);
		tmp_bits = (desired_bits == 8) ? SND_PCM_FORMAT_S16 : SND_PCM_FORMAT_U8;
		err = hx2snd_pcm_hw_params_set_format (pcm, hw, tmp_bits);
		ALSA_CHECK_ERR(err, "Neither 8 nor 16 bit format supported. %s\n", hx2snd_strerror (err));
	}
	tmp_bits = (tmp_bits == SND_PCM_FORMAT_U8) ? 8 : 16;

	tmp_chan = desired_channels;
	err = hx2snd_pcm_hw_params_set_channels (pcm, hw, tmp_chan);
	if (err < 0)
	{
		Con_Printf ("Problems setting channels to ");
		if (desired_channels == 2)
			Con_Printf ("stereo, retrying for mono\n");
		else
			Con_Printf ("mono, retrying for stereo\n");
		tmp_chan = (desired_channels == 2) ? 1 : 2;
		err = hx2snd_pcm_hw_params_set_channels (pcm, hw, tmp_chan);
		ALSA_CHECK_ERR(err, "unable to set desired channels. %s\n", hx2snd_strerror (err));
	}

	rate = desired_speed;
	err = hx2snd_pcm_hw_params_set_rate_near (pcm, hw, &rate, 0);
	if (err < 0)
	{
		Con_Printf("Problems setting sample rate, trying alternatives..\n");
		for (i=0 ; i<MAX_TRYRATES ; i++)
		{
			rate = tryrates[i];
			err = hx2snd_pcm_hw_params_set_rate_near (pcm, hw, &rate, 0);
			if (err < 0)
			{
				Con_DPrintf ("Unable to set sample rate %d\n", tryrates[i]);
				rate = 0;
			}
			else
			{
				if (rate != tryrates[i])
				{
					Con_Printf ("Warning: Rate set (%d) didn't match requested rate (%d)!\n", rate, tryrates[i]);
				//	goto error;
				}
				break;
			}
		}
		if (rate == 0)
		{
			Con_Printf ("ALSA: Unable to set any sample rate !\n");
			goto error;
		}
	}
	else
	{
		if (rate != desired_speed)
		{
			Con_Printf ("Warning: Rate set (%d) didn't match requested rate (%d)!\n", rate, desired_speed);
		//	goto error;
		}
	}

	frag_size = 8 * tmp_bits * rate / 11025;

	err = hx2snd_pcm_hw_params_set_period_size_near (pcm, hw, &frag_size, 0);
	ALSA_CHECK_ERR(err, "unable to set period size near %i. %s\n",
			    (int) frag_size, hx2snd_strerror (err));

	err = hx2snd_pcm_hw_params (pcm, hw);
	ALSA_CHECK_ERR(err, "unable to install hardware params. %s\n", hx2snd_strerror (err));

	err = hx2snd_pcm_sw_params_malloc (&sw);
	ALSA_CHECK_ERR(err, "unable to allocate software params. %s\n", hx2snd_strerror (err));

	err = hx2snd_pcm_sw_params_current (pcm, sw);
	ALSA_CHECK_ERR(err, "unable to determine current software params. %s\n", hx2snd_strerror (err));

	err = hx2snd_pcm_sw_params_set_start_threshold (pcm, sw, ~0U);
	ALSA_CHECK_ERR(err, "unable to set playback threshold. %s\n", hx2snd_strerror (err));

	err = hx2snd_pcm_sw_params_set_stop_threshold (pcm, sw, ~0U);
	ALSA_CHECK_ERR(err, "unable to set playback stop threshold. %s\n", hx2snd_strerror (err));

	err = hx2snd_pcm_sw_params (pcm, sw);
	ALSA_CHECK_ERR(err, "unable to install software params. %s\n", hx2snd_strerror (err));

	shm = &sn;
	memset ((dma_t *) shm, 0, sizeof (*shm));
	shm->splitbuffer = 0;
	shm->channels = tmp_chan;

	// don't mix less than this in mono samples:
/*	err = hx2snd_pcm_hw_params_get_period_size (hw, 
			(snd_pcm_uframes_t *) (&shm->submission_chunk), 0);
	ALSA_CHECK_ERR(err, "unable to get period size. %s\n", hx2snd_strerror (err));
*/
	shm->submission_chunk = 1;
	shm->samplepos = 0;
	shm->samplebits = tmp_bits;
	err = hx2snd_pcm_hw_params_get_buffer_size (hw, &buffer_size);
	ALSA_CHECK_ERR(err, "unable to get buffer size. %s\n", hx2snd_strerror(err));

	if (buffer_size != round_buffer_size (buffer_size))
	{
		Sys_Printf ("ALSA: WARNING: non-power of 2 buffer size. sound may be\n");
		Sys_Printf ("unsatisfactory. Recommend using either the plughw or hw\n");
		Sys_Printf ("devices or adjusting dmix to have a power of 2 buf size\n");
	}

	shm->samples = buffer_size * shm->channels; // mono samples in buffer
	shm->speed = rate;

	snd_inited = 1;
	S_ALSA_GetDMAPos ();	// sets shm->buffer
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
	if (hw)
		hx2snd_pcm_hw_params_free(hw);
	if (sw)
		hx2snd_pcm_sw_params_free(sw);
	return 0;
}

int S_ALSA_GetDMAPos (void)
{
	snd_pcm_uframes_t offset;
	snd_pcm_uframes_t nframes = shm->samples/shm->channels;
	const snd_pcm_channel_area_t *areas;

	if (!snd_inited)
		return 0;

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
	if (snd_inited)
	{
		Con_Printf ("Shutting down ALSA sound\n");
		snd_inited = 0;
		hx2snd_pcm_drop (pcm);
		hx2snd_pcm_close (pcm);
		hx2snd_pcm_hw_params_free(hw);
		hx2snd_pcm_sw_params_free(sw);
		shm->buffer = NULL;
	}
}

/*
	SNDDMA_Submit
	Send sound to the device
*/
void S_ALSA_Submit (void)
{
	snd_pcm_uframes_t offset;
	snd_pcm_uframes_t nframes;
	const snd_pcm_channel_area_t *areas;
	int state;
	int count = paintedtime - soundtime;

	nframes = count / shm->channels;
	hx2snd_pcm_avail_update (pcm);
	hx2snd_pcm_mmap_begin (pcm, &areas, &offset, &nframes);
	state = hx2snd_pcm_state (pcm);

	switch (state)
	{
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
 * Revision 1.14  2006/01/12 13:11:16  sezero
 * do not change the desined_XXX sound variables in case of failures.
 * sound wnitespace cleanup #2 in snd_oss.
 *
 * Revision 1.13  2006/01/12 12:57:45  sezero
 * moved init of platform specific variables and function pointers to snd_sys
 *
 * Revision 1.12  2006/01/12 12:48:12  sezero
 * small alsa buffersize update taken from the quakeforge tree. also added
 * a ALSA_CHECK_ERR macro to make the init procedure more readable. coding
 * style cleanup.
 *
 * Revision 1.11  2005/07/05 17:12:01  sezero
 * Updated alsa driver (various insignificant things)
 *
 * Revision 1.10  2005/06/12 07:31:18  sezero
 * enabled alsa only on linux platforms
 *
 * Revision 1.9  2005/06/12 07:28:54  sezero
 * clean-up of includes and a fix (hopefully) for endianness detection
 *
 * Revision 1.8  2005/03/05 14:34:21  sezero
 * Try to use what frequency is actually set:
 * I first naively thought that requested-provided frequency
 * mismatches wouldn't happen, but I got one weird oss report
 * which includes this case. Let's see what this does now...
 *
 * Revision 1.7  2005/02/20 12:46:43  sezero
 * - Process all command line options in snd_dma.c, S_Startup() only.
 *   Targets will do to its bidding first. And don't die immediately,
 *   try setting alternative hw parameters. (FWIW, snd_oss.c now applies
 *   all hardware settings before mmaping the buffer)
 * - Check for requested and set rate mismatches and fail (Found in alsa
 *   examples, is it necessary at all? Commented out for now.)
 *
 * Revision 1.6  2005/02/14 15:12:52  sezero
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
 * Revision 1.3  2005/02/11 23:44:32  sezero
 * add 48000 to the alsa rate switch
 *
 * Revision 1.2  2005/02/05 16:38:58  sezero
 * fix silly copy+paste error in snd_alsa.c
 *
 * Revision 1.1  2005/02/04 11:59:17  sezero
 * add ALSA sound driver (from the quakeforge project)
 *
 */
