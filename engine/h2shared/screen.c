/*
 * screen.c -- master for refresh, status bar, console, chat, notify, etc
 * $Id$
 *
 * Copyright (C) 1996-1997  Id Software, Inc.
 * Copyright (C) 1997-1998  Raven Software Corp.
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

/*=============================================================================

	background clear
	rendering
	turtle/net/ram icons
	sbar
	centerprint / slow centerprint
	notify lines
	intermission / finale overlay
	loading plaque
	console
	menu

	required background clears
	required update regions

	syncronous draw mode or async
	One off screen buffer, with updates either copied or xblited
	Need to double buffer?

	async draw will require the refresh area to be cleared, because
	it will be xblited, but sync draw can just ignore it.

	sync
	draw

	CenterPrint ()
	SlowPrint ()
	Screen_Update ();
	Con_Printf ();

	net
	turn off messages option

	the refresh is always rendered, unless the console is full screen

	console is:
		notify lines
		half
		full

=============================================================================*/

#include "quakedef.h"
//#include "r_local.h"
#ifdef PLATFORM_WINDOWS
#include "winquake.h"
#endif


static qboolean	scr_initialized;	// ready to draw

vrect_t		scr_vrect;
//vrect_t		*pconupdate;

/* these are only functional in the software renderer */
int		scr_copytop;		// only the refresh window will be updated
int		scr_copyeverything;	// unless these variables are flagged
int		scr_topupdate;
int		scr_fullupdate;
static qboolean	scr_needfull = false;

static int	clearconsole;
int		clearnotify;

float		scr_con_current;
float		scr_conlines;		// lines of console to display

int		trans_level = 0;

cvar_t		scr_viewsize = {"viewsize", "110", CVAR_ARCHIVE};
cvar_t		scr_fov = {"fov", "90", CVAR_NONE};	// 10 - 170
cvar_t		scr_fov_adapt = {"fov_adapt", "1", CVAR_ARCHIVE};	// "Hor+" scaling
cvar_t		scr_contrans = {"contrans", "0", CVAR_ARCHIVE};
static	cvar_t	scr_conspeed = {"scr_conspeed", "300", CVAR_NONE};
static	cvar_t	scr_centertime = {"scr_centertime", "4", CVAR_NONE};
static	cvar_t	scr_showram = {"showram", "1", CVAR_NONE};
static	cvar_t	scr_showturtle = {"showturtle", "0", CVAR_NONE};
static	cvar_t	scr_showpause = {"showpause", "1", CVAR_NONE};
static	cvar_t	scr_showfps = {"showfps", "0", CVAR_NONE};

#if !defined(H2W)
static qboolean	scr_drawloading;
static float	scr_disabled_time;
int		total_loading_size, current_loading_size, loading_stage;
#endif	/* H2W */
qboolean	scr_disabled_for_loading;
qboolean	scr_skipupdate;
qboolean	block_drawing;

static qpic_t	*scr_ram;
static qpic_t	*scr_net;
static qpic_t	*scr_turtle;

static void SCR_ScreenShot_f (void);

static const char	*plaquemessage = "";	// pointer to current plaque message

static void Plaque_Draw (const char *message, qboolean AlwaysDraw);
#if !defined(H2W)
/* procedures for the mission pack intro messages and objectives */
static void Info_Plaque_Draw (const char *message);
static void Bottom_Plaque_Draw (const char *message);
#endif	/* H2W */


/*
===============================================================================

CENTER PRINTING

===============================================================================
*/

static char	scr_centerstring[1024];
float		scr_centertime_off;
static int	scr_center_lines;
static int	scr_erase_lines;
//static int	scr_erase_center;

#define	MAXLINES	27
static int	lines;
static int	StartC[MAXLINES], EndC[MAXLINES];

#if !defined(H2W)
/* mission pack objectives: */
#define	MAX_INFO	1024
static char	infomessage[MAX_INFO];

static void UpdateInfoMessage (void)
{
	unsigned int i, check;
	const char *newmessage;

	q_strlcpy(infomessage, "Objectives:", sizeof(infomessage));

	if (!info_string_count)
		return;

	for (i = 0; i < 32; i++)
	{
		check = (1 << i);

		if (cl.info_mask & check)
		{
			newmessage = CL_GetInfoString(i);
			q_strlcat(infomessage, "@@", sizeof(infomessage));
			q_strlcat(infomessage, newmessage, sizeof(infomessage));
		}
	}

	for (i = 0; i < 32; i++)
	{
		check = (1 << i);

		if (cl.info_mask2 & check)
		{
			newmessage = CL_GetInfoString(i + 32);
			q_strlcat(infomessage, "@@", sizeof(infomessage));
			q_strlcat(infomessage, newmessage, sizeof(infomessage));
		}
	}
}
#endif	/* H2W */

