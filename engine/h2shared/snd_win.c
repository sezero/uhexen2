/*
 * snd_win.c
 * $Id$
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

#include "quakedef.h"
#include "snd_sys.h"

#if HAVE_WIN_SOUND

#include "winquake.h"
#include "snd_win.h"
#include <mmsystem.h>

static char s_wv_driver[] = "Windows WAVE";

//#define SNDBUFSIZE		65536
// 64K is > 1 second at 16-bit, 22050 Hz
//#define	WAV_BUFFERS		64
#define	WAV_BUFFERS		128
#define	WAV_MASK		(WAV_BUFFERS - 1)

static int	sample16;
static int	snd_sent, snd_completed;
static int	wv_buf_size;

/* whether to use hunk for wave sound memory, either 1 or 0  */
#define USE_HUNK_ALLOC		0

#if USE_HUNK_ALLOC
static int	allocMark = 0;
#else
static HANDLE	hData;
static HGLOBAL	hWaveHdr;
#endif

static HPSTR	lpData;
static LPWAVEHDR	lpWaveHdr;
static HWAVEOUT	hWaveOut;
//WAVEOUTCAPS	wavecaps;

static DWORD	gSndBufSize;


/*
==================
FreeSound
==================
*/
static void FreeSound (void)
{
	int		i;

	if (hWaveOut)
	{
		waveOutReset (hWaveOut);

		if (lpWaveHdr)
		{
			for (i = 0; i < WAV_BUFFERS; i++)
				waveOutUnprepareHeader (hWaveOut, lpWaveHdr+i, sizeof(WAVEHDR));
		}

		waveOutClose (hWaveOut);

#if USE_HUNK_ALLOC
/* These are now on the hunk and we have to be wary about deallocating them:
   Other stuff might have been allocated above.  A nonzero allocMark is the
   case only if wave init failed, and we are here immediately after that,
   in which case it's safe to free whatever we allocated. In any other case,
   allocMark will be 0 and no action is performed.	Pa3PyX	*/
		if (allocMark)
		{
			Hunk_FreeToLowMark(allocMark);
			allocMark = 0;
		}
#else
		if (hWaveHdr)
		{
			GlobalUnlock(hWaveHdr);
			GlobalFree(hWaveHdr);
		}
		if (hData)
		{
			GlobalUnlock(hData);
			GlobalFree(hData);
		}
#endif
	}

	hWaveOut = 0;
#if !USE_HUNK_ALLOC
	hData = 0;
	hWaveHdr = 0;
#endif
	lpData = NULL;
	lpWaveHdr = NULL;
}


/*
==================
SNDDM_InitWav

Crappy windows multimedia base
==================
*/
static qboolean S_WIN_Init (dma_t *dma)
{
	WAVEFORMATEX	format;
	int			i;
	HRESULT			hr;

	snd_sent = 0;
	snd_completed = 0;

	memset((void *) dma, 0, sizeof(dma_t));
	shm = dma;

	shm->channels = desired_channels;
	shm->samplebits = desired_bits;
	shm->speed = desired_speed;
	/* Calculate Wave buffer size to store 2 secs
	 * of data, round up to the next power of 2.  */
	wv_buf_size = 1 << (Q_log2((desired_speed << 3) / WAV_BUFFERS) + 1);

	memset (&format, 0, sizeof(format));
	format.wFormatTag = WAVE_FORMAT_PCM;
	format.nChannels = shm->channels;
	format.wBitsPerSample = shm->samplebits;
	format.nSamplesPerSec = shm->speed;
	format.nBlockAlign = format.nChannels * format.wBitsPerSample / 8;
	format.cbSize = 0;
	format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;

	/* Open a waveform device for output using window callback. */
	hr = waveOutOpen((LPHWAVEOUT)&hWaveOut, WAVE_MAPPER, &format, 0, 0L, CALLBACK_NULL);
	if (hr != MMSYSERR_NOERROR)
	{
		if (hr != MMSYSERR_ALLOCATED)
		{
			Con_SafePrintf ("waveOutOpen failed\n");
			return false;
		}

		Con_SafePrintf ("waveOutOpen failure, hardware already in use\n");
		return false;
	}

	/*
	 * Allocate and lock memory for the waveform data. The memory
	 * for waveform data must be globally allocated with
	 * GMEM_MOVEABLE and GMEM_SHARE flags.
	 */
	gSndBufSize = WAV_BUFFERS * wv_buf_size;
#if USE_HUNK_ALLOC
	allocMark = Hunk_LowMark();
	lpData = (HPSTR) Hunk_AllocName(gSndBufSize, "sndbuff");
#else
	hData = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, gSndBufSize);
	if (!hData)
	{
		Con_SafePrintf ("Sound: Out of memory.\n");
		goto fail;
	}
	lpData = (HPSTR) GlobalLock(hData);
	if (!lpData)
	{
		Con_SafePrintf ("Sound: Failed to lock.\n");
		goto fail;
	}
	memset (lpData, 0, gSndBufSize);
