/*
 * vid_svgalib.c:	Linux SVGALIB specific video driver.
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

#if 0
/* these should be for vt stuff */
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/vt.h>
#include <stdarg.h>
#include <stdio.h>
#include <signal.h>
#endif

#include "q_stdinc.h"
/*#include <sys/io.h>*/	/* outb()*/

#include "vga.h"

#include "quakedef.h"
#include "d_local.h"


unsigned short	d_8to16table[256];	/* not used in 8 bpp mode */
unsigned int	d_8to24table[256];	/* not used in 8 bpp mode */

byte		globalcolormap[VID_GRADES*256], lastglobalcolor = 0;
byte		*lastsourcecolormap = NULL;

viddef_t	vid;			/* global video state	*/

int		VGA_width, VGA_height, VGA_rowbytes, VGA_bufferrowbytes;
byte		*VGA_pagebase;
static byte	*framebuffer_ptr;
static int	VGA_planar;

static byte	backingbuf[48*24];

static byte	*vid_surfcache;
static int	VID_highhunkmark;

static int	num_modes;
static vga_modeinfo	*modes;
static int	current_mode;

static byte	vid_current_palette[768];

static int	svgalib_inited = 0;
static int	svgalib_backgrounded = 0;

static cvar_t	vid_mode = {"vid_mode", "5", CVAR_NONE};
static cvar_t	vid_redrawfull = {"vid_redrawfull", "0", CVAR_NONE};
static cvar_t	vid_waitforrefresh = {"vid_waitforrefresh", "0", CVAR_ARCHIVE};

/* globals for compatibility: */
modestate_t	modestate = MS_UNINIT;
cvar_t	_enable_mouse = {"_enable_mouse", "1", CVAR_ARCHIVE};


#if 0
static void vtswitch (int newconsole)
{
	int		fd;
	struct vt_stat	x;

/* switch consoles and wait until reactivated */
	fd = open("/dev/console", O_RDONLY);
	ioctl(fd, VT_GETSTATE, &x);
	ioctl(fd, VT_ACTIVATE, newconsole);
	ioctl(fd, VT_WAITACTIVE, x.v_active);
	close(fd);
}
#endif

/*
=================
VID_Gamma_f

Keybinding command
=================
*/
#if 0
static void VID_Gamma_f (void)
{
	float	gamma, f, inf;
	unsigned char	palette[768];
	int		i;

	if (Cmd_Argc () == 2)
	{
		gamma = atof (Cmd_Argv(1));

		for (i = 0; i < 768; i++)
		{
			f = pow ((host_basepal[i] + 1) / 256.0, gamma);
			inf = f*255 + 0.5;
			if (inf < 0)
				inf = 0;
			if (inf > 255)
				inf = 255;
			palette[i] = inf;
		}

		VID_SetPalette (palette);

		vid.recalc_refdef = 1;	/* force a surface cache flush */
	}
}
#endif

static void VID_DescribeMode_f (void)
{
	int	modenum;

	modenum = atoi (Cmd_Argv(1));
	if ((modenum >= num_modes) || (modenum < 0 ) || !modes[modenum].width)
		Con_Printf("Invalid video mode: %d!\n", modenum);
	else
	{
		Con_Printf("%d: %d x %d - ", modenum, modes[modenum].width, modes[modenum].height);
		if (modes[modenum].bytesperpixel == 0)
			Con_Printf("ModeX\n");
		else
			Con_Printf("%d bpp\n", modes[modenum].bytesperpixel<<3);
	}
}

static void VID_DescribeModes_f (void)
{
	int	i;

	for (i = 0; i < num_modes; i++)
	{
		if (modes[i].width)
		{
			Con_Printf("%d: %d x %d - ", i, modes[i].width, modes[i].height);
			if (modes[i].bytesperpixel == 0)
				Con_Printf("ModeX\n");
			else
				Con_Printf("%d bpp\n", modes[i].bytesperpixel<<3);
		}
	}
}

/*
================
VID_NumModes
================
*/
static int VID_NumModes (void)
{
	int	i, i1 = 0;

	for (i = 0; i < num_modes; i++)
		i1 += (modes[i].width ? 1 : 0);
	return (i1);
}

