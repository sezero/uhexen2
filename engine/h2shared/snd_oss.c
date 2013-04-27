/*
 * snd_oss.c
 * $Id$
 *
 * Copyright (C) 1996-1997  Id Software, Inc.
 * Copyright (C) 2005-2012  O.Sezer <sezero@users.sourceforge.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "quakedef.h"
#include "snd_sys.h"

#if HAVE_OSS_SOUND

#include "snd_oss.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/wait.h>
/* FIXME:  <sys/soundcard.h> is by the book, but we might
 * have to take care of <soundcard.h>, <linux/soundcard.h>
 * and <machine/soundcard.h> someday.  */
#include <sys/soundcard.h>
#include <errno.h>

#if ENDIAN_RUNTIME_DETECT
static int		FORMAT_S16;
#elif defined(AFMT_S16_NE)
#define	FORMAT_S16	AFMT_S16_NE
#elif (BYTE_ORDER == BIG_ENDIAN)
#define	FORMAT_S16	AFMT_S16_BE
#elif (BYTE_ORDER == LITTLE_ENDIAN)
#define	FORMAT_S16	AFMT_S16_LE
#else
#error "Unsupported endianness."
#endif	/* BYTE_ORDER */

static char s_oss_driver[] = "OSS";

static int audio_fd = -1;
static const char oss_default[] = "/dev/dsp";
static const char *ossdev = oss_default;
static unsigned long mmaplen;


static qboolean S_OSS_Init (dma_t *dma)
{
	int		i, caps, tmp;
	unsigned long		sz;
	struct audio_buf_info	info;

#if ENDIAN_RUNTIME_DETECT
	switch (host_byteorder)
	{
	case BIG_ENDIAN:
		FORMAT_S16 = AFMT_S16_BE;
		break;
	case LITTLE_ENDIAN:
		FORMAT_S16 = AFMT_S16_LE;
		break;
	default:
		Sys_Error("%s: Unsupported byte order.", __thisfunc__);
		break;
	}
#endif	/* ENDIAN_RUNTIME_DETECT */

	tmp = COM_CheckParm("-ossdev");
	if (tmp != 0 && tmp < com_argc - 1)
		ossdev = com_argv[tmp + 1];
	Con_Printf ("OSS: Using device: %s\n", ossdev);

	audio_fd = open(ossdev, O_RDWR|O_NONBLOCK);
	if (audio_fd == -1)
	{	/* retry up to 3 times if it's busy */
		tmp = 3;
		while (audio_fd == -1 && tmp-- &&
				(errno == EAGAIN || errno == EBUSY))
		{
			usleep (300000);
			audio_fd = open(ossdev, O_RDWR|O_NONBLOCK);
		}
		if (audio_fd == -1)
		{
			Con_Printf("Could not open %s. %s\n", ossdev, strerror(errno));
			return false;
		}
	}

	if (ioctl(audio_fd, SNDCTL_DSP_RESET, 0) == -1)
	{
		Con_Printf("Could not reset %s. %s\n", ossdev, strerror(errno));
		goto error;
	}

	if (ioctl(audio_fd, SNDCTL_DSP_GETCAPS, &caps) == -1)
	{
		Con_Printf("Couldn't retrieve soundcard capabilities. %s\n", strerror(errno));
		goto error;
	}

	if (!(caps & DSP_CAP_TRIGGER) || !(caps & DSP_CAP_MMAP))
	{
		Con_Printf("Audio driver doesn't support mmap or trigger\n");
		goto error;
	}

	memset ((void *) dma, 0, sizeof(dma_t));
	shm = dma;

	/* set format & rate */
	tmp = (desired_bits == 16) ? FORMAT_S16 : AFMT_U8;
	if (ioctl(audio_fd, SNDCTL_DSP_SETFMT, &tmp) == -1)
	{
		Con_Printf("Problems setting %d bit format, trying alternatives..\n", desired_bits);
		/* try what the device gives us */
		if (ioctl(audio_fd, SNDCTL_DSP_GETFMTS, &tmp) == -1)
		{
			Con_Printf("Unable to retrieve supported formats. %s\n", strerror(errno));
			goto error;
		}
		i = tmp;
		if (i & FORMAT_S16)
			tmp = FORMAT_S16;
		else if (i & AFMT_U8)
			tmp = AFMT_U8;
		else if (i & AFMT_S8)
			tmp = AFMT_S8;
		else
		{
			Con_Printf("Neither 8 nor 16 bit format supported.\n");
			goto error;
		}
		if (ioctl(audio_fd, SNDCTL_DSP_SETFMT, &tmp) == -1)
		{
			Con_Printf("Unable to set sound format. %s\n", strerror(errno));
			goto error;
		}
	}
	if (tmp == FORMAT_S16)
		shm->samplebits = 16;
	else if (tmp == AFMT_U8)
		shm->samplebits = 8;
	else if (tmp == AFMT_S8)
	{
		shm->signed8 = 1;
		shm->samplebits = 8;
	}
	else /* unreached */
	{
		goto error;
	}

	tmp = desired_speed;
	if (ioctl(audio_fd, SNDCTL_DSP_SPEED, &tmp) == -1)
	{
		Con_Printf("Problems setting sample rate, trying alternatives..\n");
		shm->speed = 0;
		for (i = 0; i < MAX_TRYRATES; i++)
		{
			tmp = tryrates[i];
			if (ioctl(audio_fd, SNDCTL_DSP_SPEED, &tmp) == -1)
			{
				Con_DPrintf ("Unable to set sample rate %d\n", tryrates[i]);
			}
			else
			{
				if (tmp != tryrates[i])
				{
					Con_Printf ("Warning: Rate set (%d) didn't match requested rate (%d)!\n", tmp, tryrates[i]);
				/*	goto error;*/
				}
				shm->speed = tmp;
				break;
			}
		}
		if (shm->speed == 0)
		{
			Con_Printf("Unable to set any sample rates.\n");
			goto error;
		}
	}
	else
	{
		if (tmp != desired_speed)
		{
			Con_Printf ("Warning: Rate set (%d) didn't match requested rate (%d)!\n", tmp, desired_speed);
		/*	goto error;*/
		}
		shm->speed = tmp;
	}

	tmp = (desired_channels == 2) ? 1 : 0;
	if (ioctl(audio_fd, SNDCTL_DSP_STEREO, &tmp) == -1)
	{
		Con_Printf ("Problems setting channels to %s, retrying for %s\n",
				(desired_channels == 2) ? "stereo" : "mono",
				(desired_channels == 2) ? "mono" : "stereo");
		tmp = (desired_channels == 2) ? 0 : 1;
		if (ioctl(audio_fd, SNDCTL_DSP_STEREO, &tmp) == -1)
		{
			Con_Printf("unable to set desired channels. %s\n", strerror(errno));
			goto error;
		}
	}
	shm->channels = tmp +1;

	if (ioctl(audio_fd, SNDCTL_DSP_GETOSPACE, &info) == -1)
	{
		Con_Printf("Couldn't retrieve buffer status. %s\n", strerror(errno));
		goto error;
	}

	shm->samples = info.fragstotal * info.fragsize / (shm->samplebits / 8);
	shm->submission_chunk = 1;

	/* memory map the dma buffer */
	sz = sysconf (_SC_PAGESIZE);
	mmaplen = info.fragstotal * info.fragsize;
	mmaplen += sz - 1;
	mmaplen &= ~(sz - 1);
	shm->buffer = (unsigned char *) mmap(NULL, mmaplen, PROT_READ|PROT_WRITE,
					     MAP_FILE|MAP_SHARED, audio_fd, 0);
	if (shm->buffer == MAP_FAILED)
	{
		Con_Printf("Could not mmap %s. %s\n", ossdev, strerror(errno));
		goto error;
	}
	Con_Printf ("OSS: mmaped %lu bytes buffer\n", mmaplen);

	/* toggle the trigger & start her up */
	tmp = 0;
	if (ioctl(audio_fd, SNDCTL_DSP_SETTRIGGER, &tmp) == -1)
	{
		Con_Printf("Could not toggle %s. %s\n", ossdev, strerror(errno));
		goto error;
	}
	tmp = PCM_ENABLE_OUTPUT;
	if (ioctl(audio_fd, SNDCTL_DSP_SETTRIGGER, &tmp) == -1)
	{
		Con_Printf("Could not toggle %s. %s\n", ossdev, strerror(errno));
		goto error;
	}

	shm->samplepos = 0;

	return true;

error:
	if (shm->buffer && shm->buffer != MAP_FAILED)
		munmap (shm->buffer, mmaplen);
	shm->buffer = NULL;
	shm = NULL;
	close(audio_fd);
	audio_fd = -1;
	return false;
}

