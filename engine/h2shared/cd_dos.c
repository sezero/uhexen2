/*
 * cd_audio.c -- dos cdaudio support.
 * from quake1 source with minor adaptations for uhexen2.
 *
 * $Id$
 *
 * Quake is a trademark of Id Software, Inc., (c) 1996 Id Software, Inc.
 * All rights reserved.
 *
 * Copyright (C) 1996-1997  Id Software, Inc.
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

#include <dpmi.h>
#include <go32.h>
#include "quakedef.h"
#include "cdaudio.h"
#include "dosisms.h"

#define ADDRESS_MODE_HSG		0
#define ADDRESS_MODE_RED_BOOK		1

#define STATUS_ERROR_BIT		0x8000
#define STATUS_BUSY_BIT			0x0200
#define STATUS_DONE_BIT			0x0100
#define STATUS_ERROR_MASK		0x00ff

#define ERROR_WRITE_PROTECT		0
#define ERROR_UNKNOWN_UNIT		1
#define ERROR_DRIVE_NOT_READY		2
#define ERROR_UNKNOWN_COMMAND		3
#define ERROR_CRC_ERROR			4
#define ERROR_BAD_REQUEST_LEN		5
#define ERROR_SEEK_ERROR		6
#define ERROR_UNKNOWN_MEDIA		7
#define ERROR_SECTOR_NOT_FOUND		8
#define ERROR_OUT_OF_PAPER		9
#define ERROR_WRITE_FAULT		10
#define ERROR_READ_FAULT		11
#define ERROR_GENERAL_FAILURE		12
#define ERROR_RESERVED_13		13
#define ERROR_RESERVED_14		14
#define ERROR_BAD_DISK_CHANGE		15

#define COMMAND_READ			3
#define COMMAND_WRITE			12
#define COMMAND_PLAY_AUDIO		132
#define COMMAND_STOP_AUDIO		133
#define COMMAND_RESUME_AUDIO		136

#define READ_REQUEST_AUDIO_CHANNEL_INFO		4
#define READ_REQUEST_DEVICE_STATUS		6
#define READ_REQUEST_MEDIA_CHANGE		9
#define READ_REQUEST_AUDIO_DISK_INFO		10
#define READ_REQUEST_AUDIO_TRACK_INFO		11
#define READ_REQUEST_AUDIO_STATUS		15

#define WRITE_REQUEST_EJECT			0
#define WRITE_REQUEST_RESET			2
#define WRITE_REQUEST_AUDIO_CHANNEL_INFO	3

#define STATUS_DOOR_OPEN			0x00000001
#define STATUS_DOOR_UNLOCKED			0x00000002
#define STATUS_RAW_SUPPORT			0x00000004
#define STATUS_READ_WRITE			0x00000008
#define STATUS_AUDIO_SUPPORT			0x00000010
#define STATUS_INTERLEAVE_SUPPORT		0x00000020
#define STATUS_BIT_6_RESERVED			0x00000040
#define STATUS_PREFETCH_SUPPORT			0x00000080
#define STATUS_AUDIO_MANIPLUATION_SUPPORT	0x00000100
#define STATUS_RED_BOOK_ADDRESS_SUPPORT		0x00000200

#define MEDIA_NOT_CHANGED			1
#define MEDIA_STATUS_UNKNOWN			0
#define MEDIA_CHANGED				(-1)

#define AUDIO_CONTROL_MASK			0xd0
#define AUDIO_CONTROL_DATA_TRACK		0x40
#define AUDIO_CONTROL_AUDIO_2_TRACK		0x00
#define AUDIO_CONTROL_AUDIO_2P_TRACK		0x10
#define AUDIO_CONTROL_AUDIO_4_TRACK		0x80
#define AUDIO_CONTROL_AUDIO_4P_TRACK		0x90

#define AUDIO_STATUS_PAUSED			0x0001

#pragma pack(1)

struct playAudioRequest
{
	char	addressingMode;
	int		startLocation;
	int		sectors;
};

struct readRequest
{
	char	mediaDescriptor;
	short	bufferOffset;
	short	bufferSegment;
	short	length;
	short	startSector;
	int		volumeID;
};

struct writeRequest
{
	char	mediaDescriptor;
	short	bufferOffset;
	short	bufferSegment;
	short	length;
	short	startSector;
	int		volumeID;
};

struct cd_request
{
	char	headerLength;
	char	unit;
	char	command;
	short	status;
	char	reserved[8];
	union
	{
		struct	playAudioRequest	playAudio;
		struct	readRequest		read;
		struct	writeRequest		write;
	} x;
};


struct audioChannelInfo_s
{
	char	code;
	char	channel0input;
	char	channel0volume;
	char	channel1input;
	char	channel1volume;
	char	channel2input;
	char	channel2volume;
	char	channel3input;
	char	channel3volume;
};

struct deviceStatus_s
{
	char	code;
	int	status;
};

struct mediaChange_s
{
	char	code;
	char	status;
};

struct audioDiskInfo_s
{
	char	code;
	char	lowTrack;
	char	highTrack;
	int	leadOutStart;
};

struct audioTrackInfo_s
{
	char	code;
	char	track;
	int	start;
	char	control;
};

struct audioStatus_s
{
	char	code;
	short	status;
	int	PRstartLocation;
	int	PRendLocation;
};

struct reset_s
{
	char	code;
};

union readInfo_u
{
	struct audioChannelInfo_s	audioChannelInfo;
	struct deviceStatus_s		deviceStatus;
	struct mediaChange_s		mediaChange;
	struct audioDiskInfo_s		audioDiskInfo;
	struct audioTrackInfo_s		audioTrackInfo;
	struct audioStatus_s		audioStatus;
	struct reset_s			reset;
};

#pragma pack()

#define MAXIMUM_TRACKS			100

typedef struct
{
	int			start;
	int			length;
	qboolean	isData;
} track_info;

typedef struct
{
	qboolean	valid;
	int			leadOutAddress;
	track_info	track[MAXIMUM_TRACKS];
	byte		lowTrack;
	byte		highTrack;
} cd_info;

static struct cd_request	*cdRequest;
static union readInfo_u		*readInfo;
static cd_info			cd;

static qboolean	playing = false;
static qboolean	wasPlaying = false;
static qboolean	mediaCheck = false;
static qboolean	initialized = false;
static qboolean	enabled = true;
static qboolean	playLooping = false;
static short	cdRequestSegment;
static short	cdRequestOffset;
static short	readInfoSegment;
static short	readInfoOffset;
static byte	remap[256];
static byte	cdrom;
static byte	firstcdrom, numcdroms;
static byte	cdroms_list[32];
static byte	playTrack;
static float	old_cdvolume;


static int RedBookToSector (int rb)
{
	byte	minute;
	byte	second;
	byte	frame;

	minute = (rb >> 16) & 0xff;
	second = (rb >> 8) & 0xff;
	frame = rb & 0xff;
	return minute * 60 * 75 + second * 75 + frame;
}


static void CDAudio_Reset (void)
{
	cdRequest->headerLength = 13;
	cdRequest->unit = 0;
	cdRequest->command = COMMAND_WRITE;
	cdRequest->status = 0;

	cdRequest->x.write.mediaDescriptor = 0;
	cdRequest->x.write.bufferOffset = readInfoOffset;
	cdRequest->x.write.bufferSegment = readInfoSegment;
	cdRequest->x.write.length = sizeof(struct reset_s);
	cdRequest->x.write.startSector = 0;
	cdRequest->x.write.volumeID = 0;

	readInfo->reset.code = WRITE_REQUEST_RESET;

	regs.x.ax = 0x1510;
	regs.x.cx = cdrom;
	regs.x.es = cdRequestSegment;
	regs.x.bx = cdRequestOffset;
	dos_int86 (0x2f);
}


static void CDAudio_Eject (void)
{
	cdRequest->headerLength = 13;
	cdRequest->unit = 0;
	cdRequest->command = COMMAND_WRITE;
	cdRequest->status = 0;

	cdRequest->x.write.mediaDescriptor = 0;
	cdRequest->x.write.bufferOffset = readInfoOffset;
	cdRequest->x.write.bufferSegment = readInfoSegment;
	cdRequest->x.write.length = sizeof(struct reset_s);
	cdRequest->x.write.startSector = 0;
	cdRequest->x.write.volumeID = 0;

	readInfo->reset.code = WRITE_REQUEST_EJECT;

	regs.x.ax = 0x1510;
	regs.x.cx = cdrom;
	regs.x.es = cdRequestSegment;
	regs.x.bx = cdRequestOffset;
	dos_int86 (0x2f);
}


static int CDAudio_GetAudioTrackInfo (byte track, int *start)
{
	byte	control;

	cdRequest->headerLength = 13;
	cdRequest->unit = 0;
	cdRequest->command = COMMAND_READ;
	cdRequest->status = 0;

	cdRequest->x.read.mediaDescriptor = 0;
	cdRequest->x.read.bufferOffset = readInfoOffset;
	cdRequest->x.read.bufferSegment = readInfoSegment;
	cdRequest->x.read.length = sizeof(struct audioTrackInfo_s);
	cdRequest->x.read.startSector = 0;
	cdRequest->x.read.volumeID = 0;

	readInfo->audioTrackInfo.code = READ_REQUEST_AUDIO_TRACK_INFO;
	readInfo->audioTrackInfo.track = track;

	regs.x.ax = 0x1510;
	regs.x.cx = cdrom;
	regs.x.es = cdRequestSegment;
	regs.x.bx = cdRequestOffset;
	dos_int86 (0x2f);

	if (cdRequest->status & STATUS_ERROR_BIT)
	{
		Con_DPrintf("%s %04x\n", __thisfunc__, cdRequest->status & 0xffff);
		return -1;
	}

	*start = readInfo->audioTrackInfo.start;
	control = readInfo->audioTrackInfo.control & AUDIO_CONTROL_MASK;
	return (control & AUDIO_CONTROL_DATA_TRACK);
}


static int CDAudio_GetAudioDiskInfo (void)
{
	int		n;

	cdRequest->headerLength = 13;
	cdRequest->unit = 0;
	cdRequest->command = COMMAND_READ;
	cdRequest->status = 0;

	cdRequest->x.read.mediaDescriptor = 0;
	cdRequest->x.read.bufferOffset = readInfoOffset;
	cdRequest->x.read.bufferSegment = readInfoSegment;
	cdRequest->x.read.length = sizeof(struct audioDiskInfo_s);
	cdRequest->x.read.startSector = 0;
	cdRequest->x.read.volumeID = 0;

	readInfo->audioDiskInfo.code = READ_REQUEST_AUDIO_DISK_INFO;

	regs.x.ax = 0x1510;
	regs.x.cx = cdrom;
	regs.x.es = cdRequestSegment;
	regs.x.bx = cdRequestOffset;
	dos_int86 (0x2f);

	if (cdRequest->status & STATUS_ERROR_BIT)
	{
		Con_DPrintf("%s %04x\n", __thisfunc__, cdRequest->status & 0xffff);
		return -1;
	}

	cd.valid = true;
	cd.lowTrack = readInfo->audioDiskInfo.lowTrack;
	cd.highTrack = readInfo->audioDiskInfo.highTrack;
	cd.leadOutAddress = readInfo->audioDiskInfo.leadOutStart;

	for (n = cd.lowTrack; n <= cd.highTrack; n++)
	{
		cd.track[n].isData = CDAudio_GetAudioTrackInfo (n, &cd.track[n].start);
		if (n > cd.lowTrack)
		{
			cd.track[n-1].length = RedBookToSector(cd.track[n].start) - RedBookToSector(cd.track[n-1].start);
			if (n == cd.highTrack)
				cd.track[n].length = RedBookToSector(cd.leadOutAddress) - RedBookToSector(cd.track[n].start);
		}
	}

	return 0;
}


static int CDAudio_GetAudioStatus (void)
{
	cdRequest->headerLength = 13;
	cdRequest->unit = 0;
	cdRequest->command = COMMAND_READ;
	cdRequest->status = 0;

	cdRequest->x.read.mediaDescriptor = 0;
	cdRequest->x.read.bufferOffset = readInfoOffset;
	cdRequest->x.read.bufferSegment = readInfoSegment;
	cdRequest->x.read.length = sizeof(struct audioStatus_s);
	cdRequest->x.read.startSector = 0;
	cdRequest->x.read.volumeID = 0;

	readInfo->audioDiskInfo.code = READ_REQUEST_AUDIO_STATUS;

	regs.x.ax = 0x1510;
	regs.x.cx = cdrom;
	regs.x.es = cdRequestSegment;
	regs.x.bx = cdRequestOffset;
	dos_int86 (0x2f);

	if (cdRequest->status & STATUS_ERROR_BIT)
		return -1;
	return 0;
}


static int CDAudio_MediaChange (void)
{
	cdRequest->headerLength = 13;
	cdRequest->unit = 0;
	cdRequest->command = COMMAND_READ;
	cdRequest->status = 0;

	cdRequest->x.read.mediaDescriptor = 0;
	cdRequest->x.read.bufferOffset = readInfoOffset;
	cdRequest->x.read.bufferSegment = readInfoSegment;
	cdRequest->x.read.length = sizeof(struct mediaChange_s);
	cdRequest->x.read.startSector = 0;
	cdRequest->x.read.volumeID = 0;

	readInfo->mediaChange.code = READ_REQUEST_MEDIA_CHANGE;

	regs.x.ax = 0x1510;
	regs.x.cx = cdrom;
	regs.x.es = cdRequestSegment;
	regs.x.bx = cdRequestOffset;
	dos_int86 (0x2f);

	return readInfo->mediaChange.status;
}


// we set the volume to 0 first and then to the desired volume
// some cd-rom drivers seem to need it done this way
static qboolean CD_SetVolume (byte volume)
{
	if (!initialized || !enabled)
		return false;

	cdRequest->headerLength = 13;
	cdRequest->unit = 0;
	cdRequest->command = COMMAND_WRITE;
	cdRequest->status = 0;

	cdRequest->x.read.mediaDescriptor = 0;
	cdRequest->x.read.bufferOffset = readInfoOffset;
	cdRequest->x.read.bufferSegment = readInfoSegment;
	cdRequest->x.read.length = sizeof(struct audioChannelInfo_s);
	cdRequest->x.read.startSector = 0;
	cdRequest->x.read.volumeID = 0;

	readInfo->audioChannelInfo.code = WRITE_REQUEST_AUDIO_CHANNEL_INFO;
	readInfo->audioChannelInfo.channel0input = 0;
	readInfo->audioChannelInfo.channel0volume = 0;
	readInfo->audioChannelInfo.channel1input = 1;
	readInfo->audioChannelInfo.channel1volume = 0;
	readInfo->audioChannelInfo.channel2input = 2;
	readInfo->audioChannelInfo.channel2volume = 0;
	readInfo->audioChannelInfo.channel3input = 3;
	readInfo->audioChannelInfo.channel3volume = 0;

	regs.x.ax = 0x1510;
	regs.x.cx = cdrom;
	regs.x.es = cdRequestSegment;
	regs.x.bx = cdRequestOffset;
	dos_int86 (0x2f);

	readInfo->audioChannelInfo.channel0volume = volume;
	readInfo->audioChannelInfo.channel1volume = volume;

	regs.x.ax = 0x1510;
	regs.x.cx = cdrom;
	regs.x.es = cdRequestSegment;
	regs.x.bx = cdRequestOffset;
	dos_int86 (0x2f);

	return true;
}

static qboolean CDAudio_SetVolume (float value)
{
	byte	volume;

	if (!initialized || !enabled)
		return false;

	old_cdvolume = value;

	/* should I pause if value == 0 ? */
	volume = (int)(value * 255.0f);
	return CD_SetVolume (volume);
}


