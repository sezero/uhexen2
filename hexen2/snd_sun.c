/*
	snd_sun.c
	$Id: snd_sun.c,v 1.1 2006-06-15 09:20:36 sezero Exp $

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

#if defined(HAVE_SUN_SOUND)

#include <sys/param.h>
#include <sys/audioio.h>
#ifndef SUNOS
#include <sys/endian.h>
#endif
#include <sys/ioctl.h>

#include <fcntl.h>
#ifndef SUNOS
#include <paths.h>
#endif
#include <unistd.h>


static int audio_fd = -1;

// TODO: allocate them in SNDDMA_Init, with a size depending on
// the sound format (enough for 0.5 sec of sound for instance)
#define SND_BUFF_SIZE 65536
static unsigned char dma_buffer [SND_BUFF_SIZE];
static unsigned char writebuf [SND_BUFF_SIZE];

void S_SUN_Shutdown (void);


qboolean S_SUN_Init (void)
{
	unsigned int	i;
	const char	*snddev;
	audio_info_t	info;

	// Open the audio device
#ifdef _PATH_SOUND
	snddev = _PATH_SOUND;
#else
#ifndef SUNOS
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

	memset ((void *)&sn, 0, sizeof (sn));
	shm = &sn;

	// Look for an appropriate sound format
	// TODO: we should also test mono/stereo and bits
	// TODO: support "-sndspeed", "-sndbits", "-sndmono" and "-sndstereo"
	shm->channels = 2;
	shm->samplebits = 16;
	for (i = 0; i < MAX_TRYRATES; i++)
	{
		shm->speed = tryrates[i];

		AUDIO_INITINFO (&info);
		info.play.sample_rate = shm->speed;
		info.play.channels = shm->channels;
		info.play.precision = shm->samplebits;
// We only handle sound cards of the same endianess than the CPU
#if BYTE_ORDER == BIG_ENDIAN
		info.play.encoding = AUDIO_ENCODING_SLINEAR_BE;
#else
#ifndef SUNOS
		info.play.encoding = AUDIO_ENCODING_SLINEAR_LE;
#else
		info.play.encoding = AUDIO_ENCODING_LINEAR;
#endif
#endif
		if (ioctl (audio_fd, AUDIO_SETINFO, &info) == 0)
			break;
	}
	if (i == MAX_TRYRATES)
	{
		Con_Printf("Can't select an appropriate sound output format\n");
		close (audio_fd);
		shm = NULL;
		return false;
	}

	// Print some information
	Con_Printf("SUN Audio initialized (%d bit, %s, %d Hz)\n",
				info.play.precision,
				(info.play.channels == 2) ? "stereo" : "mono",
				info.play.sample_rate);

	shm->samples = shm->channels * sizeof (dma_buffer) / (shm->samplebits/8) / shm->channels;
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

	if (ioctl (audio_fd, AUDIO_GETINFO, &info) < 0)
	{
		Con_Printf("Error: can't get audio info\n");
		S_SUN_Shutdown ();
		return 0;
	}

	return ((info.play.samples * shm->channels) % shm->samples);
}

void S_SUN_Shutdown (void)
{
	close (audio_fd);
	audio_fd = -1;
	shm = NULL;
}

/*
==============
SNDDMA_Submit

Send sound to the device
===============
*/
void S_SUN_Submit (void)
{
	int		bsize;
	int		bytes, b;
	static int	wbufp = 0;
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

	if (bytes > sizeof (writebuf))
	{
		bytes = sizeof (writebuf);
		stop = wbufp + bytes / bsize;
	}

	// Transfert the sound data from the circular dma_buffer to writebuf
	// TODO: using 2 memcpys instead of this loop should be faster
	p = writebuf;
	idx = (wbufp*bsize) & (sizeof (dma_buffer) - 1);
	for (b = bytes; b; b--)
	{
		*p++ = dma_buffer[idx];
		idx = (idx + 1) & (sizeof (dma_buffer) - 1);
	}

	if (write (audio_fd, writebuf, bytes) < bytes)
		Con_Printf("audio can't keep up!\n");

	wbufp = stop;
}

#endif	// HAVE_SUN_SOUND

