/*
 * vid_dos.c -- DOS-specific video routines.
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

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dos.h>
#include <dpmi.h>
#include <go32.h>

#include "quakedef.h"
#include "d_local.h"
#include "dosisms.h"
#include "vid_dos.h"
#include "cfgfile.h"

modestate_t	modestate = MS_UNINIT;
static int	vid_modenum;
static vmode_t	*pcurrentmode = NULL;
static int	vid_testingmode, vid_realmode;
static double	vid_testendtime;

static cvar_t	vid_mode = {"vid_mode", "0", CVAR_NONE};
cvar_t		vid_wait = {"vid_wait", "0", CVAR_NONE};
cvar_t		vid_nopageflip = {"vid_nopageflip", "0", CVAR_ARCHIVE};
cvar_t		_vid_wait_override = {"_vid_wait_override", "0", CVAR_ARCHIVE};
static cvar_t	_vid_default_mode = {"_vid_default_mode", "0", CVAR_ARCHIVE};
// compatibility with windows version:
static cvar_t	_vid_default_mode_win = {"_vid_default_mode_win", "1", CVAR_ARCHIVE};
static cvar_t	vid_config_x = {"vid_config_x", "800", CVAR_ARCHIVE};
static cvar_t	vid_config_y = {"vid_config_y", "600", CVAR_ARCHIVE};
static cvar_t	vid_stretch_by_2 = {"vid_stretch_by_2", "1", CVAR_ARCHIVE};
cvar_t		_enable_mouse = {"_enable_mouse", "0", CVAR_ARCHIVE};
static cvar_t	vid_fullscreen_mode = {"vid_fullscreen_mode", "3", CVAR_ARCHIVE};
static cvar_t	vid_windowed_mode = {"vid_windowed_mode", "0", CVAR_ARCHIVE};

viddef_t	vid;		/* global video state	*/
int		numvidmodes;
vmode_t		*pvidmodes;

static int	firstupdate = 1;

static void VID_TestMode_f (void);
static void VID_NumModes_f (void);
static void VID_DescribeCurrentMode_f (void);
static void VID_DescribeMode_f (void);
static void VID_DescribeModes_f (void);

static qboolean VID_SetMode (int modenum, unsigned char *palette);

static byte	vid_current_palette[768];	/* save for mode changes */

static qboolean	nomodecheck = false;

unsigned short	d_8to16table[256];	/* not used in 8 bpp mode */
unsigned int	d_8to24table[256];	/* not used in 8 bpp mode */

byte		globalcolormap[VID_GRADES*256], lastglobalcolor = 0;
byte		*lastsourcecolormap = NULL;

static void VID_MenuDraw (void);
static void VID_MenuKey (int key);


/*
================
VID_Init
================
*/
void    VID_Init (unsigned char *palette)
{
	const char	*read_vars[] = {
				"_vid_default_mode" };
#define num_readvars	( sizeof(read_vars)/sizeof(read_vars[0]) )

	Cvar_RegisterVariable (&vid_mode);
	Cvar_RegisterVariable (&vid_wait);
	Cvar_RegisterVariable (&vid_nopageflip);
	Cvar_RegisterVariable (&_vid_wait_override);
	Cvar_RegisterVariable (&_vid_default_mode);
	Cvar_RegisterVariable (&_vid_default_mode_win);
	Cvar_RegisterVariable (&vid_config_x);
	Cvar_RegisterVariable (&vid_config_y);
	Cvar_RegisterVariable (&vid_stretch_by_2);
	Cvar_RegisterVariable (&_enable_mouse);
	Cvar_RegisterVariable (&vid_fullscreen_mode);
	Cvar_RegisterVariable (&vid_windowed_mode);

	Cmd_AddCommand ("vid_testmode", VID_TestMode_f);
	Cmd_AddCommand ("vid_nummodes", VID_NumModes_f);
	Cmd_AddCommand ("vid_describecurrentmode", VID_DescribeCurrentMode_f);
	Cmd_AddCommand ("vid_describemode", VID_DescribeMode_f);
	Cmd_AddCommand ("vid_describemodes", VID_DescribeModes_f);

// perform an early read of config.cfg
	CFG_ReadCvars (read_vars, num_readvars);

// set up the mode list; note that later inits link in their modes ahead of
// earlier ones, so the standard VGA modes are always first in the list. This
// is important because mode 0 must always be VGA mode 0x13
	if (!safemode && !COM_CheckParm("-stdvid"))
		VID_InitExtra ();
	VGA_Init ();

	vid_testingmode = 0;
	vid_modenum = vid_mode.integer;

	if (_vid_default_mode.integer < 0 || _vid_default_mode.integer >= numvidmodes)
		Cvar_SetQuick (&_vid_default_mode, "0");
	Cvar_LockVar ("_vid_default_mode");

	Cvar_SetROM ("sys_nostdout", "1");	// disable printing to terminal
	VID_SetMode (vid_modenum, palette);

	vid_realmode = vid_modenum;

	vid_menudrawfn = VID_MenuDraw;
	vid_menukeyfn = VID_MenuKey;
}


