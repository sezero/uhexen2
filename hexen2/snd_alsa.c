/*
	snd_alsa.c
	$Id: snd_alsa.c,v 1.44 2010-11-13 07:42:07 sezero Exp $

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


#include "quakedef.h"
#include "snd_sys.h"

#if HAVE_ALSA_SOUND

#include <dlfcn.h>
#include <alsa/asoundlib.h>

/* all of these functions must be properly
   assigned in LinkFuncs() below	*/
static qboolean S_ALSA_Init (dma_t *dma);
static int S_ALSA_GetDMAPos (void);
static void S_ALSA_Shutdown (void);
static void S_ALSA_LockBuffer (void);
static void S_ALSA_Submit (void);
static void S_ALSA_BlockSound (void);
static void S_ALSA_UnblockSound (void);
static const char *S_ALSA_DrvName (void);

static char s_alsa_driver[] = "ALSA";

static void *alsa_handle = NULL;
//static const char alsa_default[] = "hw:0,0";
static const char alsa_default[] = "default";
static const char *pcmname = alsa_default;
static snd_pcm_t *pcm = NULL;
static snd_pcm_uframes_t buffer_size;

#define ALSA_FUNC(ret, func, params)	\
static ret (*hx2##func) params;
#include "alsa_funcs.h"
#undef ALSA_FUNC


void S_ALSA_LinkFuncs (snd_driver_t *p)
{
	p->Init		= S_ALSA_Init;
	p->Shutdown	= S_ALSA_Shutdown;
	p->GetDMAPos	= S_ALSA_GetDMAPos;
	p->LockBuffer	= S_ALSA_LockBuffer;
	p->Submit	= S_ALSA_Submit;
	p->BlockSound	= S_ALSA_BlockSound;
	p->UnblockSound	= S_ALSA_UnblockSound;
	p->DrvName	= S_ALSA_DrvName;
}


static qboolean load_libasound (void)
{
	alsa_handle = (void *) dlopen ("libasound.so.2", RTLD_GLOBAL | RTLD_NOW);
	if (!alsa_handle)
	{
		Con_Printf ("Couldn't load libasound.so.2: %s\n", dlerror());
		return false;
	}

#define ALSA_FUNC(ret, func, params)					\
    do {								\
	hx2##func = (ret (*) params) dlsym (alsa_handle, #func);	\
	if (hx2##func == NULL)						\
	{								\
		Con_Printf ("Couldn't load ALSA function %s\n", #func);	\
		dlclose (alsa_handle);					\
		alsa_handle = NULL;					\
		return false;						\
	}								\
    } while (0);

#include "alsa_funcs.h"
#undef ALSA_FUNC

	return true;
}

#if defined(__GNUC__) &&  \
  !(defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L)
#define ALSA_CHECK_ERR(check, fmt, args...)		\
    do {						\
	if (check < 0) {				\
		Con_Printf ("ALSA: " fmt, ##args);	\
		goto error;				\
	}						\
    } while (0)
#else
#define ALSA_CHECK_ERR(check, ...)			\
    do {						\
	if (check < 0) {				\
		Con_Printf ("ALSA: " __VA_ARGS__);	\
		goto error;				\
	}						\
    } while (0)
#endif

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

static qboolean S_ALSA_Init (dma_t *dma)
{
	int			i, err;
	unsigned int		rate;
	int			tmp_bits, tmp_chan;
	snd_pcm_hw_params_t	*hw = NULL;
	snd_pcm_sw_params_t	*sw = NULL;
	snd_pcm_uframes_t	frag_size;

	if (!load_libasound())
		return false;

	i = COM_CheckParm("-alsadev");
	if (i != 0 && i < com_argc - 1)
		pcmname = com_argv[i + 1];

	err = hx2snd_pcm_open (&pcm, pcmname, SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK);
	if (err < 0)
	{
		Con_Printf ("ALSA: error opening device \"%s\": %s\n", pcmname, hx2snd_strerror(err));
		return false;
	}
	Con_Printf ("ALSA: Using device: %s\n", pcmname);

	err = hx2snd_pcm_hw_params_malloc (&hw);
	ALSA_CHECK_ERR(err, "unable to allocate hardware params. %s\n", hx2snd_strerror(err));

	err = hx2snd_pcm_hw_params_any (pcm, hw);
	ALSA_CHECK_ERR(err, "unable to init hardware params. %s\n", hx2snd_strerror(err));

	err = hx2snd_pcm_hw_params_set_access (pcm, hw, SND_PCM_ACCESS_MMAP_INTERLEAVED);
	ALSA_CHECK_ERR(err, "unable to set interleaved access. %s\n", hx2snd_strerror(err));

	tmp_bits = (desired_bits == 8) ? SND_PCM_FORMAT_U8 : SND_PCM_FORMAT_S16;
	err = hx2snd_pcm_hw_params_set_format (pcm, hw, (snd_pcm_format_t) tmp_bits);
	if (err < 0)
	{
		Con_Printf ("Problems setting %d bit format, trying alternatives..\n", desired_bits);
		tmp_bits = (desired_bits == 8) ? SND_PCM_FORMAT_S16 : SND_PCM_FORMAT_U8;
		err = hx2snd_pcm_hw_params_set_format (pcm, hw, (snd_pcm_format_t) tmp_bits);
		ALSA_CHECK_ERR(err, "Neither 8 nor 16 bit format supported. %s\n", hx2snd_strerror(err));
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
		ALSA_CHECK_ERR(err, "unable to set desired channels. %s\n", hx2snd_strerror(err));
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
					Con_Printf ("Warning: Rate set (%u) didn't match requested rate (%d)!\n", rate, tryrates[i]);
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
			Con_Printf ("Warning: Rate set (%u) didn't match requested rate (%d)!\n", rate, desired_speed);
		//	goto error;
		}
	}

	frag_size = 8 * tmp_bits * rate / 11025;

	err = hx2snd_pcm_hw_params_set_period_size_near (pcm, hw, &frag_size, 0);
	ALSA_CHECK_ERR(err, "unable to set period size near %i. %s\n",
				(int)frag_size, hx2snd_strerror(err));

	err = hx2snd_pcm_hw_params (pcm, hw);
	ALSA_CHECK_ERR(err, "unable to install hardware params. %s\n", hx2snd_strerror(err));

	err = hx2snd_pcm_sw_params_malloc (&sw);
	ALSA_CHECK_ERR(err, "unable to allocate software params. %s\n", hx2snd_strerror(err));

	err = hx2snd_pcm_sw_params_current (pcm, sw);
	ALSA_CHECK_ERR(err, "unable to determine current software params. %s\n", hx2snd_strerror(err));

	err = hx2snd_pcm_sw_params_set_start_threshold (pcm, sw, ~0U);
	ALSA_CHECK_ERR(err, "unable to set playback threshold. %s\n", hx2snd_strerror(err));

	err = hx2snd_pcm_sw_params_set_stop_threshold (pcm, sw, ~0U);
	ALSA_CHECK_ERR(err, "unable to set playback stop threshold. %s\n", hx2snd_strerror(err));

	err = hx2snd_pcm_sw_params (pcm, sw);
	ALSA_CHECK_ERR(err, "unable to install software params. %s\n", hx2snd_strerror(err));

	memset ((void *) dma, 0, sizeof(dma_t));
	shm = dma;

	shm->channels = tmp_chan;

	// don't mix less than this in mono samples:
/*	err = hx2snd_pcm_hw_params_get_period_size (hw, 
			(snd_pcm_uframes_t *) (char *) (&shm->submission_chunk), 0);
	ALSA_CHECK_ERR(err, "unable to get period size. %s\n", hx2snd_strerror(err));
*/
	shm->submission_chunk = 1;
	shm->samplepos = 0;
	shm->samplebits = tmp_bits;
	err = hx2snd_pcm_hw_params_get_buffer_size (hw, &buffer_size);
	ALSA_CHECK_ERR(err, "unable to get buffer size. %s\n", hx2snd_strerror(err));

	Con_Printf ("ALSA: %lu bytes buffer with mmap interleaved access\n", (unsigned long)buffer_size);

	if (buffer_size != round_buffer_size (buffer_size))
	{
		Con_Printf ("ALSA: WARNING: non-power of 2 buffer size. sound may be\n");
		Con_Printf ("unsatisfactory. Recommend using either the plughw or hw\n");
		Con_Printf ("devices or adjusting dmix to have a power of 2 buf size\n");
	}

	shm->samples = buffer_size * shm->channels; // mono samples in buffer
	shm->speed = rate;

	S_ALSA_GetDMAPos ();	// sets shm->buffer

	hx2snd_pcm_hw_params_free(hw);
	hx2snd_pcm_sw_params_free(sw);

	return true;

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
	return false;
}

static int S_ALSA_GetDMAPos (void)
{
	snd_pcm_uframes_t offset;
	snd_pcm_uframes_t nframes;
	const snd_pcm_channel_area_t *areas;

	if (!shm)
		return 0;

	nframes = shm->samples/shm->channels;
	hx2snd_pcm_avail_update (pcm);
	hx2snd_pcm_mmap_begin (pcm, &areas, &offset, &nframes);
	// The following commit was absent in QF, causing the
	// very first sound to be corrupted
	hx2snd_pcm_mmap_commit (pcm, offset, nframes);
	offset *= shm->channels;
	nframes *= shm->channels;
	shm->samplepos = offset;
	shm->buffer = (unsigned char *) areas->addr;	// FIXME! there's an area per channel
	return shm->samplepos;
}

static void S_ALSA_Shutdown (void)
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
==============
SNDDMA_LockBuffer

Makes sure dma buffer is valid
==============
*/
static void S_ALSA_LockBuffer (void)
{
	/* nothing to do here */
}

/*
==============
SNDDMA_Submit

Unlock the dma buffer /
Send sound to the device
==============
*/
static void S_ALSA_Submit (void)
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

static void S_ALSA_BlockSound (void)
{
	hx2snd_pcm_pause (pcm, 1);
}

static void S_ALSA_UnblockSound (void)
{
	hx2snd_pcm_pause (pcm, 0);
}

static const char *S_ALSA_DrvName (void)
{
	return s_alsa_driver;
}

#endif	/* HAVE_ALSA_SOUND */

