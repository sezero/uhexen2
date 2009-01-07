/*
	screen.c
	master for refresh, status bar, console, chat, notify, etc

	$Header: /home/ozzie/Download/0000/uhexen2/hexen2/screen.c,v 1.59 2009-01-07 09:30:23 sezero Exp $
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
#include <io.h>
#endif
#ifdef PLATFORM_UNIX
#include <unistd.h>
#endif
#ifdef PLATFORM_DOS
#include <unistd.h>
#endif


static qboolean		scr_initialized;	// ready to draw

vrect_t			scr_vrect;
//vrect_t		*pconupdate;

// these are only functional in the software
// renderer.
int			scr_copytop;		// only the refresh window will be updated
int			scr_copyeverything;	// unless these variables are flagged
int			scr_topupdate;
int			scr_fullupdate;
static qboolean		scr_needfull = false;

static int		clearconsole;
int			clearnotify;

float		scr_con_current;
float		scr_conlines;		// lines of console to display

int		oldscreensize, oldfov;

cvar_t		scr_viewsize = {"viewsize", "110", CVAR_ARCHIVE};
cvar_t		scr_fov = {"fov", "90", CVAR_NONE};	// 10 - 170
static	cvar_t	scr_conspeed = {"scr_conspeed", "300", CVAR_NONE};
static	cvar_t	scr_centertime = {"scr_centertime", "4", CVAR_NONE};
static	cvar_t	scr_showram = {"showram", "1", CVAR_NONE};
static	cvar_t	scr_showturtle = {"showturtle", "0", CVAR_NONE};
static	cvar_t	scr_showpause = {"showpause", "1", CVAR_NONE};

static qboolean	scr_drawloading;
static int	ls_offset;
static float	scr_disabled_time;
int		total_loading_size, current_loading_size, loading_stage;
qboolean	scr_disabled_for_loading;
qboolean	scr_skipupdate;
qboolean	block_drawing;

static qpic_t	*scr_ram;
static qpic_t	*scr_net;
static qpic_t	*scr_turtle;

static void SCR_ScreenShot_f (void);

const char	*plaquemessage = NULL;	// pointer to current plaque message

static void Plaque_Draw (const char *message, qboolean AlwaysDraw);
// procedures for the mission pack intro messages
static void Info_Plaque_Draw (const char *message);
static void Bottom_Plaque_Draw (const char *message);


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

// Objectives thing of the mission pack
#define	MAX_INFO	1024
static char	infomessage[MAX_INFO];
extern qboolean	info_up;

static void UpdateInfoMessage (void)
{
	unsigned int i;
	unsigned int check;
	char *newmessage;

	strcpy(infomessage, "Objectives:");

	if (!pr_info_string_count || !pr_global_info_strings)
		return;

	for (i = 0; i < 32; i++)
	{
		check = (1 << i);
		
		if (cl.info_mask & check)
		{
			newmessage = &pr_global_info_strings[pr_info_string_index[i]];
			strcat(infomessage, "@@");
			strcat(infomessage, newmessage);
		}
	}

	for (i = 0; i < 32; i++)
	{
		check = (1 << i);
		
		if (cl.info_mask2 & check)
		{
			newmessage = &pr_global_info_strings[pr_info_string_index[i+32]];
			strcat(infomessage, "@@");
			strcat(infomessage, newmessage);
		}
	}
}

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

	y = ((25-lines) * 8) / 2;

	scr_copytop = 1;
	height = q_min(8 * scr_erase_lines, (int)vid.height - y - 1);
	Draw_TileClear (0, y, vid.width, height);
}
*/