/*
=================
VID_GetModePtr
=================
*/
static vmode_t *VID_GetModePtr (int modenum)
{
	vmode_t	*pv;

	pv = pvidmodes;
	if (!pv)
		Sys_Error ("VID_GetModePtr: empty vid mode list");

	while (modenum--)
	{
		pv = pv->pnext;
		if (!pv)
			Sys_Error ("VID_GetModePtr: corrupt vid mode list");
	}

	return pv;
}

/*
================
VID_NumModes
================
*/
static int VID_NumModes (void)
{
	return (numvidmodes);
}


/*
================
VID_ModeInfo
================
*/
static const char *VID_ModeInfo (int modenum, const char **ppheader)
{
	static const char	badmodestr[] = "Bad mode number";
	vmode_t		*pv;

	pv = VID_GetModePtr (modenum);

	if (!pv)
	{
		if (ppheader)
			*ppheader = NULL;
		return badmodestr;
	}
	else
	{
		if (ppheader)
			*ppheader = pv->header;
		return pv->name;
	}
}


/*
================
VID_SetMode
================
*/
static qboolean VID_SetMode (int modenum, unsigned char *palette)
{
	int		status;
	vmode_t	*pnewmode, *poldmode;

	if ((modenum >= numvidmodes) || (modenum < 0))
	{
		Cvar_SetValueQuick (&vid_mode, (float)vid_modenum);

		nomodecheck = true;
		Con_Printf ("No such video mode: %d\n", modenum);
		nomodecheck = false;

		if (pcurrentmode == NULL)
		{
			modenum = 0;	// mode hasn't been set yet, so initialize to base
					//  mode since they gave us an invalid initial mode
		}
		else
		{
			return false;
		}
	}

	pnewmode = VID_GetModePtr (modenum);

	if (pnewmode == pcurrentmode)
		return true;	// already in the desired mode

// initialize the new mode
	poldmode = pcurrentmode;
	pcurrentmode = pnewmode;

	vid.width = pcurrentmode->width;
	vid.height = pcurrentmode->height;
	vid.aspect = pcurrentmode->aspect;
	vid.rowbytes = pcurrentmode->rowbytes;

	status = (*pcurrentmode->setmode) (&vid, pcurrentmode);

	if (status < 1)
	{
		if (status == 0)
		{
		// real, hard failure that requires resetting the mode
			if (!VID_SetMode (vid_modenum, palette))	// restore prior mode
				Sys_Error ("VID_SetMode: Unable to set any mode, probably "
						   "because there's not enough memory available");
			Con_Printf ("Failed to set mode %d\n", modenum);
			return false;
		}
		else if (status == -1)
		{
		// not enough memory; just put things back the way they were
			pcurrentmode = poldmode;
			vid.width = pcurrentmode->width;
			vid.height = pcurrentmode->height;
			vid.aspect = pcurrentmode->aspect;
			vid.rowbytes = pcurrentmode->rowbytes;
			return false;
		}
		else
		{
			Sys_Error ("VID_SetMode: invalid setmode return code %d", status);
		}
	}

	(*pcurrentmode->setpalette) (&vid, pcurrentmode, palette);

	vid_modenum = modenum;
	Cvar_SetValueQuick (&vid_mode, (float)vid_modenum);

	nomodecheck = true;
	Con_Printf ("%s\n", VID_ModeInfo (vid_modenum, NULL));
	nomodecheck = false;

	vid.recalc_refdef = 1;

	return true;
}


