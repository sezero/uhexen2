/* midi_camd.c -- MIDI module for Amiga using CAMD
 * Based on PlayMF by Dan Baker, Christian Buchner
 *
 * Copyright (C) 2017 Szilard Biro <col.lawrence@gmail.com>
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
#include "bgmusic.h"
#include "midi_drv.h"

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/realtime.h>
#include <dos/dostags.h>
#include <libraries/realtime.h>
#include <libraries/iffparse.h> /* MAKE_ID */
#include <clib/alib_protos.h>

#ifdef __MORPHOS__
#define USE_INLINE_STDARG
#endif
#include <proto/camd.h>
#include <midi/camd.h>
#include <midi/mididefs.h>

#include <SDI/SDI_compiler.h> /* IPTR */


/* prototypes of functions exported to BGM: */
static void *MIDI_Play (const char *filename);
static void MIDI_Update (void **handle);
static void MIDI_Rewind (void **handle);
static void MIDI_Stop (void **handle);
static void MIDI_Pause (void **handle);
static void MIDI_Resume (void **handle);
static void MIDI_SetVolume (void **handle, float value);

static midi_driver_t midi_amiga_camd =
{
	false, /* init success */
	"CAMD MIDI for Amiga",
	MIDI_Init,
	MIDI_Cleanup,
	MIDI_Play,
	MIDI_Update,
	MIDI_Rewind,
	MIDI_Stop,
	MIDI_Pause,
	MIDI_Resume,
	MIDI_SetVolume,
	NULL
};

#define ID_MTHD MAKE_ID('M','T','h','d')
#define ID_MTRK MAKE_ID('M','T','r','k')

struct SMFHeader
{
	LONG  ChunkID;  /* 4 ASCII characters */
	LONG  VarLeng;
	WORD  Format;
	UWORD Ntrks;
	WORD  Division;
};

typedef enum
{
	Event_ignore,
	Event_playable,
	Event_sysex,
	Event_tempo,
	Event_trackend
} Eventtype;

struct DecTrack
{
	UWORD tracknum;  /* number of this track */
	BOOL  trackend;  /* end of track flag */
	ULONG absdelta;  /* 32-bit delta */
	ULONG nexclock;  /* storage */
	UBYTE status;   /* status from file */
	UBYTE rstatus;  /* running status from track */
	UBYTE comsize;  /* size of current command */
	UBYTE d1;     /* data byte 1 */
	UBYTE d2;     /* data byte 2 */
	UBYTE *endmarker;
	Eventtype eventtype;
	UBYTE *eventptr;
};

struct SysEx
{
	struct MinNode se_node;	/* node for linking */
	//ULONG se_size;			/* sysex size */
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
	UBYTE se_data[];		/* sysex data */
#else
	UBYTE se_data[0];		/* sysex data */
#endif
};

#define MAXTRAX 40
#define MIDIBUFSIZE 256

struct Global
{
	UBYTE trackct;
	UBYTE donecount;

	UBYTE masterswitch;

	ULONG division;
	ULONG tempo;

	ULONG lastclock;
	//ULONG abstimeHI, abstimeLO;
	uint64_t abstime;

	//struct DecTrack *pDTrack[MAXTRAX];
	struct DecTrack dtrack[MAXTRAX];
	UBYTE *ptrack[MAXTRAX];
	UBYTE *ptrackstart[MAXTRAX];
	UBYTE *ptrackend[MAXTRAX];

	ULONG fillclock[2];
	ULONG fillstate[2];
	ULONG buftempo[2];

	UBYTE lastRSchan;

	/* These buffers hold the notes translated */
	/* from the midifile file for playback */
	UBYTE pfillbuf[2][MIDIBUFSIZE];

	struct MinList SysExList[2];
};

static UBYTE CommandSize[7]=
{
	2, /* 80-8f */
	2, /* 90-9f */
	2, /* a0-af */
	2, /* b0-bf */
	1, /* c0-cf */
	1, /* d0-df */
	2, /* e0-ef */
};

static UBYTE CommonSize[8]=
{
	0, /* f0 */
	1, /* f1 */
	2, /* f2 */
	1, /* f3 */
	0, /* f4 */
	0, /* f5 */
	0, /* f6 */
	0, /* f7 */
};

