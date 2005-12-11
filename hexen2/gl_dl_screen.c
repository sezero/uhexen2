/*
	screen.c
	master for refresh, status bar, console, chat, notify, etc

	$Id: gl_dl_screen.c,v 1.22 2005-12-11 11:56:33 sezero Exp $
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
extern	cvar_t	crosshair;

qboolean	scr_initialized;		// ready to draw
qboolean	ls_invalid = true;		// whether we need to redraw the loading screen plaque
static int	ls_offset;

qpic_t		*scr_ram;
qpic_t		*scr_net;
qpic_t		*scr_turtle;

float		introTime = 0.0;

int			scr_fullupdate;
int			scr_topupdate;

int			clearconsole;
int			clearnotify;

int			sb_lines;

viddef_t	vid;				// global video state

vrect_t		scr_vrect;

qboolean	scr_disabled_for_loading;
qboolean	scr_drawloading;
float		scr_disabled_time;

static qboolean scr_needfull = false;

int		total_loading_size, current_loading_size, loading_stage;

void SCR_ScreenShot_f (void);
void Plaque_Draw (char *message, qboolean AlwaysDraw);
void Info_Plaque_Draw (char *message);
void Bottom_Plaque_Draw (char *message);

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
#define MAXLINES 27
static int StartC[MAXLINES],EndC[MAXLINES];

#define MAX_INFO 1024
char infomessage[MAX_INFO];
qboolean info_up = false;

extern int	*pr_info_string_index;
#ifdef H2MP
extern char	*pr_global_info_strings;
#endif
extern int	 pr_info_string_count;

#ifdef H2MP
void UpdateInfoMessage(void)
{
	unsigned int i;
	unsigned int check;
	char *newmessage;

	strcpy(infomessage, "Objectives:");

	if (!pr_global_info_strings)
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
#endif

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

	if(intro_playing)
		Bottom_Plaque_Draw(scr_centerstring);
	else
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
	Sbar_Changed();

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
	Cvar_SetValue ("viewsize",scr_viewsize.value+10);
	SB_ViewSizeChanged();
	vid.recalc_refdef = 1;
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
	SB_ViewSizeChanged();
	vid.recalc_refdef = 1;
}

//============================================================================

/*
==================
SCR_Init
==================
*/
void SCR_Init (void)
{
	Cvar_RegisterVariable (&scr_fov);
	Cvar_RegisterVariable (&scr_viewsize);
	Cvar_RegisterVariable (&scr_conspeed);
	Cvar_RegisterVariable (&scr_showram);
	Cvar_RegisterVariable (&scr_showturtle);
	Cvar_RegisterVariable (&scr_showpause);
	Cvar_RegisterVariable (&scr_centertime);
	Cvar_RegisterVariable (&scr_printspeed);

//
// register our commands
//
	Cmd_AddCommand ("screenshot",SCR_ScreenShot_f);
	Cmd_AddCommand ("sizeup",SCR_SizeUp_f);
	Cmd_AddCommand ("sizedown",SCR_SizeDown_f);

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


/*
==============
SCR_DrawLoading
==============
*/
void SCR_DrawLoading (void)
{
	int	size, count;
	qpic_t	*pic;

	if (!scr_drawloading && loading_stage == 0)
		return;

/*	draw first time only (since drawing this into GL_FRONT).
	otherwise the image flickers because of redrawing
*/
	if (ls_invalid)
	{
		pic = Draw_CachePic ("gfx/menu/loading.lmp");
		ls_offset = (vid.width - pic->width)/2;
		Draw_TransPic (ls_offset , 0, pic);
	}

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



//=============================================================================


/*
==================
SCR_SetUpToDrawConsole
==================
*/
void SCR_SetUpToDrawConsole (void)
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
void SCR_DrawConsole (void)
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
	strcpy(pcxname,"shots/hexen00.tga");

	for (i=0 ; i<=99 ; i++)
	{
		pcxname[11] = i/10 + '0';
		pcxname[12] = i%10 + '0';
		sprintf (checkname, "%s/%s", com_userdir, pcxname);
		if (access(checkname, F_OK) == -1)
			break;	// file doesn't exist
	}
	if (i==100)
	{
		Con_Printf ("SCR_ScreenShot_f: Couldn't create a TGA file\n");
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
	if (r_refdef.vrect.x > 0)
	{
		Draw_TileClear (0,0,r_refdef.vrect.x,vid.height);
		Draw_TileClear (r_refdef.vrect.x + r_refdef.vrect.width, 0
			, vid.width - r_refdef.vrect.x + r_refdef.vrect.width,vid.height);
	}
//	if (r_refdef.vrect.height < vid.height-44)
	{
		Draw_TileClear (r_refdef.vrect.x, 0, r_refdef.vrect.width, r_refdef.vrect.y);
		Draw_TileClear (r_refdef.vrect.x, r_refdef.vrect.y + r_refdef.vrect.height,
			r_refdef.vrect.width,
			vid.height - (r_refdef.vrect.y + r_refdef.vrect.height) );
	}
}

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
	M_DrawTextBox2 (32, by-16, 30, lines+2,false);

	for(i=0;i<lines;i++,by+=8)
	{
		strncpy(temp,&message[StartC[i]],EndC[i]-StartC[i]);
		temp[EndC[i]-StartC[i]] = 0;
		bx = ((40-strlen(temp)) * 8) / 2;
	  	M_Print2 (bx, by, temp);
	}
}

void Info_Plaque_Draw (char *message)
{
	int i;
	char temp[80];
	int bx,by;

	if (scr_con_current == vid.height)
		return;		// console is full screen

	if (!*message) 
		return;

	scr_needfull = true;

	FindTextBreaks(message, PLAQUE_WIDTH+4);

	if (lines == MAXLINES) 
	{
		Con_DPrintf("Info_Plaque_Draw: line overflow error\n");
		lines = MAXLINES-1;
	}

	by = ((25-lines) * 8) / 2;
	M_DrawTextBox2 (15, by-16, PLAQUE_WIDTH+4+4, lines+2,false);

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

void Bottom_Plaque_Draw (char *message)
{
	int i;
	char temp[80];
	int bx,by;

	if (!*message) 
		return;

	scr_needfull = true;

	FindTextBreaks(message, PLAQUE_WIDTH);

	by = (((vid.height)/8)-lines-2) * 8;

	M_DrawTextBox2 (32, by-16, 30, lines+2,true);

	for(i=0;i<lines;i++,by+=8)
	{
		strncpy(temp,&message[StartC[i]],EndC[i]-StartC[i]);
		temp[EndC[i]-StartC[i]] = 0;
		bx = ((40-strlen(temp)) * 8) / 2;
	  	M_Print(bx, by, temp);
	}
}
//==========================================================================
//
// SB_IntermissionOverlay
//
//==========================================================================

void SB_IntermissionOverlay(void)
{
	qpic_t	*pic;
	int		elapsed, size, bx, by, i;
	char	*message,temp[80];

	scr_copyeverything = 1;
	scr_fullupdate = 0;

	if (cl.gametype == GAME_DEATHMATCH)
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
		case 10:
			pic = Draw_CachePicNoTrans ("gfx/mpend.lmp");
			break;
		case 11:
			pic = Draw_CachePicNoTrans ("gfx/mpmid.lmp");
			break;
		case 12:
			pic = Draw_CachePicNoTrans ("gfx/end-3.lmp");
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
	else if (cl.intermission == 12)
	{
		elapsed = (introTime);
		if (introTime < 500)
			introTime+=0.25;
	}
	else
	{
		elapsed = (cl.time - cl.completed_time) * 20;
	}

	if (cl.intermission <= 4 && cl.intermission + 394 <= pr_string_count)
		message = &pr_global_strings[pr_string_index[cl.intermission + 394]];
/* O.S: Why do calculate and not say 408? Anyone weird enough to play the demo
   with strings.txt file from H2MP ?!!!! The thing below was off-by-one, btw..
*/	else if (cl.intermission == 5)
		message = &pr_global_strings[pr_string_index[ABILITIES_STR_INDEX+MAX_PLAYER_CLASS*2]];
	else if (cl.intermission >= 6 && cl.intermission <= 8 && cl.intermission + 386 <= pr_string_count)
		message = &pr_global_strings[pr_string_index[cl.intermission + 386]];
	else if (cl.intermission == 9)
		message = &pr_global_strings[pr_string_index[391]];
	else
		message = "";
	
	if (cl.intermission == 10)
		message = &pr_global_strings[pr_string_index[538]];
	else if (cl.intermission == 11)
		message = &pr_global_strings[pr_string_index[545]];
	else if (cl.intermission == 12)
		message = &pr_global_strings[pr_string_index[561]];

	FindTextBreaks(message, 38);

	if (cl.intermission == 8)
		by = 16;
	else
		by = ((25-lines) * 8) / 2;

	// final message hacks - S.A
	// eidolon cl.intermission == 6,7,8
	// "by" is the y offset in pixels to begin printing at
	// each line is 8 pixels
	if (cl.intermission >= 6 && cl.intermission <= 8)
		by = (vid.height/2 - lines*4);

	// different story for the mission pack
	if (cl.intermission == 10)	// tibet10 cl.intermission == 10
		by=33;

	// printf ("cl.intermission == %i, lines == %i,vid(x,y)=%i,%i by=%i\n\n",cl.intermission,lines,vid.width,vid.height,by);

	for(i=0;i<lines;i++,by+=8)
	{
		size = EndC[i]-StartC[i];
		strncpy(temp,&message[StartC[i]],size);

		if (size > elapsed) size = elapsed;
		temp[size] = 0;

		bx = ((40-strlen(temp)) * 8) / 2;
	  	if (cl.intermission < 6 || cl.intermission > 9)
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

//==========================================================================
//
// SB_FinaleOverlay
//
//==========================================================================

void SB_FinaleOverlay(void)
{
	qpic_t	*pic;

	scr_copyeverything = 1;

	pic = Draw_CachePic("gfx/finale.lmp");
	Draw_TransPic((vid.width-pic->width)/2, 16, pic);
}

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
	scr_copytop = 0;
	scr_copyeverything = 0;

	if (scr_disabled_for_loading)
	{
		if (realtime - scr_disabled_time > 60)
		{
			scr_disabled_for_loading = false;
			total_loading_size = 0;
			loading_stage = 0;
			// loading plaque redraw needed
			ls_invalid = true;
			Con_Printf ("load failed.\n");
		}
		else
			return;
	}

	if (!scr_initialized || !con_initialized)
		return;				// not initialized yet


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
	//if (cl.intermission > 1 || cl.intermission <= 12) {
	if (cl.intermission < 1 || cl.intermission > 12)
	{
		V_RenderView ();
	}

	GL_Set2D ();

	//
	// draw any areas not covered by the refresh
	//
	SCR_TileClear ();

	if (scr_drawdialog)
	{
		Sbar_Draw();
		Draw_FadeScreen ();
		SCR_DrawNotifyString ();
		scr_copyeverything = true;
	}
//	Pa3PyX: this clobbers intermission screens
/*	else if (scr_drawloading)
	{
		Sbar_Draw();
		Draw_FadeScreen ();
		SCR_DrawLoading ();
	} */
	else if (cl.intermission >= 1 && cl.intermission <= 12)
	{
		SB_IntermissionOverlay();
		if (cl.intermission < 12)
		{
			SCR_DrawConsole();
			M_Draw();
		}
		// Pa3PyX
		if (scr_drawloading)
			SCR_DrawLoading();
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
		SCR_DrawNet();
		SCR_DrawTurtle();
		SCR_DrawPause();
		SCR_CheckDrawCenterString();
		Sbar_Draw();

		// Pa3PyX: draw loading plaque and dim screen if loading
		if (scr_drawloading)
		{
			Draw_FadeScreen();
			SCR_DrawLoading();
		}
		else {
			Plaque_Draw(plaquemessage,0);
			SCR_DrawConsole();
			M_Draw();
			if (errormessage)
				Plaque_Draw(errormessage,1);

			if (info_up)
			{
#ifdef H2MP
				UpdateInfoMessage();
#endif
				Info_Plaque_Draw(infomessage);
			}
		}
	}

	V_UpdatePalette ();

	GL_EndRendering ();
}

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.21  2005/10/25 19:59:44  sezero
 * added a prototype for Draw_Crosshair to draw.h
 *
 * Revision 1.20  2005/09/28 06:09:31  sezero
 * took care of flickering problem while drawing the loading
 * plaque (from Pa3PyX.) glFlush is now required.
 *
 * Revision 1.19  2005/09/19 19:50:10  sezero
 * fixed those famous spelling errors
 *
 * Revision 1.18  2005/09/19 19:25:35  sezero
 * matched the software renderer counterpart
 *
 * Revision 1.17  2005/08/23 12:24:11  sezero
 * safe clean-ups before 1.3.0-final #1:
 * killed some stupid 'may be used uninitialized' warnings
 *
 * Revision 1.16  2005/08/07 10:59:05  sezero
 * killed the Sys_FileTime crap. now using standart access() function.
 *
 * Revision 1.15  2005/07/17 15:19:34  sezero
 * added crosshair 2 support of hexenworld to hexen2.
 * gl versions issue, that crosshair 2 won't show-up
 * without a proper low conwidth, still remains to be
 * solved.
 *
 * Revision 1.14  2005/07/09 07:29:40  sezero
 * use hunk instead of malloc
 *
 * Revision 1.13  2005/06/15 13:18:17  sezero
 * killed the glfunc struct
 *
 * Revision 1.12  2005/05/29 08:53:57  sezero
 * get rid of silly name changes
 *
 * Revision 1.11  2005/05/26 21:36:36  sezero
 * whitespace...
 *
 * Revision 1.10  2005/05/26 21:32:17  sezero
 * whitespace...
 *
 * Revision 1.9  2005/05/22 11:50:59  sezero
 * changed default viewsize to 110
 *
 * Revision 1.8  2005/01/12 22:11:05  sezero
 * Fix off-by-one error in demo ending message line calculation.
 *
 * Revision 1.7  2004/12/18 14:20:40  sezero
 * Clean-up and kill warnings: 11
 * A lot of whitespace cleanups.
 *
 * Revision 1.6  2004/12/18 14:08:07  sezero
 * Clean-up and kill warnings 9:
 * Kill many unused vars.
 *
 * Revision 1.5  2004/12/12 14:25:18  sezero
 * add and fix some comments
 *
 * Revision 1.4  2004/12/12 14:14:42  sezero
 * style changes to our liking
 *
 * Revision 1.3  2004/12/05 10:52:18  sezero
 * Sync with Steven, 2004-12-04 :
 *  Fix the "Old Mission" menu PoP
 *  Also release the windowed mouse on pause
 *  Heapsize is now 32768 default
 *  The final splash screens now centre the messages properly
 *  Add more mods to the video mods table
 *  Add the docs folder and update it
 *
 * Revision 1.2  2004/11/29 12:17:46  sezero
 * draw fullscreen intermission pics. borrowed from Pa3PyX sources.
 *
 * Revision 1.1.1.1  2004/11/28 00:03:15  sezero
 * Initial import of AoT 1.2.0 code
 *
 * Revision 1.2  2002/01/04 14:50:09  phneutre
 * save screenshots in ~/.aot/shots
 *
 * Revision 1.1  2002/01/02 15:14:44  phneutre
 * dlsym'ed all calls to GL functions with SDL_GL_GetProcAddress
 *
 * Revision 1.2  2001/11/12 01:27:54  theoddone33
 * Make glhexen2 build, even though it still doesn't work
 *
 * 
 * 20    3/18/98 11:34p Jmonroe
 * fixed gl renderheight in intermission, fixed bottom plaque draw, added
 * credit cd track
 * 
 * 19    3/16/98 11:23a Jweier
 * 
 * 18    3/16/98 12:01a Jweier
 * 
 * 17    3/15/98 10:33p Jweier
 * 
 * 16    3/14/98 5:39p Jmonroe
 * made info_plaque draw safe, fixed bit checking
 * 
 * 15    3/12/98 6:31p Mgummelt
 * 
 * 14    3/12/98 3:32p Jweier
 * 
 * 13    3/11/98 7:12p Mgummelt
 * 
 * 12    3/11/98 6:20p Mgummelt
 * 
 * 11    3/09/98 7:03p Jweier
 * 
 * 10    3/01/98 8:46p Jweier
 * 
 * 9     3/01/98 3:21p Jweier
 * 
 * 8     2/28/98 6:42p Jweier
 * 
 * 7     2/28/98 5:15p Jweier
 * 
 * 6     2/28/98 12:29a Jweier
 * 
 * 5     2/27/98 11:53p Jweier
 * 
 * 4     2/26/98 4:59p Jweier
 * 
 * 3     2/10/98 1:26p Jmonroe
 * 
 * 2     2/09/98 3:44p Jweier
 * 
 * 36    10/29/97 5:39p Jheitzman
 * 
 * 34    9/25/97 11:55p Rjohnson
 * Changed the dialog box for the yes / no requestor
 * 
 * 33    9/25/97 2:10p Rjohnson
 * Smaller status bar
 * 
 * 32    9/25/97 1:55p Rjohnson
 * Updates
 * 
 * 31    9/23/97 8:56p Rjohnson
 * Updates
 * 
 * 30    9/15/97 11:15a Rjohnson
 * Updates
 * 
 * 29    9/09/97 10:49a Rjohnson
 * Updates
 * 
 * 28    9/04/97 4:44p Rjohnson
 * Updates
 * 
 * 27    8/31/97 10:23p Rjohnson
 * Ending updates
 * 
 * 26    8/31/97 10:09p Rjohnson
 * End of game sequence
 * 
 * 25    8/31/97 9:27p Rjohnson
 * GL Updates
 * 
 * 24    8/30/97 6:16p Rjohnson
 * Centering text
 * 
 * 23    8/21/97 2:42p Rjohnson
 * Fix for screen resizing
 * 
 * 22    8/21/97 12:44a Rjohnson
 * End of demo text
 * 
 * 21    8/20/97 2:25p Rjohnson
 * Fixes
 * 
 * 20    8/19/97 10:35p Rjohnson
 * Fix for loading plaque
 * 
 * 19    8/19/97 7:46p Rjohnson
 * End of game updates
 * 
 * 18    8/18/97 12:03a Rjohnson
 * Added loading progress
 * 
 * 17    8/05/97 2:41p Rjohnson
 * Fix for end of hub
 * 
 * 16    8/05/97 11:56a Rjohnson
 * Update for end of hub
 * 
 * 15    7/24/97 12:30p Rjohnson
 * Updates for center strings
 * 
 * 14    7/14/97 3:01p Rjohnson
 * Added support for line breaks for centered text
 * 
 * 13    7/08/97 4:16p Rjohnson
 * Modified plaque drawing routines
 * 
 * 12    6/27/97 11:54a Rjohnson
 * Changed screenshot from quake to hexen
 * 
 * 11    6/27/97 11:53a Rjohnson
 * Changed screen shots from quake to hexen
 * 
 * 10    6/17/97 10:28a Bgokey
 * 
 * 9     6/16/97 3:13a Rjohnson
 * Fixes for: allocating less memory, models clipping out, and plaques in
 * gl version
 * 
 * 8     6/15/97 7:44p Rjohnson
 * Added new pause and loading graphics
 * 
 * 7     5/15/97 2:39p Rjohnson
 * Made it so that the screen shots are placed in their own directory
 * 
 * 6     4/30/97 11:20p Bgokey
 * 
 * 5     4/17/97 3:42p Rjohnson
 * Modifications for the gl version for menus
 * 
 * 4     4/15/97 6:37p Rjohnson
 * Added loading plaque
 * 
 * 3     3/13/97 11:16p Rjohnson
 * Fixed rendering view size
 */