static void FindTextBreaks (const char *message, int Width)
{
	int	pos, start, lastspace, oldlast;

	lines = pos = start = 0;
	lastspace = -1;

	while (1)
	{
		if (pos-start >= Width || message[pos] == '@' || message[pos] == 0)
		{
			oldlast = lastspace;
			if (message[pos] == '@' || lastspace == -1 || message[pos] == 0)
				lastspace = pos;

			StartC[lines] = start;
			EndC[lines] = lastspace;
			lines++;
			if (lines == MAXLINES)
				return;
			if (message[pos] == '@')
				start = pos + 1;
			else if (oldlast == -1)
				start = lastspace;
			else
				start = lastspace + 1;

			lastspace = -1;
		}

		if (message[pos] == 32)
			lastspace = pos;
		else if (message[pos] == 0)
			break;

		pos++;
	}
}

/*
==============
SCR_CenterPrint

Called for important messages that should stay in the center of the screen
for a few moments
==============
*/
void SCR_CenterPrint (const char *str)
{
	strncpy (scr_centerstring, str, sizeof(scr_centerstring)-1);
	scr_centertime_off = scr_centertime.value;

	FindTextBreaks(scr_centerstring, 38);
	scr_center_lines = lines;
}

/*
static void SCR_EraseCenterString (void)
{
	int	y, height;

	if (scr_erase_center++ > vid.numpages)
	{
		scr_erase_lines = 0;
		return;
	}

//	y = (25-lines) * 8 / 2;
	y = (scr_center_lines <= 4) ? vid.height*0.35 : 48;

	scr_copytop = 1;
	height = q_min(8 * scr_erase_lines, vid.height - y - 1);
	Draw_TileClear (0, y, vid.width, height);
}
*/

static void SCR_DrawCenterString (void)
{
	int	i, cnt;
	int	bx, by;
	char	temp[80];

//	scr_erase_center = 0;

	FindTextBreaks(scr_centerstring, 38);

	by = (25-lines) * 8 / 2 + ((vid.height - 200)>>1);

	for (i = 0; i < lines; i++, by += 8)
	{
		cnt = EndC[i] - StartC[i];
		strncpy (temp, &scr_centerstring[StartC[i]], cnt);
		temp[cnt] = 0;
		bx = (40-strlen(temp)) * 8 / 2;
		M_Print (bx, by, temp);
	}
}

static qboolean SCR_CheckDrawCenterString2 (void)
{
	scr_copytop = 1;
	if (scr_center_lines > scr_erase_lines)
		scr_erase_lines = scr_center_lines;

	scr_centertime_off -= host_frametime;

	if (scr_centertime_off <= 0 && !cl.intermission)
		return false;
	if (Key_GetDest() != key_game)
		return false;

	return true;
}

static void SCR_CheckDrawCenterString (void)
{
	if (! SCR_CheckDrawCenterString2())
		return;
#if !defined(H2W)
	if (intro_playing)
	{
		Bottom_Plaque_Draw(scr_centerstring);
		return;
	}
#endif	/* H2W */
	SCR_DrawCenterString ();
}

//=============================================================================


/*
====================
AdaptFovx
Adapt a 4:3 horizontal FOV to the current screen size using the "Hor+" scaling:
2.0 * atan(width / height * 3.0 / 4.0 * tan(fov43 / 2.0))
====================
*/
static float AdaptFovx (float fov_x, float width, float height)
{
	float	a, x;

	if (fov_x < 1 || fov_x > 179)
		Sys_Error ("Bad fov: %f", fov_x);

#if defined(PLATFORM_DOS) || defined(SVGAQUAKE)
	if (vid.aspect > 1.10f)
		return fov_x;		/* no fov_adapt for weird VGA modes */
#endif
#if defined(__AMIGA__) && !defined(__MORPHOS__)
	if (vid.noadapt)
		return fov_x;		/* not for AmigaOS native chipset modes. */
#endif
	if (!scr_fov_adapt.integer)
		return fov_x;
	if ((x = height / width) == 0.75)
		return fov_x;
	a = atan(0.75 / x * tan(fov_x / 360 * M_PI));
	a = a * 360 / M_PI;
	return a;
}

/*
====================
CalcFovy
====================
*/
static float CalcFovy (float fov_x, float width, float height)
{
	float	a, x;

	if (fov_x < 1 || fov_x > 179)
		Sys_Error ("Bad fov: %f", fov_x);

	x = width / tan(fov_x / 360 * M_PI);
	a = atan(height / x);
	a = a * 360 / M_PI;
	return a;
}

