/*
	cd_linux.c
	$Id: cd_linux.c,v 1.27 2007-11-11 13:17:44 sezero Exp $

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


#include "cd_unix.h"

#ifdef	__USE_LINUX_CDROM__

#include "quakedef.h"

#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>

#include <linux/cdrom.h>
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

static int	cdfile = -1;
static const char	default_dev[] = _PATH_DEV "cdrom"; // user can always do -cddev
static const char	*cd_dev = default_dev;

static float	old_cdvolume;
static qboolean	hw_vol_works = true;
static struct cdrom_volctrl	orig_vol;	// orig. setting to be restored upon exit
static struct cdrom_volctrl	drv_vol;	// the volume setting we'll be using


#define IOCTL_FAILURE(IoctlName)	{					\
	Con_DPrintf("ioctl failed: %s (errno: %d)\n", #IoctlName, errno);	\
}

static void CDAudio_Eject(void)
{
	if (cdfile == -1 || !enabled)
		return; // no cd init'd

	if ( ioctl(cdfile, CDROMEJECT) == -1 )
		IOCTL_FAILURE(CDROMEJECT);
}

static void CDAudio_CloseDoor(void)
{
	if (cdfile == -1 || !enabled)
		return; // no cd init'd

	if ( ioctl(cdfile, CDROMCLOSETRAY) == -1 )
		IOCTL_FAILURE(CDROMCLOSETRAY);
}

static int CDAudio_GetAudioDiskInfo(void)
{
	struct cdrom_tochdr tochdr;

	if (cdfile == -1)
		return -1;

	cdValid = false;

	if ( ioctl(cdfile, CDROMREADTOCHDR, &tochdr) == -1 )
	{
		IOCTL_FAILURE(CDROMREADTOCHDR);
		return -1;
	}

	if (tochdr.cdth_trk0 < 1)
	{
		Con_DPrintf("CDAudio: no music tracks\n");
		return -1;
	}

	cdValid = true;
	maxTrack = tochdr.cdth_trk1;

	return 0;
}

void CDAudio_Play(byte track, qboolean looping)
{
	struct cdrom_tocentry entry;
	struct cdrom_ti ti;

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
	entry.cdte_track = track;
	entry.cdte_format = CDROM_MSF;
	if ( ioctl(cdfile, CDROMREADTOCENTRY, &entry) == -1 )
	{
		IOCTL_FAILURE(CDROMREADTOCENTRY);
		return;
	}
	if (entry.cdte_ctrl == CDROM_DATA_TRACK)
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

	ti.cdti_trk0 = track;
	ti.cdti_trk1 = track;
	ti.cdti_ind0 = 1;
	ti.cdti_ind1 = 99;

	if ( ioctl(cdfile, CDROMPLAYTRKIND, &ti) == -1 )
	{
		IOCTL_FAILURE(CDROMPLAYTRKIND);
		return;
	}

	if ( ioctl(cdfile, CDROMRESUME) == -1 )
		IOCTL_FAILURE(CDROMRESUME);

	playLooping = looping;
	playTrack = track;
	playing = true;

	if (!hw_vol_works && bgmvolume.value == 0.0)
		CDAudio_Pause ();
}

void CDAudio_Stop(void)
{
	if (cdfile == -1 || !enabled)
		return;

	if (!playing)
		return;

	if ( ioctl(cdfile, CDROMSTOP) == -1 )
		IOCTL_FAILURE(CDROMSTOP);

	wasPlaying = false;
	playing = false;
}

void CDAudio_Pause(void)
{
	if (cdfile == -1 || !enabled)
		return;

	if (!playing)
		return;

	if ( ioctl(cdfile, CDROMPAUSE) == -1 )
		IOCTL_FAILURE(CDROMPAUSE);

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

	if ( ioctl(cdfile, CDROMRESUME) == -1 )
		IOCTL_FAILURE(CDROMRESUME);
	playing = true;
}

static void CD_f (void)
{
	const char	*command;
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

	if (q_strcasecmp(command, "on") == 0)
	{
		enabled = true;
		return;
	}

	if (q_strcasecmp(command, "off") == 0)
	{
		if (playing)
			CDAudio_Stop();
		enabled = false;
		return;
	}

	if (q_strcasecmp(command, "reset") == 0)
	{
		enabled = true;
		if (playing)
			CDAudio_Stop();
		for (n = 0; n < 100; n++)
			remap[n] = n;
		CDAudio_GetAudioDiskInfo();
		return;
	}

	if (q_strcasecmp(command, "remap") == 0)
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

	if (q_strcasecmp(command, "close") == 0)
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

	if (q_strcasecmp(command, "play") == 0)
	{
		CDAudio_Play((byte)atoi(Cmd_Argv (2)), false);
		return;
	}

	if (q_strcasecmp(command, "loop") == 0)
	{
		CDAudio_Play((byte)atoi(Cmd_Argv (2)), true);
		return;
	}

	if (q_strcasecmp(command, "stop") == 0)
	{
		CDAudio_Stop();
		return;
	}

	if (q_strcasecmp(command, "pause") == 0)
	{
		CDAudio_Pause();
		return;
	}

	if (q_strcasecmp(command, "resume") == 0)
	{
		CDAudio_Resume();
		return;
	}

	if (q_strcasecmp(command, "eject") == 0)
	{
		if (playing)
			CDAudio_Stop();
		CDAudio_Eject();
		cdValid = false;
		return;
	}

	if (q_strcasecmp(command, "info") == 0)
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

static qboolean CD_GetVolume (struct cdrom_volctrl *vol)
{
	if (ioctl(cdfile, CDROMVOLREAD, vol) == -1)
	{
		IOCTL_FAILURE(CDROMVOLREAD);
		return false;
	}
	return true;
}

static qboolean CD_SetVolume (struct cdrom_volctrl *vol)
{
	if (ioctl(cdfile, CDROMVOLCTRL, vol) == -1 )
	{
		IOCTL_FAILURE(CDROMVOLCTRL);
		return false;
	}
	return true;
}

static qboolean CDAudio_SetVolume (cvar_t *var)
{
	if (cdfile == -1 || !enabled)
		return false;

	if (var->value < 0.0)
		Cvar_SetValue (var->name, 0.0);
	else if (var->value > 1.0)
		Cvar_SetValue (var->name, 1.0);
	old_cdvolume = var->value;
	if (hw_vol_works)
	{
		drv_vol.channel0 = drv_vol.channel2 =
		drv_vol.channel1 = drv_vol.channel3 = var->value * 255.0;
		return CD_SetVolume (&drv_vol);
	}
	else
	{
		if (old_cdvolume == 0.0)
			CDAudio_Pause ();
		else
			CDAudio_Resume();
		return false;
	}
}

void CDAudio_Update(void)
{
	struct cdrom_subchnl subchnl;
	static time_t lastchk;

	if (cdfile == -1 || !enabled)
		return;

	if (old_cdvolume != bgmvolume.value)
		CDAudio_SetVolume (&bgmvolume);

	if (playing && lastchk < time(NULL))
	{
		lastchk = time(NULL) + 2; //two seconds between chks
		subchnl.cdsc_format = CDROM_MSF;
		if (ioctl(cdfile, CDROMSUBCHNL, &subchnl) == -1 )
		{
			IOCTL_FAILURE(CDROMSUBCHNL);
			playing = false;
			return;
		}
		if (subchnl.cdsc_audiostatus != CDROM_AUDIO_PLAY &&
			subchnl.cdsc_audiostatus != CDROM_AUDIO_PAUSED)
		{
			playing = false;
			if (playLooping)
				CDAudio_Play(playTrack, true);
		}
		else
		{
			playTrack = subchnl.cdsc_trk;
		}
	}
}

int CDAudio_Init(void)
{
	int i;

	if (safemode || COM_CheckParm("-nocdaudio"))
		return -1;

	if ((i = COM_CheckParm("-cddev")) != 0 && i < com_argc - 1)
		cd_dev = com_argv[i + 1];

	if ((cdfile = open(cd_dev, O_RDONLY | O_NONBLOCK)) == -1)
	{
		Con_Printf("%s: open of \"%s\" failed (%i)\n", __thisfunc__, cd_dev, errno);
		cdfile = -1;
		return -1;
	}

	for (i = 0; i < 100; i++)
		remap[i] = i;
	initialized = true;
	enabled = true;
	old_cdvolume = bgmvolume.value;

	Con_Printf("CDAudio initialized (using Linux ioctls)\n");

	if (CDAudio_GetAudioDiskInfo())
	{
		Con_Printf("%s: No CD in drive\n", __thisfunc__);
		cdValid = false;
	}

	Cmd_AddCommand ("cd", CD_f);

	hw_vol_works = CD_GetVolume (&orig_vol);
	if (hw_vol_works)
		hw_vol_works = CDAudio_SetVolume (&bgmvolume);

	return 0;
}

void CDAudio_Shutdown(void)
{
	if (!initialized)
		return;
	CDAudio_Stop();
	if (hw_vol_works)
		CD_SetVolume (&orig_vol);
	close(cdfile);
	cdfile = -1;
}

#endif	// __USE_LINUX_CDROM__

