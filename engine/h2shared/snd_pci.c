/* snd_pci.c -- PCI sound card support for DOS through libau
 * Copyright (C) 2015 O.Sezer <sezero@users.sourceforge.net>
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

#if HAVE_DOS_PCI_SOUND

#include "snd_pci.h"
#include "sys_dxe.h"
#include "libau.h"

static char s_pci_driver[] = "PCI Audio";
static au_context *ctx = NULL;

#ifndef SNDPCI_DXE
#define AU_search_fp AU_search
#define AU_start_fp AU_start
#define AU_close_fp AU_close
#define AU_getinfo_fp AU_getinfo
#define AU_setrate_fp AU_setrate
#define AU_setmixer_all_fp AU_setmixer_all
#define AU_cardbuf_space_fp AU_cardbuf_space
#else
static void *sndpci_dxe;
static au_context * (*AU_search_fp)(unsigned int);
static void (*AU_start_fp)(au_context *);
static void (*AU_close_fp)(au_context *);
static const struct auinfo_s * (*AU_getinfo_fp)(au_context *);
static void (*AU_setrate_fp)(au_context *, unsigned int *, unsigned int *, unsigned int *);
static void (*AU_setmixer_all_fp)(au_context *, unsigned int);
static unsigned int (*AU_cardbuf_space_fp)(au_context *);

static int load_sndpci_dxe(void)
{
	if ((sndpci_dxe = Sys_dlopen("sndpci.dxe", 0)) == NULL)
		goto fail;
	AU_search_fp = (au_context * (*)(unsigned int)) Sys_dlsym(sndpci_dxe, "_AU_search");
	AU_start_fp = (void (*)(au_context*)) Sys_dlsym(sndpci_dxe, "_AU_start");
	AU_close_fp = (void (*)(au_context*)) Sys_dlsym(sndpci_dxe, "_AU_close");
	AU_getinfo_fp = (const struct auinfo_s * (*)(au_context*)) Sys_dlsym(sndpci_dxe, "_AU_getinfo");
	AU_setrate_fp = (void (*)(au_context*, unsigned int*, unsigned int*, unsigned int*)) Sys_dlsym(sndpci_dxe, "_AU_setrate");
	AU_setmixer_all_fp = (void (*)(au_context*, unsigned int)) Sys_dlsym(sndpci_dxe, "_AU_setmixer_all");
	AU_cardbuf_space_fp = (unsigned int (*)(au_context*)) Sys_dlsym(sndpci_dxe, "_AU_cardbuf_space");
	if (!AU_search_fp || !AU_start_fp || !AU_close_fp || !AU_getinfo_fp ||
	    !AU_setrate_fp || !AU_setmixer_all_fp || !AU_cardbuf_space_fp) {
	fail:	Con_Printf("PCI Audio: failed loading sndpci.dxe\n");
		return -1;
	}
	return 0;
}

static void close_sndpci_dxe(void)
{
	if (sndpci_dxe) Sys_dlclose(sndpci_dxe);
	sndpci_dxe = NULL;
	AU_search_fp = NULL;
	AU_start_fp = NULL;
	AU_close_fp = NULL;
	AU_getinfo_fp = NULL;
	AU_setrate_fp = NULL;
	AU_setmixer_all_fp = NULL;
	AU_cardbuf_space_fp = NULL;
}
#endif

static qboolean S_PCI_Init(dma_t *dma)
{
	const struct auinfo_s *aui;
	unsigned int speed, samplebits, channels;

	if (!COM_CheckParm("-sndpci"))
		return false;

#ifdef SNDPCI_DXE
	if (load_sndpci_dxe() < 0)
		return false;
#endif

	ctx = AU_search_fp(1);/* 1: stereo speaker output (meaningful only for Intel HDA chips) */
	if (!ctx)
	{
		Con_Printf("PCI Audio: Detection failed.\n");
		return false;
	}

	speed = desired_speed;
	samplebits = 16;
	channels = 2;
	AU_setrate_fp(ctx, &speed, &samplebits, &channels);

	memset (dma, 0, sizeof(dma_t));
	shm = dma;
	aui = AU_getinfo_fp(ctx);

	shm->speed = aui->freq_card;
	shm->samplebits = aui->bits_set;
	shm->channels = aui->chan_set;
	shm->samples = aui->card_dmasize / aui->bytespersample_card;
	shm->samplepos = 0;
	shm->submission_chunk = 1;
	shm->buffer = (unsigned char *) aui->card_DMABUFF;

	Con_Printf("%s\n", aui->infostr);

	AU_setmixer_all_fp(ctx, 80); /* 80% volume */
	AU_start_fp(ctx); /* also clears dma buffer */

	return true;
}

static int S_PCI_GetDMAPos(void)
{
	shm->samplepos = AU_cardbuf_space_fp(ctx);
	return shm->samplepos;
}

static void S_PCI_Shutdown(void)
{
	if (shm)
	{
		AU_close_fp(ctx);
		ctx = NULL;
		shm->buffer = NULL;
		shm = NULL;
	}
#ifdef SNDPCI_DXE
	close_sndpci_dxe();
#endif
}

static void S_PCI_LockBuffer (void)
{
}

static void S_PCI_Submit (void)
{
}

static void S_PCI_BlockSound (void)
{
}

static void S_PCI_UnblockSound (void)
{
}

snd_driver_t snddrv_pci =
{
	S_PCI_Init,
	S_PCI_Shutdown,
	S_PCI_GetDMAPos,
	S_PCI_LockBuffer,
	S_PCI_Submit,
	S_PCI_BlockSound,
	S_PCI_UnblockSound,
	s_pci_driver,
	SNDDRV_ID_PCI_DOS,
	false,
	NULL
};

#endif /* HAVE_DOS_PCI_SOUND */