static void VID_NumModes_f (void)
{
	Con_Printf("%d modes\n", VID_NumModes());
}

static void VID_Debug_f (void)
{
	Con_Printf("mode: %d\n", current_mode);
	Con_Printf("height x width: %d x %d\n", vid.height, vid.width);
	Con_Printf("bpp: %d\n", modes[current_mode].bytesperpixel*8);
	Con_Printf("vid.aspect: %f\n", vid.aspect);
}


static void VID_InitModes (void)
{
	int	i;

/* get complete information on all modes */
	num_modes = vga_lastmodenumber() + 1;
	modes = (vga_modeinfo *) Z_Malloc(num_modes * sizeof(vga_modeinfo), Z_MAINZONE);
	for (i = 0; i < num_modes; i++)
	{
		if (vga_hasmode(i))
			memcpy(&modes[i], vga_getmodeinfo(i), sizeof(vga_modeinfo));
		else
			modes[i].width = 0;	/* means not available */
	}

/* filter for modes i don't support */
	for (i = 0; i < num_modes; i++)
	{
		if (modes[i].bytesperpixel != 1 && modes[i].colors != 256)
			modes[i].width = 0;
	}
}

static int get_mode (char *name, int width, int height, int depth)
{
	int	i;

	if (name)
	{
		i = vga_getmodenumber(name);
		if (i < 0 || !modes[i].width)
		{
			Sys_Printf("Mode [%s] not supported\n", name);
			i = -1;
		}
	}
	else
	{
		int	need, match;
		need = (!!width) | ((!!height) << 1) | ((!!depth) << 2);

		for (i = 0; i < num_modes; i++)
		{
			if (!modes[i].width)
				continue;
			if (width && modes[i].width != width)
				continue;
			if (height && modes[i].height != height)
				continue;
			if (depth && modes[i].bytesperpixel != depth/8)
				continue;
			match =	((modes[i].width == width)	<< 0)	|
				((modes[i].height == height)	<< 1)	|
				((modes[i].bytesperpixel == depth/8) << 2);
			if (match & need)
				break;	/* got a match */
		}
		if (i == num_modes)
		{
			Sys_Printf("Mode %dx%d (%d bits) not supported\n", width, height, depth);
			i = -1;
		}
	}

	return i;
}

void VID_Shutdown (void)
{
	if (!svgalib_inited)
		return;

	vga_setmode(TEXT);

	svgalib_inited = 0;
}

void VID_SetPalette (byte *palette)
{
	static int	tmppal[256*3];
	int		*tp;
	int		i;

	if (!svgalib_inited)
		return;
	if (svgalib_backgrounded)
		return;

	memcpy(vid_current_palette, palette, sizeof(vid_current_palette));

	if (vga_getcolors() == 256)
	{
		tp = tmppal;
		for (i = 256 * 3; i; i--)
			*(tp++) = *(palette++) >> 2;

		if (vga_oktowrite())
			vga_setpalvec(0, 256, tmppal);
	}
}

void VID_ShiftPalette (unsigned char *p)
{
	VID_SetPalette(p);
}

