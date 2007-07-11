/*
	snd_mix.c
	portable code to mix sounds for snd_dma.c

	$Id: snd_mix.c,v 1.18 2007-07-11 16:47:16 sezero Exp $
*/

#include "quakedef.h"

#ifdef PLATFORM_WINDOWS
#include "winquake.h"
#endif

#define	PAINTBUFFER_SIZE	512
portable_samplepair_t paintbuffer[PAINTBUFFER_SIZE];
int		snd_scaletable[32][256];
int		*snd_p, snd_linear_count, snd_vol;
short		*snd_out;

#if	id386
extern void Snd_WriteLinearBlastStereo16 (void);
#else
static void Snd_WriteLinearBlastStereo16 (void)
{
	int		i;
	int		val;

	for (i = 0; i < snd_linear_count; i += 2)
	{
		val = (snd_p[i]*snd_vol) >> 8;
		if (val > 0x7fff)
			snd_out[i] = 0x7fff;
		else if (val < (short)0x8000)
			snd_out[i] = (short)0x8000;
		else
			snd_out[i] = val;

		val = (snd_p[i+1]*snd_vol) >> 8;
		if (val > 0x7fff)
			snd_out[i+1] = 0x7fff;
		else if (val < (short)0x8000)
			snd_out[i+1] = (short)0x8000;
		else
			snd_out[i+1] = val;
	}
}
#endif

static void S_TransferStereo16 (int endtime)
{
	int		lpos;
	int		lpaintedtime;
	LPVOID	pbuf;
#ifdef PLATFORM_WINDOWS
// FIXME: move this to its platform driver!
	int		reps;
	DWORD	dwSize, dwSize2;
	LPVOID	pbuf2;
	HRESULT	hresult;
#endif

	snd_vol = sfxvolume.value * 256;

	snd_p = (int *) paintbuffer;
	lpaintedtime = paintedtime;

#ifdef PLATFORM_WINDOWS
// FIXME: move this to its platform driver!
	if (pDSBuf)
	{
		reps = 0;

		while ((hresult = IDirectSoundBuffer_Lock(pDSBuf, 0, gSndBufSize, (LPVOID *) &pbuf, &dwSize, 
								  (LPVOID *) &pbuf2, &dwSize2, 0)) != DS_OK)
		{
			if (hresult != DSERR_BUFFERLOST)
			{
				Con_Printf ("%s: DS::Lock Sound Buffer Failed\n", __thisfunc__);
				S_Shutdown ();
				S_Startup ();
				return;
			}

			if (++reps > 10000)
			{
				Con_Printf ("%s: DS: couldn't restore buffer\n", __thisfunc__);
				S_Shutdown ();
				S_Startup ();
				return;
			}
		}
	}
	else
#endif
	{
		pbuf = shm->buffer;
	}

	while (lpaintedtime < endtime)
	{
	// handle recirculating buffer issues
		lpos = lpaintedtime & ((shm->samples >> 1) - 1);

		snd_out = (short *) pbuf + (lpos << 1);

		snd_linear_count = (shm->samples >> 1) - lpos;
		if (lpaintedtime + snd_linear_count > endtime)
			snd_linear_count = endtime - lpaintedtime;

		snd_linear_count <<= 1;

	// write a linear blast of samples
		Snd_WriteLinearBlastStereo16 ();

		snd_p += snd_linear_count;
		lpaintedtime += (snd_linear_count >> 1);
	}

#ifdef PLATFORM_WINDOWS
// FIXME: move this to its platform driver!
	if (pDSBuf)
		IDirectSoundBuffer_Unlock(pDSBuf, pbuf, dwSize, NULL, 0);
#endif
}

static void S_TransferPaintBuffer (int endtime)
{
	int	out_idx, out_mask;
	int	count, step, val;
	int	*p;
	LPVOID	pbuf;
#ifdef PLATFORM_WINDOWS
// FIXME: move this to its platform driver!
	int		reps;
	DWORD	dwSize, dwSize2;
	LPVOID	pbuf2;
	HRESULT	hresult;
#endif

	if (shm->samplebits == 16 && shm->channels == 2)
	{
		S_TransferStereo16 (endtime);
		return;
	}

	p = (int *) paintbuffer;
	count = (endtime - paintedtime) * shm->channels;
	out_mask = shm->samples - 1;
	out_idx = paintedtime * shm->channels & out_mask;
	step = 3 - shm->channels;
	snd_vol = sfxvolume.value * 256;

#ifdef PLATFORM_WINDOWS
// FIXME: move this to its platform driver!
	if (pDSBuf)
	{
		reps = 0;

		while ((hresult = IDirectSoundBuffer_Lock(pDSBuf, 0, gSndBufSize, (LPVOID *) &pbuf, &dwSize, 
								  (LPVOID *) &pbuf2, &dwSize2, 0)) != DS_OK)
		{
			if (hresult != DSERR_BUFFERLOST)
			{
				Con_Printf ("%s: DS::Lock Sound Buffer Failed\n", __thisfunc__);
				S_Shutdown ();
				S_Startup ();
				return;
			}

			if (++reps > 10000)
			{
				Con_Printf ("%s: DS: couldn't restore buffer\n", __thisfunc__);
				S_Shutdown ();
				S_Startup ();
				return;
			}
		}
	}
	else
#endif
	{
		pbuf = shm->buffer;
	}

	if (shm->samplebits == 16)
	{
		short *out = (short *) pbuf;
		while (count--)
		{
			val = (*p * snd_vol) >> 8;
			p+= step;
			if (val > 0x7fff)
				val = 0x7fff;
			else if (val < (short)0x8000)
				val = (short)0x8000;
			out[out_idx] = val;
			out_idx = (out_idx + 1) & out_mask;
		}
	}
	else if (shm->samplebits == 8)
	{
		unsigned char *out = (unsigned char *) pbuf;
		while (count--)
		{
			val = (*p * snd_vol) >> 8;
			p+= step;
			if (val > 0x7fff)
				val = 0x7fff;
			else if (val < (short)0x8000)
				val = (short)0x8000;
			out[out_idx] = (val >> 8) + 128;
			out_idx = (out_idx + 1) & out_mask;
		}
	}

#ifdef PLATFORM_WINDOWS
// FIXME: move this to its platform driver!
	if (pDSBuf)
	{
		DWORD dwNewpos, dwWrite;
		int il = paintedtime;
		int ir = endtime - paintedtime;

		ir += il;

		IDirectSoundBuffer_Unlock(pDSBuf, pbuf, dwSize, NULL, 0);
		IDirectSoundBuffer_GetCurrentPosition(pDSBuf, &dwNewpos, &dwWrite);

//		if ((dwNewpos >= il) && (dwNewpos <= ir))
//			Con_Printf("%d-%d p %d c\n", il, ir, dwNewpos);
	}
#endif
}


