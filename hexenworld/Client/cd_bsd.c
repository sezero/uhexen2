/*
	cd_bsd.c
	$Id: cd_bsd.c,v 1.6 2005-08-07 10:57:26 sezero Exp $

	Copyright (C) 1996-1997  Id Software, Inc.
	A few BSD bits taken from the Dark Places project for Hammer
	of Thyrion (Linux Hexen II)

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

#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>

#include <sys/cdio.h>
#include <paths.h>

static qboolean cdValid = false;
static qboolean	playing = false;
static qboolean	wasPlaying = false;
static qboolean	initialized = false;
static qboolean	enabled = true;
static qboolean playLooping = false;
static byte 	remap[100];
static byte	playTrack;
static byte	maxTrack;

static int cdfile = -1;
//static char cd_dev[64] = _PATH_DEV "cd0";
static char cd_dev[64] = _PATH_DEV "acd0"; // user can always do -cddev
static struct ioc_vol drv_vol0;	// orig. setting to be restored upon exit
static struct ioc_vol drv_vol;	// the volume setting we'll be using

static void CDAudio_Eject(void)
{
	if (cdfile == -1 || !enabled)
		return; // no cd init'd

	ioctl(cdfile, CDIOCALLOW);
	if ( ioctl(cdfile, CDIOCEJECT) == -1 ) 
		Con_DPrintf("ioctl CDIOCEJECT failed\n");
}


static void CDAudio_CloseDoor(void)
{
	if (cdfile == -1 || !enabled)
		return; // no cd init'd

	ioctl(cdfile, CDIOCALLOW);
	if ( ioctl(cdfile, CDIOCCLOSE) == -1 ) 
		Con_DPrintf("ioctl CDIOCCLOSE failed\n");
}

static int CDAudio_GetAudioDiskInfo(void)
{
	struct ioc_toc_header tochdr;

	if (cdfile == -1)
		return -1;

	cdValid = false;

	if ( ioctl(cdfile, CDIOREADTOCHEADER, &tochdr) == -1 ) 
	{
		Con_DPrintf("ioctl CDIOREADTOCHEADER failed\n");
		return -1;
	}

	if (tochdr.starting_track < 1)
	{
		Con_DPrintf("CDAudio: no music tracks\n");
		return -1;
	}

	cdValid = true;
	maxTrack = tochdr.ending_track;

	return 0;
}


void CDAudio_Play(byte track, qboolean looping)
{
	struct ioc_read_toc_entry entry;
	struct cd_toc_entry toc_buffer;
	struct ioc_play_track ti;

	if (cdfile == -1 || !enabled)
		return;
	
	if (!cdValid)
	{
		CDAudio_GetAudioDiskInfo();
		if (!cdValid)
			return;
	}

	track = remap[track];

	if (track < 1 || track > maxTrack)
	{
		Con_DPrintf("CDAudio: Bad track number %u.\n", track);
		return;
	}

	// don't try to play a non-audio track
	#define CDROM_DATA_TRACK 4
	memset((char *)&toc_buffer, 0, sizeof(toc_buffer));
	entry.data_len = sizeof(toc_buffer);
	entry.data = &toc_buffer;
	entry.starting_track = track;
	entry.address_format = CD_MSF_FORMAT;
	if ( ioctl(cdfile, CDIOREADTOCENTRYS, &entry) == -1 )
	{
		Con_DPrintf("ioctl CDIOREADTOCENTRYS failed\n");
		return;
	}
	if (toc_buffer.control & CDROM_DATA_TRACK)
	{
		Con_Printf("CDAudio: track %i is not audio\n", track);
		return;
	}

	if (playing)
	{
		if (playTrack == track)
			return;
		CDAudio_Stop();
	}

	ti.start_track = track;
	ti.end_track = track;
	ti.start_index = 1;
	ti.end_index = 99;

	if ( ioctl(cdfile, CDIOCPLAYTRACKS, &ti) == -1 )
	{
		Con_DPrintf("ioctl CDIOCPLAYTRACKS failed\n");
		return;
	}

	if ( ioctl(cdfile, CDIOCRESUME) == -1 )
		Con_DPrintf("ioctl CDIOCRESUME failed\n");

	playLooping = looping;
	playTrack = track;
	playing = true;
}


void CDAudio_Stop(void)
{
	if (cdfile == -1 || !enabled)
		return;
	
	if (!playing)
		return;

	if ( ioctl(cdfile, CDIOCSTOP) == -1 ) {
		Con_DPrintf("ioctl CDIOCSTOP failed (%d)\n", errno);
		return;
	}
	ioctl(cdfile, CDIOCALLOW);

	wasPlaying = false;
	playing = false;
}

void CDAudio_Pause(void)
{
	if (cdfile == -1 || !enabled)
		return;

	if (!playing)
		return;

	if ( ioctl(cdfile, CDIOCPAUSE) == -1 ) 
		Con_DPrintf("ioctl CDIOCPAUSE failed\n");

	wasPlaying = playing;
	playing = false;
}


void CDAudio_Resume(void)
{
	if (cdfile == -1 || !enabled)
		return;
	
	if (!cdValid)
		return;

	if (!wasPlaying)
		return;
	
	if ( ioctl(cdfile, CDIOCRESUME) == -1 ) 
		Con_DPrintf("ioctl CDIOCRESUME failed\n");
	playing = true;
}

static void CD_f (void)
{
	char	*command;
	int		ret;
	int		n;

	if (Cmd_Argc() < 2)
	{
		Con_Printf("commands:");
		Con_Printf("on, off, reset, remap, \n");
		Con_Printf("play, stop, loop, pause, resume\n");
		Con_Printf("eject, close, info\n");
		return;
	}

	command = Cmd_Argv (1);

	if (Q_strcasecmp(command, "on") == 0)
	{
		enabled = true;
		return;
	}

	if (Q_strcasecmp(command, "off") == 0)
	{
		if (playing)
			CDAudio_Stop();
		enabled = false;
		return;
	}

	if (Q_strcasecmp(command, "reset") == 0)
	{
		enabled = true;
		if (playing)
			CDAudio_Stop();
		for (n = 0; n < 100; n++)
			remap[n] = n;
		CDAudio_GetAudioDiskInfo();
		return;
	}

	if (Q_strcasecmp(command, "remap") == 0)
	{
		ret = Cmd_Argc() - 2;
		if (ret <= 0)
		{
			for (n = 1; n < 100; n++)
				if (remap[n] != n)
					Con_Printf("  %u -> %u\n", n, remap[n]);
			return;
		}
		for (n = 1; n <= ret; n++)
			remap[n] = atoi(Cmd_Argv (n+1));
		return;
	}

	if (Q_strcasecmp(command, "close") == 0)
	{
		CDAudio_CloseDoor();
		return;
	}

	if (!cdValid)
	{
		CDAudio_GetAudioDiskInfo();
		if (!cdValid)
		{
			Con_Printf("No CD in player.\n");
			return;
		}
	}

	if (Q_strcasecmp(command, "play") == 0)
	{
		CDAudio_Play((byte)atoi(Cmd_Argv (2)), false);
		return;
	}

	if (Q_strcasecmp(command, "loop") == 0)
	{
		CDAudio_Play((byte)atoi(Cmd_Argv (2)), true);
		return;
	}

	if (Q_strcasecmp(command, "stop") == 0)
	{
		CDAudio_Stop();
		return;
	}

	if (Q_strcasecmp(command, "pause") == 0)
	{
		CDAudio_Pause();
		return;
	}

	if (Q_strcasecmp(command, "resume") == 0)
	{
		CDAudio_Resume();
		return;
	}

	if (Q_strcasecmp(command, "eject") == 0)
	{
		if (playing)
			CDAudio_Stop();
		CDAudio_Eject();
		cdValid = false;
		return;
	}

	if (Q_strcasecmp(command, "info") == 0)
	{
		Con_Printf("%u tracks\n", maxTrack);
		if (playing)
			Con_Printf("Currently %s track %u\n", playLooping ? "looping" : "playing", playTrack);
		else if (wasPlaying)
			Con_Printf("Paused %s track %u\n", playLooping ? "looping" : "playing", playTrack);
		Con_Printf("Volume is %f\n", bgmvolume.value);
		return;
	}
}

void CDAudio_Update(void)
{
	struct ioc_read_subchannel subchnl;
	struct cd_sub_channel_info data;
	static time_t lastchk;

	if (cdfile == -1 || !enabled)
		return;

	if (playing && lastchk < time(NULL)) {
		lastchk = time(NULL) + 2; //two seconds between chks

		memset (&subchnl, 0, sizeof(subchnl));
		subchnl.data = &data;
		subchnl.data_len = sizeof(data);
		subchnl.address_format = CD_MSF_FORMAT;
		subchnl.data_format = CD_CURRENT_POSITION;
		subchnl.track = playTrack;
		if (ioctl(cdfile, CDIOCREADSUBCHANNEL, &subchnl) == -1 ) {
			Con_DPrintf("ioctl CDIOCREADSUBCHANNEL failed\n");
			playing = false;
			return;
		}
		if (data.header.audio_status != CD_AS_PLAY_IN_PROGRESS &&
			data.header.audio_status != CD_AS_PLAY_PAUSED) {
			playing = false;
			if (playLooping)
				CDAudio_Play(playTrack, true);
		}
		/*else {
			playTrack = data.what.position.track_number;
		}*/
	}
}