static qboolean VID_SetMode (int modenum, unsigned char *palette)
{
	int	bsize, zsize, tsize;

	if ((modenum >= num_modes) || (modenum < 0) || !modes[modenum].width)
	{
		Cvar_SetValueQuick (&vid_mode, (float)current_mode);
		Con_Printf("No such video mode: %d\n",modenum);
		return false;
	}

	Cvar_SetValueQuick (&vid_mode, (float)modenum);

	current_mode = modenum;

	vid.width = modes[current_mode].width;
	vid.height = modes[current_mode].height;

	VGA_width = modes[current_mode].width;
	VGA_height = modes[current_mode].height;
	VGA_planar = modes[current_mode].bytesperpixel == 0;
	VGA_rowbytes = modes[current_mode].linewidth;
	vid.rowbytes = modes[current_mode].linewidth;
	if (VGA_planar)
	{
		VGA_bufferrowbytes = modes[current_mode].linewidth * 4;
		vid.rowbytes = modes[current_mode].linewidth*4;
	}

	vid.aspect = ((float)vid.height / (float)vid.width) * (320.0 / 240.0);
	vid.colormap = (pixel_t *) host_colormap;
	vid.fullbright = 256 - LittleLong (*((int *)vid.colormap + 2048));
	vid.conrowbytes = vid.rowbytes;
	vid.conwidth = vid.width;
	vid.conheight = vid.height;
	vid.numpages = 1;

	vid.maxwarpwidth = WARP_WIDTH;
	vid.maxwarpheight = WARP_HEIGHT;

	/* alloc zbuffer and surface cache */
	if (d_pzbuffer)
	{
		D_FlushCaches();
		Hunk_FreeToHighMark (VID_highhunkmark);
		d_pzbuffer = NULL;
		vid_surfcache = NULL;
	}

	bsize = vid.rowbytes * vid.height;
	tsize = D_SurfaceCacheForRes (vid.width, vid.height);
	zsize = vid.width * vid.height * sizeof(*d_pzbuffer);

	VID_highhunkmark = Hunk_HighMark ();

	d_pzbuffer = (short *) Hunk_HighAllocName (bsize + tsize + zsize, "video");

	vid_surfcache = ((byte *)d_pzbuffer) + zsize;

	vid.conbuffer = vid.buffer = (pixel_t *)(((byte *)d_pzbuffer) + zsize + tsize);

	D_InitCaches (vid_surfcache, tsize);

	/* get goin' */
	if (vga_setmode(current_mode) != 0)
		Sys_Error("Unable to set mode %d", current_mode);

	VGA_pagebase = vid.direct = framebuffer_ptr = (byte *) vga_getgraphmem();
	/*
	if (vga_setlinearaddressing() > 0)
		framebuffer_ptr = (char *) vga_getgraphmem();
	*/
	if (!framebuffer_ptr)
		Sys_Error("Unable to get framebuffer ptr for mode %d", current_mode);

	VID_SetPalette(palette);
	vid.recalc_refdef = 1;	/* force a surface cache flush */

	vga_setpage(0);
	svgalib_inited = 1;

	return true;
}

/* backgrounding fixes (quakeforge) */
static void goto_background (void)
{
	svgalib_backgrounded = 1;
}

static void comefrom_background (void)
{
	svgalib_backgrounded = 0;
	VID_SetPalette(vid_current_palette);
}

void VID_Init (unsigned char *palette)
{
	int	i, w, h, d;
	char	*modename;

	if (svgalib_inited)
		return;

	if (vga_init() != 0)
		Sys_Error ("SVGALib failed to allocate a new VC");

	i = vga_runinbackground_version();
	if (i > 0)
	{
		Sys_Printf ("SVGALIB background support %i detected\n", i);
		vga_runinbackground (VGA_GOTOBACK, goto_background);
		vga_runinbackground (VGA_COMEFROMBACK, comefrom_background);
		vga_runinbackground (1);
	}
	else
	{
		vga_runinbackground (0);
	}

	VID_InitModes();
	if (!VID_NumModes())
		Sys_Error ("No supported modes reported by SVGAlib");

//	Cvar_RegisterVariable (&_enable_mouse);
	Cvar_RegisterVariable (&vid_mode);
	Cvar_RegisterVariable (&vid_redrawfull);
	Cvar_RegisterVariable (&vid_waitforrefresh);

//	Cmd_AddCommand ("gamma", VID_Gamma_f);
	Cmd_AddCommand("vid_nummodes", VID_NumModes_f);
	Cmd_AddCommand("vid_describemode", VID_DescribeMode_f);
	Cmd_AddCommand("vid_describemodes", VID_DescribeModes_f);
	Cmd_AddCommand("vid_debug", VID_Debug_f);

/* interpret command-line params */
	current_mode = -1;
	w = h = d = 0;
	modename = getenv("GSVGAMODE");
	if (modename)
		current_mode = get_mode(modename, 0, 0, 0);
	else
	{
		i = COM_CheckParm("-mode");
		if (i)
		{
			if (i >= com_argc - 1)
				Sys_Error("%s: -mode <modename>", __thisfunc__);
			current_mode = get_mode(com_argv[i + 1], 0, 0, 0);
		}
	}
	if (current_mode == -1)
	{
		i = COM_CheckParm("-width");
		if (i)
		{
			if (i >= com_argc - 1)
				Sys_Error("%s: -width <width>", __thisfunc__);
			w = atoi(com_argv[i + 1]);
		}
		i = COM_CheckParm("-height");
		if (i)
		{
			if (i >= com_argc - 1)
				Sys_Error("%s: -height <height>", __thisfunc__);
			h = atoi(com_argv[i + 1]);
		}
		i = COM_CheckParm("-bpp");
		if (i)
		{
			if (i >= com_argc - 1)
				Sys_Error("%s: -bpp <depth>", __thisfunc__);
			d = atoi(com_argv[i + 1]);
		}
		if (w || h || d)
			current_mode = get_mode(NULL, w, h, d);
	}
	if (current_mode == -1)
	{
		if (vga_hasmode(G320x200x256))
			current_mode = G320x200x256;
		else
		{
			Sys_Printf ("Mode 13h not supported\n");
#if defined(G320x200x256V) /* svgalib-1.9 may do this */
			if (vga_hasmode(G320x200x256V))
				current_mode = G320x200x256V;
			else
#endif
			/* try the first available */
			for (i = 0; i < num_modes; i++)
			{
				if (modes[i].width != 0)
				{
					current_mode = i;
					break;
				}
			}
			Sys_Printf ("Will try setting mode %d\n", current_mode);
		}
	}

/* set vid parameters */
	if (!VID_SetMode (current_mode, palette))
		Sys_Error ("Unable to set a video mode");

	VID_SetPalette(palette);
}