/*
=================
SCR_CalcRefdef

Must be called whenever vid changes
Internal use only
=================
*/
static void SCR_CalcRefdef (void)
{
	vrect_t		vrect;

	scr_fullupdate = 0;		// force a background redraw

// bound viewsize
	if (scr_viewsize.integer < 30)
		Cvar_SetQuick (&scr_viewsize, "30");
	else if (scr_viewsize.integer > 130)
		Cvar_SetQuick (&scr_viewsize, "130");

// bound field of view
	if (scr_fov.integer < 10)
		Cvar_SetQuick (&scr_fov, "10");
	else if (scr_fov.integer > 110)
		Cvar_SetQuick (&scr_fov, "110");

	vid.recalc_refdef = 0;

// force the status bar to redraw
	SB_ViewSizeChanged ();
	Sbar_Changed();

// intermission is always full screen
	if (scr_viewsize.integer >= 110 || cl.intermission)
		sb_lines = 0;		// no status bar
	else
		sb_lines = 46;

// these calculations mirror those in R_Init() for r_refdef, but take no
// account of water warping
	vrect.x = 0;
	vrect.y = 0;
	vrect.width = vid.width;
	vrect.height = vid.height;

	R_SetVrect (&vrect, &scr_vrect, sb_lines);
	r_refdef.vrect = scr_vrect;
	r_refdef.fov_x = AdaptFovx (scr_fov.value, r_refdef.vrect.width, r_refdef.vrect.height);
	r_refdef.fov_y = CalcFovy (r_refdef.fov_x, r_refdef.vrect.width, r_refdef.vrect.height);

// guard against going from one mode to another that's less than half the
// vertical resolution
	if (scr_con_current > vid.height)
		scr_con_current = vid.height;

// notify the refresh of the change
	R_ViewChanged (vid.aspect);
}

//=============================================================================


/*
=================
SCR_SizeUp_f

Keybinding command
=================
*/
static void SCR_SizeUp_f (void)
{
	Cvar_SetValueQuick (&scr_viewsize, scr_viewsize.integer + 10);
}

/*
=================
SCR_SizeDown_f

Keybinding command
=================
*/
static void SCR_SizeDown_f (void)
{
	Cvar_SetValueQuick (&scr_viewsize, scr_viewsize.integer - 10);
}

static void SCR_Callback_refdef (cvar_t *var)
{
	vid.recalc_refdef = 1;
}

//=============================================================================


/*
==================
SCR_Init
==================
*/
void SCR_Init (void)
{
	scr_ram = Draw_PicFromWad ("ram");
	scr_net = Draw_PicFromWad ("net");
	scr_turtle = Draw_PicFromWad ("turtle");

	if (draw_reinit)
		return;

	Cvar_SetCallback (&scr_fov, SCR_Callback_refdef);
	Cvar_SetCallback (&scr_fov_adapt, SCR_Callback_refdef);
	Cvar_SetCallback (&scr_viewsize, SCR_Callback_refdef);
	Cvar_RegisterVariable (&scr_fov);
	Cvar_RegisterVariable (&scr_fov_adapt);
	Cvar_RegisterVariable (&scr_viewsize);
	Cvar_RegisterVariable (&scr_contrans);
	Cvar_RegisterVariable (&scr_conspeed);
	Cvar_RegisterVariable (&scr_showram);
	Cvar_RegisterVariable (&scr_showturtle);
	Cvar_RegisterVariable (&scr_showpause);
	Cvar_RegisterVariable (&scr_showfps);
	Cvar_RegisterVariable (&scr_centertime);

	Cmd_AddCommand ("screenshot",SCR_ScreenShot_f);
	Cmd_AddCommand ("sizeup",SCR_SizeUp_f);
	Cmd_AddCommand ("sizedown",SCR_SizeDown_f);

	scr_initialized = true;
	con_forcedup = true;	// we're just initialized and not connected yet
}

//=============================================================================


/*
==============
SCR_DrawRam
==============
*/
static void SCR_DrawRam (void)
{
	if (!scr_showram.integer)
		return;

	if (!r_cache_thrash)
		return;

	Draw_Pic (scr_vrect.x+32, scr_vrect.y, scr_ram);
}

/*
==============
SCR_DrawTurtle
==============
*/
static void SCR_DrawTurtle (void)
{
	static int	count;

	if (!scr_showturtle.integer)
		return;

	if (host_frametime < HX_FRAME_TIME)
	{
		count = 0;
		return;
	}

	count++;
	if (count < 3)
		return;

	Draw_Pic (scr_vrect.x, scr_vrect.y, scr_turtle);
}

/*
==============
SCR_DrawNet
==============
*/
static void SCR_DrawNet (void)
{
#if !defined(H2W)
	if (realtime - cl.last_received_message < 0.3)
		return;
#else
	if (cls.netchan.outgoing_sequence -
			cls.netchan.incoming_acknowledged < UPDATE_BACKUP-1)
		return;
#endif
	if (cls.demoplayback)
		return;

	Draw_Pic (scr_vrect.x+64, scr_vrect.y, scr_net);
}