static UBYTE ProgramReset[] =
{
	MS_Prog |  0, 0,
	MS_Prog |  1, 0,
	MS_Prog |  2, 0,
	MS_Prog |  3, 0,
	MS_Prog |  4, 0,
	MS_Prog |  5, 0,
	MS_Prog |  6, 0,
	MS_Prog |  7, 0,
	MS_Prog |  8, 0,
	MS_Prog |  9, 0,
	MS_Prog | 10, 0,
	MS_Prog | 11, 0,
	MS_Prog | 12, 0,
	MS_Prog | 13, 0,
	MS_Prog | 14, 0,
	MS_Prog | 15, 0
};

static UBYTE ResetAllControllers[] =
{
	MS_Ctrl | 0,  MM_ResetCtrl, 0,
	MS_Ctrl | 1,  MM_ResetCtrl, 0,
	MS_Ctrl | 2,  MM_ResetCtrl, 0,
	MS_Ctrl | 3,  MM_ResetCtrl, 0,
	MS_Ctrl | 4,  MM_ResetCtrl, 0,
	MS_Ctrl | 5,  MM_ResetCtrl, 0,
	MS_Ctrl | 6,  MM_ResetCtrl, 0,
	MS_Ctrl | 7,  MM_ResetCtrl, 0,
	MS_Ctrl | 8,  MM_ResetCtrl, 0,
	MS_Ctrl | 9,  MM_ResetCtrl, 0,
	MS_Ctrl | 10, MM_ResetCtrl, 0,
	MS_Ctrl | 11, MM_ResetCtrl, 0,
	MS_Ctrl | 12, MM_ResetCtrl, 0,
	MS_Ctrl | 13, MM_ResetCtrl, 0,
	MS_Ctrl | 14, MM_ResetCtrl, 0,
	MS_Ctrl | 15, MM_ResetCtrl, 0
};

static UBYTE AllNotesOff[] =
{
	MS_Ctrl | 0,  MM_AllOff, 0,
	MS_Ctrl | 1,  MM_AllOff, 0,
	MS_Ctrl | 2,  MM_AllOff, 0,
	MS_Ctrl | 3,  MM_AllOff, 0,
	MS_Ctrl | 4,  MM_AllOff, 0,
	MS_Ctrl | 5,  MM_AllOff, 0,
	MS_Ctrl | 6,  MM_AllOff, 0,
	MS_Ctrl | 7,  MM_AllOff, 0,
	MS_Ctrl | 8,  MM_AllOff, 0,
	MS_Ctrl | 9,  MM_AllOff, 0,
	MS_Ctrl | 10, MM_AllOff, 0,
	MS_Ctrl | 11, MM_AllOff, 0,
	MS_Ctrl | 12, MM_AllOff, 0,
	MS_Ctrl | 13, MM_AllOff, 0,
	MS_Ctrl | 14, MM_AllOff, 0,
	MS_Ctrl | 15, MM_AllOff, 0
};

static UBYTE AllSoundsOff[] =
{
	MS_Ctrl | 0,  0x78, 0,
	MS_Ctrl | 1,  0x78, 0,
	MS_Ctrl | 2,  0x78, 0,
	MS_Ctrl | 3,  0x78, 0,
	MS_Ctrl | 4,  0x78, 0,
	MS_Ctrl | 5,  0x78, 0,
	MS_Ctrl | 6,  0x78, 0,
	MS_Ctrl | 7,  0x78, 0,
	MS_Ctrl | 8,  0x78, 0,
	MS_Ctrl | 9,  0x78, 0,
	MS_Ctrl | 10, 0x78, 0,
	MS_Ctrl | 11, 0x78, 0,
	MS_Ctrl | 12, 0x78, 0,
	MS_Ctrl | 13, 0x78, 0,
	MS_Ctrl | 14, 0x78, 0,
	MS_Ctrl | 15, 0x78, 0
};

struct Library *CamdBase;
#ifdef PLATFORM_AMIGAOS3
struct RealTimeBase *RealTimeBase;
#else
struct Library *RealTimeBase;
#endif
static qboolean	midi_playing, midi_paused;
static UBYTE *smfdata;
static struct Global *glob;
static struct MidiNode *pMidiNode;
static struct MidiLink *pMidiLink;
static struct Task *playerTask;
static struct Task *parentTask;
static struct Player *pPlayer;