void VID_Update (vrect_t *rects)
{
	if (!svgalib_inited)
		return;
	if (svgalib_backgrounded)
		return;
	if (!vga_oktowrite())
		return;	/* can't update screen if it's not active */

	if (vid_waitforrefresh.integer)
		vga_waitretrace();

	if (VGA_planar)
	{
	/* VGA_UpdatePlanarScreen() of d_copy.asm only works with
	 * /dev/mem, i.e. without the svgalib_helper kernel module
	 * introduced in svgalib-1.9.x.  must use an appropriate
	 * svgalib api function, such as vga_copytoplanar256() */
		vga_copytoplanar256(vid.buffer, VGA_bufferrowbytes, 0, VGA_rowbytes, VGA_width, VGA_height);
	}
	else if (vid_redrawfull.integer)
	{
		int total = vid.rowbytes * vid.height;
		int offset;

		for (offset = 0; offset < total; offset += 0x10000)
		{
			vga_setpage(offset / 0x10000);
			memcpy(framebuffer_ptr, vid.buffer + offset, ((total-offset > 0x10000) ? 0x10000 : (total - offset)));
		}
	}
	else
	{
		int ycount;
		int offset;
		int vidpage = 0;

		vga_setpage(0);

		while (rects)
		{
			ycount = rects->height;
			offset = rects->y * vid.rowbytes + rects->x;
			while (ycount--)
			{
				register int i = offset % 0x10000;

				if ((offset / 0x10000) != vidpage)
				{
					vidpage = offset / 0x10000;
					vga_setpage(vidpage);
				}
				if (rects->width + i > 0x10000)
				{
					memcpy(framebuffer_ptr + i, vid.buffer + offset, 0x10000 - i);
					vga_setpage(++vidpage);
					memcpy(framebuffer_ptr, vid.buffer + offset + 0x10000 - i, rects->width - 0x10000 + i);
				}
				else
					memcpy(framebuffer_ptr + i, vid.buffer + offset, rects->width);

				offset += vid.rowbytes;
			}

			rects = rects->pnext;
		}
	}

	if (vid_mode.integer != current_mode)
		VID_SetMode (vid_mode.integer, vid_current_palette);
}


