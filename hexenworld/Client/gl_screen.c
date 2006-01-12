/*
	screen.c
	master for refresh, status bar, console, chat, notify, etc

	$Id: gl_screen.c,v 1.19 2006-01-12 12:34:38 sezero Exp $
*/

#include "quakedef.h"
#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

/*

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


async draw will require the refresh area to be cleared, because it will be
xblited, but sync draw can just ignore it.

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
	

*/


int			glx, gly, glwidth, glheight;

// only the refresh window will be updated unless these variables are flagged 
int			scr_copytop;
int			scr_copyeverything;

float		scr_con_current;
float		scr_conlines;		// lines of console to display

float		oldscreensize, oldfov;
cvar_t		scr_viewsize = {"viewsize","110", true};
cvar_t		scr_fov = {"fov","90"};	// 10 - 170
cvar_t		scr_conspeed = {"scr_conspeed","300"};
cvar_t		scr_centertime = {"scr_centertime","4"};
cvar_t		scr_showram = {"showram","1"};
cvar_t		scr_showturtle = {"showturtle","0"};
cvar_t		scr_showpause = {"showpause","1"};
cvar_t		scr_printspeed = {"scr_printspeed","8"};
cvar_t		gl_triplebuffer = {"gl_triplebuffer", "0", true };
extern	cvar_t	crosshair;

extern qboolean	draw_reinit;
qboolean	scr_initialized;		// ready to draw

qpic_t		*scr_ram;
qpic_t		*scr_net;
qpic_t		*scr_turtle;

int			scr_fullupdate;
int			scr_topupdate;

int			clearconsole;
int			clearnotify;

int			sb_lines;

viddef_t	vid;				// global video state

vrect_t		scr_vrect;

qboolean	scr_disabled_for_loading;

qboolean	block_drawing;

void SCR_ScreenShot_f (void);
void Plaque_Draw (char *message, qboolean AlwaysDraw);

/*
===============================================================================

CENTER PRINTING

===============================================================================
*/

char		scr_centerstring[1024];
float		scr_centertime_start;	// for slow victory printing
float		scr_centertime_off;
int			scr_center_lines;
int			scr_erase_lines;
int			scr_erase_center;

static int lines;
static int StartC[20],EndC[20];

