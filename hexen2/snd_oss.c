/*
	snd_oss.c
	$Id: snd_oss.c,v 1.35 2007-12-22 12:28:38 sezero Exp $

	Copyright (C) 1996-1997  Id Software, Inc.

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

#if HAVE_OSS_SOUND

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/wait.h>
// FIXME: <sys/soundcard.h> is "by the book" but we should take care of
// <soundcard.h>, <linux/soundcard.h> and <machine/soundcard.h> someday.
#include <sys/soundcard.h>
#include <errno.h>

#if defined(AFMT_S16_NE)
#define	FORMAT_S16	AFMT_S16_NE
#elif (BYTE_ORDER == BIG_ENDIAN)
#define	FORMAT_S16	AFMT_S16_BE
#else	/* LITTLE_ENDIAN */
#define	FORMAT_S16	AFMT_S16_LE
#endif

/* all of these functions must be properly
   assigned in LinkFuncs() below	*/
static qboolean S_OSS_Init (dma_t *dma);
static int S_OSS_GetDMAPos (void);
static void S_OSS_Shutdown (void);
static void S_OSS_LockBuffer (void);
static void S_OSS_Submit (void);
static const char *S_OSS_DrvName (void);

static char s_oss_driver[] = "OSS";

static int audio_fd = -1;
static const char oss_default[] = "/dev/dsp";
static const char *ossdev = oss_default;
static unsigned long mmaplen;


void S_OSS_LinkFuncs (snd_driver_t *p)
{
	p->Init		= S_OSS_Init;
	p->Shutdown	= S_OSS_Shutdown;
	p->GetDMAPos	= S_OSS_GetDMAPos;
	p->LockBuffer	= S_OSS_LockBuffer;
	p->Submit	= S_OSS_Submit;
	p->DrvName	= S_OSS_DrvName;
}


static qboolean S_OSS_Init (dma_t *dma)
{
	int		i, caps, tmp;
	unsigned long		sz;
	struct audio_buf_info	info;

	tmp = COM_CheckParm("-ossdev");
	if (tmp != 0 && tmp < com_argc - 1)
		ossdev = com_argv[tmp + 1];
	Con_Printf ("OSS: Using device: %s\n", ossdev);

// open /dev/dsp, confirm capability to mmap, and get size of dma buffer
	audio_fd = open(ossdev, O_RDWR|O_NONBLOCK);
	if (audio_fd < 0)
	{	// Failed open, retry up to 3 times if it's busy
		tmp = 3;
		while ( (audio_fd < 0) && tmp-- &&
			((errno == EAGAIN) || (errno == EBUSY)) )
		{
			sleep (1);
			audio_fd = open(ossdev, O_RDWR|O_NONBLOCK);
		}
		if (audio_fd < 0)
		{
			Con_Printf("Could not open %s. %s\n", ossdev, strerror(errno));
			return false;
		}
	}

	memset ((void *) dma, 0, sizeof(dma_t));
	shm = dma;

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

// set sample bits & speed
	i = desired_bits;
	tmp = (desired_bits == 16) ? FORMAT_S16 : AFMT_U8;
	if (ioctl(audio_fd, SNDCTL_DSP_SETFMT, &tmp) == -1)
	{
		Con_Printf("Problems setting %d bit format, trying alternatives..\n", i);
		// try what the device gives us
		if (ioctl(audio_fd, SNDCTL_DSP_GETFMTS, &tmp) == -1)
		{
			Con_Printf("Unable to retrieve supported formats. %s\n", strerror(errno));
			goto error;
		}
		if (tmp & FORMAT_S16)
		{
			i = 16;
			tmp = FORMAT_S16;
		}
		else if (tmp & AFMT_U8)
		{
			i = 8;
			tmp = AFMT_U8;
		}
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
	shm->samplebits = i;

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
				//	goto error;
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
		//	goto error;
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

// memory map the dma buffer
	sz = sysconf (_SC_PAGESIZE);
	mmaplen = info.fragstotal * info.fragsize;
	mmaplen = (mmaplen + sz - 1) & ~(sz - 1);
	shm->buffer = (unsigned char *) mmap(NULL, mmaplen, PROT_READ|PROT_WRITE,
					     MAP_FILE|MAP_SHARED, audio_fd, 0);
	if (!shm->buffer || shm->buffer == MAP_FAILED)
	{
		Con_Printf("Could not mmap %s. %s\n", ossdev, strerror(errno));
		goto error;
	}
	Con_Printf ("OSS: mmaped %lu bytes buffer\n", mmaplen);

// toggle the trigger & start her up
	tmp = 0;
	if (ioctl(audio_fd, SNDCTL_DSP_SETTRIGGER, &tmp) == -1)
	{
		Con_Printf("Could not toggle %s. %s\n", ossdev, strerror(errno));
		munmap (shm->buffer, mmaplen);
		goto error;
	}
	tmp = PCM_ENABLE_OUTPUT;
	if (ioctl(audio_fd, SNDCTL_DSP_SETTRIGGER, &tmp) == -1)
	{
		Con_Printf("Could not toggle %s. %s\n", ossdev, strerror(errno));
		munmap (shm->buffer, mmaplen);
		goto error;
	}

	shm->samplepos = 0;

	return true;

error:
	close(audio_fd);
	audio_fd = -1;
	shm->buffer = NULL;
	shm = NULL;
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
//	shm->samplepos = (count.bytes / (shm->samplebits / 8)) & (shm->samples-1);
//	fprintf(stderr, "%d    \r", count.ptr);
	shm->samplepos = count.ptr / (shm->samplebits / 8);

	return shm->samplepos;
}

static void S_OSS_Shutdown (void)
{
	int	tmp = 0;
	if (shm)
	{
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

static const char *S_OSS_DrvName (void)
{
	return s_oss_driver;
}

#endif	/* HAVE_OSS_SOUND */