static void SCR_DrawFPS (void)
{
	static double	oldtime = 0;
	static double	lastfps = 0;
	static int	oldframecount = 0;
	double	elapsed_time;
	int	frames;

	elapsed_time = realtime - oldtime;
	frames = r_framecount - oldframecount;

	if (elapsed_time < 0 || frames < 0)
	{
		oldtime = realtime;
		oldframecount = r_framecount;
		return;
	}
	// update value every 3/4 second
	if (elapsed_time > 0.75)
	{
		lastfps = frames / elapsed_time;
		oldtime = realtime;
		oldframecount = r_framecount;
	}

	if (scr_showfps.integer)
	{
		char	st[16];
		int	x, y;
		sprintf(st, "%4.0f FPS", lastfps);
		x = vid.width - strlen(st) * 8 - 8;
		y = vid.height - sb_lines - 8;
	//	Draw_TileClear(x, y, strlen(st) * 8, 8);
		Draw_String(x, y, st);
	}
}

/*
==============
DrawPause
==============
*/
static void SCR_DrawPause (void)
{
	static qboolean	newdraw = false;
	static float	LogoPercent, LogoTargetPercent;
	qpic_t	*pic;
	int	finaly;
	float	delta;

	if (!scr_showpause.integer)	// turn off for screenshots
		return;

	if (!cl.paused)
	{
		newdraw = false;
		return;
	}

	if (!newdraw)
	{
		newdraw = true;
		LogoTargetPercent = 1;
		LogoPercent = 0;
	}

	pic = Draw_CachePic ("gfx/menu/paused.lmp");
//	Draw_Pic ( (vid.width - pic->width)/2, (vid.height - 48 - pic->height)/2, pic);

	if (LogoPercent < LogoTargetPercent)
	{
		delta = ((LogoTargetPercent - LogoPercent) / .5) * host_frametime;
		if (delta < 0.004)
			delta = 0.004;
		LogoPercent += delta;
		if (LogoPercent > LogoTargetPercent)
			LogoPercent = LogoTargetPercent;
	}

	finaly = ((float)pic->height * LogoPercent) - pic->height;
	Draw_TransPicCropped ( (vid.width - pic->width)/2, finaly, pic);
}

#if !defined(H2W)
/*
==============
SCR_DrawLoading
==============
*/
#if !defined(DRAW_PROGRESSBARS)
void SCR_DrawLoading (void)
{
	int	offset;
	qpic_t	*pic;

	if (!scr_drawloading && loading_stage == 0)
		return;

	pic = Draw_CacheLoadingPic ();
	offset = (vid.width - pic->width) / 2;
	Draw_TransPic (offset, 0, pic);
}
#else
void SCR_DrawLoading (void)
{
	int	size, count, offset;
	qpic_t	*pic;

	if (!scr_drawloading && loading_stage == 0)
		return;

	pic = Draw_CachePic ("gfx/menu/loading.lmp");
	offset = (vid.width - pic->width) / 2;
	Draw_TransPic (offset, 0, pic);

	if (loading_stage == 0)
		return;

	size = (total_loading_size) ?
		(current_loading_size * 106 / total_loading_size) : 0;
	offset += 42;

	count = (loading_stage == 1) ? size : 106;
	if (count)
	{
		Draw_Fill (offset, 87+0, count, 1, 136);
		Draw_Fill (offset, 87+1, count, 4, 138);
		Draw_Fill (offset, 87+5, count, 1, 136);
	}

	count = (loading_stage == 2) ? size : 0;
	if (count)
	{
		Draw_Fill (offset, 97+0, count, 1, 168);
		Draw_Fill (offset, 97+1, count, 4, 170);
		Draw_Fill (offset, 97+5, count, 1, 168);
	}
}
#endif	/* !DRAW_PROGRESSBARS */

/*
===============
SCR_BeginLoadingPlaque

================
*/
void SCR_BeginLoadingPlaque (void)
{
	S_StopAllSounds (true);

	if (cls.state != ca_connected)
		return;
	if (cls.signon != SIGNONS)
		return;

// redraw with no console and the loading plaque
	Con_ClearNotify ();
	scr_centertime_off = 0;
	scr_con_current = 0;

	scr_drawloading = true;
	scr_fullupdate = 0;
	Sbar_Changed();
	SCR_UpdateScreen ();
	scr_drawloading = false;

	scr_disabled_for_loading = true;
	scr_disabled_time = realtime;
}

/*
===============
SCR_EndLoadingPlaque

================
*/
void SCR_EndLoadingPlaque (void)
{
	scr_disabled_for_loading = false;
	scr_fullupdate = 0;
	scr_topupdate = 0;
	Con_ClearNotify ();
}
#endif	/* H2W */

//=============================================================================


