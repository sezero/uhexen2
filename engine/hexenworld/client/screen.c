/*
	screen.c
	master for refresh, status bar, console, chat, notify, etc

	$Id$
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
cvar_t		scr_contrans = {"contrans", "0", CVAR_ARCHIVE};
static	cvar_t	scr_conspeed = {"scr_conspeed", "300", CVAR_NONE};
static	cvar_t	scr_centertime = {"scr_centertime", "4", CVAR_NONE};
static	cvar_t	scr_showram = {"showram", "1", CVAR_NONE};
static	cvar_t	scr_showturtle = {"showturtle", "0", CVAR_NONE};
static	cvar_t	scr_showpause = {"showpause", "1", CVAR_NONE};
static	cvar_t	scr_showfps = {"showfps", "0", CVAR_NONE};

qboolean	scr_disabled_for_loading;
qboolean	scr_skipupdate;
qboolean	block_drawing;

static qpic_t	*scr_ram;
static qpic_t	*scr_net;
static qpic_t	*scr_turtle;

static void SCR_ScreenShot_f (void);

const char	*plaquemessage = NULL;	// pointer to current plaque message

static void Plaque_Draw (const char *message, qboolean AlwaysDraw);


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

	if (scr_center_lines <= 4)
		y = vid.height*0.35;
	else
		y = 48;

	scr_copytop = 1;
	height = q_min(8 * scr_erase_lines, (int)vid.height - y - 1);
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
	if (key_dest != key_game)
		return false;

	return true;
}

static void SCR_CheckDrawCenterString (void)
{
	if (! SCR_CheckDrawCenterString2())
		return;

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

// bound viewsize
	if (scr_viewsize.integer < 30)
		Cvar_SetQuick (&scr_viewsize, "30");
	else if (scr_viewsize.integer > 120)
		Cvar_SetQuick (&scr_viewsize, "120");

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
	Cvar_SetCallback (&scr_viewsize, SCR_Callback_refdef);
	Cvar_RegisterVariable (&scr_fov);
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
	if (cls.netchan.outgoing_sequence - cls.netchan.incoming_acknowledged < UPDATE_BACKUP-1)
		return;
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
		char	st[32];
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

//=============================================================================


/*
==================
SCR_SetUpToDrawConsole
==================
*/
static void SCR_SetUpToDrawConsole (void)
{
	Con_CheckResize ();

// decide on the height of the console
	con_forcedup = cls.state != ca_active;

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
		Con_DrawConsole (scr_con_current);
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
static void SCR_ScreenShot_f (void)
{
	char	pcxname[80];
	char	checkname[MAX_OSPATH];
	int	i;

	q_snprintf (checkname, sizeof(checkname), "%s/shots", fs_userdir);
	Sys_mkdir (checkname, false);
//
// find a file name to save it to
//
	q_strlcpy (pcxname, "shots/hw00.pcx", sizeof(pcxname));

	for (i = 0; i <= 99; i++)
	{
		pcxname[8] = i/10 + '0';
		pcxname[9] = i%10 + '0';
		q_snprintf (checkname, sizeof(checkname), "%s/%s", fs_userdir, pcxname);
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
	for (i1 = 0; i1 < length; )
	{
		holdmessage = hold2;

		if ((i1 + 28) >= length)	// Past end of line
		{
			strcpy (line, hold2);
			M_Print (col, row, line);
			i1 += 28;
		}
		else
		{
			// looking for a space to break line at
			for (i2 = 28; i2 > 0; --i2)
			{
				if (*(holdmessage + i2) == ' ')
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

//=============================================================================


#if 0	/* intermission and finale overlay code not used in H2W */

static void I_DrawCharacter (int cx, int line, int num)
{
	Draw_Character (cx + ((vid.width - 320)>>1), line + ((vid.height - 200)>>1), num);
}

static void I_Print (int cx, int cy, char *str)
{
	while (*str)
	{
		I_DrawCharacter (cx, cy, ((unsigned char)(*str)) + 256);
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
	int	elapsed, size, bx, by, i;
	char		temp[80];
	const char	*message;

	scr_copyeverything = 1;
	scr_fullupdate = 0;

//	if (cl.gametype == GAME_DEATHMATCH)
	if (!cl_siege)
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
		// siege
		case 10:
			//Defender win - wipe out or time limit
			pic = Load_IntermissionPic_FN ("gfx/defwin.lmp", vid.width, vid.height);
			break;
		case 11:
			//Attacker win - caught crown
			pic = Load_IntermissionPic_FN ("gfx/attwin.lmp", vid.width, vid.height);
			break;
		case 12:
			//Attacker win 2 - wiped out
			pic = Load_IntermissionPic_FN ("gfx/attwin2.lmp", vid.width, vid.height);
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
	else
	{
		elapsed = (cl.time - cl.completed_time) * 20;
	}

	if (cl.intermission <= 4 && cl.intermission + 394 <= host_string_count)
		message = &host_strings[host_string_index[cl.intermission + 394]];
	else if (cl.intermission == 5)	// finale for the demo
		message = &host_strings[host_string_index[DEMO_MSG_INDEX]];
	else if (cl.intermission >= 6 && cl.intermission <= 8 && cl.intermission + 386 <= host_string_count)
		message = &host_strings[host_string_index[cl.intermission + 386]];
	else if (cl.intermission == 9)	// finale for the bundle (oem) version
		message = &host_strings[host_string_index[391]];
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
	else
		by = (25-lines) * 8 / 2;

	for (i = 0; i < lines; i++, by += 8)
	{
		size = EndC[i] - StartC[i];
		strncpy (temp, &message[StartC[i]], size);

		if (size > elapsed)
			size = elapsed;
		temp[size] = 0;

		bx = (40-strlen(temp)) * 8 / 2;
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
#if 0	/* not used in Hexen II */
static void SB_FinaleOverlay(void)
{
	qpic_t	*pic;

	scr_copyeverything = 1;

	pic = Draw_CachePic("gfx/finale.lmp");
	Draw_TransPic((vid.width-pic->width)/2, 16, pic);
}
#endif

#endif	/* end of unused intermission and finale overlay code */

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
			return;
	}

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
#if 0	/* not in H2W */
		SB_IntermissionOverlay();
		if (cl.intermission < 12)
		{
			SCR_DrawConsole();
			M_Draw();
		}
#endif
	}
/*	else if (cl.intermission == 2 && key_dest == key_game)
	{
		SB_FinaleOverlay();
		SCR_CheckDrawCenterString();
	}
*/
	else
	{
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

