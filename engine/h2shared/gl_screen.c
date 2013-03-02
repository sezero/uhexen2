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
#ifdef PLATFORM_WINDOWS
#include "winquake.h"
#endif


static qboolean	scr_initialized;	// ready to draw

vrect_t		scr_vrect;
int		glx, gly, glwidth, glheight;

/* these are only functional in the software renderer */
int		scr_copytop;		// only the refresh window will be updated
int		scr_copyeverything;	// unless these variables are flagged
int		scr_topupdate;
int		scr_fullupdate;

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
//static	cvar_t	gl_triplebuffer = {"gl_triplebuffer", "0", CVAR_ARCHIVE};

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

static void SCR_DrawCenterString (void)
{
	int	i, cnt;
	int	bx, by;
	char	temp[80];

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

static void SCR_CheckDrawCenterString (void)
{
	scr_copytop = 1;
	if (scr_center_lines > scr_erase_lines)
		scr_erase_lines = scr_center_lines;

	scr_centertime_off -= host_frametime;

	if (scr_centertime_off <= 0 && !cl.intermission)
		return;
	if (Key_GetDest() != key_game)
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
	float	size;
	int	h;

	scr_fullupdate = 0;		// force a background redraw

// bound viewsize
	if (scr_viewsize.integer < 30)
		Cvar_SetQuick (&scr_viewsize, "30");
	else if (scr_viewsize.integer > 130)
		Cvar_SetQuick (&scr_viewsize, "130");

// bound field of view
	if (scr_fov.integer < 10)
		Cvar_SetQuick (&scr_fov, "10");
	else if (scr_fov.integer > 170)
		Cvar_SetQuick (&scr_fov, "170");

	vid.recalc_refdef = 0;

// force the status bar to redraw
	SB_ViewSizeChanged ();
	Sbar_Changed();

	if (scr_viewsize.integer >= 110)
		sb_lines = 0;		// no status bar
	else
		sb_lines = 36;	// FIXME: why not 46, i.e. BAR_TOP_HEIGHT?

	size = scr_viewsize.integer > 100 ? 100.0 : scr_viewsize.integer;
	if (cl.intermission)
	{
		size = 100.0;		// intermission is always full screen
		sb_lines = 0;
	}
	size /= 100.0;

	h = vid.height - sb_lines;
	r_refdef.vrect.width = vid.width * size;
	if (r_refdef.vrect.width < 96)
	{
		size = 96.0 / vid.width;
		r_refdef.vrect.width = 96;	// min for icons
	}

	r_refdef.vrect.height = vid.height * size;
	if (r_refdef.vrect.height > vid.height - sb_lines)
		r_refdef.vrect.height = vid.height - sb_lines;

	r_refdef.vrect.x = (vid.width - r_refdef.vrect.width)/2;
	r_refdef.vrect.y = (h - r_refdef.vrect.height)/2;

	r_refdef.fov_x = AdaptFovx (scr_fov.value, r_refdef.vrect.width, r_refdef.vrect.height);
	r_refdef.fov_y = CalcFovy (r_refdef.fov_x, r_refdef.vrect.width, r_refdef.vrect.height);

	scr_vrect = r_refdef.vrect;
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
//	Cvar_RegisterVariable (&gl_triplebuffer);

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

	if (host_frametime < 0.1)
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
	scr_fullupdate = 0;
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
		Sbar_Changed();
	}
	else if (clearnotify++ < vid.numpages)
	{
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

typedef struct _TargaHeader {
	unsigned char	id_length, colormap_type, image_type;
	unsigned short	colormap_index, colormap_length;
	unsigned char	colormap_size;
	unsigned short	x_origin, y_origin, width, height;
	unsigned char	pixel_size, attributes;
} TargaHeader;


/*
==================
SCR_ScreenShot_f
==================
*/
#if !defined(H2W)
static const char scr_shotbase[] = "shots/hexen00.tga";
#define SHOTNUM_POS 11
#else
static const char scr_shotbase[] = "shots/hw00.tga";
#define SHOTNUM_POS 8
#endif
static void SCR_ScreenShot_f (void)
{
	char	pcxname[80];
	char	checkname[MAX_OSPATH];
	int	i, size, temp;
	int	mark;
	byte	*buffer;

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
		Con_Printf ("%s: Couldn't create a TGA file\n", __thisfunc__);
		return;
	}

	size = glwidth * glheight * 3 + 18;
	mark = Hunk_LowMark();
	buffer = (byte *) Hunk_AllocName(size, "buffer_sshot");
	memset (buffer, 0, 18);
	buffer[2] = 2;		// uncompressed type
	buffer[12] = glwidth & 255;
	buffer[13] = glwidth >> 8;
	buffer[14] = glheight & 255;
	buffer[15] = glheight >> 8;
	buffer[16] = 24;	// pixel size

	glReadPixels_fp (glx, gly, glwidth, glheight, GL_RGB, GL_UNSIGNED_BYTE, buffer+18);

	// swap rgb to bgr
	for (i = 18; i < size; i += 3)
	{
		temp = buffer[i];
		buffer[i] = buffer[i+2];
		buffer[i+2] = temp;
	}

	i = FS_WriteFile (pcxname, buffer, size);

	Hunk_FreeToLowMark(mark);

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
#	define	Load_IntermissionPic_FN(X,Y,Z)	Draw_CachePicNoTrans((X))
#	define	Draw_IntermissionPic_FN(X,Y,Z)	Draw_IntermissionPic((Z))
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
===============
SCR_TileClear

================
*/
static void SCR_TileClear (void)
{
    if (vid.conwidth > 320) {
	if (r_refdef.vrect.x > 0)
	{
		// left
		Draw_TileClear (0, 0, r_refdef.vrect.x, vid.height);
		// right
		Draw_TileClear (r_refdef.vrect.x + r_refdef.vrect.width, 0,
			vid.width - r_refdef.vrect.x + r_refdef.vrect.width, vid.height);
	}
//	if (r_refdef.vrect.y > 0) // if (r_refdef.vrect.height < vid.height - 44)
	{
		// top
		Draw_TileClear (r_refdef.vrect.x, 0,
			r_refdef.vrect.x + r_refdef.vrect.width, r_refdef.vrect.y);
		// bottom
		Draw_TileClear (r_refdef.vrect.x, r_refdef.vrect.y + r_refdef.vrect.height,
			r_refdef.vrect.width, vid.height - (r_refdef.vrect.height + r_refdef.vrect.y));
	}
    } else {
	if (r_refdef.vrect.x > 0)
	{
		// left
		Draw_TileClear (0, 0, r_refdef.vrect.x, vid.height - sb_lines);
		// right
		Draw_TileClear (r_refdef.vrect.x + r_refdef.vrect.width, 0,
			vid.width - r_refdef.vrect.x + r_refdef.vrect.width, vid.height - sb_lines);
	}
	if (r_refdef.vrect.y > 0)
	{
		// top
		Draw_TileClear (r_refdef.vrect.x, 0,
			r_refdef.vrect.x + r_refdef.vrect.width, r_refdef.vrect.y);
		// bottom
		Draw_TileClear (r_refdef.vrect.x, r_refdef.vrect.y + r_refdef.vrect.height,
			r_refdef.vrect.width, vid.height - sb_lines - (r_refdef.vrect.height + r_refdef.vrect.y));
	}
    }
}

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
	if (block_drawing)
		return;

//	vid.numpages = (gl_triplebuffer.integer) ? 3 : 2;
	scr_copytop = 0;
	scr_copyeverything = 0;

#if defined(H2W)
	if (scr_disabled_for_loading)
		return;
#else
	if (scr_disabled_for_loading)
	{
		if (realtime - scr_disabled_time > 20)
		{
		/* this can happen with clients connected to servers
		 * older than uHexen2-1.5.6 who don't issue an error
		 * upon changelevel failures.
		 */
			scr_disabled_for_loading = false;
			total_loading_size = 0;
			loading_stage = 0;
			Con_Printf ("load failed.\n");
		}
		else
			return;
	}

	if (cls.state == ca_dedicated)
		return;		// stdout only
#endif	/* H2W */

	if (!scr_initialized || !con_initialized)
		return;		// not initialized yet

	GL_BeginRendering (&glx, &gly, &glwidth, &glheight);

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
	SCR_SetUpToDrawConsole ();

#if FULLSCREEN_INTERMISSIONS
	// no need to draw view in fullscreen intermission screens
	if (!cl.intermission)
#endif
		V_RenderView ();

	GL_Set2D ();
	SCR_TileClear ();	// draw any areas not covered by the refresh

#if defined(H2W)
	if (r_netgraph.integer)
		R_NetGraph ();
#endif	/* H2W */

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

	V_UpdatePalette ();

	GL_EndRendering ();
}

