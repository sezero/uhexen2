/* cd_amiga.c
 * Copyright (C) 1996-1997  Id Software, Inc.
 * Copyright (C) 2021 Szilard Biro <col.lawrence@gmail.com>
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
#include "cdaudio.h"

#include <dos/dostags.h>
#include <dos/dosextens.h>
#include <dos/filehandler.h>
#include <libraries/cdplayer.h>
#include <proto/cdplayer.h>
#include <proto/dos.h>
#include <proto/exec.h>

static qboolean cdValid = false;
static qboolean	playing = false;
static qboolean	wasPlaying = false;
static qboolean	initialized = false;
static qboolean	enabled = true;
static qboolean playLooping = false;
static byte	remap[100];
static byte	playTrack;
static byte	maxTrack;

struct Library *CDPlayerBase = NULL;
static struct IOStdReq *cdRequest = NULL;
static struct MsgPort *cdPort = NULL;
static BYTE	cdDevice = -1;
static struct CD_TOC cdTOC;
static const char	default_dev[] = "CD0:"; /* user can always do -cddev */
static const char	*cd_dev = default_dev;

static float	old_cdvolume;
static qboolean	hw_vol_works = true;
static struct CD_Volume	orig_vol;	/* original setting to be restored upon exit */
static struct CD_Volume	drv_vol;	/* the volume setting we'll be using */


static void CDAudio_Eject(void)
{
	BYTE error;

	if (cdDevice == -1 || !enabled)
		return;

	error = CDEject(cdRequest);
	if (error)
		Con_DPrintf("CDEject failed (%d)\n", (int)error);
}

static int CDAudio_GetAudioDiskInfo(void)
{
	BYTE error;

	if (cdDevice == -1)
		return -1;

	cdValid = false;

	error = CDReadTOC(&cdTOC, cdRequest);
	if (error)
	{
		Con_DPrintf("CDReadTOC failed (%d)\n", (int)error);
		return -1;
	}

	if (cdTOC.cdc_NumTracks < 1)
	{
		Con_DPrintf("CDAudio: no music tracks\n");
		return -1;
	}

	cdValid = true;
	maxTrack = cdTOC.cdc_NumTracks;

	return 0;
}

int CDAudio_Play(byte track, qboolean looping)
{
	BYTE error;

	if (cdDevice == -1 || !enabled)
		return -1;

	if (!cdValid)
	{
		CDAudio_GetAudioDiskInfo();
		if (!cdValid)
			return -1;
	}

	track = remap[track];

	if (track < 1 || track > maxTrack)
	{
		Con_DPrintf("CDAudio: Bad track number %u.\n", track);
		return -1;
	}

	/* don't try to play a non-audio track */
	if (cdTOC.cdc_Flags[track])
	{
		Con_Printf("CDAudio: track %i is not audio\n", track);
		return -1;
	}

	if (playing)
	{
		if (playTrack == track)
			return 0;
		CDAudio_Stop();
	}

	error = CDPlay(track, track, cdRequest);
	if (error)
	{
		Con_DPrintf("CDPlay failed (%d)\n", (int)error);
		return -1;
	}

	playLooping = looping;
	playTrack = track;
	playing = true;

	if (bgmvolume.value == 0) /* don't bother advancing */
		CDAudio_Pause ();

	return 0;
}

void CDAudio_Stop(void)
{
	BYTE error;

	if (cdDevice == -1 || !enabled)
		return;

	if (!playing)
		return;

	error = CDStop(cdRequest);
	if (error)
		Con_DPrintf("CDStop failed (%d)\n", (int)error);

	wasPlaying = false;
	playing = false;
}

void CDAudio_Pause(void)
{
	BYTE error;

	if (cdDevice == -1 || !enabled)
		return;

	if (!playing)
		return;

	error = CDResume(TRUE, cdRequest);
	if (error)
		Con_DPrintf("CDResume failed (%d)\n", (int)error);

	wasPlaying = playing;
	playing = false;
}

void CDAudio_Resume(void)
{
	BYTE error;

	if (cdDevice == -1 || !enabled)
		return;

	if (!cdValid)
		return;

	if (!wasPlaying)
		return;

	error = CDResume(FALSE, cdRequest);
	if (error)
		Con_DPrintf("CDResume failed (%d)\n", (int)error);
	playing = true;
}