/*
================
D_BeginDirectRect
================
*/
void D_BeginDirectRect (int x, int y, byte *pbitmap, int width, int height)
{
	int	i, j, reps, repshift, offset, vidpage, off;

	if (!svgalib_inited || !vid.direct || !vga_oktowrite())
		return;
	if (svgalib_backgrounded)
		return;

	if (vid.aspect > 1.5)
	{
		reps = 2;
		repshift = 1;
	}
	else
	{
		reps = 1;
		repshift = 0;
	}

	vidpage = 0;
	vga_setpage(0);

	if (VGA_planar)
	{
#if 0
	/* with svgalib-1.9.x, we can't use outb() directly because
	 * it only works for /dev/mem, i.e. without svgalib_helper
	 * kernel module.  the equivalent __svgalib_port_out () is
	 * an internal helper function of svgalib (libvga.h) and is
	 * not officially exported.  therefore disabling this part
	 * which isn't an important functionality at all. */
		int			k, plane;
		for (plane = 0; plane < 4; plane++)
		{
		/* select the correct plane for reading and writing */
			outb(0x02, 0x3C4);
			outb(1 << plane, 0x3C5);
			outb(4, 0x3CE);
			outb(plane, 0x3CF);

			for (i = 0; i < (height << repshift); i += reps)
			{
				for (k = 0; k < reps; k++)
				{
					for (j = 0; j < (width >> 2); j++)
					{
						backingbuf[(i + k) * 24 + (j << 2) + plane] =
								vid.direct[(y + i + k) * VGA_rowbytes +
								(x >> 2) + j];
						vid.direct[(y + i + k) * VGA_rowbytes + (x>>2) + j] =
								pbitmap[(i >> repshift) * 24 +
								(j << 2) + plane];
					}
				}
			}
		}
#endif
	}
	else
	{
		for (i = 0; i < (height << repshift); i += reps)
		{
			for (j = 0; j < reps; j++)
			{
				offset = x + ((y << repshift) + i + j) * vid.rowbytes;
				off = offset % 0x10000;
				if ((offset / 0x10000) != vidpage)
				{
					vidpage = offset / 0x10000;
					vga_setpage(vidpage);
				}
				memcpy (&backingbuf[(i + j) * 24], vid.direct + off, width);
				memcpy (vid.direct + off, &pbitmap[(i >> repshift)*width], width);
			}
		}
	}
}

/*
================
D_EndDirectRect
================
*/
void D_EndDirectRect (int x, int y, int width, int height)
{
	int	i, j, reps, repshift, offset, vidpage, off;

	if (!svgalib_inited || !vid.direct || !vga_oktowrite())
		return;
	if (svgalib_backgrounded)
		return;

	if (vid.aspect > 1.5)
	{
		reps = 2;
		repshift = 1;
	}
	else
	{
		reps = 1;
		repshift = 0;
	}

	vidpage = 0;
	vga_setpage(0);

	if (VGA_planar)
	{
#if 0	/* see in D_BeginDirectRect() */
		int			k, plane;
		for (plane = 0; plane < 4; plane++)
		{
		/* select the correct plane for writing */
			outb(2, 0x3C4);
			outb(1 << plane, 0x3C5);
			outb(4, 0x3CE);
			outb(plane, 0x3CF);

			for (i = 0; i < (height << repshift); i += reps)
			{
				for (k = 0; k < reps; k++)
				{
					for (j = 0; j < (width >> 2); j++)
					{
						vid.direct[(y + i + k) * VGA_rowbytes + (x>>2) + j] =
								backingbuf[(i + k) * 24 + (j << 2) + plane];
					}
				}
			}
		}
#endif
	}
	else
	{
		for (i = 0; i < (height << repshift); i += reps)
		{
			for (j = 0; j < reps; j++)
			{
				offset = x + ((y << repshift) + i + j) * vid.rowbytes;
				off = offset % 0x10000;
				if ((offset / 0x10000) != vidpage)
				{
					vidpage = offset / 0x10000;
					vga_setpage(vidpage);
				}
				memcpy (vid.direct + off, &backingbuf[(i +j)*24], width);
			}
		}
	}
}

void D_ShowLoadingSize (void)
{
#if defined(DRAW_PROGRESSBARS)
/* to be implemented. */
#endif	/* !DRAW_PROGRESSBARS */
}

void VID_LockBuffer (void)
{
/* nothing to do */
}

void VID_UnlockBuffer (void)
{
/* nothing to do */
}


void VID_HandlePause (qboolean paused)
{
	if (paused)	IN_DeactivateMouse ();
	else		IN_ActivateMouse ();
}

