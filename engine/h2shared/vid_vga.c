/*
 * vid_vga.c -- VGA-specific DOS video stuff
 * TODO: proper handling of page-swap failure
 * from quake1 source with minor adaptations for uhexen2.
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

#include <dos.h>

#include "quakedef.h"
#include "d_local.h"
#include "dosisms.h"
#include "vid_dos.h"


int		VGA_width, VGA_height, VGA_rowbytes, VGA_bufferrowbytes;
byte		*VGA_pagebase;
vmode_t		*VGA_pcurmode;

static int	VGA_planar;
static int	VGA_numpages;
static int	VGA_buffersize;

static int	VGA_highhunkmark;
/* these two are shared with vid_ext.c: */
void		*vid_surfcache;
int		vid_surfcachesize;

static byte	backingbuf[48*24];

static int VGA_InitMode (viddef_t *lvid, vmode_t *pcurrentmode);
static void VGA_SetPalette (viddef_t *lvid, vmode_t *pcurrentmode, const unsigned char *pal);
static void VGA_SwapBuffers (viddef_t *lvid, vmode_t *pcurrentmode, vrect_t *rects);
static void VGA_SwapBuffersCopy (viddef_t *lvid, vmode_t *pcurrentmode, vrect_t *rects);

#include "vgamodes.h"

#define NUMVIDMODES		(sizeof(vgavidmodes) / sizeof(vgavidmodes[0]))


/*
================
VGA_BeginDirectRect
================
*/
void VGA_BeginDirectRect (viddef_t *lvid, struct vmode_s *pcurrentmode, int x,
	int y, byte *pbitmap, int width, int height)
{
	int		i, j, k, plane, reps, repshift;

	if (!lvid->direct)
		return;

	if (lvid->aspect > 1.5)
	{
		reps = 2;
		repshift = 1;
	}
	else
	{
		reps = 1;
		repshift = 0;
	}

	if (pcurrentmode->planar)
	{
		for (plane = 0; plane < 4; plane++)
		{
		// select the correct plane for reading and writing
			outportb (SC_INDEX, MAP_MASK);
			outportb (SC_DATA, 1 << plane);
			outportb (GC_INDEX, READ_MAP);
			outportb (GC_DATA, plane);

			for (i = 0; i < (height << repshift); i += reps)
			{
				for (k = 0; k < reps; k++)
				{
					for (j = 0; j < (width >> 2); j++)
					{
						backingbuf[(i + k) * 24 + (j << 2) + plane] =
								lvid->direct[(y + i + k) * VGA_rowbytes +
								(x >> 2) + j];
						lvid->direct[(y + i + k) * VGA_rowbytes + (x>>2) + j] =
								pbitmap[(i >> repshift) * 24 +
								(j << 2) + plane];
					}
				}
			}
		}
	}
	else
	{
		for (i = 0; i < (height << repshift); i += reps)
		{
			for (j = 0; j < reps; j++)
			{
				memcpy (&backingbuf[(i + j) * 24],
						lvid->direct + x + ((y << repshift) + i + j) * VGA_rowbytes,
						width);
				memcpy (lvid->direct + x + ((y << repshift) + i + j) * VGA_rowbytes,
						&pbitmap[(i >> repshift) * width],
						width);
			}
		}
	}
}


/*
================
VGA_EndDirectRect
================
*/
void VGA_EndDirectRect (viddef_t *lvid, struct vmode_s *pcurrentmode, int x,
	int y, int width, int height)
{
	int		i, j, k, plane, reps, repshift;

	if (!lvid->direct)
		return;

	if (lvid->aspect > 1.5)
	{
		reps = 2;
		repshift = 1;
	}
	else
	{
		reps = 1;
		repshift = 0;
	}

	if (pcurrentmode->planar)
	{
		for (plane = 0; plane < 4; plane++)
		{
		// select the correct plane for writing
			outportb (SC_INDEX, MAP_MASK);
			outportb (SC_DATA, 1 << plane);

			for (i = 0; i < (height << repshift); i += reps)
			{
				for (k = 0; k < reps; k++)
				{
					for (j = 0; j < (width >> 2); j++)
					{
						lvid->direct[(y + i + k) * VGA_rowbytes + (x >> 2) + j] =
								backingbuf[(i + k) * 24 + (j << 2) + plane];
					}
				}
			}
		}
	}
	else
	{
		for (i = 0; i < (height << repshift); i += reps)
		{
			for (j = 0; j < reps; j++)
			{
				memcpy (lvid->direct + x + ((y << repshift) + i + j) * VGA_rowbytes,
						&backingbuf[(i + j) * 24],
						width);
			}
		}
	}
}


/*
================
VGA_Init
================
*/
void VGA_Init (void)
{
	int		i;

// link together all the VGA modes
	for (i = 0; i < (NUMVIDMODES - 1); i++)
	{
		vgavidmodes[i].pnext = &vgavidmodes[i+1];
	}

// add the VGA modes at the start of the mode list
	vgavidmodes[NUMVIDMODES-1].pnext = pvidmodes;
	pvidmodes = &vgavidmodes[0];

	numvidmodes += NUMVIDMODES;
}