/*
================
VID_SetPalette
================
*/
void VID_SetPalette (unsigned char *palette)
{
	if (palette != vid_current_palette)
		memcpy(vid_current_palette, palette, 768);
	(*pcurrentmode->setpalette)(&vid, pcurrentmode, vid_current_palette);
}


/*
================
VID_ShiftPalette
================
*/
void VID_ShiftPalette (unsigned char *palette)
{
	VID_SetPalette (palette);
}


/*
================
VID_Shutdown
================
*/
void VID_Shutdown (void)
{
	regs.h.ah = 0;
	regs.h.al = 0x3;
	dos_int86(0x10);

	vid_testingmode = 0;
}


/*
================
VID_Update
================
*/
void VID_Update (vrect_t *rects)
{
	if (firstupdate && host_initialized)
	{
		firstupdate = 0;
		Cvar_SetValueQuick (&vid_mode, _vid_default_mode.integer);
	}

	(*pcurrentmode->swapbuffers)(&vid, pcurrentmode, rects);

	if (nomodecheck)
		return;

	if (vid_testingmode)
	{
		if (realtime >= vid_testendtime)
		{
			VID_SetMode (vid_realmode, vid_current_palette);
			vid_testingmode = 0;
		}
	}
	else
	{
		if (vid_mode.integer != vid_realmode)
		{
			VID_SetMode (vid_mode.integer, vid_current_palette);
			Cvar_SetValueQuick (&vid_mode, (float)vid_modenum);
							// so if mode set fails, we don't keep on
							//  trying to set it
			vid_realmode = vid_modenum;
		}
	}
}


/*
=================
VID_NumModes_f
=================
*/
static void VID_NumModes_f (void)
{
	int		nummodes;

	nummodes = VID_NumModes ();
	if (nummodes == 1)
		Con_Printf ("%d video mode is available\n", VID_NumModes ());
	else
		Con_Printf ("%d video modes are available\n", VID_NumModes ());
}


/*
=================
VID_DescribeCurrentMode_f
=================
*/
static void VID_DescribeCurrentMode_f (void)
{
	Con_Printf ("%s\n", VID_ModeInfo (vid_modenum, NULL));
}


/*
=================
VID_DescribeMode_f
=================
*/
void VID_DescribeMode_f (void)
{
	int		modenum;

	modenum = atoi (Cmd_Argv(1));

	Con_Printf ("%s\n", VID_ModeInfo (modenum, NULL));
}


/*
=================
VID_DescribeModes_f
=================
*/
void VID_DescribeModes_f (void)
{
	int		i, nummodes;
	const char	*pinfo;
	const char	*pheader;
	vmode_t		*pv;
	qboolean	na;

	na = false;

	nummodes = VID_NumModes ();
	for (i = 0; i < nummodes; i++)
	{
		pv = VID_GetModePtr (i);
		pinfo = VID_ModeInfo (i, &pheader);
		if (pheader)
			Con_Printf ("\n%s\n", pheader);

		if (VGA_CheckAdequateMem (pv->width, pv->height, pv->rowbytes,
				(pv->numpages == 1) || vid_nopageflip.integer))
		{
			Con_Printf ("%2d: %s\n", i, pinfo);
		}
		else
		{
			Con_Printf ("**: %s\n", pinfo);
			na = true;
		}
	}

	if (na)
	{
		Con_Printf ("\n[**: not enough system RAM for mode]\n");
	}
}