int CDAudio_Play (byte track, qboolean looping)
{
	if (!initialized || !enabled)
		return -1;

	if (!cd.valid)
		return -1;

	track = remap[track];

	if (playing)
	{
		if (playTrack == track)
			return 0;
		CDAudio_Stop();
	}

	playLooping = looping;

	if (track < cd.lowTrack || track > cd.highTrack)
	{
		Con_DPrintf("%s: Bad track number %u.\n", __thisfunc__, track);
		return -1;
	}

	playTrack = track;

	if (cd.track[track].isData)
	{
		Con_DPrintf("%s: Can not play data.\n", __thisfunc__);
		return -1;
	}

	CDAudio_SetVolume (bgmvolume.value);

	cdRequest->headerLength = 13;
	cdRequest->unit = 0;
	cdRequest->command = COMMAND_PLAY_AUDIO;
	cdRequest->status = 0;

	cdRequest->x.playAudio.addressingMode = ADDRESS_MODE_RED_BOOK;
	cdRequest->x.playAudio.startLocation = cd.track[track].start;
	cdRequest->x.playAudio.sectors = cd.track[track].length;

	regs.x.ax = 0x1510;
	regs.x.cx = cdrom;
	regs.x.es = cdRequestSegment;
	regs.x.bx = cdRequestOffset;
	dos_int86 (0x2f);

	if (cdRequest->status & STATUS_ERROR_BIT)
	{
		Con_DPrintf("%s: track %u failed\n", __thisfunc__, track);
		cd.valid = false;
		playing = false;
		return -1;
	}

	playing = true;

	/* should I pause if bgmvolume.value == 0 ? */

	return 0;
}


