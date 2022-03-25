/*
 * Sound support using Amiga Paula, based on original work by
 * Frank Wille <frank@phoenix.owl.de>
 * Adapted to uHexen2 by Szilard Biro <col.lawrence@gmail.com>
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
 *
 */

#include "quakedef.h"
#include "snd_sys.h"

#if HAVE_PAULA_SOUND

#include "snd_paula.h"

#include <exec/memory.h>
#include <exec/tasks.h>
#include <exec/interrupts.h>
#include <exec/libraries.h>
#include <devices/audio.h>
#include <hardware/custom.h>
#include <hardware/cia.h>
#include <hardware/dmabits.h>
#include <hardware/intbits.h>
#include <graphics/gfxbase.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/timer.h>
#include <proto/graphics.h>

#define CLOCK_PAL       3546895
#define CLOCK_NTSC      3579545

static char s_paula_driver[] = "Paula";

#define NSAMPLES 0x8000

/* custom chip base address */
static volatile struct Custom *custom = (struct Custom *)0xdff000;

/* CIA A base address */
static volatile struct CIA *ciaa = (struct CIA *)0xbfe001;

static unsigned char *dmabuf = NULL;
static BYTE audio_dev = -1;
static double speed;
static struct MsgPort *audioport=NULL;
static struct IOAudio *audioio=NULL;
static struct Interrupt AudioInt;
static struct Interrupt *OldInt;
static UBYTE saved_filter;
static double aud_start_time;

static void AudioIntCode(void)
{
	aud_start_time = Sys_DoubleTime();
	custom->intreq = INTF_AUD0;
}

static qboolean S_PAULA_Init(dma_t *dma)
{
	UBYTE channelalloc[] = {1|2};
	UWORD period, length;
	LONG sysclock;
	UWORD *leftptr, *rightptr;

	shm = dma;
	memset ((void *) dma, 0, sizeof(dma_t));

	/* set the clock constant */
	if (GfxBase->DisplayFlags & REALLY_PAL)
		sysclock = CLOCK_PAL;
	else
		sysclock = CLOCK_NTSC;

	period = (UWORD)((float)sysclock / (float)desired_speed + 0.5f);

	/* allocate dma buffer */
	if (!(dmabuf = AllocVec(NSAMPLES, MEMF_CHIP|MEMF_PUBLIC|MEMF_CLEAR))) {
		Con_Printf("Paula: Can't allocate the DMA buffer\n");
		return false;
	}

	/* init shm */
	shm->buffer = dmabuf;
	shm->channels = desired_channels;
	if (shm->channels > 2) shm->channels = 2;
	shm->speed = sysclock/(LONG)period;
	shm->samplebits = 8;
	shm->samples = NSAMPLES;
	shm->signed8 = 2; // signed + de-interleaved
	speed = (double)(shm->speed * shm->channels);

	/* open audio.device */
	if ((audioport = CreateMsgPort())) {
		if ((audioio = (struct IOAudio *)CreateIORequest(audioport, sizeof(struct IOAudio)))) {
			audioio->ioa_Request.io_Message.mn_Node.ln_Pri = ADALLOC_MAXPREC;
			audioio->ioa_Request.io_Command = ADCMD_ALLOCATE;
			audioio->ioa_Request.io_Flags = ADIOF_NOWAIT;
			audioio->ioa_AllocKey = 0;
			audioio->ioa_Data = channelalloc;
			audioio->ioa_Length = sizeof(channelalloc);
			audio_dev = OpenDevice((STRPTR)AUDIONAME, 0, &audioio->ioa_Request, 0);
		}
	}

	if (audio_dev) {
		Con_Printf("Paula: Couldn't open audio.device\n");
		return false;
	}

	/* disable the DMA and interrupts */
	custom->dmacon = DMAF_AUD0|DMAF_AUD1;
	custom->intena = INTF_AUD0;
	custom->intreq = INTF_AUD0;

	/* set up audio interrupt */
	AudioInt.is_Node.ln_Type = NT_INTERRUPT;
	AudioInt.is_Node.ln_Pri = 100;
	//AudioInt.is_Data = (APTR)&aud_start_time;
	AudioInt.is_Code = (void(*)())AudioIntCode;
	OldInt = SetIntVector(INTB_AUD0,&AudioInt);

	if (shm->channels == 2) {
		length = NSAMPLES / sizeof(WORD) / 2;
		leftptr = (UWORD *)dmabuf;
		rightptr = (UWORD *)(dmabuf + (NSAMPLES / 2));
	} else {
		length = NSAMPLES / sizeof(WORD);
		leftptr = rightptr = (UWORD *)dmabuf;
	}

	/* set up left channel */
	custom->aud[0].ac_len = length;
	custom->aud[0].ac_per = period;
	custom->aud[0].ac_vol = 64;
	custom->aud[0].ac_ptr = leftptr;

	/* set up right channel */
	custom->aud[1].ac_len = length;
	custom->aud[1].ac_per = period;
	custom->aud[1].ac_vol = 64;
	custom->aud[1].ac_ptr = rightptr;

	aud_start_time = 0;

	/* enable the DMA and interrupts */
	custom->intena = INTF_SETCLR|INTF_INTEN|INTF_AUD0;
	custom->dmacon = DMAF_SETCLR|DMAF_AUD0|DMAF_AUD1;

	saved_filter = ciaa->ciapra & CIAF_LED;
	ciaa->ciapra |= CIAF_LED;
	Con_Printf ("Paula initialized\n");

	return true;
}

static int S_PAULA_GetDMAPos(void)
{
	int pos = (int)((Sys_DoubleTime()-aud_start_time)*speed);
	pos &= NSAMPLES-1;

	return pos;
}

static void S_PAULA_Shutdown(void)
{
	if (!saved_filter) {
		ciaa->ciapra &= ~CIAF_LED;
	}
	if (OldInt) {
		custom->dmacon = DMAF_AUD0|DMAF_AUD1;
		custom->intena = INTF_AUD0;
		SetIntVector(INTB_AUD0,OldInt);
		OldInt = NULL;
	}

	if (audio_dev == 0) {
		/* this is probably overkill */
		audioio->ioa_Request.io_Command = CMD_RESET;
		DoIO((struct IORequest *)audioio);
		CloseDevice((struct IORequest *)audioio);
	}

	if (audioio) {
		DeleteIORequest((struct IORequest *)audioio);
		audioio = NULL;
	}
	if (audioport) {
		DeleteMsgPort(audioport);
		audioport = NULL;
	}

	if (dmabuf) {
		FreeVec(dmabuf);
		dmabuf = NULL;
	}

	shm = NULL;
}

static void S_PAULA_LockBuffer (void)
{
}

static void S_PAULA_BlockSound (void)
{
}

static void S_PAULA_UnblockSound (void)
{
}

static void S_PAULA_Submit(void)
{
}

snd_driver_t snddrv_paula =
{
	S_PAULA_Init,
	S_PAULA_Shutdown,
	S_PAULA_GetDMAPos,
	S_PAULA_LockBuffer,
	S_PAULA_Submit,
	S_PAULA_BlockSound,
	S_PAULA_UnblockSound,
	s_paula_driver,
	SNDDRV_ID_PAULA,
	false,
	NULL
};

#endif	/* HAVE_PAULA_SOUND */