#define CHECK_MIDI_ALIVE()		\
do {					\
	if (!midi_playing)		\
	{				\
		if (handle)		\
			*handle = NULL;	\
		return;			\
	}				\
} while (0)

static void MIDI_SetVolume (void **handle, float value)
{
	CHECK_MIDI_ALIVE();

}

static void MIDI_Rewind (void **handle)
{
	CHECK_MIDI_ALIVE();
	// mididrv_rewind unused
}

static void MIDI_Update (void **handle)
{
	CHECK_MIDI_ALIVE();
	// mididrv_advance nothing to do here
}

static char *MIDI_GetDeviceName(char *dst, size_t dstsize)
{
	APTR key;
	struct MidiCluster *cluster;
	char *retname = NULL;

	key = LockCAMD(CD_Linkages);
	if (key)
	{
		cluster = NextCluster(NULL);
		while (cluster)
		{
			//Con_Printf("%s CAMD device: %s\n", __thisfunc__, cluster->mcl_Node.ln_Name);
			if (strstr(cluster->mcl_Node.ln_Name, "out"))
			{
				q_strlcpy(dst, cluster->mcl_Node.ln_Name, dstsize);
				retname = dst;
				break;
			}
			cluster = NextCluster(cluster);
		}
		UnlockCAMD(key);
	}

	return retname;
}

qboolean MIDI_Init(void)
{
	static const char midi_name[] = "Hexen II Player";
	static const char mlink_comment[] = "Hexen II Player Link";
	static const char player_name[] = "Hexen II player";

	char linkName[32];

	if (midi_amiga_camd.available)
		return true;

	BGM_RegisterMidiDRV(&midi_amiga_camd);

	if (safemode || COM_CheckParm("-nomidi"))
		return false;

	if (!(CamdBase = OpenLibrary("camd.library", 37)))
	{
		Con_Printf ("Can't open camd.library\n");
		return false;
	}

#ifdef PLATFORM_AMIGAOS3
	RealTimeBase = (struct RealTimeBase *) OpenLibrary("realtime.library", 37);
#else
	RealTimeBase = OpenLibrary("realtime.library", 37);
#endif
	if (!RealTimeBase)
	{
		Con_Printf ("Can't open realtime.library\n");
		MIDI_Cleanup();
		return false;
	}

	if (!MIDI_GetDeviceName(linkName, sizeof(linkName)))
	{
		Con_Printf ("No output device found\n");
		MIDI_Cleanup();
		return false;
	}

	if (!(pMidiNode = CreateMidi(
		MIDI_Name, (IPTR)midi_name,
		MIDI_MsgQueue, 0,
		MIDI_SysExSize, 4096,
		TAG_END)))
	{
		Con_Printf("Can't create MIDI Node\n");
		MIDI_Cleanup();
		return false;
	}

	if (!(pMidiLink = AddMidiLink(
		pMidiNode, MLTYPE_Sender,
		MLINK_Comment, (IPTR)mlink_comment,
		MLINK_Parse, TRUE,
		MLINK_Location, (IPTR)linkName,
		TAG_END)))
	{
		Con_Printf("Can't create MIDI Link\n");
		MIDI_Cleanup();
		return false;
	}

	ParseMidi(pMidiLink, ProgramReset, sizeof(ProgramReset));
	Delay(5);
	ParseMidi(pMidiLink, ResetAllControllers, sizeof(ResetAllControllers));
	Delay(5);

	if (!(pPlayer = CreatePlayer(
		PLAYER_Name, (IPTR)player_name,
		PLAYER_Conductor, (IPTR)-1,
		TAG_END)))
	{
		Con_Printf("Can't create the RealTime player\n");
		MIDI_Cleanup();
		return false;
	}

	parentTask = FindTask(NULL);

	Con_Printf("%s initialized.\nMIDI link: %s\n", midi_amiga_camd.desc, linkName);

	midi_paused = false;
	midi_amiga_camd.available = true;

	return true;
}

static ULONG GetDelta(UBYTE **value)
{
	ULONG newval = 0;
	UWORD i;
	UBYTE dat;

	for (i = 0; i < 4; i++) {
		dat = *((*value)++);
		newval = newval<<7;
		newval |= dat & 0x7f;
		if (dat < 0x80) break;
	}
	return newval;
}