/*
=================
VID_GetModeDescription
=================
*/
static const char *VID_GetModeDescription (int modenum)
{
	const char	*pinfo;
	const char	*pheader;
	vmode_t		*pv;

	pv = VID_GetModePtr (modenum);
	pinfo = VID_ModeInfo (modenum, &pheader);

	if (VGA_CheckAdequateMem (pv->width, pv->height, pv->rowbytes,
			(pv->numpages == 1) || vid_nopageflip.integer))
	{
		return pinfo;
	}
	else
	{
		return NULL;
	}
}


/*
=================
VID_TestMode_f
=================
*/
static void VID_TestMode_f (void)
{
	int		modenum;
	double	testduration;

	if (!vid_testingmode)
	{
		modenum = atoi (Cmd_Argv(1));

		if (VID_SetMode (modenum, vid_current_palette))
		{
			vid_testingmode = 1;
			testduration = atof (Cmd_Argv(2));
			if (testduration == 0)
				testduration = 5.0;
			vid_testendtime = realtime + testduration;
		}
	}
}


/*
================
D_BeginDirectRect
================
*/
void D_BeginDirectRect (int x, int y, byte *pbitmap, int width, int height)
{

	if (!vid.direct || !pcurrentmode)
		return;

	if ((width > 24) || (height > 24) || (width < 1) || (height < 1))
		return;

	if (width & 0x03)
		return;

	(*pcurrentmode->begindirectrect) (&vid, pcurrentmode, x, y, pbitmap, width, height);
}