/*
===============================================================================

CHANNEL MIXING

===============================================================================
*/

#if	id386
extern void SND_PaintChannelFrom8 (channel_t *ch, sfxcache_t *sc, int endtime);
#else
static void SND_PaintChannelFrom8 (channel_t *ch, sfxcache_t *sc, int endtime);
#endif
static void SND_PaintChannelFrom16 (channel_t *ch, sfxcache_t *sc, int endtime);

void S_PaintChannels (int endtime)
{
	int		i;
	int		end, ltime, count;
	channel_t	*ch;
	sfxcache_t	*sc;

	while (paintedtime < endtime)
	{
	// if paintbuffer is smaller than DMA buffer
		end = endtime;
		if (endtime - paintedtime > PAINTBUFFER_SIZE)
			end = paintedtime + PAINTBUFFER_SIZE;

	// clear the paint buffer
		memset(paintbuffer, 0, (end - paintedtime) * sizeof(portable_samplepair_t));

	// paint in the channels.
		ch = snd_channels;
		for (i = 0; i < total_channels; i++, ch++)
		{
			if (!ch->sfx)
				continue;
			if (!ch->leftvol && !ch->rightvol)
				continue;
			sc = S_LoadSound (ch->sfx);
			if (!sc)
				continue;

			ltime = paintedtime;

			while (ltime < end)
			{	// paint up to end
				if (ch->end < end)
					count = ch->end - ltime;
				else
					count = end - ltime;

				if (count > 0)
				{
					if (sc->width == 1)
						SND_PaintChannelFrom8(ch, sc, count);
					else
						SND_PaintChannelFrom16(ch, sc, count);

					ltime += count;
				}

			// if at end of loop, restart
				if (ltime >= ch->end)
				{
					if (sc->loopstart >= 0)
					{
						ch->pos = sc->loopstart;
						ch->end = ltime + sc->length - ch->pos;
					}
					else
					{	// channel just stopped
						ch->sfx = NULL;
						break;
					}
				}
			}
		}

	// transfer out according to DMA format
		S_TransferPaintBuffer(end);
		paintedtime = end;
	}
}

void SND_InitScaletable (void)
{
	int		i, j;

	for (i = 0; i < 32; i++)
	{
		for (j = 0; j < 256; j++)
		/* When compiling with gcc-4.1.0 at optimisations O1 and
		   higher, the tricky signed char type conversion is not
		   guaranteed. Therefore we explicity calculate the signed
		   value from the index as required. From Kevin Shanahan.
		   Also see: http://gcc.gnu.org/bugzilla/show_bug.cgi?id=26719
		*/
		//	snd_scaletable[i][j] = ((signed char)j) * i * 8;
			snd_scaletable[i][j] = ((j < 128) ? j : j - 0xff) * i * 8;
	}
}


#if	!id386

static void SND_PaintChannelFrom8 (channel_t *ch, sfxcache_t *sc, int count)
{
	int	data;
	int		*lscale, *rscale;
	unsigned char *sfx;
	int		i;

	if (ch->leftvol > 255)
		ch->leftvol = 255;
	if (ch->rightvol > 255)
		ch->rightvol = 255;

	lscale = snd_scaletable[ch->leftvol >> 3];
	rscale = snd_scaletable[ch->rightvol >> 3];
	sfx = (unsigned char *) ((signed char *)sc->data + ch->pos);

	for (i = 0; i < count; i++)
	{
		data = sfx[i];
		paintbuffer[i].left += lscale[data];
		paintbuffer[i].right += rscale[data];
	}

	ch->pos += count;
}

#endif	// !id386


static void SND_PaintChannelFrom16 (channel_t *ch, sfxcache_t *sc, int count)
{
	int	data;
	int	left, right;
	int	leftvol, rightvol;
	signed short *sfx;
	int	i;

	leftvol = ch->leftvol;
	rightvol = ch->rightvol;
	sfx = (signed short *)sc->data + ch->pos;

	for (i = 0; i < count; i++)
	{
		data = sfx[i];
		left = (data * leftvol) >> 8;
		right = (data * rightvol) >> 8;
		paintbuffer[i].left += left;
		paintbuffer[i].right += right;
	}

	ch->pos += count;
}