/*
==================
SCR_SetUpToDrawConsole
==================
*/
static void SCR_SetUpToDrawConsole (void)
{
	Con_CheckResize ();

#if !defined(H2W)
	if (scr_drawloading)
		return;		// never a console with loading plaque

	con_forcedup = !cl.worldmodel || cls.signon != SIGNONS;
#else
	con_forcedup = cls.state != ca_active;
#endif	/* H2W */

// decide on the height of the console
	if (con_forcedup)
	{
		scr_conlines = vid.height;	// full screen
		scr_con_current = scr_conlines;
	}
	else if (Key_GetDest() == key_console)
		scr_conlines = vid.height / 2;	// half screen
	else
		scr_conlines = 0;		// none visible

	if (scr_conlines < scr_con_current)
	{
		scr_con_current -= scr_conspeed.value * host_frametime;
		if (scr_conlines > scr_con_current)
			scr_con_current = scr_conlines;
	}
	else if (scr_conlines > scr_con_current)
	{
		scr_con_current += scr_conspeed.value * host_frametime;
		if (scr_conlines < scr_con_current)
			scr_con_current = scr_conlines;
	}

	if (clearconsole++ < vid.numpages)
	{
		scr_copytop = 1;
		Draw_TileClear (0,(int)scr_con_current,vid.width, vid.height - (int)scr_con_current);
		Sbar_Changed();
	}
	else if (clearnotify++ < vid.numpages)
	{
		scr_copytop = 1;
		Draw_TileClear (0,0,vid.width, con_notifylines);
	}
	else
		con_notifylines = 0;
}

/*
==================
SCR_DrawConsole
==================
*/
static void SCR_DrawConsole (void)
{
	if (scr_con_current)
	{
		scr_copyeverything = 1;
		Con_DrawConsole (scr_con_current);
		clearconsole = 0;
	}
	else
	{
		keydest_t dest = Key_GetDest();
		if (dest == key_game || dest == key_message)
			Con_DrawNotify ();	// only draw notify in game
	}
}


/*
==============================================================================

SCREEN SHOTS

==============================================================================
*/

#if !defined(H2W)	/* FIXME!!! */
typedef struct
{
	char	manufacturer;
	char	version;
	char	encoding;
	char	bits_per_pixel;
	unsigned short	xmin,ymin,xmax,ymax;
	unsigned short	hres,vres;
	unsigned char	palette[48];
	char	reserved;
	char	color_planes;
	unsigned short	bytes_per_line;
	unsigned short	palette_type;
	char	filler[58];
	unsigned char	data;	// unbounded
} pcx_t;
#endif	/* H2W */

/*
==============
WritePCXfile
==============
*/
static int WritePCXfile (const char *filename, byte *data, int width, int height, int rowbytes, byte *palette)
{
	int	i, j;
	size_t	length;
	pcx_t	*pcx;
	byte	*pack;

	pcx = (pcx_t *) Hunk_TempAlloc (width*height*2+1000);
	if (pcx == NULL)
	{
		Con_Printf("%s: not enough memory\n", __thisfunc__);
		return -1;
	}

	pcx->manufacturer = 0x0a;	// PCX id
	pcx->version = 5;		// 256 color
	pcx->encoding = 1;		// uncompressed
	pcx->bits_per_pixel = 8;	// 256 color
	pcx->xmin = 0;
	pcx->ymin = 0;
	pcx->xmax = LittleShort((short)(width-1));
	pcx->ymax = LittleShort((short)(height-1));
	pcx->hres = LittleShort((short)width);
	pcx->vres = LittleShort((short)height);
	memset (pcx->palette, 0, sizeof(pcx->palette));
	pcx->color_planes = 1;		// chunky image
	pcx->bytes_per_line = LittleShort((short)width);
	pcx->palette_type = LittleShort(2);	// not a grey scale
	memset (pcx->filler, 0, sizeof(pcx->filler));

// pack the image
	pack = &pcx->data;

	for (i = 0; i < height; i++)
	{
		for (j = 0; j < width; j++)
		{
			if ( (*data & 0xc0) != 0xc0)
				*pack++ = *data++;
			else
			{
				*pack++ = 0xc1;
				*pack++ = *data++;
			}
		}

		data += rowbytes - width;
	}

// write the palette
	*pack++ = 0x0c;	// palette ID byte
	for (i = 0; i < 768; i++)
		*pack++ = *palette++;

// write output file 
	length = pack - (byte *)pcx;
	return FS_WriteFile(filename, pcx, length);
}