static void CD_f (void)
{
	const char	*command;
	int		ret, n;

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

static qboolean CD_GetVolume (struct CD_Volume *vol)
{
	BYTE error;
	error = CDGetVolume(vol, cdRequest);
	if (error)
	{
		Con_DPrintf("CDGetVolume failed (%d)\n", (int)error);
		return false;
	}
	return true;
}

static qboolean CD_SetVolume (struct CD_Volume *vol)
{
	BYTE error;
	error = CDSetVolume(vol, cdRequest);
	if (error)
	{
		Con_DPrintf("CDSetVolume failed (%d)\n", (int)error);
		return false;
	}
	return true;
}

static qboolean CDAudio_SetVolume (float value)
{
	if (cdDevice == -1 || !enabled)
		return false;

	old_cdvolume = value;

	if (value == 0.0f)
		CDAudio_Pause ();
	else
		CDAudio_Resume();

	if (!hw_vol_works)
	{
		return false;
	}
	else
	{
		drv_vol.cdv_Chan0 = drv_vol.cdv_Chan2 =
		drv_vol.cdv_Chan1 = drv_vol.cdv_Chan3 = value * 255.0f;
		return CD_SetVolume (&drv_vol);
	}
}

void CDAudio_Update(void)
{
	BOOL status;
	static double lastCheck;

	if (cdDevice == -1 || !enabled)
		return;

	if (old_cdvolume != bgmvolume.value)
		CDAudio_SetVolume (bgmvolume.value);

	if (playing && lastCheck < realtime)
	{
		lastCheck = realtime + 2.0; /* two seconds between chks */
		status = CDActive(cdRequest);
		if (!status)
		{
			playing = false;
			if (playLooping)
				CDAudio_Play(playTrack, true);
		}
	}
}

static void CD_CloseDevice(void)
{
	if (cdRequest)
	{
		if (!cdDevice)
		{
			CloseDevice((struct IORequest *)cdRequest);
			cdDevice = -1;
		}
		DeleteIORequest(cdRequest);
		cdRequest = NULL;
	}
	if (cdPort)
	{
		DeleteMsgPort(cdPort);
		cdPort = NULL;
	}
}

static int CD_OpenDevice(const char *volume)
{
	struct FileLock *fl;
	struct DosList *dol = NULL;
	struct DosList *doslist;
	BPTR lock;
	struct Process *me;
	APTR oldwindow;

	me = (struct Process *)FindTask(NULL);
	oldwindow = me->pr_WindowPtr;
	me->pr_WindowPtr = (APTR)-1;
	lock = Lock((STRPTR)volume, ACCESS_READ);
	me->pr_WindowPtr = oldwindow;

	if (!lock)
		return -1; // IoErr()

	UnLock(lock);

	// look for the device
	fl = (struct FileLock *)BADDR(lock);
	if ((doslist = LockDosList(LDF_DEVICES | LDF_READ)))
	{
		while ((doslist = NextDosEntry(doslist, LDF_DEVICES)))
		{
			if (doslist->dol_Task == fl->fl_Task)
			{
				dol = doslist;
				break;
			}
		}
		UnLockDosList(LDF_DEVICES | LDF_READ);
	}

	if (dol)
	{
		struct FileSysStartupMsg *fssm = (struct FileSysStartupMsg *)BADDR(dol->dol_misc.dol_handler.dol_Startup);
		if ((ULONG)fssm > 0x400 && TypeOfMem(fssm) && fssm->fssm_Unit <= 0x00ffffff)
		{
			STRPTR device = (STRPTR)BADDR(fssm->fssm_Device);
			if (device && TypeOfMem(device) && device[0] != 0 && device[1] != '\0')
			{
				if ((cdPort = CreateMsgPort()))
				{
					if ((cdRequest = (struct IOStdReq *)CreateIORequest(cdPort, sizeof(struct IOStdReq))))
					{
						cdDevice = OpenDevice(device + 1, fssm->fssm_Unit, (struct IORequest *)cdRequest, 0);
						return cdDevice;
					}
				}
			}
		}
	}

	CD_CloseDevice();

	return -1;
}

int CDAudio_Init(void)
{
	int i;

	if (safemode || COM_CheckParm("-nocdaudio"))
		return -1;

	if (!(CDPlayerBase = OpenLibrary((STRPTR)CDPLAYERNAME, CDPLAYERVERSION)))
	{
		Con_Printf ("%s: can't open cdplayer.library, CD Audio is disabled.\n", __thisfunc__);
		return -1;
	}

	if ((i = COM_CheckParm("-cddev")) != 0 && i < com_argc - 1)
		cd_dev = com_argv[i + 1];

	if ((cdDevice = CD_OpenDevice(cd_dev)) == -1)
	{
		i = cdDevice;
		Con_Printf("%s: open of \"%s\" failed (%d)\n",
				__thisfunc__, cd_dev, i);
		return -1;
	}

	for (i = 0; i < 100; i++)
		remap[i] = i;
	initialized = true;
	enabled = true;
	old_cdvolume = bgmvolume.value;

	Con_Printf("CDAudio initialized (using cdplayer.library)\n");

	if (CDAudio_GetAudioDiskInfo())
	{
		Con_Printf("%s: No CD in drive\n", __thisfunc__);
		cdValid = false;
	}

	Cmd_AddCommand ("cd", CD_f);

	hw_vol_works = CD_GetVolume (&orig_vol);
	if (hw_vol_works)
		hw_vol_works = CDAudio_SetVolume (bgmvolume.value);

	return 0;
}

void CDAudio_Shutdown(void)
{
	if (!initialized)
		return;
	CDAudio_Stop();
	if (hw_vol_works)
		CD_SetVolume (&orig_vol);
	CD_CloseDevice();
	if (CDPlayerBase)
	{
		CloseLibrary(CDPlayerBase);
		CDPlayerBase = NULL;
	}
}
