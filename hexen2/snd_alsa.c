/*
	snd_alsa.c
	$Id: snd_alsa.c,v 1.25 2007-01-15 12:01:09 sezero Exp $

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


#define _SND_SYS_MACROS_ONLY

#include "quakedef.h"
#include "snd_sys.h"

#if HAVE_ALSA_SOUND

#include <dlfcn.h>
#include <alsa/asoundlib.h>

static void *alsa_handle = NULL;
//static char *pcmname = "hw:0,0";
static char *pcmname = "default";
static snd_pcm_t *pcm = NULL;
static snd_pcm_uframes_t buffer_size;
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
	snd_pcm_hw_params_t	*hw = NULL;
	snd_pcm_sw_params_t	*sw = NULL;
	snd_pcm_uframes_t	frag_size;

	if (!load_libasound ())
		return false;

	err = COM_CheckParm("-alsadev");
	if (err != 0 && err < com_argc-1)
		pcmname = com_argv[err+1];

	err = hx2snd_pcm_open (&pcm, pcmname, SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK);
	if (err < 0)
	{
		Con_Printf ("ALSA: audio open error: %s\n", hx2snd_strerror (err));
		return 0;
	}
	Con_Printf ("ALSA: Using device: %s\n", pcmname);

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
		Con_Printf ("Problems setting %d bit format, trying alternatives..\n", desired_bits);
		tmp_bits = (desired_bits == 8) ? SND_PCM_FORMAT_S16 : SND_PCM_FORMAT_U8;
		err = hx2snd_pcm_hw_params_set_format (pcm, hw, tmp_bits);
		ALSA_CHECK_ERR(err, "Neither 8 nor 16 bit format supported. %s\n", hx2snd_strerror (err));
	}
	tmp_bits = (tmp_bits == SND_PCM_FORMAT_U8) ? 8 : 16;

	tmp_chan = desired_channels;
	err = hx2snd_pcm_hw_params_set_channels (pcm, hw, tmp_chan);
	if (err < 0)
	{
		Con_Printf ("Problems setting channels to %s, retrying for %s\n",
				(desired_channels == 2) ? "stereo" : "mono",
				(desired_channels == 2) ? "mono" : "stereo");
		tmp_chan = (desired_channels == 2) ? 1 : 2;
		err = hx2snd_pcm_hw_params_set_channels (pcm, hw, tmp_chan);
		ALSA_CHECK_ERR(err, "unable to set desired channels. %s\n", hx2snd_strerror (err));
	}

	rate = desired_speed;
	err = hx2snd_pcm_hw_params_set_rate_near (pcm, hw, &rate, 0);
	if (err < 0)
	{
		Con_Printf("Problems setting sample rate, trying alternatives..\n");
		for (i = 0; i < MAX_TRYRATES; i++)
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
			Con_Printf ("Unable to set any sample rates.\n");
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

	memset ((dma_t *) &sn, 0, sizeof(sn));
	shm = &sn;

	shm->splitbuffer = 0;
	shm->channels = tmp_chan;

	// don't mix less than this in mono samples:
/*	err = hx2snd_pcm_hw_params_get_period_size (hw, 
			(snd_pcm_uframes_t *) (char *) (&shm->submission_chunk), 0);
	ALSA_CHECK_ERR(err, "unable to get period size. %s\n", hx2snd_strerror (err));
*/
	shm->submission_chunk = 1;
	shm->samplepos = 0;
	shm->samplebits = tmp_bits;
	err = hx2snd_pcm_hw_params_get_buffer_size (hw, &buffer_size);
	ALSA_CHECK_ERR(err, "unable to get buffer size. %s\n", hx2snd_strerror(err));

	Con_Printf ("ALSA: %lu bytes buffer with mmap interleaved access\n", (unsigned long)buffer_size);

	if (buffer_size != round_buffer_size (buffer_size))
	{
		Sys_Printf ("ALSA: WARNING: non-power of 2 buffer size. sound may be\n");
		Sys_Printf ("unsatisfactory. Recommend using either the plughw or hw\n");
		Sys_Printf ("devices or adjusting dmix to have a power of 2 buf size\n");
	}

	shm->samples = buffer_size * shm->channels; // mono samples in buffer
	shm->speed = rate;

	S_ALSA_GetDMAPos ();	// sets shm->buffer

	hx2snd_pcm_hw_params_free(hw);
	hx2snd_pcm_sw_params_free(sw);

	return 1;