static UBYTE *DecodeEvent(UBYTE *ptdata, struct DecTrack *pDT)
{
	ULONG length;
	BOOL skipit;

	UBYTE status;
	UBYTE data;
	UBYTE comsize;

	pDT->absdelta = 0;

	/* is this track all used up? */
	if (pDT->trackend)
		return NULL;

	if (ptdata >= pDT->endmarker)
	{
		Con_DPrintf("Warning: missing proper track end marker in track %d.\n", pDT->tracknum);

		pDT->eventptr = ptdata;
		pDT->eventtype = Event_trackend;
		pDT->trackend = TRUE;
		return ptdata;
	}

	skipit = FALSE;
	do
	{
		/* Decode delta */
		ULONG delta = GetDelta(&ptdata);
		pDT->absdelta += delta;
		pDT->nexclock += delta;

		pDT->eventtype = Event_ignore;

		data = *ptdata;

		if (data & 0x80) /* Event with status ($80-$FF): decode new status */
		{
			ptdata++;

			if (data < 0xf0)	/* "Normal" events? */
			{
				status  = data;
				comsize = CommandSize[(data&0x7f)>>4];

				pDT->status  = status;
				pDT->comsize = comsize;
				pDT->rstatus = 0;	 /* No running status was used */

				pDT->eventtype = Event_playable;
				skipit = FALSE;
			}
			else
			{
				if (data < 0xf8)	/* System Common Event? */
				{
					status  = data;
					comsize = CommonSize[status-0xf0];

					skipit = TRUE;

					if (status==0xf0 || status==0xf7)   /* It's a sysex event */
					{
						pDT->eventptr = ptdata-1;
						pDT->eventtype = Event_sysex;
						skipit = FALSE;

						length = GetDelta(&ptdata);
						ptdata += length;
					}
				}
				else
				{
					status  = data;
					comsize = 0;

					skipit = TRUE;

					if (data == 0xff)	/* It's a meta event ($ff) */
					{
						UBYTE metatype;

						metatype = *(ptdata++);

						if (metatype == 0x2F)		/* track end marker */
						{
							pDT->eventptr = ptdata;
							pDT->eventtype = Event_trackend;
							pDT->trackend = TRUE;
							skipit = FALSE;
						}
						else
						{
							if (metatype == 0x51)	/* Tempo change */
							{
								pDT->eventptr = ptdata;
								pDT->eventtype = Event_tempo;
								skipit = FALSE;
							}

							length = GetDelta(&ptdata);
							ptdata += length;
						}
					}
				}
			}
		}
		else /* Event without status ($00-$7F): use running status */
		{
			status = pDT->status;

			if (status == 0)
			{
				Con_DPrintf("Warning: Data bytes without initial status in track %d.\n", pDT->tracknum);
				comsize = 1;
				skipit = TRUE;
			}
			else
			{
				skipit = FALSE;
				comsize = pDT->comsize;
				pDT->rstatus = status;
				pDT->eventtype = Event_playable;
			}
		}

		if (comsize > 0)
			pDT->d1 = *ptdata++;
		else
			pDT->d1 = 0;

		if (comsize > 1)
			pDT->d2 = *ptdata++;
		else
			pDT->d2 = 0;
	}
	while (skipit);

	return ptdata;
}