static void SCR_DrawCenterString (void)
{
	int		i;
	int		bx, by;
	char	temp[80];

//	scr_erase_center = 0;

	FindTextBreaks(scr_centerstring, 38);

	by = ((25-lines) * 8) / 2;

	for (i = 0; i < lines; i++, by += 8)
	{
		strncpy (temp, &scr_centerstring[StartC[i]], EndC[i] - StartC[i]);
		temp[EndC[i] - StartC[i]] = 0;
		bx = ((40-strlen(temp)) * 8) / 2;
	  	M_Print2 (bx, by, temp);
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
	if (key_dest != key_game)
		return false;

	return true;
}

static void SCR_CheckDrawCenterString (void)
{
	if (! SCR_CheckDrawCenterString2())
		return;

	if (intro_playing)
	{
		Bottom_Plaque_Draw(scr_centerstring);
		return;
	}
	SCR_DrawCenterString ();
}

//=============================================================================


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
	vid.recalc_refdef = 0;

// bound viewsize
	if (scr_viewsize.integer < 30)
		Cvar_SetValue ("viewsize", 30);
	else if (scr_viewsize.integer > 120)
		Cvar_SetValue ("viewsize", 120);

// bound field of view
	if (scr_fov.integer < 10)
		Cvar_SetValue ("fov", 10);
	else if (scr_fov.integer > 110)
		Cvar_SetValue ("fov", 110);

	oldfov = scr_fov.integer;
	oldscreensize = scr_viewsize.integer;

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

// guard against going from one mode to another that's less than half the
// vertical resolution
	if (scr_con_current > vid.height)
		scr_con_current = vid.height;

// notify the refresh of the change
	R_ViewChanged (&vrect, sb_lines, vid.aspect);
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
	Cvar_SetValue ("viewsize", scr_viewsize.integer + 10);
	vid.recalc_refdef = 1;
}

/*
=================
SCR_SizeDown_f

Keybinding command
=================
*/
static void SCR_SizeDown_f (void)
{
	Cvar_SetValue ("viewsize", scr_viewsize.integer - 10);
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

	Cvar_RegisterVariable (&scr_fov);
	Cvar_RegisterVariable (&scr_viewsize);
	Cvar_RegisterVariable (&scr_conspeed);
	Cvar_RegisterVariable (&scr_showram);
	Cvar_RegisterVariable (&scr_showturtle);
	Cvar_RegisterVariable (&scr_showpause);
	Cvar_RegisterVariable (&scr_centertime);

//
// register our commands
//
	Cmd_AddCommand ("screenshot",SCR_ScreenShot_f);
	Cmd_AddCommand ("sizeup",SCR_SizeUp_f);
	Cmd_AddCommand ("sizedown",SCR_SizeDown_f);

	scr_initialized = true;
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

	if (host_frametime < 0.05)
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
	if (realtime - cl.last_received_message < 0.3)
		return;
	if (cls.demoplayback)
		return;

	Draw_Pic (scr_vrect.x+64, scr_vrect.y, scr_net);
}

/*
==============
DrawPause
==============
*/
static void SCR_DrawPause (void)
{
	qpic_t	*pic;
	float delta;
	static qboolean newdraw = false;
	int finaly;
	static float LogoPercent, LogoTargetPercent;

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
//	Draw_Pic ( (vid.width - pic->width)/2, 
//		(vid.height - 48 - pic->height)/2, pic);

	if (LogoPercent < LogoTargetPercent)
	{
		delta = ((LogoTargetPercent - LogoPercent) / .5) * host_frametime;
		if (delta < 0.004)
		{
			delta = 0.004;
		}
		LogoPercent += delta;
		if (LogoPercent > LogoTargetPercent)
		{
			LogoPercent = LogoTargetPercent;
		}
	}

	finaly = ((float)pic->height * LogoPercent) - pic->height;
	Draw_TransPicCropped ( (vid.width - pic->width)/2, finaly, pic);
}

/*
==============
SCR_DrawLoading
==============
*/
void SCR_DrawLoading (void)
{
	int		size, count;
	qpic_t	*pic;

	if (!scr_drawloading && loading_stage == 0)
		return;

	pic = Draw_CachePic ("gfx/menu/loading.lmp");
	ls_offset = (vid.width - pic->width) / 2;
	Draw_TransPic (ls_offset, 0, pic);

	if (loading_stage == 0)
		return;

	if (total_loading_size)
		size = current_loading_size * 106 / total_loading_size;
	else
		size = 0;

	if (loading_stage == 1)
		count = size;
	else
		count = 106;

	Draw_Fill (ls_offset+42, 87, count, 1, 136);
	Draw_Fill (ls_offset+42, 87+1, count, 4, 138);
	Draw_Fill (ls_offset+42, 87+5, count, 1, 136);

	if (loading_stage == 2)
		count = size;
	else
		count = 0;

	Draw_Fill (ls_offset+42, 97, count, 1, 168);
	Draw_Fill (ls_offset+42, 97+1, count, 4, 170);
	Draw_Fill (ls_offset+42, 97+5, count, 1, 168);
}

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

