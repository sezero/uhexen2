/*
 * snd_dsound.c
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

#if HAVE_WIN_DX_SOUND

#include "winquake.h"
#include "snd_dsound.h"
#include <mmsystem.h>
#include <dsound.h>

static char s_ds_driver[] = "DirectSound";

/* DirectSound : */
#ifndef DSBSIZE_MIN
#define DSBSIZE_MIN		4
#endif
#ifndef DSBSIZE_MAX
#define DSBSIZE_MAX		0x0FFFFFFF
#endif

static LPDIRECTSOUND		pDS;
static LPDIRECTSOUNDBUFFER	pDSBuf, pDSPBuf;

#if defined(DX_DLSYM)	/* dynamic loading of dsound symbols */
static HINSTANCE	hInstDS;
static HRESULT (WINAPI *pDirectSoundCreate)(GUID FAR *lpGUID, LPDIRECTSOUND FAR *lplpDS, IUnknown FAR *pUnkOuter);
#else	/* ! DX_DLSYM : we're linked to dsound */
#define	pDirectSoundCreate		DirectSoundCreate
#endif	/* DX_DLSYM */

static qboolean	primary_format_set;

static int	sample16;
static int	ds_sbuf_size;

static HPSTR	lpData;

static DWORD	gSndBufSize;
static MMTIME	mmstarttime;


/*
==================
FreeSound
==================
*/
static void FreeSound (void)
{
	if (pDSBuf)
	{
		IDirectSoundBuffer_Stop(pDSBuf);
		IDirectSound_Release(pDSBuf);
	}

// only release primary buffer if it's not also the mixing buffer we just released
	if (pDSPBuf && (pDSBuf != pDSPBuf))
	{
		IDirectSound_Release(pDSPBuf);
	}

	if (pDS)
	{
		IDirectSound_SetCooperativeLevel(pDS, mainwindow, DSSCL_NORMAL);
		IDirectSound_Release(pDS);
	}

	pDS = NULL;
	pDSBuf = NULL;
	pDSPBuf = NULL;
	lpData = NULL;
}