/*
================
D_EndDirectRect
================
*/
void D_EndDirectRect (int x, int y, int width, int height)
{

	if (!vid.direct || !pcurrentmode)
		return;

	if ((width > 24) || (height > 24) || (width < 1) || (height < 1))
		return;

	if ((width & 0x03) || (height & 0x03))
		return;

	(*pcurrentmode->enddirectrect) (&vid, pcurrentmode, x, y, width, height);
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


//===========================================================================

static int	vid_line, vid_wmodes, vid_column_size;

typedef struct
{
	int		modenum;
	const char	*desc;
	int		iscur;
} modedesc_t;

#define	MAX_COLUMN_SIZE	11
#define MAX_MODEDESCS	(MAX_COLUMN_SIZE * 3)

static modedesc_t	modedescs[MAX_MODEDESCS];

/*
================
VID_MenuDraw
================
*/
static void VID_MenuDraw (void)
{
	const char	*ptr;
	int		nummodes, i, j, column, row, dup;
	char		temp[100];

	vid_wmodes = 0;
	nummodes = VID_NumModes ();

	ScrollTitle("gfx/menu/title7.lmp");

	for (i = 0; i < nummodes; i++)
	{
		if (vid_wmodes < MAX_MODEDESCS)
		{
			if (i != 1)
			{
				ptr = VID_GetModeDescription (i);

				if (ptr)
				{
					dup = 0;

					for (j = 0; j < vid_wmodes; j++)
					{
						if (!strcmp (modedescs[j].desc, ptr))
						{
							if (modedescs[j].modenum != 0)
							{
								modedescs[j].modenum = i;
								dup = 1;

								if (i == vid_modenum)
									modedescs[j].iscur = 1;
							}
							else
							{
								dup = 1;
							}

							break;
						}
					}

					if (!dup)
					{
						modedescs[vid_wmodes].modenum = i;
						modedescs[vid_wmodes].desc = ptr;
						modedescs[vid_wmodes].iscur = 0;

						if (i == vid_modenum)
							modedescs[vid_wmodes].iscur = 1;

						vid_wmodes++;
					}
				}
			}
		}
	}

	vid_column_size = (vid_wmodes + 2) / 3;

	column = 16;
	row = 36;

	for (i = 0; i < vid_wmodes; i++)
	{
		if (modedescs[i].iscur)
			M_PrintWhite (column, row, modedescs[i].desc);
		else
			M_Print (column, row, modedescs[i].desc);

		row += 8;

		if ((i % vid_column_size) == (vid_column_size - 1))
		{
			column += 13*8;
			row = 36;
		}
	}

// line cursor
	if (vid_testingmode)
	{
		q_snprintf (temp, sizeof(temp), "TESTING %s",
				modedescs[vid_line].desc);
		M_Print (13*8, 36 + MAX_COLUMN_SIZE * 8 + 8*4, temp);
		M_Print (9*8, 36 + MAX_COLUMN_SIZE * 8 + 8*6,
				"Please wait 5 seconds...");
	}
	else
	{
		M_Print (9*8, 36 + MAX_COLUMN_SIZE * 8 + 8,
				"Press Enter to set mode");
		M_Print (6*8, 36 + MAX_COLUMN_SIZE * 8 + 8*3,
				"T to test mode for 5 seconds");
		ptr = VID_GetModeDescription (vid_modenum);
		q_snprintf (temp, sizeof(temp), "D to make %s the default", ptr);
		M_Print (6*8, 36 + MAX_COLUMN_SIZE * 8 + 8*5, temp);
		ptr = VID_GetModeDescription (_vid_default_mode.integer);

		if (ptr)
		{
			q_snprintf (temp, sizeof(temp), "Current default is %s", ptr);
			M_Print (7*8, 36 + MAX_COLUMN_SIZE * 8 + 8*6, temp);
		}

		M_Print (15*8, 36 + MAX_COLUMN_SIZE * 8 + 8*8,
				"Esc to exit");

		row = 36 + (vid_line % vid_column_size) * 8;
		column = 8 + (vid_line / vid_column_size) * 13*8;

		M_DrawCharacter (column, row, 12+((int)(realtime*4)&1));
	}
}


/*
================
VID_MenuKey
================
*/
static void VID_MenuKey (int key)
{
	if (vid_testingmode)
		return;

	switch (key)
	{
	case K_ESCAPE:
		S_LocalSound ("raven/menu1.wav");
		M_Menu_Options_f ();
		break;

	case K_UPARROW:
		S_LocalSound ("raven/menu1.wav");
		vid_line--;

		if (vid_line < 0)
			vid_line = vid_wmodes - 1;
		break;

	case K_DOWNARROW:
		S_LocalSound ("raven/menu1.wav");
		vid_line++;

		if (vid_line >= vid_wmodes)
			vid_line = 0;
		break;

	case K_LEFTARROW:
		S_LocalSound ("raven/menu1.wav");
		vid_line -= vid_column_size;

		if (vid_line < 0)
		{
			vid_line += ((vid_wmodes + (vid_column_size - 1)) / vid_column_size) * vid_column_size;
			while (vid_line >= vid_wmodes)
				vid_line -= vid_column_size;
		}
		break;

	case K_RIGHTARROW:
		S_LocalSound ("raven/menu1.wav");
		vid_line += vid_column_size;

		if (vid_line >= vid_wmodes)
		{
			vid_line -= ((vid_wmodes + (vid_column_size - 1)) / vid_column_size) * vid_column_size;
			while (vid_line < 0)
				vid_line += vid_column_size;
		}
		break;

	case K_ENTER:
		S_LocalSound ("raven/menu1.wav");
		VID_SetMode (modedescs[vid_line].modenum, vid_current_palette);
		break;

	case 'T':
	case 't':
		S_LocalSound ("raven/menu1.wav");
		if (VID_SetMode (modedescs[vid_line].modenum, vid_current_palette))
		{
			vid_testingmode = 1;
			vid_testendtime = realtime + 5.0;
		}
		break;

	case 'D':
	case 'd':
		S_LocalSound ("raven/menu1.wav");
		firstupdate = 0;
		Cvar_SetValueQuick (&_vid_default_mode, vid_modenum);
		break;

	default:
		break;
	}
}

