/*
	snd_oss.c
	$Id: snd_oss.c,v 1.5 2005-02-14 10:06:23 sezero Exp $

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

#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <linux/soundcard.h>
#include <stdio.h>
#include <errno.h>
#include "quakedef.h"

int audio_fd = -1;
int snd_inited;
// unsigned long mmaplen;

static int tryrates[] = { 11025, 22051, 44100, 8000 };

qboolean S_OSS_Init(void)
{

	int i, caps, fmt, rc, tmp;
	int retries = 3;
	// unsigned long sz;
	unsigned long sz, mmaplen;
	struct audio_buf_info info;

	snd_inited = 0;

// open /dev/dsp, confirm capability to mmap, and get size of dma buffer

	audio_fd = open("/dev/dsp", O_RDWR|O_NONBLOCK);
	if (audio_fd < 0) {	// Failed open, retry up to 3 times
		// if it's busy
		while ((audio_fd < 0) && retries-- &&
			((errno == EAGAIN) || (errno == EBUSY))) {
			sleep (1);
			audio_fd = open("/dev/dsp", O_RDWR|O_NONBLOCK);
		}
		if (audio_fd < 0) {
			perror("/dev/dsp");
			Con_Printf("Could not open /dev/dsp\n");
			return 0;
		}
	}

	rc = ioctl(audio_fd, SNDCTL_DSP_RESET, 0);
	if (rc < 0)
	{
		perror("/dev/dsp");
		Con_Printf("Could not reset /dev/dsp\n");
		close(audio_fd);
		return 0;
	}

	if (ioctl(audio_fd, SNDCTL_DSP_GETCAPS, &caps)==-1)
	{
		perror("/dev/dsp");
		Con_Printf("Sound driver too old\n");
		close(audio_fd);
		return 0;
	}

	if (!(caps & DSP_CAP_TRIGGER) || !(caps & DSP_CAP_MMAP))
	{
		Con_Printf("Sorry but your soundcard can't do this\n");
		close(audio_fd);
		return 0;
	}

	if (ioctl(audio_fd, SNDCTL_DSP_GETOSPACE, &info)==-1)
	{   
		perror("GETOSPACE");
		Con_Printf("Um, can't do GETOSPACE?\n");
		close(audio_fd);
		return 0;
	}

	shm = &sn;
	shm->splitbuffer = 0;

// set sample bits & speed

	if ((i = COM_CheckParm("-sndbits")) != 0)
		shm->samplebits = atoi(com_argv[i+1]);
	if (shm->samplebits != 16 && shm->samplebits != 8)
	{
		ioctl(audio_fd, SNDCTL_DSP_GETFMTS, &fmt);
		if (fmt & AFMT_S16_LE)
			shm->samplebits = 16;
		else if (fmt & AFMT_U8)
			shm->samplebits = 8;
	}

	if ((i = COM_CheckParm("-sndspeed")) != 0)
		shm->speed = atoi(com_argv[i+1]);
	else
	{
		for (i=0 ; i<sizeof(tryrates)/4 ; i++)
			if (!ioctl(audio_fd, SNDCTL_DSP_SPEED, &tryrates[i]))
			    break;
		shm->speed = tryrates[i];
	}

	if ((i = COM_CheckParm("-sndmono")) != 0)
		shm->channels = 1;
	else
		shm->channels = 2;

	shm->samples = info.fragstotal * info.fragsize / (shm->samplebits/8);
	shm->submission_chunk = 1;

// memory map the dma buffer
	sz = sysconf (_SC_PAGESIZE);
	mmaplen = info.fragstotal * info.fragsize;
	mmaplen = (mmaplen + sz - 1) & ~(sz - 1);
	shm->buffer = (unsigned char *) mmap(NULL, mmaplen, PROT_WRITE,
					     MAP_FILE|MAP_SHARED, audio_fd, 0);
	if (!shm->buffer || shm->buffer == MAP_FAILED)
	{
		perror("/dev/dsp");
		Con_Printf("Could not mmap /dev/dsp\n");
		close(audio_fd);
		return 0;
	}

	tmp = 0;
	if (shm->channels == 2)
		tmp = 1;
	rc = ioctl(audio_fd, SNDCTL_DSP_STEREO, &tmp);
	if (rc < 0)
	{
		perror("/dev/dsp");
		Con_Printf("Could not set /dev/dsp to stereo=%d", shm->channels);
		close(audio_fd);
		return 0;
	}
	if (tmp)
		shm->channels = 2;
	else
		shm->channels = 1;

	rc = ioctl(audio_fd, SNDCTL_DSP_SPEED, &shm->speed);
	if (rc < 0)
	{
		perror("/dev/dsp");
		Con_Printf("Could not set /dev/dsp speed to %d", shm->speed);
		close(audio_fd);
		return 0;
	}

	if (shm->samplebits == 16)
	{
		rc = AFMT_S16_LE;
		rc = ioctl(audio_fd, SNDCTL_DSP_SETFMT, &rc);
		if (rc < 0)
		{
			perror("/dev/dsp");
			Con_Printf("Could not support 16-bit data.  Try 8-bit.\n");
			close(audio_fd);
			return 0;
		}
	}
	else if (shm->samplebits == 8)
	{
		rc = AFMT_U8;
		rc = ioctl(audio_fd, SNDCTL_DSP_SETFMT, &rc);
		if (rc < 0)
		{
			perror("/dev/dsp");
			Con_Printf("Could not support 8-bit data.\n");
			close(audio_fd);
			return 0;
		}
	}
	else
	{
		perror("/dev/dsp");
		Con_Printf("%d-bit sound not supported.", shm->samplebits);
		close(audio_fd);
		return 0;
	}

// toggle the trigger & start her up

	tmp = 0;
	rc  = ioctl(audio_fd, SNDCTL_DSP_SETTRIGGER, &tmp);
	if (rc < 0)
	{
		perror("/dev/dsp");
		Con_Printf("Could not toggle.\n");
		close(audio_fd);
		return 0;
	}
	tmp = PCM_ENABLE_OUTPUT;
	rc = ioctl(audio_fd, SNDCTL_DSP_SETTRIGGER, &tmp);
	if (rc < 0)
	{
		perror("/dev/dsp");
		Con_Printf("Could not toggle.\n");
		close(audio_fd);
		return 0;
	}

	shm->samplepos = 0;

	snd_inited = 1;
	Con_Printf("Audio Subsystem initialized in OSS mode.\n");
	return 1;
}

int S_OSS_GetDMAPos(void)
{
	struct count_info count;

	if (!snd_inited) return 0;

	if (ioctl(audio_fd, SNDCTL_DSP_GETOPTR, &count)==-1)
	{
		perror("/dev/dsp");
		Con_Printf("Uh, sound dead.\n");
		close(audio_fd);
		snd_inited = 0;
		return 0;
	}
//	shm->samplepos = (count.bytes / (shm->samplebits / 8)) & (shm->samples-1);
//	fprintf(stderr, "%d    \r", count.ptr);
	shm->samplepos = count.ptr / (shm->samplebits / 8);

	return shm->samplepos;
}

void S_OSS_Shutdown(void)
{
	int tmp = 0;
	if (snd_inited)
	{
		Con_Printf ("Shutting down OSS sound\n");
		snd_inited = 0;
		// munmap (shm->buffer, mmaplen);
		ioctl(audio_fd, SNDCTL_DSP_SETTRIGGER, &tmp);
		ioctl(audio_fd, SNDCTL_DSP_RESET, 0);
		close(audio_fd);
		audio_fd = -1;
	}
}

/*
==============
SNDDMA_Submit

Send sound to device if buffer isn't really the dma buffer
===============
*/
void S_OSS_Submit(void)
{
}