void CDAudio_Stop (void)
{
	if (!initialized || !enabled)
		return;

	cdRequest->headerLength = 13;
	cdRequest->unit = 0;
	cdRequest->command = COMMAND_STOP_AUDIO;
	cdRequest->status = 0;

	regs.x.ax = 0x1510;
	regs.x.cx = cdrom;
	regs.x.es = cdRequestSegment;
	regs.x.bx = cdRequestOffset;
	dos_int86 (0x2f);

	wasPlaying = playing;
	playing = false;
}


void CDAudio_Pause (void)
{
	CDAudio_Stop();
}


void CDAudio_Resume (void)
{
	if (!initialized || !enabled)
		return;

	if (!cd.valid)
		return;

	if (!wasPlaying)
		return;

	cdRequest->headerLength = 13;
	cdRequest->unit = 0;
	cdRequest->command = COMMAND_RESUME_AUDIO;
	cdRequest->status = 0;

	regs.x.ax = 0x1510;
	regs.x.cx = cdrom;
	regs.x.es = cdRequestSegment;
	regs.x.bx = cdRequestOffset;
	dos_int86 (0x2f);

	playing = true;
}


static void CD_f (void)
{
	const char	*command;
	int		ret, n;
	int		startAddress;

	if (Cmd_Argc() < 2)
		return;

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
		for (n = 0; n < 256; n++)
			remap[n] = n;
		CDAudio_Reset();
		CDAudio_GetAudioDiskInfo();
		return;
	}

	if (q_strcasecmp(command, "remap") == 0)
	{
		ret = Cmd_Argc() - 2;
		if (ret <= 0)
		{
			for (n = 1; n < 256; n++)
				if (remap[n] != n)
					Con_Printf("  %u -> %u\n", n, remap[n]);
			return;
		}
		for (n = 1; n <= ret; n++)
			remap[n] = atoi(Cmd_Argv (n+1));
		return;
	}

	if (!cd.valid)
	{
		Con_Printf("No CD in player.\n");
		return;
	}

	if (q_strcasecmp(command, "play") == 0)
	{
		CDAudio_Play(atoi(Cmd_Argv (2)), false);
		return;
	}

	if (q_strcasecmp(command, "loop") == 0)
	{
		CDAudio_Play(atoi(Cmd_Argv (2)), true);
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
		cd.valid = false;
		return;
	}

	if (q_strcasecmp(command, "info") == 0)
	{
		Con_Printf("%u tracks\n", cd.highTrack - cd.lowTrack + 1);
		for (n = cd.lowTrack; n <= cd.highTrack; n++)
		{
			ret = CDAudio_GetAudioTrackInfo (n, &startAddress);
			Con_Printf("Track %2u: %s at %2u:%02u\n", n, ret ? "data " : "music", (startAddress >> 16) & 0xff, (startAddress >> 8) & 0xff);
		}
		if (playing)
			Con_Printf("Currently %s track %u\n", playLooping ? "looping" : "playing", playTrack);
		Con_Printf("Volume is %f\n", bgmvolume.value);
		CDAudio_MediaChange();
		Con_Printf("Status %04x\n", cdRequest->status & 0xffff);
		return;
	}
}