/*
==================
SNDDMA_InitDirect

Direct-Sound support
==================
*/
static qboolean S_DS_Init (dma_t *dma)
{
	DSBUFFERDESC	dsbuf;
	DSBCAPS			dsbcaps;
	DWORD			dwSize, dwWrite;
	DSCAPS			dscaps;
	WAVEFORMATEX	format, pformat;
	HRESULT			hresult;
	int				reps;

	memset((void *) dma, 0, sizeof(dma_t));
	shm = dma;

	shm->channels = desired_channels;
	shm->samplebits = desired_bits;
	shm->speed = desired_speed;
	/* Calculate the DS buffer size to store 2 secs
	 * of data, round up to the next power of 2.  */
	ds_sbuf_size = 1 << (Q_log2((desired_bits  >> 3) * (desired_speed << 1)) + 1);

	memset (&format, 0, sizeof(format));
	format.wFormatTag = WAVE_FORMAT_PCM;
	format.nChannels = shm->channels;
	format.wBitsPerSample = shm->samplebits;
	format.nSamplesPerSec = shm->speed;
	format.nBlockAlign = format.nChannels * format.wBitsPerSample / 8;
	format.cbSize = 0;
	format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;

#if defined(DX_DLSYM)
	if (!hInstDS)
	{
		hInstDS = LoadLibrary("dsound.dll");

		if (hInstDS == NULL)
		{
			Con_SafePrintf ("Couldn't load dsound.dll\n");
			return false;
		}

		pDirectSoundCreate = (HRESULT (WINAPI *)(GUID FAR *, LPDIRECTSOUND FAR *, IUnknown FAR *))
								GetProcAddress(hInstDS,"DirectSoundCreate");

		if (!pDirectSoundCreate)
		{
			Con_SafePrintf ("Couldn't get DS proc addr\n");
			return false;
		}
	}
#endif	/* DX_DLSYM */

	hresult = pDirectSoundCreate(NULL, &pDS, NULL);
	if (hresult != DS_OK)
	{
		if (hresult != DSERR_ALLOCATED)
		{
			Con_SafePrintf ("DirectSound create failed\n");
			return false;
		}

		Con_SafePrintf ("DirectSoundCreate failure, hardware already in use\n");
		return false;
	}

	dscaps.dwSize = sizeof(dscaps);

	if (DS_OK != IDirectSound_GetCaps(pDS, &dscaps))
	{
		Con_SafePrintf ("Couldn't get DS caps\n");
	}

	if (dscaps.dwFlags & DSCAPS_EMULDRIVER)
	{
		Con_SafePrintf ("No DirectSound driver installed\n");
		goto fail;
	}

	// if (DS_OK != IDirectSound_SetCooperativeLevel(pDS, mainwindow, DSSCL_EXCLUSIVE))
	/* Pa3PyX: Some MIDI synthesizers are software and require access to
	   waveOut; so if we set the coop level to exclusive, MIDI will fail
	   to init because the device is locked. We use priority level instead.
	   That way we don't lock out software synths and other apps, but can
	   still set the sound buffer format. */
	if (DS_OK != IDirectSound_SetCooperativeLevel(pDS, mainwindow, DSSCL_PRIORITY))
	{
		Con_SafePrintf ("Set coop level failed\n");
		goto fail;
	}

// get access to the primary buffer, if possible, so we can set the
// sound hardware format
	memset (&dsbuf, 0, sizeof(dsbuf));
	dsbuf.dwSize = sizeof(DSBUFFERDESC);
	dsbuf.dwFlags = DSBCAPS_PRIMARYBUFFER;
	dsbuf.dwBufferBytes = 0;
	dsbuf.lpwfxFormat = NULL;

	memset(&dsbcaps, 0, sizeof(dsbcaps));
	dsbcaps.dwSize = sizeof(dsbcaps);
	primary_format_set = false;

	if (!COM_CheckParm ("-snoforceformat"))
	{
		if (DS_OK == IDirectSound_CreateSoundBuffer(pDS, &dsbuf, &pDSPBuf, NULL))
		{
			pformat = format;

			if (DS_OK != IDirectSoundBuffer_SetFormat(pDSPBuf, &pformat))
			{
				Con_SafePrintf ("Set primary sound buffer format: no\n");
			}
			else
			{
				Con_SafePrintf ("Set primary sound buffer format: yes\n");
				primary_format_set = true;
			}
		}
	}

	if (!primary_format_set || !COM_CheckParm ("-primarysound"))
	{
	// create the secondary buffer we'll actually work with
		memset (&dsbuf, 0, sizeof(dsbuf));
		dsbuf.dwSize = sizeof(DSBUFFERDESC);
		dsbuf.dwFlags = DSBCAPS_CTRLFREQUENCY | DSBCAPS_LOCSOFTWARE;
		if (ds_sbuf_size < DSBSIZE_MIN)
			ds_sbuf_size = 1 << (Q_log2(DSBSIZE_MIN) + 1);
		if (ds_sbuf_size > DSBSIZE_MAX)
			ds_sbuf_size = 1 << Q_log2(DSBSIZE_MAX);
		dsbuf.dwBufferBytes = ds_sbuf_size;
		dsbuf.lpwfxFormat = &format;

		memset(&dsbcaps, 0, sizeof(dsbcaps));
		dsbcaps.dwSize = sizeof(dsbcaps);

		if (DS_OK != IDirectSound_CreateSoundBuffer(pDS, &dsbuf, &pDSBuf, NULL))
		{
			Con_SafePrintf ("DS:CreateSoundBuffer Failed");
			goto fail;
		}

		shm->channels = format.nChannels;
		shm->samplebits = format.wBitsPerSample;
		shm->speed = format.nSamplesPerSec;

		if (DS_OK != IDirectSound_GetCaps(pDSBuf, &dsbcaps))
		{
			Con_SafePrintf ("DS:GetCaps failed\n");
			goto fail;
		}

		Con_SafePrintf ("Using secondary sound buffer\n");
	}
	else
	{
		if (DS_OK != IDirectSound_SetCooperativeLevel(pDS, mainwindow, DSSCL_WRITEPRIMARY))
		{
			Con_SafePrintf ("Set coop level failed\n");
			goto fail;
		}

		if (DS_OK != IDirectSound_GetCaps(pDSPBuf, &dsbcaps))
		{
			Con_Printf ("DS:GetCaps failed\n");
			return false;
		}

		pDSBuf = pDSPBuf;
		Con_SafePrintf ("Using primary sound buffer\n");
	}

	// Make sure mixer is active
	IDirectSoundBuffer_Play(pDSBuf, 0, 0, DSBPLAY_LOOPING);

	Con_SafePrintf ("%lu bytes in sound buffer\n", (unsigned long)dsbcaps.dwBufferBytes);

	gSndBufSize = dsbcaps.dwBufferBytes;

// initialize the buffer
	reps = 0;

	while ((hresult = IDirectSoundBuffer_Lock(pDSBuf, 0, gSndBufSize, (LPVOID *) (HPSTR) &lpData, &dwSize, NULL, NULL, 0)) != DS_OK)
	{
		if (hresult != DSERR_BUFFERLOST)
		{
			Con_SafePrintf ("%s: DS::Lock Sound Buffer Failed\n", __thisfunc__);
			goto fail;
		}

		if (++reps > 10000)
		{
			Con_SafePrintf ("%s: DS: couldn't restore buffer\n", __thisfunc__);
			goto fail;
		}
	}

	memset(lpData, 0, dwSize);
//	lpData[4] = lpData[5] = 0x7f;	// force a pop for debugging

	IDirectSoundBuffer_Unlock(pDSBuf, lpData, dwSize, NULL, 0);

	/* we don't want anyone to access the buffer directly w/o locking it first. */
	lpData = NULL;

	IDirectSoundBuffer_Stop(pDSBuf);
	IDirectSoundBuffer_GetCurrentPosition(pDSBuf, &mmstarttime.u.sample, &dwWrite);
	IDirectSoundBuffer_Play(pDSBuf, 0, 0, DSBPLAY_LOOPING);

	shm->samples = gSndBufSize / (shm->samplebits / 8);
	shm->samplepos = 0;
	shm->submission_chunk = 1;
	shm->buffer = (unsigned char *) lpData;
	sample16 = (shm->samplebits / 8) - 1;

	Con_SafePrintf ("DirectSound initialized\n");
	return true;

fail:
	FreeSound ();
	Con_SafePrintf ("DirectSound failed to init\n");
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
static int S_DS_GetDMAPos (void)
{
	MMTIME	mmtime;
	int		s;
	DWORD	dwWrite;

	mmtime.wType = TIME_SAMPLES;
	IDirectSoundBuffer_GetCurrentPosition(pDSBuf, &mmtime.u.sample, &dwWrite);
	s = mmtime.u.sample - mmstarttime.u.sample;
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
static DWORD	locksize;
static void S_DS_LockBuffer (void)
{
	if (pDSBuf)
	{
		void	*pData;
		int		reps;
		HRESULT	hresult;
		DWORD	dwStatus;

		reps = 0;
		shm->buffer = NULL;

		if (IDirectSoundBuffer_GetStatus(pDSBuf, &dwStatus) != DS_OK)
			Con_Printf ("Couldn't get sound buffer status\n");

		if (dwStatus & DSBSTATUS_BUFFERLOST)
			IDirectSoundBuffer_Restore(pDSBuf);

		if (!(dwStatus & DSBSTATUS_PLAYING))
			IDirectSoundBuffer_Play(pDSBuf, 0, 0, DSBPLAY_LOOPING);

		while ((hresult = IDirectSoundBuffer_Lock(pDSBuf, 0, gSndBufSize, (void **) &pData, &locksize, NULL, NULL, 0)) != DS_OK)
		{
			if (hresult != DSERR_BUFFERLOST)
			{
				Con_Printf ("%s: DS::Lock Sound Buffer Failed\n", __thisfunc__);
				S_Shutdown ();
				return;
			}

			if (++reps > 10000)
			{
				Con_Printf ("%s: DS: couldn't restore buffer\n", __thisfunc__);
				S_Shutdown ();
				return;
			}
		}

		shm->buffer = (unsigned char *) pData;
	}
}

/*
==============
SNDDMA_Submit

Unlock the dma buffer /
Send sound to the device
===============
*/
static void S_DS_Submit (void)
{
	if (pDSBuf)
		IDirectSoundBuffer_Unlock(pDSBuf, shm->buffer, locksize, NULL, 0);
}

/*
==================
SNDDMA_BlockSound
==================
*/
static void S_DS_BlockSound (void)
{
	// DirectSound takes care of blocking itself
}


/*
==================
SNDDMA_UnblockSound
==================
*/
static void S_DS_UnblockSound (void)
{
}


/*
==============
SNDDMA_Shutdown

Reset the sound device for exiting
===============
*/
static void S_DS_Shutdown (void)
{
	FreeSound ();
#if defined(DX_DLSYM)
	if (hInstDS)
	{
		FreeLibrary(hInstDS);
		hInstDS = NULL;
	}
#endif	/* DX_DLSYM */
}

snd_driver_t snddrv_dsound =
{
	S_DS_Init,
	S_DS_Shutdown,
	S_DS_GetDMAPos,
	S_DS_LockBuffer,
	S_DS_Submit,
	S_DS_BlockSound,
	S_DS_UnblockSound,
	s_ds_driver,
	SNDDRV_ID_DSOUND,
	false,
	NULL
};

#endif	/* HAVE_WIN_DX_SOUND */