static void CollectEvents(struct Global *glob)
{
	UWORD track;
	ULONG lowclock;
	UBYTE mswitch = glob->masterswitch;
	ULONG nexttempo = glob->tempo;
	ULONG pos = 0;
	ULONG delta = 0;

	if (glob->donecount < glob->trackct)
	{
		lowclock = 0xffffffff;

		for (track = 0; track < glob->trackct; track++)
		{
			if (glob->dtrack[track].nexclock < lowclock && glob->ptrack[track])
				lowclock = glob->dtrack[track].nexclock;
		}

		delta = lowclock - glob->lastclock;
		glob->lastclock = lowclock;

		for (track = 0; track < glob->trackct; track++)
		{
			struct DecTrack *pDT = &glob->dtrack[track];

			if ((pDT->nexclock == lowclock) && glob->ptrack[track])
			{
				do
				{
					/* Transfer event to parse buffer and handle successor */
					switch (pDT->eventtype)
					{
						case Event_playable:
						{
							if (pos >= MIDIBUFSIZE-3)
							{
								Con_DPrintf("MIDI buffer overflow (>=%d), dropped event\n", MIDIBUFSIZE);
								break;
							}
							if (pDT->rstatus == glob->lastRSchan)
							{
								/* Running status */
								if (pDT->comsize>0) *(glob->pfillbuf[mswitch] + pos++) = pDT->d1;
								if (pDT->comsize>1) *(glob->pfillbuf[mswitch] + pos++) = pDT->d2;
							}
							else
							{
								/* New status so store status and data bytes */
								*(glob->pfillbuf[mswitch] + pos++) = pDT->status;
								glob->lastRSchan = pDT->status;

								if (pDT->comsize>0) *(glob->pfillbuf[mswitch] + pos++) = pDT->d1;
								if (pDT->comsize>1) *(glob->pfillbuf[mswitch] + pos++) = pDT->d2;
	 						}
						}
						break;

						case Event_sysex:
						{
							/* Link SysEx into Queue */
							UBYTE *src = pDT->eventptr;
							UBYTE  hdr = *src++;
							struct SysEx *se = NULL;

							if (hdr == 0xf0)
							{
								ULONG length = GetDelta(&src);

								if ((se = (struct SysEx *) calloc(1, sizeof(struct SysEx)+length+1)) != NULL) {
									UBYTE *dst = se->se_data;
									//se->se_size = length+1;
									*dst++ = 0xf0;
									while (length--) *dst++ = *src++;
								}
							}

							if (hdr == 0xf7)
							{
								ULONG length = GetDelta(&src);

								if ((se = (struct SysEx *) calloc(1, sizeof(struct SysEx)+length+1)) != NULL) {
									UBYTE *dst = se->se_data;
									//se->se_size = length;
									while (length--) *dst++ = *src++;
								}
							}

							if (se) {
								AddTail((struct List *)&glob->SysExList[mswitch], (struct Node *)se);
							}
						}
						break;

						case Event_tempo:
						{
							UBYTE *meta = pDT->eventptr;
							LONG metalen = GetDelta(&meta);

							nexttempo = 0;
							while (metalen-- > 0)
								nexttempo = (nexttempo << 8) | *(meta++);

							//Message("Tempo meta-event (%ld BPM) at nexclock %ld!", NULL, 60*1000000/nexttempo, pDT->nexclock);
						}
						break;

						case Event_trackend:
						{
							glob->donecount++;
						}
						break;

						default:
						{
							Con_DPrintf("Unknown MIDI event $%02x\n", pDT->eventtype);
						}
						break;
					}

					glob->ptrack[track] = DecodeEvent(glob->ptrack[track], pDT);
				} while ((pDT->absdelta == 0) && glob->ptrack[track]);
			}
		}
	}

	glob->fillstate[mswitch] = pos;
	glob->buftempo[mswitch] = nexttempo;

	//AddAbsTime(&glob->abstimeLO, &glob->abstimeHI, glob->tempo, delta, glob->division);
	glob->abstime += ((uint64_t)glob->tempo*delta)/glob->division;
	//glob->fillclock[mswitch] = CalcFillClock(glob->abstimeLO, glob->abstimeHI, 833);
	glob->fillclock[mswitch] = (ULONG)(glob->abstime / 833);

	glob->tempo = nexttempo;
}

static void PlayerFunc(void)
{
	ULONG sigs;
	struct SysEx *se, *nse;

	SetTaskPri(FindTask(NULL), 25);

	/* Transfer the events the fist buffer */
	glob->masterswitch = 0;
	CollectEvents(glob);

	while (TRUE)
	{
		/* Set the alarm */
		if (!SetPlayerAttrs(pPlayer,
			PLAYER_AlarmTime, glob->fillclock[glob->masterswitch],
			PLAYER_Ready, TRUE,
			TAG_END))
		{
			break;
		}

		/* Fill the other buffer */
		glob->masterswitch ^= 1;
		CollectEvents(glob);

		sigs = Wait(SIGBREAKF_CTRL_E | SIGBREAKF_CTRL_C);
		if (sigs & SIGBREAKF_CTRL_C) {
			break;
		}

		/* Send off one buffer... */
		for (se = (struct SysEx *)glob->SysExList[glob->masterswitch^1].mlh_Head;
			(nse = (struct SysEx *)se->se_node.mln_Succ) != NULL;
			se = nse)
		{
			PutSysEx(pMidiLink, se->se_data);
			Remove((struct Node *)se);
			free(se);
		}

		if (glob->fillstate[glob->masterswitch^1] != 0)
		{
			ParseMidi(pMidiLink, glob->pfillbuf[glob->masterswitch^1], glob->fillstate[glob->masterswitch^1]);
		}

		/* Loop the music */
		if (glob->donecount >= glob->trackct)
		{
			//break;
			UWORD track;
			//Con_Printf("Music loop\n");
			glob->donecount = 0;
			glob->lastclock = 0;
			for (track = 0; track < glob->trackct; track++)
			{
				glob->dtrack[track].nexclock = 0;
				glob->dtrack[track].trackend = FALSE;
				glob->ptrack[track] = DecodeEvent(glob->ptrackstart[track], &glob->dtrack[track]);
			}
			CollectEvents(glob);
		}
	}

	// let the parent thread know that we are done
	Forbid();
	midi_playing = false;
	Signal(parentTask, SIGBREAKF_CTRL_F);
}