static int S_OSS_GetDMAPos (void)
{
	struct count_info	count;

	if (!shm)
		return 0;

	if (ioctl(audio_fd, SNDCTL_DSP_GETOPTR, &count) == -1)
	{
		Con_Printf("Uh, sound dead. %s\n", strerror(errno));
		munmap (shm->buffer, mmaplen);
		shm->buffer = NULL;
		shm = NULL;
		close(audio_fd);
		audio_fd = -1;
		return 0;
	}
	shm->samplepos = count.ptr / (shm->samplebits / 8);

	return shm->samplepos;
}

static void S_OSS_Shutdown (void)
{
	if (shm)
	{
		int	tmp = 0;
		Con_Printf ("Shutting down OSS sound\n");
		munmap (shm->buffer, mmaplen);
		shm->buffer = NULL;
		shm = NULL;
		ioctl(audio_fd, SNDCTL_DSP_SETTRIGGER, &tmp);
		ioctl(audio_fd, SNDCTL_DSP_RESET, 0);
		close(audio_fd);
		audio_fd = -1;
	}
}

/*
==============
SNDDMA_LockBuffer

Makes sure dma buffer is valid
==============
*/
static void S_OSS_LockBuffer (void)
{
	/* nothing to do here */
}

/*
==============
SNDDMA_Submit

Unlock the dma buffer /
Send sound to the device
===============
*/
static void S_OSS_Submit(void)
{
}

static void S_OSS_BlockSound (void)
{
}

static void S_OSS_UnblockSound (void)
{
}

snd_driver_t snddrv_oss =
{
	S_OSS_Init,
	S_OSS_Shutdown,
	S_OSS_GetDMAPos,
	S_OSS_LockBuffer,
	S_OSS_Submit,
	S_OSS_BlockSound,
	S_OSS_UnblockSound,
	s_oss_driver,
	SNDDRV_ID_OSS,
	false,
	NULL
};

#endif	/* HAVE_OSS_SOUND */