error:
// full clean-up
	if (hw)
		hx2snd_pcm_hw_params_free(hw);
	if (sw)
		hx2snd_pcm_sw_params_free(sw);
	shm = NULL;
	hx2snd_pcm_close (pcm);
	pcm = NULL;
	dlclose (alsa_handle);
	alsa_handle = NULL;
	return 0;
}

int S_ALSA_GetDMAPos (void)
{
	snd_pcm_uframes_t offset;
	snd_pcm_uframes_t nframes = shm->samples/shm->channels;
	const snd_pcm_channel_area_t *areas;

	if (!shm)
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
	if (shm)
	{
	// full clean-up
		Con_Printf ("Shutting down ALSA sound\n");
		hx2snd_pcm_drop (pcm);	// do I need this?
		hx2snd_pcm_close (pcm);
		pcm = NULL;
		shm->buffer = NULL;
		shm = NULL;
		dlclose (alsa_handle);
		alsa_handle = NULL;
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

#endif	// HAVE_ALSA_SOUND

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.24  2006/10/21 18:21:28  sezero
 * various coding style clean-ups, part 5.
 *
 * Revision 1.23  2006/09/29 20:38:44  sezero
 * even more sound stuff (alsa clean-up)
 *
 * Revision 1.22  2006/09/29 18:00:35  sezero
 * even more sound stuff
 *
 * Revision 1.21  2006/09/29 11:17:51  sezero
 * more sound clean up
 *
 * Revision 1.20  2006/09/27 17:17:30  sezero
 * a lot of clean-ups in sound and midi files.
 *
 * Revision 1.19  2006/09/23 07:25:35  sezero
 * added missing com_argc checks (and fixed the incorrect ones)
 * after several COM_CheckParm calls.
 *
 * Revision 1.18  2006/09/15 09:18:40  sezero
 * fixed another gcc4 warning about type-punning (although in disabled code)
 *
 * Revision 1.17  2006/06/15 09:31:53  sezero
 * kept the same standart for alsa sound availablity in snd_sys
 *
 * Revision 1.16  2006/05/20 12:38:01  sezero
 * cleaned up sound tryrates, etc. changed tryrates array to include
 * 48000, 24000, and 16000 speeds (this should help 48khz AC97 chips,
 * from darkplaces).
 *
 * Revision 1.15  2006/02/19 12:33:24  sezero
 * continue making static functions and vars static. whitespace and coding style
 * cleanup. (part 10: sound).
 *
 * Revision 1.14  2006/01/12 13:10:49  sezero
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
 * Revision 1.11  2005/07/05 17:11:38  sezero
 * Updated alsa driver (various insignificant things)
 *
 * Revision 1.10  2005/06/12 07:31:18  sezero
 * enabled alsa only on linux platforms
 *
 * Revision 1.9  2005/06/12 07:28:51  sezero
 * clean-up of includes and a fix (hopefully) for endianness detection
 *
 * Revision 1.8  2005/03/05 14:33:32  sezero
 * Try to use what frequency is actually set:
 * I first naively thought that requested-provided frequency
 * mismatches wouldn't happen, but I got one weird oss report
 * which includes this case. Let's see what this does now...
 *
 * Revision 1.7  2005/02/20 12:44:58  sezero
 * - Process all command line options in snd_dma.c, S_Startup() only.
 *   Targets will do to its bidding first. And don't die immediately,
 *   try setting alternative hw parameters. (FWIW, snd_oss.c now applies
 *   all hardware settings before mmaping the buffer)
 * - Check for requested and set rate mismatches and fail (Found in alsa
 *   examples, is it necessary at all? Commented out for now.)
 *
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