static void *MIDI_Play (const char *filename)
{
	static const char task_name[] = "Hexen II CAMD player task";

	struct SMFHeader *hdr;
	UBYTE *pbyte;
	size_t smfdatasize;
	UWORD track;
	LONG error;
	ULONG id;

	if (!midi_amiga_camd.available)
		return NULL;

	if (!filename || !*filename)
	{
		Con_DPrintf("null music file name\n");
		return NULL;
	}

	MIDI_Stop (NULL);

	smfdata = (UBYTE *) FS_LoadMallocFile (filename, NULL);
	if (!smfdata)
	{
		Con_DPrintf("Couldn't open %s\n", filename);
		return NULL;
	}
	smfdatasize = fs_filesize;
	if (smfdatasize < 34)
	{
		Con_Printf("MIDI file too short.\n");
		MIDI_Stop (NULL);
		return false;
	}

	pbyte = smfdata;
	if (memcmp(pbyte,"RIFF",4) == 0 && memcmp(pbyte +8,"RMID",4) == 0
					&& memcmp(pbyte+12,"data",4) == 0)
		pbyte += 20; /* Microsoft RMID */

	hdr = (struct SMFHeader *)pbyte;
	hdr->ChunkID = BigLong(hdr->ChunkID);
	hdr->VarLeng = BigLong(hdr->VarLeng);
	hdr->Format = BigShort(hdr->Format);
	hdr->Ntrks = (UWORD) BigShort(hdr->Ntrks);
	hdr->Division = BigShort(hdr->Division);

	if (hdr->ChunkID != ID_MTHD || hdr->VarLeng != 6 ||
		(hdr->Format != 0 && hdr->Format != 1) ||
		!hdr->Ntrks || hdr->Ntrks > MAXTRAX || hdr->Division <= 0)
	{
		Con_Printf("Can't recognize the MIDI format.\n");
		MIDI_Stop (NULL);
		return false;
	}

	if (!(glob = (struct Global *) calloc(1, sizeof(struct Global))))
	{
		Con_DPrintf("No memory for global MIDI variables\n");
		MIDI_Stop (NULL);
		return false;
	}

	glob->division = hdr->Division;
	glob->lastRSchan = 0xf1;
	glob->tempo = 500000;

	NewList((struct List *)&glob->SysExList[0]);
	NewList((struct List *)&glob->SysExList[1]);

	glob->trackct = 0;

	while ((pbyte-smfdata < smfdatasize) && (glob->trackct < MAXTRAX))
	{
		id = (ULONG) BigLong(*(ULONG *)pbyte);
		if (id == ID_MTRK)
		{
			if (glob->trackct > 0)
				glob->ptrackend[glob->trackct-1] = pbyte;

			if (glob->trackct == hdr->Ntrks)
				break;

			glob->ptrackstart[glob->trackct] = pbyte+8;
			glob->trackct++;
			pbyte += 4;
		}
		else
		{
			pbyte++;
		}
	}

	if (glob->trackct > 0)
		glob->ptrackend[glob->trackct-1] = pbyte;

	if (glob->trackct != hdr->Ntrks)
	{
		Con_Printf("Missing tracks. Only %d tracks found (%d expected).\n", glob->trackct, hdr->Ntrks);
		MIDI_Stop (NULL);
		return false;
	}

	for (track = 0; track < glob->trackct; track++)
	{
		glob->dtrack[track].tracknum = track+1;
		glob->dtrack[track].endmarker = glob->ptrackend[track];
		glob->ptrack[track] = DecodeEvent(glob->ptrackstart[track], &glob->dtrack[track]);
	}

	/* start thread */
#ifdef __MORPHOS__
	playerTask = (struct Task *)CreateNewProcTags(
		NP_Entry, (IPTR)PlayerFunc,
		NP_CodeType, CODETYPE_PPC,
		NP_Name, (IPTR)task_name,
		TAG_DONE);
#else
	playerTask = (struct Task *)CreateNewProcTags(
		NP_Entry, (IPTR)PlayerFunc,
		NP_Name, (IPTR)task_name,
		TAG_DONE);
#endif

	if (!playerTask)
	{
		Con_Printf("Can't create the MIDI player task\n");
		MIDI_Stop (NULL);
		return false;
	}

	midi_playing = true;

	if (!SetPlayerAttrs(pPlayer,
		PLAYER_AlarmSigTask, (IPTR)playerTask,
		PLAYER_AlarmSigBit, SIGBREAKB_CTRL_E,
		//PLAYER_Ready, TRUE,
		PLAYER_ErrorCode, (IPTR)&error,
		TAG_END))
	{
		Con_Printf("Can't set the RealTime player attrs, error %ld\n", (long)error);
		MIDI_Stop (NULL);
		return false;
	}

	SetConductorState(pPlayer, CONDSTATE_RUNNING, 0);

	Con_Printf ("Started midi music %s\n", filename);

	return glob;
}