void CDAudio_Update (void)
{
	int		ret;
	static double	lastUpdate;

	if (!initialized || !enabled)
		return;

	if ((realtime - lastUpdate) < 0.25)
		return;
	lastUpdate = realtime;

	if (mediaCheck)
	{
		static double	lastCheck;

		if ((realtime - lastCheck) < 5.0)
			return;
		lastCheck = realtime;

		ret = CDAudio_MediaChange();
		if (ret == MEDIA_CHANGED)
		{
			Con_DPrintf("CDAudio: media changed\n");
			playing = false;
			wasPlaying = false;
			cd.valid = false;
			CDAudio_GetAudioDiskInfo();
			return;
		}
	}

	if (old_cdvolume != bgmvolume.value)
		CDAudio_SetVolume (bgmvolume.value);

	if (playing)
	{
		CDAudio_GetAudioStatus();
		if ((cdRequest->status & STATUS_BUSY_BIT) == 0)
		{
			playing = false;
			if (playLooping)
				CDAudio_Play(playTrack, true);
		}
	}
}


static byte get_cddev_arg (const char *arg)
{
/* arg should be like "D", "D:" or "D:\", make
 * sure it is so. Also check if this is really
 * a CDROM drive. */
	byte drivenum;
	__dpmi_regs r;

	if (!arg || ! *arg)
		return 0xff;
	if (arg[1] != '\0')
	{
		if (arg[1] != ':')
			return 0xff;
		if (arg[2] != '\0')
		{
			if (arg[2] != '\\' &&
			    arg[2] != '/')
				return 0xff;
			if (arg[3] != '\0')
				return 0xff;
		}
	}

	drivenum = *arg;
	if (drivenum >= 'A' && drivenum <= 'Z')
		drivenum -= 'A';
	else if (drivenum >= 'a' && drivenum <= 'z')
		drivenum -= 'a';
	else
		return 0xff;

	r.x.ax = 0x150b;
	r.x.cx = drivenum;	/* 0 = A: */
	__dpmi_int(0x2f, &r);
	if (r.x.ax != 0 /*&& r.x.bx == 0xadad*/)
		return drivenum;

	Con_Printf("%c: is not a CDROM drive\n", drivenum + 'A');
	return 0xff;
}