//=============================================================================


/*
==================
SCR_SetUpToDrawConsole
==================
*/
static void SCR_SetUpToDrawConsole (void)
{
	Con_CheckResize ();
	
	if (scr_drawloading)
		return;		// never a console with loading plaque

// decide on the height of the console
	con_forcedup = !cl.worldmodel || cls.signon != SIGNONS;

	if (con_forcedup)
	{
		scr_conlines = vid.height;	// full screen
		scr_con_current = scr_conlines;
	}
	else if (key_dest == key_console)
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
		Con_DrawConsole (scr_con_current, true);
		clearconsole = 0;
	}
	else
	{
		if (key_dest == key_game || key_dest == key_message)
			Con_DrawNotify ();	// only draw notify in game
	}
}


/*
==============================================================================

SCREEN SHOTS

==============================================================================
*/

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

/*
==============
WritePCXfile
==============
*/
static void WritePCXfile (const char *filename, byte *data, int width, int height, int rowbytes, byte *palette)
{
	int		i, j;
	size_t		length;
	pcx_t	*pcx;
	byte		*pack;

	pcx = (pcx_t *) Hunk_TempAlloc (width*height*2+1000);
	if (pcx == NULL)
	{
		Con_Printf("%s: not enough memory\n", __thisfunc__);
		return;
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
	memset (pcx->palette,0,sizeof(pcx->palette));
	pcx->color_planes = 1;		// chunky image
	pcx->bytes_per_line = LittleShort((short)width);
	pcx->palette_type = LittleShort(2);	// not a grey scale
	memset (pcx->filler,0,sizeof(pcx->filler));

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
	FS_WriteFile (filename, pcx, length);
}

/*
==================
SCR_ScreenShot_f
==================
*/
static void SCR_ScreenShot_f (void)
{
	int	i;
	char		pcxname[80];
	char		checkname[MAX_OSPATH];

	q_snprintf (checkname, sizeof(checkname), "%s/shots", fs_userdir);
	Sys_mkdir (checkname, false);
//
// find a file name to save it to
//
	q_strlcpy (pcxname, "shots/hexen00.pcx", sizeof(pcxname));

	for (i = 0; i <= 99; i++)
	{
		pcxname[11] = i/10 + '0';
		pcxname[12] = i%10 + '0';
		q_snprintf (checkname, sizeof(checkname), "%s/%s", fs_userdir, pcxname);
		if (access(checkname, F_OK) == -1)
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

	WritePCXfile (pcxname, vid.buffer, vid.width, vid.height, vid.rowbytes, host_basepal);

	D_DisableBackBufferAccess ();	// for adapters that can't stay mapped
					// in for linear writes all the time

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
	if (cls.state == ca_dedicated)
		return true;

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
#if 0	// all uses are commented out
void SCR_BringDownConsole (void)
{
	int		i;

	scr_centertime_off = 0;

	for (i = 0; i < 20 && scr_conlines != scr_con_current; i++)
		SCR_UpdateScreen ();

	cl.cshifts[0].percent = 0;		// no area contents palette on next frame
	VID_SetPalette (host_basepal);
}
#endif

//=============================================================================


#if 0
static void Plaque_Draw (void)
{
	int	length, line_cnt, i1, i2, row, col;
	char	*holdmessage, *hold2;
	char	line[32];

	if (scr_con_current == vid.height)
		return;		// console is full screen

	if (!*plaquemessage)
		return;

	length = strlen(plaquemessage);
	line_cnt = length/28;

	if ((line_cnt * 28) < length)
		++line_cnt;

	M_DrawTextBox (40, 76, 30, line_cnt);

	row = 84;
	col = 56;
	i2 = 0;

	hold2 = plaquemessage;
	for (i1 = 0 ; i1 < length ; )
	{
		holdmessage = hold2;

		if ((i1+28) >= length)	// Past end of line
		{
			strcpy (line, hold2);
			M_Print (col, row, line);
			i1 += 28;
		}
		else
		{
			// looking for a space to break line at
			for (i2 = 28 ; i2 > 0 ; --i2)
			{
				if (*(holdmessage+i2) == ' ')
				{
					memcpy (line, hold2, i2);
					line[i2] = '\0';
					M_Print (col, row, line);
					row += 8;
					++i2;	// Jump over space
					break;
				}
			}
			i1 += i2;
			hold2 += i2;
		}
	}
}
#endif

static void Plaque_Draw (const char *message, qboolean AlwaysDraw)
{
	int	i;
	char	temp[80];
	int	bx, by;

	if (scr_con_current == vid.height && !AlwaysDraw)
		return;		// console is full screen

	if (!*message)
		return;

	scr_needfull = true;

	FindTextBreaks(message, PLAQUE_WIDTH);

	by = ((25-lines) * 8) / 2;
	M_DrawTextBox2 (32, by-16, PLAQUE_WIDTH+4, lines+2, false);

	for (i = 0; i < lines; i++, by += 8)
	{
		strncpy (temp, &message[StartC[i]], EndC[i] - StartC[i]);
		temp[EndC[i] - StartC[i]] = 0;
		bx = ((40-strlen(temp)) * 8) / 2;
		M_Print2 (bx, by, temp);
	}
}

static void Info_Plaque_Draw (const char *message)
{
	int i;
	char temp[80];
	int bx,by;

	if (scr_con_current == vid.height)
		return;		// console is full screen

	if (!pr_info_string_count || !*message)
		return;

	scr_needfull = true;

	FindTextBreaks(message, PLAQUE_WIDTH+4);

	if (lines == MAXLINES) 
	{
		Con_DPrintf("Info_Plaque_Draw: line overflow error\n");
		lines = MAXLINES-1;
	}

	by = ((25-lines) * 8) / 2;
	M_DrawTextBox2 (15, by-16, PLAQUE_WIDTH+4+4, lines+2, false);

	for (i = 0; i < lines; i++, by += 8)
	{
		strncpy (temp, &message[StartC[i]], EndC[i] - StartC[i]);
		temp[EndC[i] - StartC[i]] = 0;
		bx = ((40-strlen(temp)) * 8) / 2;
	  	M_Print2 (bx, by, temp);
	}
}

static void Bottom_Plaque_Draw (const char *message)
{
	int i;
	char temp[80];
	int bx,by;

	if (!*message)
		return;

	scr_needfull = true;

	FindTextBreaks(message, PLAQUE_WIDTH);

	by = (((vid.height) / 8) - lines - 2) * 8;

	M_DrawTextBox2 (32, by-16, PLAQUE_WIDTH+4, lines+2, true);

	for (i = 0; i < lines; i++, by += 8)
	{
		strncpy (temp, &message[StartC[i]], EndC[i] - StartC[i]);
		temp[EndC[i] - StartC[i]] = 0;
		bx = ((40-strlen(temp)) * 8) / 2;
	  	M_Print(bx, by, temp);
	}
}

//=============================================================================


static void I_DrawCharacter (int cx, int line, int num)
{
	Draw_Character ( cx + ((vid.width - 320)>>1), line + ((vid.height - 200)>>1), num);
}

static void I_Print (int cx, int cy, char *str)
{
	while (*str)
	{
		I_DrawCharacter (cx, cy, ((unsigned char)(*str))+256);
		str++;
		cx += 8;
	}
}

#if FULLSCREEN_INTERMISSIONS
#	define	Load_IntermissionPic_FN(X,Y,Z)	Draw_CachePicResize((X),(Y),(Z))
#	define	Draw_IntermissionPic_FN(X,Y,Z)	Draw_Pic(0,0,(Z))
#else
#	define	Load_IntermissionPic_FN(X,Y,Z)	Draw_CachePic((X))
#	define	Draw_IntermissionPic_FN(X,Y,Z)	Draw_Pic((X),(Y),(Z))
#endif

#define	DEMO_MSG_INDEX	408
// in Hammer of Thyrion, the demo version isn't allowed in combination
// with the mission pack. therefore, the formula below isn't necessary
//#define	DEMO_MSG_INDEX	(ABILITIES_STR_INDEX+MAX_PLAYER_CLASS*2)
//			408 for H2, 410 for H2MP strings.txt

/*
===============
SB_IntermissionOverlay
===============
*/
static void SB_IntermissionOverlay (void)
{
	qpic_t	*pic = NULL;
	int		elapsed, size, bx, by, i;
	char		temp[80];
	const char	*message;

	scr_copyeverything = 1;
	scr_fullupdate = 0;

	if (cl.gametype == GAME_DEATHMATCH)
	{
		Sbar_DeathmatchOverlay ();
		return;
	}

	switch (cl.intermission)
	{
		case 1:
			pic = Load_IntermissionPic_FN ("gfx/meso.lmp", vid.width, vid.height);
			break;
		case 2:
			pic = Load_IntermissionPic_FN ("gfx/egypt.lmp", vid.width, vid.height);
			break;
		case 3:
			pic = Load_IntermissionPic_FN ("gfx/roman.lmp", vid.width, vid.height);
			break;
		case 4:
			pic = Load_IntermissionPic_FN ("gfx/castle.lmp", vid.width, vid.height);
			break;
		case 5:
			pic = Load_IntermissionPic_FN ("gfx/castle.lmp", vid.width, vid.height);
			break;
		case 6:
			pic = Load_IntermissionPic_FN ("gfx/end-1.lmp", vid.width, vid.height);
			break;
		case 7:
			pic = Load_IntermissionPic_FN ("gfx/end-2.lmp", vid.width, vid.height);
			break;
		case 8:
			pic = Load_IntermissionPic_FN ("gfx/end-3.lmp", vid.width, vid.height);
			break;
		case 9:
			pic = Load_IntermissionPic_FN ("gfx/castle.lmp", vid.width, vid.height);
			break;
		// mission pack
		case 10:
			pic = Load_IntermissionPic_FN ("gfx/mpend.lmp", vid.width, vid.height);
			break;
		case 11:
			pic = Load_IntermissionPic_FN ("gfx/mpmid.lmp", vid.width, vid.height);
			break;
		case 12:
			pic = Load_IntermissionPic_FN ("gfx/end-3.lmp", vid.width, vid.height);
			break;
	}
	if (pic == NULL)
	{
		Host_Error ("%s: Bad episode ending number %d", __thisfunc__, cl.intermission);
		return;
	}

	Draw_IntermissionPic_FN (((vid.width - 320)>>1), ((vid.height - 200)>>1), pic);

	if (cl.intermission >= 6 && cl.intermission <= 8)
	{
		elapsed = (cl.time - cl.completed_time) * 20;
		elapsed -= 50;
		if (elapsed < 0)
			elapsed = 0;
	}
	else if (cl.intermission == 12)	// mission pack entry screen
	{
	// this intermission is NOT triggered by a server message, but
	// by starting a new game through the menu system. therefore,
	// you cannot use cl.time, and cl.completed_time must be set by
	// the menu sytem, as well.
		elapsed = (realtime - cl.completed_time) * 20;
	}
	else
	{
		elapsed = (cl.time - cl.completed_time) * 20;
	}

	if (cl.intermission <= 4 && cl.intermission + 394 <= pr_string_count)
		message = &pr_global_strings[pr_string_index[cl.intermission + 394]];
	else if (cl.intermission == 5)	// finale for the demo
		message = &pr_global_strings[pr_string_index[DEMO_MSG_INDEX]];
	else if (cl.intermission >= 6 && cl.intermission <= 8 && cl.intermission + 386 <= pr_string_count)
		message = &pr_global_strings[pr_string_index[cl.intermission + 386]];
	else if (cl.intermission == 9)	// finale for the bundle (oem) version
		message = &pr_global_strings[pr_string_index[391]];
	// mission pack
	else if (cl.intermission == 10)
		message = &pr_global_strings[pr_string_index[538]];
	else if (cl.intermission == 11)
		message = &pr_global_strings[pr_string_index[545]];
	else if (cl.intermission == 12)
		message = &pr_global_strings[pr_string_index[561]];
	else
		message = "";

	FindTextBreaks(message, 38);

	// hacks to print the final messages centered: "by" is the y offset
	// in pixels to begin printing at. each line is 8 pixels - S.A
	//if (cl.intermission == 8)
	//	by = 16;
	if (cl.intermission >= 6 && cl.intermission <= 8)
		// eidolon, endings. num == 6,7,8
		by = (vid.height/2 - lines*4);
	else if (cl.intermission == 10)
		// mission pack: tibet10. num == 10
		by = 33;
	else
		by = ((25-lines) * 8) / 2;

	for (i = 0; i < lines; i++, by += 8)
	{
		size = EndC[i] - StartC[i];
		strncpy (temp, &message[StartC[i]], size);

		if (size > elapsed)
			size = elapsed;
		temp[size] = 0;

		bx = ((40-strlen(temp)) * 8) / 2;
		if (cl.intermission < 6 || cl.intermission > 9)
			I_Print (bx, by, temp);
		else
			M_PrintWhite (bx, by, temp);

		elapsed -= size;
		if (elapsed <= 0)
			break;
	}

	if (i == lines && elapsed >= 300 && cl.intermission >= 6 && cl.intermission <= 7)
	{
		cl.intermission++;
		cl.completed_time = cl.time;
	}
}

/*
===============
SB_FinaleOverlay
===============
*/
#if 0	// not used in Hexen II
static void SB_FinaleOverlay(void)
{
	qpic_t	*pic;

	scr_copyeverything = 1;

	pic = Draw_CachePic("gfx/finale.lmp");
	Draw_TransPic((vid.width-pic->width)/2, 16, pic);
}
#endif

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

	if (scr_disabled_for_loading)
	{
	/* FIXME -- This really needs to be fixed properly:
	 * Simply starting a new game and typing "changelevel fubar"
	 * will hang the engine for 20s if fubar.bsp does not exist.
	 * See docs/SrcNotes.txt for details.
	 */
		if (realtime - scr_disabled_time > 20)
		{
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

	if (!scr_initialized || !con_initialized)
		return;		// not initialized yet

//
// check for vid changes
//
	if (oldfov != scr_fov.integer ||
	    oldscreensize != scr_viewsize.integer)
		vid.recalc_refdef = true;

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
	if (cl.intermission < 1)
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
	else if (cl.intermission >= 1 && cl.intermission <= 12)
	{
		SB_IntermissionOverlay();
		if (cl.intermission < 12)
		{
			SCR_DrawConsole();
			M_Draw();
		}

		if (scr_drawloading)
			SCR_DrawLoading();
	}
/*	else if (cl.intermission == 2 && key_dest == key_game)
	{
		SB_FinaleOverlay();
		SCR_CheckDrawCenterString();
	}
*/
	else if (scr_drawloading)
	{
	//	Sbar_Draw();
		Draw_FadeScreen ();
		SCR_DrawLoading ();
	}
	else
	{
		SCR_DrawRam();
		SCR_DrawNet();
		SCR_DrawTurtle();
		SCR_DrawPause();
		SCR_CheckDrawCenterString();
		Sbar_Draw();

		Plaque_Draw(plaquemessage, false);
		SCR_DrawConsole();
		M_Draw();

		if (info_up)
		{
			UpdateInfoMessage();
			Info_Plaque_Draw(infomessage);
		}
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
		vrect.pnext = 0;

		VID_Update (&vrect);
	}
	else if (scr_copytop)
	{
		vrect.x = 0;
		vrect.y = 0;
		vrect.width = vid.width;
		vrect.height = vid.height - sb_lines;
		vrect.pnext = 0;

		VID_Update (&vrect);
	}	
	else
	{
		vrect.x = scr_vrect.x;
		vrect.y = scr_vrect.y;
		vrect.width = scr_vrect.width;
		vrect.height = scr_vrect.height;
		vrect.pnext = 0;

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