/*
==================
SCR_ScreenShot_f
==================
*/
#if !defined(H2W)
static const char scr_shotbase[] = "shots/hexen00.pcx";
#define SHOTNUM_POS 11
#else
static const char scr_shotbase[] = "shots/hw00.pcx";
#define SHOTNUM_POS 8
#endif
static void SCR_ScreenShot_f (void)
{
	char	pcxname[80];
	char	checkname[MAX_OSPATH];
	int	i;

	FS_MakePath_BUF (FS_USERDIR, NULL, checkname, sizeof(checkname), "shots");
	Sys_mkdir (checkname, false);
	// find a slot to save it to
	q_strlcpy (pcxname, scr_shotbase, sizeof(pcxname));
	for (i = 0; i <= 99; i++)
	{
		pcxname[SHOTNUM_POS+0] = i/10 + '0';
		pcxname[SHOTNUM_POS+1] = i%10 + '0';
		FS_MakePath_BUF (FS_USERDIR, NULL, checkname, sizeof(checkname), pcxname);
		if (Sys_FileType(checkname) == FS_ENT_NONE)
			break;	// file doesn't exist
	}
	if (i == 100)
	{
		Con_Printf ("%s: Couldn't create a PCX file\n", __thisfunc__);
		return;
	}

//
// save the pcx file
//
	D_EnableBackBufferAccess ();	// enable direct drawing of console
					// to back buffer

	i = WritePCXfile (pcxname, vid.buffer, vid.width, vid.height, vid.rowbytes, host_basepal);

	D_DisableBackBufferAccess ();	// for adapters that can't stay mapped
					// in for linear writes all the time

	if (i == 0)
		Con_Printf ("Wrote %s\n", pcxname);
}

//=============================================================================


static const char	*scr_notifystring;
static qboolean	scr_drawdialog;

static void SCR_DrawNotifyString (void)
{
	Plaque_Draw(scr_notifystring, true);
}

/*
==================
SCR_ModalMessage

Displays a text string in the center of the screen
and waits for a Y or N keypress.
==================
*/
int SCR_ModalMessage (const char *text)
{
#if !defined(H2W)
	if (cls.state == ca_dedicated)
		return true;
#endif	/* H2W */
	scr_notifystring = text;

// draw a fresh screen
	scr_fullupdate = 0;
	scr_drawdialog = true;
	SCR_UpdateScreen ();
	scr_drawdialog = false;

	S_ClearBuffer ();		// so dma doesn't loop current sound

	do
	{
		key_count = -1;		// wait for a key down and up
		Sys_SendKeyEvents ();
	} while (key_lastpress != 'y' && key_lastpress != 'n' && key_lastpress != K_ESCAPE);

	scr_fullupdate = 0;
	SCR_UpdateScreen ();

	return key_lastpress == 'y';
}

//=============================================================================

/*
===============
SCR_BringDownConsole

Brings the console down and fades the palettes back to normal
================
*/
#if 0	/* all uses are commented out */
void SCR_BringDownConsole (void)
{
	int	i;

	scr_centertime_off = 0;

	for (i = 0; i < 20 && scr_conlines != scr_con_current; i++)
		SCR_UpdateScreen ();

	cl.cshifts[0].percent = 0;	// no area contents palette on next frame
	VID_SetPalette (host_basepal);
}
#endif

//=============================================================================

void SCR_SetPlaqueMessage (const char *msg)
{
	plaquemessage = msg;
}

static void Plaque_Draw (const char *message, qboolean AlwaysDraw)
{
	int	i, cnt;
	int	bx, by;
	char	temp[80];

	if (scr_con_current == vid.height && !AlwaysDraw)
		return;		// console is full screen

	if (!*message)
		return;

	scr_needfull = true;

	FindTextBreaks(message, PLAQUE_WIDTH);

	by = (25-lines) * 8 / 2 + ((vid.height - 200)>>1);
	M_DrawTextBox (32, by - 16, PLAQUE_WIDTH + 4, lines + 2);

	for (i = 0; i < lines; i++, by += 8)
	{
		cnt = EndC[i] - StartC[i];
		strncpy (temp, &message[StartC[i]], cnt);
		temp[cnt] = 0;
		bx = (40-strlen(temp)) * 8 / 2;
		M_Print (bx, by, temp);
	}
}

#if !defined(H2W)
static void Info_Plaque_Draw (const char *message)
{
	int	i, cnt;
	int	bx, by;
	char	temp[80];

	if (scr_con_current == vid.height)
		return;		// console is full screen

	if (!info_string_count || !*message)
		return;

	scr_needfull = true;

	FindTextBreaks(message, PLAQUE_WIDTH+4);

	if (lines == MAXLINES)
	{
		Con_DPrintf("%s: line overflow error\n", __thisfunc__);
		lines = MAXLINES-1;
	}

	by = (25-lines) * 8 / 2 + ((vid.height - 200)>>1);
	M_DrawTextBox (15, by - 16, PLAQUE_WIDTH + 4 + 4, lines + 2);

	for (i = 0; i < lines; i++, by += 8)
	{
		cnt = EndC[i] - StartC[i];
		strncpy (temp, &message[StartC[i]], cnt);
		temp[cnt] = 0;
		bx = (40-strlen(temp)) * 8 / 2;
		M_Print (bx, by, temp);
	}
}