/*
================
VGA_WaitVsync
================
*/
void VGA_WaitVsync (void)
{
	while ((inportb (0x3DA) & 0x08) == 0)
		;
}


/*
================
VGA_ClearVideoMem
================
*/
void VGA_ClearVideoMem (int planar)
{

	if (planar)
	{
	// enable all planes for writing
		outportb (SC_INDEX, MAP_MASK);
		outportb (SC_DATA, 0x0F);
	}

	memset (VGA_pagebase, 0, VGA_rowbytes * VGA_height);
}

/*
================
VGA_FreeAndAllocVidbuffer
================
*/
qboolean VGA_FreeAndAllocVidbuffer (viddef_t *lvid, int allocnewbuffer)
{
	int		tsize, tbuffersize;

	if (allocnewbuffer)
	{
	// alloc an extra line in case we want to wrap, and allocate the z-buffer
		tbuffersize = (lvid->rowbytes * (lvid->height + 1)) +
				(lvid->width * lvid->height * sizeof (*d_pzbuffer));
	}
	else
	{
	// just allocate the z-buffer
		tbuffersize = lvid->width * lvid->height * sizeof (*d_pzbuffer);
	}

	tsize = D_SurfaceCacheForRes (lvid->width, lvid->height);

	tbuffersize += tsize;

// see if there's enough memory, allowing for the normal mode 0x13 pixel,
// z, and surface buffers
	//if ((host_parms->memsize - tbuffersize + SURFCACHE_SIZE_AT_320X200 +
	//	 0x10000 * 3) < MINIMUM_MEMORY)
	// Pa3PyX: using hopefully better estimation now
	// if total memory < needed surface cache + (minimum operational memory
	// less surface cache for 320x200 and typical hunk state after init)
	if (host_parms->memsize < tbuffersize + 0x180000 + 0xC00000)
	{
		Con_Printf ("Not enough memory for video mode\n");
		VGA_pcurmode = NULL;	// so no further accesses to the buffer are
								//  attempted, particularly when clearing
		return false;		// not enough memory for mode
	}

	VGA_buffersize = tbuffersize;
	vid_surfcachesize = tsize;

	if (d_pzbuffer)
	{
		D_FlushCaches ();
		Hunk_FreeToHighMark (VGA_highhunkmark);
		d_pzbuffer = NULL;
	}

	VGA_highhunkmark = Hunk_HighMark ();

	d_pzbuffer = (short *) Hunk_HighAllocName (VGA_buffersize, "video");

	vid_surfcache = (byte *)d_pzbuffer
		+ lvid->width * lvid->height * sizeof (*d_pzbuffer);
	
	if (allocnewbuffer)
	{
		lvid->buffer = (pixel_t *)( (byte *)vid_surfcache + vid_surfcachesize);
		lvid->conbuffer = lvid->buffer;
	}

	return true;
}


/*
================
VGA_CheckAdequateMem
================
*/
qboolean VGA_CheckAdequateMem (int width, int height, int rowbytes, int allocnewbuffer)
{
	int		tbuffersize;

	tbuffersize = width * height * sizeof (*d_pzbuffer);

	if (allocnewbuffer)
	{
	// alloc an extra line in case we want to wrap, and allocate the z-buffer
		tbuffersize += (rowbytes * (height + 1));
	}

	tbuffersize += D_SurfaceCacheForRes (width, height);

// see if there's enough memory, allowing for the normal mode 0x13 pixel,
// z, and surface buffers
	//if ((host_parms->memsize - tbuffersize + SURFCACHE_SIZE_AT_320X200 +
	//	 0x10000 * 3) < MINIMUM_MEMORY)
	// Pa3PyX: using hopefully better estimation now
	// if total memory < needed surface cache + (minimum operational memory
	// less surface cache for 320x200 and typical hunk state after init)
	if (host_parms->memsize < tbuffersize + 0x180000 + 0xC00000)
	{
		return false;		// not enough memory for mode
	}

	return true;
}