#endif

	/*
	 * Allocate and lock memory for the header. This memory must
	 * also be globally allocated with GMEM_MOVEABLE and
	 * GMEM_SHARE flags.
	 */
#if USE_HUNK_ALLOC
	lpWaveHdr = (LPWAVEHDR) Hunk_AllocName((DWORD)sizeof(WAVEHDR) * WAV_BUFFERS, "wavehdr");
#else
	hWaveHdr = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, (DWORD) sizeof(WAVEHDR) * WAV_BUFFERS);
	if (hWaveHdr == NULL)
	{
		Con_SafePrintf ("Sound: Failed to Alloc header.\n");
		goto fail;
	}
	lpWaveHdr = (LPWAVEHDR) GlobalLock(hWaveHdr);
	if (lpWaveHdr == NULL)
	{
		Con_SafePrintf ("Sound: Failed to lock header.\n");
		goto fail;
	}
	memset (lpWaveHdr, 0, sizeof(WAVEHDR) * WAV_BUFFERS);
#endif

	/* After allocation, set up and prepare headers. */
	for (i = 0; i < WAV_BUFFERS; i++)
	{
		lpWaveHdr[i].dwBufferLength = wv_buf_size;
		lpWaveHdr[i].lpData = lpData + i * wv_buf_size;

		if (waveOutPrepareHeader(hWaveOut, lpWaveHdr+i, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
		{
			Con_SafePrintf ("Sound: failed to prepare wave headers\n");
			goto fail;
		}
	}

	shm->samples = gSndBufSize / (shm->samplebits / 8);
	shm->samplepos = 0;
	shm->submission_chunk = 1;
	shm->buffer = (unsigned char *) lpData;
	sample16 = (shm->samplebits / 8) - 1;

#if USE_HUNK_ALLOC
	/* Wave init succeeded, so DO NOT attempt to deallocate sound buffers
	   from the hunk later on, otherwise we risk trashing everything that
	   was allocated after them.	Pa3PyX	*/
	allocMark = 0;
#endif

	Con_SafePrintf ("%d sound buffers, %d bytes/sound buffer\n", WAV_BUFFERS, wv_buf_size);

	Con_SafePrintf ("Wave sound initialized\n");
	return true;

fail:
	FreeSound ();

	Con_SafePrintf ("Wave sound failed to init\n");
	return false;
}

/*
==============
SNDDMA_GetDMAPos

return the current sample position (in mono samples read)
inside the recirculating dma buffer, so the mixing code will know
how many sample are required to fill it up.
===============
*/
static int S_WIN_GetDMAPos (void)
{
	int		s;

	s = snd_sent * wv_buf_size;
	s >>= sample16;
	s &= (shm->samples - 1);

	return s;
}

/*
==============
SNDDMA_LockBuffer

Makes sure dma buffer is valid
===============
*/
static void S_WIN_LockBuffer (void)
{
}

/*
==============
SNDDMA_Submit

Unlock the dma buffer /
Send sound to the device
===============
*/
static void S_WIN_Submit (void)
{
	LPWAVEHDR	h;
	int			wResult;

	//
	// find which sound blocks have completed
	//
	while (1)
	{
		if ( snd_completed == snd_sent )
		{
			Con_DPrintf ("Sound overrun\n");
			break;
		}

		if ( ! (lpWaveHdr[snd_completed & WAV_MASK].dwFlags & WHDR_DONE) )
		{
			break;
		}

		snd_completed++;	// this buffer has been played
	}

	//
	// submit two new sound blocks
	//
	while (((snd_sent - snd_completed) >> sample16) < 4)
	{
		h = lpWaveHdr + (snd_sent & WAV_MASK);

		snd_sent++;
		/*
		 * Now the data block can be sent to the output device. The
		 * waveOutWrite function returns immediately and waveform
		 * data is sent to the output device in the background.
		 */
		wResult = waveOutWrite(hWaveOut, h, sizeof(WAVEHDR));

		if (wResult != MMSYSERR_NOERROR)
		{
			Con_SafePrintf ("Failed to write block to device\n");
			FreeSound ();
			return;
		}
	}
}

/*
==================
SNDDMA_BlockSound
==================
*/
static void S_WIN_BlockSound (void)
{
	waveOutReset (hWaveOut);
}


/*
==================
SNDDMA_UnblockSound
==================
*/
static void S_WIN_UnblockSound (void)
{
}


/*
==============
SNDDMA_Shutdown

Reset the sound device for exiting
===============
*/
static void S_WIN_Shutdown (void)
{
	FreeSound ();
}

snd_driver_t snddrv_win =
{
	S_WIN_Init,
	S_WIN_Shutdown,
	S_WIN_GetDMAPos,
	S_WIN_LockBuffer,
	S_WIN_Submit,
	S_WIN_BlockSound,
	S_WIN_UnblockSound,
	s_wv_driver,
	SNDDRV_ID_WIN,
	false,
	NULL
};

#endif	/* HAVE_WIN_SOUND */