static const byte *get_cdroms_list (void)
{
	__dpmi_regs r;

	memset (cdroms_list, 0, sizeof(cdroms_list));
	r.x.ax = 0x150d;
	r.x.bx = __tb & 0x0f;
	r.x.es = (__tb >> 4) & 0xffff;
	if (__dpmi_int(0x2f, &r) != 0)
		return NULL;

	dosmemget(__tb, numcdroms, cdroms_list);
	return cdroms_list;
}

int CDAudio_Init (void)
{
	char	*memory;
	int		n;

	if (safemode || COM_CheckParm("-nocdaudio") || COM_CheckParm("-nocd"))
		return -1;

	if (COM_CheckParm("-cdmediacheck"))
		mediaCheck = true;

	regs.x.ax = 0x1500;
	regs.x.bx = 0;
	dos_int86 (0x2f);
	if (regs.x.bx == 0)
	{
		Con_Printf ("MSCDEX not loaded, CD Audio is disabled.\n");
		return -1;
	}

	numcdroms = regs.x.bx;
	firstcdrom = regs.x.cx;
	cdrom = firstcdrom;

	regs.x.ax = 0x150c;
	regs.x.bx = 0;
	dos_int86 (0x2f);
	if (regs.x.bx == 0)
	{
		Con_Printf("%s: MSCDEX version 2.00 or later required.\n", __thisfunc__);
		return -1;
	}

	if (!get_cdroms_list ())
	{
		cdroms_list[0] = firstcdrom;
		Con_Printf("%s: Couldn't get available CD drive letters\n", __thisfunc__);
	}

	Con_DPrintf("%s: %u CD-ROM drive(s) available:", __thisfunc__, numcdroms);
	for (n = 0; n < numcdroms && cdroms_list[n]; n++)
		Con_DPrintf (" %c", cdroms_list[n] + 'A');
	Con_DPrintf(".\n");

	if ((n = COM_CheckParm("-cddev")) != 0 && n < com_argc - 1)
	{
		cdrom = get_cddev_arg(com_argv[n + 1]);
		if (cdrom == 0xff)
		{
			Con_Printf("Invalid argument to -cddev\n");
			return -1;
		}
	}
	Con_Printf("%s: Using CD-ROM drive %c:\n", __thisfunc__, cdrom + 'A');

	memory = (char *) dos_getmemory(sizeof(struct cd_request) + sizeof(union readInfo_u));
	if (memory == NULL)
	{
		Con_DPrintf("%s: Unable to allocate low memory.\n", __thisfunc__);
		return -1;
	}

	cdRequest = (struct cd_request *)memory;
	cdRequestSegment = ptr2real(cdRequest) >> 4;
	cdRequestOffset = ptr2real(cdRequest) & 0xf;

	readInfo = (union readInfo_u *)(memory + sizeof(struct cd_request));
	readInfoSegment = ptr2real(readInfo) >> 4;
	readInfoOffset = ptr2real(readInfo) & 0xf;

	for (n = 0; n < 256; n++)
		remap[n] = n;
	initialized = true;
	old_cdvolume = bgmvolume.value;

	CD_SetVolume (255);
	if (CDAudio_GetAudioDiskInfo())
	{
		Con_Printf("%s: No CD in player.\n", __thisfunc__);
		enabled = false;
	}

	Cmd_AddCommand ("cd", CD_f);

	Con_Printf("CD Audio Initialized\n");

	return 0;
}


void CDAudio_Shutdown (void)
{
	if (!initialized)
		return;
	CDAudio_Stop();
	CD_SetVolume (255);
}

