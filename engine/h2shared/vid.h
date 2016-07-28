/*
 * vid.h -- video driver defs
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

#ifndef __VID_DEFS_H
#define __VID_DEFS_H

#define VID_CBITS	6
#define VID_GRADES	(1 << VID_CBITS)

#define GAMMA_MAX	3.0

// a pixel can be one, two, or four bytes
typedef byte pixel_t;

typedef struct vrect_s
{
	int		x, y, width, height;
	struct vrect_s	*pnext;
} vrect_t;

typedef enum
{
	MS_WINDOWED,
	MS_FULLSCREEN,
	MS_FULLDIB,
	MS_FULLDIRECT,
	MS_UNINIT
} modestate_t;

#define MODE_WINDOWED		0
#define NO_MODE			(MODE_WINDOWED - 1)
#define MODE_FULLSCREEN_DEFAULT	(MODE_WINDOWED + 3)
#define MODE_SETTABLE_WINDOW	2

typedef struct
{
	pixel_t		*buffer;	// invisible buffer
	pixel_t		*colormap;	// 256 * VID_GRADES size
	unsigned short	*colormap16;	// 256 * VID_GRADES size
	int		fullbright;	// index of first fullbright color
	int		rowbytes;	// may be > width if displayed in a window
	int		width;
	int		height;
	float		aspect;		// width / height -- < 0 is taller than wide
	int		numpages;
	int		recalc_refdef;	// if true, recalc vid-based stuff
	pixel_t		*conbuffer;
	int		conrowbytes;
	int		conwidth;
	int		conheight;
	int		maxwarpwidth;
	int		maxwarpheight;
	pixel_t		*direct;	// direct drawing to framebuffer,
					// if not NULL
#if defined(PLATFORM_AMIGAOS3) && !defined(GLQUAKE)
	qboolean	noadapt;	// no fov_adapt for Amiga native chipset modes
#endif
} viddef_t;

extern	byte		globalcolormap[VID_GRADES*256];
extern	byte		lastglobalcolor, *lastsourcecolormap;

extern	viddef_t	vid;			// global video state
extern	modestate_t	modestate;
extern	qboolean	in_mode_set;

extern	unsigned short	d_8to16table[256];
extern	unsigned int	d_8to24table[256];
extern	unsigned int	d_8to24TranslucentTable[256];

extern	cvar_t		_enable_mouse;

void VID_SetPalette (const unsigned char *palette);
// called at startup and after any gamma correction

void VID_ShiftPalette (const unsigned char *palette);
// called for bonus and pain flashes, and for underwater color changes
// in gl mode, used to update hardware gamma.

void VID_Init (const unsigned char *palette);
// Called at startup to set up translation tables, takes 256 8 bit RGB values
// the palette data will go away after the call, so it must be copied off if
// the video driver will need it again

void VID_Shutdown (void);
// Called at shutdown

void VID_Update (vrect_t *rects);
// flushes the given rectangles from the view buffer to the screen

void VID_LockBuffer (void);
void VID_UnlockBuffer (void);
// video buffer locking. some drivers don't need it.

void VID_HandlePause (qboolean paused);
// releases the mouse when pause happens

void VID_ToggleFullscreen (void);	// from Steven
// toggles between windowed/fullscreen modes. for unix/sdl

#if defined(GLQUAKE)
void VID_ChangeConsize (int dir);
// changes effective console size. callback for the opengl features menu

float VID_ReportConsize(void);
// returns console size scale for the opengl features menu.
#endif	/* ! GLQUAKE */

void D_ShowLoadingSize (void);
// displays progress bars while loading a map. (not used in hexenworld.)

extern void (*vid_menudrawfn)(void);
extern void (*vid_menukeyfn)(int key);
// video menu function pointers

#if !defined(PLATFORM_WINDOWS)
#define	msg_suppress_1		false
#else
extern	qboolean	msg_suppress_1;
// suppresses resolution and cache size console output a fullscreen
// DIB focus gain/loss. used by the Win32 software (MGL) renderer.
#endif

#endif	/* __VID_DEFS_H */