static void Bottom_Plaque_Draw (const char *message)
{
	int	i, cnt;
	int	bx, by;
	char	temp[80];

	if (!*message)
		return;

	scr_needfull = true;

	FindTextBreaks(message, PLAQUE_WIDTH);

	by = (((vid.height) / 8) - lines - 2) * 8;
	M_DrawTextBox (32, by - 16, PLAQUE_WIDTH + 4, lines + 2);

	for (i = 0; i < lines; i++, by += 8)
	{
		cnt = EndC[i] - StartC[i];
		strncpy (temp, &message[StartC[i]], cnt);
		temp[cnt] = 0;
		bx = (40-strlen(temp)) * 8 / 2;
		M_Print (bx, by, temp);
	}
}

//=============================================================================


static void I_Print (int cx, int cy, const char *str, int flags)
{
	int	num, x, y;
	const char	*s;

	x = cx + ((vid.width - 320)>>1);
	y = cy;
	if (!(flags & (INTERMISSION_PRINT_TOP|INTERMISSION_PRINT_TOPMOST)))
		y += ((vid.height - 200)>>1);
	s = str;

	while (*s)
	{
		num = (unsigned char)(*s);
		if (!(flags & INTERMISSION_PRINT_WHITE))
			num += 256;
		Draw_Character (x, y, num);
		s++;
		x += 8;
	}
}

#if FULLSCREEN_INTERMISSIONS
#	define	Load_IntermissionPic_FN(X,Y,Z)	Draw_CachePicResize((X),(Y),(Z))
#	define	Draw_IntermissionPic_FN(X,Y,Z)	Draw_Pic(0,0,(Z))
#else
#	define	Load_IntermissionPic_FN(X,Y,Z)	Draw_CachePic((X))
#	define	Draw_IntermissionPic_FN(X,Y,Z)	Draw_Pic((X),(Y),(Z))
#endif

/*
===============
SB_IntermissionOverlay
===============
*/
static void SB_IntermissionOverlay (void)
{
	qpic_t	*pic;
	int	elapsed, size, bx, by, i;
	char		temp[80];
	const char	*message;

	scr_copyeverything = 1;
	scr_fullupdate = 0;

#if !defined(H2W)
	if (cl.gametype == GAME_DEATHMATCH)
#else
	if (!cl_siege)
#endif
	{
		Sbar_DeathmatchOverlay ();
		return;
	}

	if (cl.intermission_pic == NULL)
		Host_Error ("%s: NULL intermission picture", __thisfunc__);
	else
	{
		pic = Load_IntermissionPic_FN (cl.intermission_pic, vid.width, vid.height);
		Draw_IntermissionPic_FN (((vid.width - 320)>>1), ((vid.height - 200)>>1), pic);
	}

	if (cl.message_index >= 0 && cl.message_index < host_string_count)
		message = Host_GetString (cl.message_index);
	else if (cl.intermission_flags & INTERMISSION_NO_MESSAGE)
		message = "";
	else
	{
		message = ""; /* silence compilers */
		Host_Error ("%s: Intermission string #%d not available (host_string_count: %d)",
					__thisfunc__, cl.message_index, host_string_count);
	}

	if (cl.intermission_flags & INTERMISSION_NOT_CONNECTED)
		elapsed = (realtime - cl.completed_time) * 20;
	else	elapsed = (cl.time  - cl.completed_time) * 20;
	if (cl.intermission_flags & INTERMISSION_PRINT_DELAY)
	{
		elapsed -= 50;	/* delay about 2.5 seconds */
		if (elapsed < 0)
			elapsed = 0;
	}

	FindTextBreaks(message, 38);

	if (cl.intermission_flags & INTERMISSION_PRINT_TOPMOST)
		by =  16;
	else	by = (25-lines) * 8 / 2;

	for (i = 0; i < lines; i++, by += 8)
	{
		size = EndC[i] - StartC[i];
		strncpy (temp, &message[StartC[i]], size);

		if (size > elapsed)
			size = elapsed;
		temp[size] = 0;

		bx = (40-strlen(temp)) * 8 / 2;
		I_Print (bx, by, temp, cl.intermission_flags);

		elapsed -= size;
		if (elapsed <= 0)
			break;
	}

	if (i == lines && cl.lasting_time && elapsed >= 20*cl.lasting_time)
		CL_SetupIntermission (cl.intermission_next);
}
#endif	/* H2W */

//=============================================================================


