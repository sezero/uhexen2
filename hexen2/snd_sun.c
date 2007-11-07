/*
	snd_sun.c
	$Id: snd_sun.c,v 1.11 2007-11-07 16:54:58 sezero Exp $

	SUN Audio driver for BSD and SunOS

	Copyright (C) 1996-1997  Id Software, Inc.
	Taken from the DarkPlaces project with small adaptations
	to make it work with Hexen II: Hammer of Thyrion

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
		51 Franklin St, Fifth Floor, 
		Boston, MA  02110-1301  USA
*/

#define _SND_SYS_MACROS_ONLY

#include "quakedef.h"
#include "snd_sys.h"

#if HAVE_SUN_SOUND

#include <sys/param.h>
#include <sys/audioio.h>
#ifndef __SOLARIS__
#include <sys/endian.h>
#endif
#include <sys/ioctl.h>

#include <fcntl.h>
#ifndef __SOLARIS__
#include <paths.h>
#endif
#include <unistd.h>

#if defined(__SOLARIS__)

#define	FORMAT_U8	AUDIO_ENCODING_LINEAR8
#define	FORMAT_S16	AUDIO_ENCODING_LINEAR

#else	/* BSD */

#define	FORMAT_U8	AUDIO_ENCODING_LINEAR8
#if BYTE_ORDER == BIG_ENDIAN
#define	FORMAT_S16	AUDIO_ENCODING_SLINEAR_BE
#else
#define	FORMAT_S16	AUDIO_ENCODING_SLINEAR_LE
#endif

#endif

static int	audio_fd = -1;

//#define	SND_BUFF_SIZE	65536
#define	SND_BUFF_SIZE	8192
static unsigned char	dma_buffer [SND_BUFF_SIZE];
//static unsigned char	writebuf [SND_BUFF_SIZE];
static unsigned char	writebuf [1024];
static int	wbufp;

void S_SUN_Shutdown (void);


qboolean S_SUN_Init (void)
{
	const char	*snddev;
	audio_info_t	info;

	// Open the audio device
#ifdef _PATH_SOUND
	snddev = _PATH_SOUND;
#else
#ifndef __SOLARIS__
	snddev = "/dev/sound";
#else
	snddev = "/dev/audio";
#endif
#endif
	audio_fd = open (snddev, O_WRONLY | O_NDELAY | O_NONBLOCK);
	if (audio_fd < 0)
	{
		Con_Printf("Can't open the sound device (%s)\n", snddev);
		return false;
	}

	memset ((void *) &sn, 0, sizeof(sn));
	shm = &sn;

	AUDIO_INITINFO (&info);

	// these desired values are decided in snd_dma.c according
	// to the defaults and the user's command line parameters.
	info.play.sample_rate = desired_speed;
	info.play.channels = desired_channels;
	info.play.precision = desired_bits;
	info.play.encoding = (desired_bits == 8) ? FORMAT_U8 : FORMAT_S16;
	if (ioctl(audio_fd, AUDIO_SETINFO, &info) != 0)
	{
	// TODO: also try other options of sampling
	//	 rate and format upon failure???
		Con_Printf("Couldn't set desired sound output format (%d bit, %s, %d Hz)\n",
				desired_bits, (desired_channels == 2) ? "stereo" : "mono", desired_speed);
		close (audio_fd);
		shm = NULL;
		return false;
	}

	shm->channels = info.play.channels;
	shm->samplebits = info.play.precision;
	shm->speed = info.play.sample_rate;

	if (shm->speed != desired_speed)
		Con_Printf ("Warning: Rate set (%d) didn't match requested rate (%d)!\n", shm->speed, desired_speed);

	shm->samples = sizeof(dma_buffer) / (shm->samplebits / 8);
	shm->submission_chunk = 1;
	shm->samplepos = 0;
	shm->buffer = dma_buffer;

	return true;
}

int S_SUN_GetDMAPos (void)
{
	audio_info_t	info;

	if (!shm)
		return 0;

	if (ioctl(audio_fd, AUDIO_GETINFO, &info) < 0)
	{
		Con_Printf("Error: can't get audio info\n");
		S_SUN_Shutdown ();
		return 0;
	}

	return ((info.play.samples * shm->channels) % shm->samples);
}

void S_SUN_Shutdown (void)
{
	if (shm)
	{
		shm = NULL;
		close (audio_fd);
		audio_fd = -1;
	}
}

/*
==============
SNDDMA_LockBuffer

Makes sure dma buffer is valid
==============
*/
void S_SUN_LockBuffer (void)
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
void S_SUN_Submit (void)
{
	int		bsize;
	int		bytes, b;
	unsigned char	*p;
	int		idx;
	int		stop = paintedtime;

	if (!shm)
		return;

	if (paintedtime < wbufp)
		wbufp = 0;	// reset

	bsize = shm->channels * shm->samplebits / 8;
	bytes = (paintedtime - wbufp) * bsize;

	if (!bytes)
		return;

	if (bytes > sizeof(writebuf))
	{
		bytes = sizeof(writebuf);
		stop = wbufp + bytes / bsize;
	}

	// transfer the sound data from the circular dma_buffer to writebuf
	// TODO: using 2 memcpys instead of this loop should be faster
	p = writebuf;
	idx = (wbufp * bsize) & (sizeof(dma_buffer) - 1);
	for (b = bytes; b; b--)
	{
		*p++ = dma_buffer[idx];
		idx = (idx + 1) & (sizeof(dma_buffer) - 1);
	}

	if (write(audio_fd, writebuf, bytes) < bytes)
		Con_Printf("audio can't keep up!\n");

	wbufp = stop;
}

#endif	/* HAVE_SUN_SOUND */