void FindTextBreaks(char *message, int Width)
{
	int length,pos,start,lastspace,oldlast;

	length = strlen(message);
	lines = pos = start = 0;
	lastspace = -1;

	while(1)
	{
		if (pos-start >= Width || message[pos] == '@' ||
			message[pos] == 0)
		{
			oldlast = lastspace;
			if (message[pos] == '@' || lastspace == -1 || message[pos] == 0)
				lastspace = pos;

			StartC[lines] = start;
			EndC[lines] = lastspace;
			lines++;

			if (message[pos] == '@')
				start = pos + 1;
			else if (oldlast == -1)
				start = lastspace;
			else
				start = lastspace + 1;

			lastspace = -1;
		}

		if (message[pos] == 32) lastspace = pos;
		else if (message[pos] == 0)
		{
			break;
		}

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
void SCR_CenterPrint (char *str)
{
	strncpy (scr_centerstring, str, sizeof(scr_centerstring)-1);
	scr_centertime_off = scr_centertime.value;
	scr_centertime_start = cl.time;

	FindTextBreaks(scr_centerstring, 38);
	scr_center_lines = lines;
}


void SCR_DrawCenterString (void)
{
	int		i;
	int		bx, by;
	int		remaining;
	char	temp[80];

// the finale prints the characters one at a time
	if (cl.intermission)
		remaining = scr_printspeed.value * (cl.time - scr_centertime_start);
	else
		remaining = 9999;

	scr_erase_center = 0;

	FindTextBreaks(scr_centerstring, 38);

	by = ((25-lines) * 8) / 2;
	for(i=0;i<lines;i++,by+=8)
	{
		strncpy(temp,&scr_centerstring[StartC[i]],EndC[i]-StartC[i]);
		temp[EndC[i]-StartC[i]] = 0;
		bx = ((40-strlen(temp)) * 8) / 2;
	  	M_Print2 (bx, by, temp);
	}
}

void SCR_CheckDrawCenterString (void)
{
	scr_copytop = 1;
	if (scr_center_lines > scr_erase_lines)
		scr_erase_lines = scr_center_lines;

	scr_centertime_off -= host_frametime;
	
	if (scr_centertime_off <= 0 && !cl.intermission)
		return;
	if (key_dest != key_game)
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
	float		size;
	int		h;

	scr_fullupdate = 0;		// force a background redraw
	vid.recalc_refdef = 0;

// force the status bar to redraw
	Sbar_Changed ();

// bound viewsize
	if (scr_viewsize.value < 30)
		Cvar_Set ("viewsize","30");
	if (scr_viewsize.value > 120)
		Cvar_Set ("viewsize","120");

// bound field of view
	if (scr_fov.value < 10)
		Cvar_Set ("fov","10");
	if (scr_fov.value > 170)
		Cvar_Set ("fov","170");

// intermission is always full screen	
	if (cl.intermission)
		size = 110;
	else
		size = scr_viewsize.value;

/*	if (size >= 120)
		sb_lines = 0;		// no status bar at all
	else if (size >= 110)
		sb_lines = 24;		// no inventory
	else
		sb_lines = 24+16+8;*/

	if (size >= 110)
	{ // No status bar
		sb_lines = 0;
	}
	else
	{
		sb_lines = 36;
	}

	size = scr_viewsize.value > 100 ? 100 : scr_viewsize.value;
	if (cl.intermission)
	{
		size = 100;
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

	scr_vrect = r_refdef.vrect;
}


/*
=================
SCR_SizeUp_f

Keybinding command
=================
*/
void SCR_SizeUp_f (void)
{
	if (scr_viewsize.value < 120)
	{
		Cvar_SetValue ("viewsize",scr_viewsize.value+10);
		vid.recalc_refdef = 1;
		SB_ViewSizeChanged();
	}
}


/*
=================
SCR_SizeDown_f

Keybinding command
=================
*/
void SCR_SizeDown_f (void)
{
	Cvar_SetValue ("viewsize",scr_viewsize.value-10);
	vid.recalc_refdef = 1;
	SB_ViewSizeChanged();
}

//============================================================================

/*
==================
SCR_Init
==================
*/
void SCR_Init (void)
{
	if (!draw_reinit)
	{
		Cvar_RegisterVariable (&scr_fov);
		Cvar_RegisterVariable (&scr_viewsize);
		Cvar_RegisterVariable (&scr_conspeed);
		Cvar_RegisterVariable (&scr_showram);
		Cvar_RegisterVariable (&scr_showturtle);
		Cvar_RegisterVariable (&scr_showpause);
		Cvar_RegisterVariable (&scr_centertime);
		Cvar_RegisterVariable (&scr_printspeed);
		Cvar_RegisterVariable (&gl_triplebuffer);

		// register our commands
		Cmd_AddCommand ("screenshot",SCR_ScreenShot_f);
		Cmd_AddCommand ("sizeup",SCR_SizeUp_f);
		Cmd_AddCommand ("sizedown",SCR_SizeDown_f);
	}

	scr_ram = Draw_PicFromWad ("ram");
	scr_net = Draw_PicFromWad ("net");
	scr_turtle = Draw_PicFromWad ("turtle");

	scr_initialized = true;
}



/*
==============
SCR_DrawRam
==============
*/
void SCR_DrawRam (void)
{
	if (!scr_showram.value)
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
void SCR_DrawTurtle (void)
{
	static int	count;
	
	if (!scr_showturtle.value)
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
void SCR_DrawNet (void)
{
	if (cls.netchan.outgoing_sequence - cls.netchan.incoming_acknowledged < UPDATE_BACKUP-1)
		return;
	if (cls.demoplayback)
		return;

	Draw_Pic (scr_vrect.x+64, scr_vrect.y, scr_net);
}

void SCR_DrawFPS (void)
{
	extern cvar_t show_fps;
	static double lastframetime;
	double t;
	extern int fps_count;
	static int lastfps;
	int x, y;
	char st[80];

	if (!show_fps.value)
		return;

	t = Sys_DoubleTime();
	if ((t - lastframetime) >= 1.0) {
		lastfps = fps_count;
		fps_count = 0;
		lastframetime = t;
	}

	sprintf(st, "%3d FPS", lastfps);
	x = vid.width - strlen(st) * 8 - 8;
	y = vid.height - sb_lines - 8;
//	Draw_TileClear(x, y, strlen(st) * 8, 8);
	Draw_String(x, y, st);
}


/*
==============
DrawPause
==============
*/
void SCR_DrawPause (void)
{
	qpic_t	*pic;
	float delta;
	static qboolean newdraw = false;
	int finaly;
	static float LogoPercent,LogoTargetPercent;

	if (!scr_showpause.value)		// turn off for screenshots
		return;

	if (!cl.paused)
	{
		newdraw = false;
		return;
	}

	if (!newdraw)
	{
		newdraw = true;
		LogoTargetPercent= 1;
		LogoPercent = 0;
	}

	pic = Draw_CachePic ("gfx/menu/paused.lmp");
//	Draw_Pic ( (vid.width - pic->width)/2, 
//		(vid.height - 48 - pic->height)/2, pic);

	if (LogoPercent < LogoTargetPercent)
	{
		delta = ((LogoTargetPercent-LogoPercent)/.5)*host_frametime;
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
void SCR_SetUpToDrawConsole (void)
{
	Con_CheckResize ();

// decide on the height of the console
	if (cls.state != ca_active)
	{
		scr_conlines = vid.height;	// full screen
		scr_con_current = scr_conlines;
	}
	else if (key_dest == key_console)
		scr_conlines = vid.height/2;	// half screen
	else
		scr_conlines = 0;		// none visible

	if (scr_conlines < scr_con_current)
	{
		scr_con_current -= scr_conspeed.value*host_frametime;
		if (scr_conlines > scr_con_current)
			scr_con_current = scr_conlines;

	}
	else if (scr_conlines > scr_con_current)
	{
		scr_con_current += scr_conspeed.value*host_frametime;
		if (scr_conlines < scr_con_current)
			scr_con_current = scr_conlines;
	}

	if (clearconsole++ < vid.numpages)
	{
		Sbar_Changed ();
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
void SCR_DrawConsole (void)
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
void SCR_ScreenShot_f (void) 
{
	byte		*buffer;
	char		pcxname[80]; 
	char		checkname[MAX_OSPATH];
	int			i, c, temp;
	int			mark;

	sprintf (checkname, "%s/shots", com_userdir);
	Sys_mkdir (checkname);

// 
// find a file name to save it to 
// 
	strcpy(pcxname,"shots/hw00.tga");

	for (i=0 ; i<=99 ; i++)
	{
		pcxname[8] = i/10 + '0';
		pcxname[9] = i%10 + '0';
		sprintf (checkname, "%s/%s", com_userdir, pcxname);
		if (access(checkname, F_OK) == -1)
			break;	// file doesn't exist
	}
	if (i==100)
	{
		Con_Printf ("SCR_ScreenShot_f: Couldn't create a PCX file\n");
		return;
	}

	mark = Hunk_LowMark();
	buffer = Hunk_AllocName(glwidth * glheight * 3 + 18, "buffer_sshot");
	memset (buffer, 0, 18);
	buffer[2] = 2;		// uncompressed type
	buffer[12] = glwidth&255;
	buffer[13] = glwidth>>8;
	buffer[14] = glheight&255;
	buffer[15] = glheight>>8;
	buffer[16] = 24;	// pixel size

	glReadPixels_fp (glx, gly, glwidth, glheight, GL_RGB, GL_UNSIGNED_BYTE, buffer+18 );

	// swap rgb to bgr
	c = 18+glwidth*glheight*3;
	for (i=18 ; i<c ; i+=3)
	{
		temp = buffer[i];
		buffer[i] = buffer[i+2];
		buffer[i+2] = temp;
	}
	COM_WriteFile (pcxname, buffer, glwidth*glheight*3 + 18 );

	Hunk_FreeToLowMark(mark);
	Con_Printf ("Wrote %s\n", pcxname);
}


//=============================================================================

char	*scr_notifystring;
qboolean	scr_drawdialog;

void SCR_DrawNotifyString (void)
{
	Plaque_Draw(scr_notifystring,1);
}

/*
==================
SCR_ModalMessage

Displays a text string in the center of the screen
and waits for a Y or N keypress.
==================
*/
int SCR_ModalMessage (char *text)
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
void SCR_BringDownConsole (void)
{
	int		i;

	scr_centertime_off = 0;

	for (i=0 ; i<20 && scr_conlines != scr_con_current ; i++)
		SCR_UpdateScreen ();

	cl.cshifts[0].percent = 0;		// no area contents palette on next frame
	VID_SetPalette (host_basepal);
}

void SCR_TileClear (void)
{
	if (vid.conwidth > 320)
	{
		if (r_refdef.vrect.x > 0)
		{
			// left
			Draw_TileClear (0, 0, r_refdef.vrect.x, vid.height);
			// right
			Draw_TileClear (r_refdef.vrect.x + r_refdef.vrect.width, 0, 
				vid.width - r_refdef.vrect.x + r_refdef.vrect.width, 
				vid.height);
		}
//		if (r_refdef.vrect.y > 0)
		{
			// top
			Draw_TileClear (r_refdef.vrect.x, 0, 
				r_refdef.vrect.x + r_refdef.vrect.width, r_refdef.vrect.y);
			// bottom
			Draw_TileClear (r_refdef.vrect.x,
				r_refdef.vrect.y + r_refdef.vrect.height, 
				r_refdef.vrect.width, 
				vid.height - (r_refdef.vrect.height + r_refdef.vrect.y));
		}
	}
	else
	{
		if (r_refdef.vrect.x > 0)
		{
			// left
			Draw_TileClear (0, 0, r_refdef.vrect.x, vid.height - sb_lines);
			// right
			Draw_TileClear (r_refdef.vrect.x + r_refdef.vrect.width, 0, 
				vid.width - r_refdef.vrect.x + r_refdef.vrect.width, 
				vid.height - sb_lines);
		}
		if (r_refdef.vrect.y > 0)
		{
			// top
			Draw_TileClear (r_refdef.vrect.x, 0, 
				r_refdef.vrect.x + r_refdef.vrect.width, 
				r_refdef.vrect.y);
			// bottom
			Draw_TileClear (r_refdef.vrect.x,
				r_refdef.vrect.y + r_refdef.vrect.height, 
				r_refdef.vrect.width, 
				vid.height - sb_lines - 
				(r_refdef.vrect.height + r_refdef.vrect.y));
		}
	}
}

float oldsbar = 0;

// This is also located in screen.c
#define PLAQUE_WIDTH 26

void Plaque_Draw (char *message, qboolean AlwaysDraw)
{
	int i;
	char temp[80];
	int bx,by;

	if (scr_con_current == vid.height && !AlwaysDraw)
		return;		// console is full screen

	if (!*message) 
		return;

	FindTextBreaks(message, PLAQUE_WIDTH);

	by = ((25-lines) * 8) / 2;
	M_DrawTextBox2 (32, by-16, 30, lines+2);

	for(i=0;i<lines;i++,by+=8)
	{
		strncpy(temp,&message[StartC[i]],EndC[i]-StartC[i]);
		temp[EndC[i]-StartC[i]] = 0;
		bx = ((40-strlen(temp)) * 8) / 2;
	  	M_Print2 (bx, by, temp);
	}
}


void I_DrawCharacter (int cx, int line, int num)
{
	Draw_Character ( cx + ((vid.width - 320)>>1), line + ((vid.height - 200)>>1), num);
}

void I_Print (int cx, int cy, char *str)
{
	while (*str)
	{
		I_DrawCharacter (cx, cy, ((unsigned char)(*str))+256);
		str++;
		cx += 8;
	}
}

//==========================================================================
//
// SB_IntermissionOverlay
//
//==========================================================================
#if 0	// the code is not used in H2W
void SB_IntermissionOverlay(void)
{
	qpic_t	*pic;
	int		elapsed, size, bx, by, i;
	char	*message,temp[80];

	scr_copyeverything = 1;
	scr_fullupdate = 0;

//	if (cl.gametype == GAME_DEATHMATCH)
	if(!cl_siege)
	{
		Sbar_DeathmatchOverlay ();
		return;
	}
	
	switch(cl.intermission)
	{
		case 1:
			pic = Draw_CachePicNoTrans ("gfx/meso.lmp");
			break;
		case 2:
			pic = Draw_CachePicNoTrans ("gfx/egypt.lmp");
			break;
		case 3:
			pic = Draw_CachePicNoTrans ("gfx/roman.lmp");
			break;
		case 4:
			pic = Draw_CachePicNoTrans ("gfx/castle.lmp");
			break;
		case 5:
			pic = Draw_CachePicNoTrans ("gfx/castle.lmp");
			break;
		case 6:
			pic = Draw_CachePicNoTrans ("gfx/end-1.lmp");
			break;
		case 7:
			pic = Draw_CachePicNoTrans ("gfx/end-2.lmp");
			break;
		case 8:
			pic = Draw_CachePicNoTrans ("gfx/end-3.lmp");
			break;
		case 9:
			pic = Draw_CachePicNoTrans ("gfx/castle.lmp");
			break;
		case 10://Defender win - wipe out or time limit
			pic = Draw_CachePicNoTrans ("gfx/defwin.lmp");
			break;
		case 11://Attacker win - caught crown
			pic = Draw_CachePicNoTrans ("gfx/attwin.lmp");
			break;
		case 12://Attacker win 2 - wiped out
			pic = Draw_CachePicNoTrans ("gfx/attwin2.lmp");
			break;
		default:
			pic = NULL;
			break;
	}
	if (pic == NULL)
		Sys_Error ("SB_IntermissionOverlay: Bad episode");

	//Draw_Pic (((vid.width - 320)>>1),((vid.height - 200)>>1), pic);
	Draw_IntermissionPic(pic);

	if (cl.intermission >= 6 && cl.intermission <= 8)
	{
		elapsed = (cl.time - cl.completed_time) * 20;
		elapsed -= 50;
		if (elapsed < 0) elapsed = 0;
	}
	else
	{
		elapsed = (cl.time - cl.completed_time) * 20;
	}

	if (cl.intermission <= 4 && cl.intermission + 394 <= pr_string_count)
		message = &pr_global_strings[pr_string_index[cl.intermission + 394]];
	else if (cl.intermission == 5)
		message = &pr_global_strings[pr_string_index[408]];
	else if (cl.intermission >= 6 && cl.intermission <= 8 && cl.intermission + 386 <= pr_string_count)
		message = &pr_global_strings[pr_string_index[cl.intermission + 386]];
	else if (cl.intermission == 9)
		message = &pr_global_strings[pr_string_index[391]];
	else
		message = "";

	FindTextBreaks(message, 38);

	if (cl.intermission == 8)
		by = 16;
	else
		by = ((25-lines) * 8) / 2;

	for(i=0;i<lines;i++,by+=8)
	{
		size = EndC[i]-StartC[i];
		strncpy(temp,&message[StartC[i]],size);

		if (size > elapsed) size = elapsed;
		temp[size] = 0;

		bx = ((40-strlen(temp)) * 8) / 2;
	  	if (cl.intermission < 6)
			I_Print (bx, by, temp);
		else
			M_PrintWhite (bx, by, temp);

		elapsed -= size;
		if (elapsed <= 0) break;
	}

	if (i == lines && elapsed >= 300 && cl.intermission >= 6 && cl.intermission <= 7)
	{
		cl.intermission++;
		cl.completed_time = cl.time;
	}
//	Con_Printf("Time is %10.2f\n",elapsed);
}
#endif	// end of unused SB_IntermissionOverlay code

//==========================================================================
//
// SB_FinaleOverlay
//
//==========================================================================
#if 0	// the code is not used in H2W
void SB_FinaleOverlay(void)
{
	qpic_t	*pic;

	scr_copyeverything = 1;

	pic = Draw_CachePic("gfx/finale.lmp");
	Draw_TransPic((vid.width-pic->width)/2, 16, pic);
}
#endif	// end of unused SB_FinaleOverlay code

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

	vid.numpages = 2 + gl_triplebuffer.value;

	scr_copytop = 0;
	scr_copyeverything = 0;

	if (scr_disabled_for_loading)
			return;

	if (!scr_initialized || !con_initialized)
		return;				// not initialized yet

	if (oldsbar != cl_sbar.value) {
		oldsbar = cl_sbar.value;
		vid.recalc_refdef = true;
	}

	GL_BeginRendering (&glx, &gly, &glwidth, &glheight);
	
	//
	// determine size of refresh window
	//
	if (vid.recalc_refdef)
		SCR_CalcRefdef ();

//
// do 3D refresh drawing, and then update the screen
//
	SCR_SetUpToDrawConsole ();
	
	// no need to draw view in fullscreen intermission screens
	//if (cl.intermission > 1 || cl.intermission <= 12)
	if (cl.intermission < 1 || cl.intermission > 12)
	{
		V_RenderView ();
	}

	GL_Set2D ();

	//
	// draw any areas not covered by the refresh
	//
	SCR_TileClear ();

	if (r_netgraph.value)
		R_NetGraph ();

	if (scr_drawdialog)
	{
		Sbar_Draw ();
		Draw_FadeScreen ();
		SCR_DrawNotifyString ();
		scr_copyeverything = true;
	}
	else if (cl.intermission >= 1 && cl.intermission <= 12)
	{
#if 0	// not in H2W
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
	}*/
	else
	{
		if (crosshair.value)
			Draw_Crosshair();

		SCR_DrawRam();
		SCR_DrawFPS();
		SCR_DrawTurtle();
		SCR_DrawPause();
		SCR_CheckDrawCenterString();
		Sbar_Draw();

			Plaque_Draw(plaquemessage,0);
			SCR_DrawNet ();
			SCR_DrawConsole();
			M_Draw();
			if (errormessage)
				Plaque_Draw(errormessage,1);
	}

	V_UpdatePalette ();

	GL_EndRendering ();
}