/*
================
VGA_InitMode
================
*/
static int VGA_InitMode (viddef_t *lvid, vmode_t *pcurrentmode)
{
	vextra_t		*pextra;

	pextra = (vextra_t *) pcurrentmode->pextradata;

	if (!VGA_FreeAndAllocVidbuffer (lvid, pextra->vidbuffer))
		return -1;	// memory alloc failed

	if (VGA_pcurmode)
		VGA_ClearVideoMem (VGA_pcurmode->planar);

// mode 0x13 is the base for all the Mode X-class mode sets
	regs.h.ah = 0;
	regs.h.al = 0x13;
	dos_int86(0x10);

	VGA_pagebase = (byte *)real2ptr(0xa0000);
	lvid->direct = (pixel_t *)VGA_pagebase;

// set additional registers as needed
	VideoRegisterSet (pextra->pregset);

	VGA_numpages = 1;
	lvid->numpages = VGA_numpages;

	VGA_width = (lvid->width + 0x1F) & ~0x1F;
	VGA_height = lvid->height;
	VGA_planar = pcurrentmode->planar;
	if (VGA_planar)
		VGA_rowbytes = lvid->rowbytes / 4;
	else
		VGA_rowbytes = lvid->rowbytes;
	VGA_bufferrowbytes = lvid->rowbytes;
	lvid->colormap = host_colormap;
	lvid->fullbright = 256 - LittleLong (*((int *)lvid->colormap + 2048));

	lvid->maxwarpwidth = WARP_WIDTH;
	lvid->maxwarpheight = WARP_HEIGHT;

	lvid->conbuffer = lvid->buffer;
	lvid->conrowbytes = lvid->rowbytes;
	lvid->conwidth = lvid->width;
	lvid->conheight = lvid->height;

	VGA_pcurmode = pcurrentmode;

	VGA_ClearVideoMem (pcurrentmode->planar);

	if (_vid_wait_override.integer)
	{
		Cvar_SetValueQuick (&vid_wait, (float)VID_WAIT_VSYNC);
	}
	else
	{
		Cvar_SetValueQuick (&vid_wait, (float)VID_WAIT_NONE);
	}

	D_InitCaches (vid_surfcache, vid_surfcachesize);

	return 1;
}


/*
================
VGA_SetPalette
================
*/
static void VGA_SetPalette (viddef_t *lvid, vmode_t *pcurrentmode, const unsigned char *pal)
{
	int	shiftcomponents = 2;
	int	i;

	Q_UNUSED(lvid);
	Q_UNUSED(pcurrentmode);

	dos_outportb(0x3c8, 0);
	for (i = 0; i < 768; i++)
		outportb(0x3c9, pal[i]>>shiftcomponents);
}

#if	!id386
void VGA_UpdatePlanarScreen (void *srcbuffer)
{
/* based on vga_copytoplanar256() of SVGAlib */
	byte *p;
	int ofs;
	int plane, x, y;

	plane = 0;
	/* Copy pixels that belong in plane. */
	_docopy:
	outportb(SC_INDEX, MAP_MASK);
	outportb(SC_DATA, 1 << plane);
	p = (byte *) srcbuffer;
	ofs = 0;
	for (y = 0; y < VGA_height; y++)
	{
		x = 0;
		while (x * 4 + 32 < VGA_width)
		{
			VGA_pagebase[ofs + x + 0] = p[x * 4 + plane + 0];
			VGA_pagebase[ofs + x + 1] = p[x * 4 + plane + 4];
			VGA_pagebase[ofs + x + 2] = p[x * 4 + plane + 8];
			VGA_pagebase[ofs + x + 3] = p[x * 4 + plane + 12];
			VGA_pagebase[ofs + x + 4] = p[x * 4 + plane + 16];
			VGA_pagebase[ofs + x + 5] = p[x * 4 + plane + 20];
			VGA_pagebase[ofs + x + 6] = p[x * 4 + plane + 24];
			VGA_pagebase[ofs + x + 7] = p[x * 4 + plane + 28];
			x += 8;
		}
		while (x * 4 < VGA_width)
		{
			VGA_pagebase[ofs + x] = p[x * 4 + plane];
			x++;
		}
		p += VGA_bufferrowbytes;	/* Next line. */
		ofs += VGA_rowbytes;
	}
	if (++plane < 4)
		goto _docopy;
}

void VGA_UpdateLinearScreen (	void *srcptr, void *destptr,
				int width, int height,
				int srcrowbytes, int destrowbytes)
{
/* quick'n'dirty C-only version */
	byte *s, *d;
	int y;

	s = (byte *) srcptr;
	d = (byte *) destptr;
	for (y = 0; y < height; y++)
	{
		memcpy (d, s, width);
		s += srcrowbytes;
		d += destrowbytes;
	}
}
#endif	/* !id386 */

/*
================
VGA_SwapBuffersCopy
================
*/
static void VGA_SwapBuffersCopy (viddef_t *lvid, vmode_t *pcurrentmode, vrect_t *rects)
{
	Q_UNUSED(pcurrentmode);

// TODO: can write a dword at a time
// TODO: put in ASM
// TODO: copy only specified rectangles
	if (VGA_planar)
	{

	// TODO: copy only specified rectangles

		VGA_UpdatePlanarScreen (lvid->buffer);
	}
	else
	{
		while (rects)
		{
			VGA_UpdateLinearScreen (
					lvid->buffer + rects->x + (rects->y * lvid->rowbytes),
		 			VGA_pagebase + rects->x + (rects->y * VGA_rowbytes),
					rects->width,
					rects->height,
					lvid->rowbytes,
					VGA_rowbytes);

			rects = rects->pnext;
		}
	}
}


/*
================
VGA_SwapBuffers
================
*/
static void VGA_SwapBuffers (viddef_t *lvid, vmode_t *pcurrentmode, vrect_t *rects)
{
	if (vid_wait.integer == VID_WAIT_VSYNC)
		VGA_WaitVsync ();

	VGA_SwapBuffersCopy (lvid, pcurrentmode, rects);
}