void CDAudio_UpdateVolume(void)
{
	if (cdfile == -1 || !enabled)
		return;

	if (bgmvolume.value != drv_vol.vol[0])
	{
		drv_vol.vol[0] = drv_vol.vol[2] =
		drv_vol.vol[1] = drv_vol.vol[3] = bgmvolume.value * 255.0;
		if (ioctl(cdfile, CDIOCSETVOL, &drv_vol) == -1)
			Con_DPrintf("ioctl CDIOCSETVOL failed\n");
	}
}

int CDAudio_Init(void)
{
	int i;

#ifndef H2W
	if (cls.state == ca_dedicated)
		return -1;
#endif
	if (COM_CheckParm("-nocdaudio"))
		return -1;

	if ((i = COM_CheckParm("-cddev")) != 0 && i < com_argc - 1) {
		strlcpy(cd_dev, com_argv[i + 1], sizeof(cd_dev));
		cd_dev[sizeof(cd_dev) - 1] = 0;
	}

	if ((cdfile = open(cd_dev, O_RDONLY | O_NONBLOCK)) == -1) {
		Con_Printf("CDAudio_Init: open of \"%s\" failed (%i)\n", cd_dev, errno);
		cdfile = -1;
		return -1;
	}

	for (i = 0; i < 100; i++)
		remap[i] = i;
	initialized = true;
	enabled = true;

	Con_Printf("CDAudio initialized (using BSD ioctls)\n");

	if (CDAudio_GetAudioDiskInfo())
	{
		Con_Printf("CDAudio_Init: No CD in drive\n");
		cdValid = false;
	}

	Cmd_AddCommand ("cd", CD_f);

	// get drives volume
	if (ioctl(cdfile, CDIOCGETVOL, &drv_vol0) == -1) {
		Con_Printf("ioctl CDIOCGETVOL failed\n");
		drv_vol0.vol[0] = drv_vol0.vol[2] =
		drv_vol0.vol[1] = drv_vol0.vol[3] = 255.0;
	}
	// set our own volume
	drv_vol.vol[0] = drv_vol.vol[2] =
	drv_vol.vol[1] = drv_vol.vol[3] = bgmvolume.value * 255.0;
	if (ioctl(cdfile, CDIOCSETVOL, &drv_vol) == -1)
		Con_Printf("ioctl CDIOCSETVOL failed\n");

	return 0;
}


void CDAudio_Shutdown(void)
{
	if (!initialized)
		return;
	CDAudio_Stop();
	// put the drives old volume back
	if (ioctl(cdfile, CDIOCSETVOL, &drv_vol0) == -1)
		Con_Printf("ioctl CDIOCSETVOL failed\n");
	close(cdfile);
	cdfile = -1;
}