static void MIDI_Pause (void **handle)
{
	CHECK_MIDI_ALIVE();
	SetConductorState(pPlayer, CONDSTATE_PAUSED, 0);
	ParseMidi(pMidiLink, AllNotesOff, sizeof(AllNotesOff));
	ParseMidi(pMidiLink, AllSoundsOff, sizeof(AllSoundsOff));
}

static void MIDI_Resume (void **handle)
{
	CHECK_MIDI_ALIVE();
	SetConductorState(pPlayer, CONDSTATE_RUNNING, 0);
}

static void MIDI_Stop (void **handle)
{
	//CHECK_MIDI_ALIVE();

	if (pPlayer)
	{
		SetConductorState(pPlayer, CONDSTATE_STOPPED, 0);
		SetPlayerAttrs(pPlayer, PLAYER_Ready, FALSE, TAG_END);
	}

	if (pMidiLink)
	{
		ParseMidi(pMidiLink, AllNotesOff, sizeof(AllNotesOff));
		ParseMidi(pMidiLink, AllSoundsOff, sizeof(AllSoundsOff));
		Delay(10);
	}

	if (playerTask)
	{
		if (midi_playing)
		{
			SetSignal(SIGBREAKF_CTRL_F, 0);
			Signal(playerTask, SIGBREAKF_CTRL_C);
			Wait(SIGBREAKF_CTRL_F);
		}
		playerTask = NULL;
	}

	if (glob)
	{
		struct MinNode *se, *nse;

		for (se = glob->SysExList[0].mlh_Head; (nse = se->mln_Succ) != NULL; se = nse)
		{
			Remove((struct Node *)se);
			free(se);
		}
		for (se = glob->SysExList[1].mlh_Head; (nse = se->mln_Succ) != NULL; se = nse)
		{
			Remove((struct Node *)se);
			free(se);
		}

		free(glob);
		glob = NULL;
	}

	if (smfdata)
	{
		free(smfdata);
		smfdata = NULL;
	}
}

void MIDI_Cleanup(void)
{
	if (midi_amiga_camd.available)
	{
		midi_amiga_camd.available = false;

		MIDI_Stop (NULL);
	}

	if (pMidiLink)
	{
		RemoveMidiLink(pMidiLink);
		pMidiLink = NULL;
	}

	if (pMidiNode)
	{
		DeleteMidi(pMidiNode);
		pMidiNode = NULL;
	}

	if (pPlayer)
	{
		DeletePlayer(pPlayer);
		pPlayer = NULL;
	}

	if (RealTimeBase)
	{
		CloseLibrary((struct Library *)RealTimeBase);
		RealTimeBase = NULL;
	}

	if (CamdBase)
	{
		CloseLibrary(CamdBase);
		CamdBase = NULL;
	}
}