/*
==================
SCR_UpdateScreen

This is called every frame, and can also be called explicitly to flush
text to the screen.

WARNING: be very careful calling this from elsewhere, because the refresh
needs almost the entire 256k of stack space!
==================
*/
void SCR_UpdateScreen (void)
{
	vrect_t		vrect;

	if (scr_skipupdate || block_drawing)
		return;

#ifdef PLATFORM_WINDOWS
	// don't suck up any cpu if minimized
	if (Minimized)
		return;
#endif

	scr_copytop = 0;
	scr_copyeverything = 0;

#if defined(H2W)
	if (scr_disabled_for_loading)
		return;
#else
	if (scr_disabled_for_loading)
	{
		if (realtime - scr_disabled_time > 25) {
		/* this can happen with clients connected to servers
		 * older than uHexen2-1.5.6 who don't issue an error
		 * upon changelevel failures. Or, it could happen if
		 * loading is taking a really long time.
		 */
			scr_disabled_for_loading = false;
			total_loading_size = 0;
			loading_stage = 0;
			Con_Printf ("load timeout.\n");
		}
		else {
			return;
		}
	}

	if (cls.state == ca_dedicated)
		return;		// stdout only
#endif	/* H2W */

	if (!scr_initialized || !con_initialized)
		return;		// not initialized yet

//
// check for vid changes
//
	if (vid.recalc_refdef)
	{
		// something changed, so reorder the screen
		SCR_CalcRefdef ();
	}

//
// do 3D refresh drawing, and then update the screen
//
	D_EnableBackBufferAccess ();	// of all overlay stuff if drawing directly

	if (scr_needfull && (!plaquemessage || !*plaquemessage || !SCR_CheckDrawCenterString2()))
		scr_fullupdate = 0;

	if (scr_fullupdate++ < vid.numpages)
	{	// clear the entire screen
		scr_copyeverything = 1;
		Draw_TileClear (0,0,vid.width,vid.height);
		Sbar_Changed();
	}
	else if (scr_topupdate++ < vid.numpages)
	{
		scr_copyeverything = 1;
		Draw_TileClear (0,0,vid.width,100);
		Sbar_Changed();
	}

//	pconupdate = NULL;

	SCR_SetUpToDrawConsole ();
//	SCR_EraseCenterString ();

	D_DisableBackBufferAccess ();	// for adapters that can't stay mapped
					// in for linear writes all the time

#if FULLSCREEN_INTERMISSIONS
	// no need to draw view in fullscreen intermission screens
	if (!cl.intermission)
#endif
	{
		VID_LockBuffer ();
		V_RenderView ();
		VID_UnlockBuffer ();
	}

	D_EnableBackBufferAccess ();	// of all overlay stuff if drawing directly

	if (scr_drawdialog)
	{
		Sbar_Draw ();
		Draw_FadeScreen ();
		SCR_DrawNotifyString ();
		scr_copyeverything = true;
	}
	else if (cl.intermission)
	{
#if !defined(H2W)
		SB_IntermissionOverlay();
		if (!(cl.intermission_flags & INTERMISSION_NO_MENUS))
		{
			SCR_DrawConsole();
			M_Draw();
		}

		if (scr_drawloading)
			SCR_DrawLoading();
#endif	/* H2W */
	}
#if !defined(H2W)
	else if (scr_drawloading)
	{
		Draw_FadeScreen ();
		SCR_DrawLoading ();
	}
#endif	/* H2W */
	else
	{
		if (crosshair.integer && !cls.demoplayback)
			Draw_Crosshair();

		SCR_DrawRam();
		SCR_DrawNet();
		SCR_DrawTurtle();
		SCR_DrawPause();
		SCR_CheckDrawCenterString();
		Sbar_Draw();
		SCR_DrawFPS();

		Plaque_Draw(plaquemessage, false);
		SCR_DrawConsole();
		M_Draw();

#if !defined(H2W)
		if (info_up)
		{
			UpdateInfoMessage();
			Info_Plaque_Draw(infomessage);
		}
#endif	/* H2W */
	}

	D_DisableBackBufferAccess ();	// for adapters that can't stay mapped
					// in for linear writes all the time
	//if (pconupdate)
	//	D_UpdateRects (pconupdate);

	V_UpdatePalette ();

//
// update one of three areas
//
	if (scr_copyeverything)
	{
		vrect.x = 0;
		vrect.y = 0;
		vrect.width = vid.width;
		vrect.height = vid.height;
		vrect.pnext = NULL;

		VID_Update (&vrect);
	}
	else if (scr_copytop)
	{
		vrect.x = 0;
		vrect.y = 0;
		vrect.width = vid.width;
		vrect.height = vid.height - sb_lines;
		vrect.pnext = NULL;

		VID_Update (&vrect);
	}
	else
	{
		vrect.x = scr_vrect.x;
		vrect.y = scr_vrect.y;
		vrect.width = scr_vrect.width;
		vrect.height = scr_vrect.height;
		vrect.pnext = NULL;

		VID_Update (&vrect);
	}
}


/*
==================
SCR_UpdateWholeScreen
==================
*/
void SCR_UpdateWholeScreen (void)
{
	scr_fullupdate = 0;
	SCR_UpdateScreen ();
}

