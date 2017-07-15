/*
 * console.c -- in-game console and chat message buffer handling
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

#include "quakedef.h"
#include "debuglog.h"


console_t	*con;

qboolean	con_initialized;

static int	con_linewidth;		// characters across screen
static int	con_vislines;
int		con_notifylines;	// scan lines to clear for notify lines
int		con_totallines;		// total lines in console scrollback
static float	con_cursorspeed = 4;
qboolean 	con_forcedup;		// because no entities to refresh
int		con_ormask;

static	cvar_t	con_notifytime = {"con_notifytime", "3", CVAR_NONE};	//seconds

#define	NUM_CON_TIMES 4
static float	con_times[NUM_CON_TIMES];	// realtime time the line was generated
						// for transparent notify lines

extern qboolean		menu_disabled_mouse;


static void Key_ClearTyping (void)
{
	key_lines[edit_line][1] = 0;	// clear any typing
	key_linepos = 1;
}

/*
================
Con_ToggleConsole_f
================
*/
void Con_ToggleConsole_f (void)
{
	keydest_t dest = Key_GetDest();

	// activate mouse when in console in
	// case it is disabled somewhere else
	menu_disabled_mouse = false;
	IN_ActivateMouse ();

	Key_ClearTyping ();

	if (dest == key_console || (dest == key_game && con_forcedup))
	{
		if (cls.state == ca_active)
			Key_SetDest (key_game);
		else
			M_Menu_Main_f ();
	}
	else
	{
		Key_SetDest (key_console);
	}

	SCR_EndLoadingPlaque ();
	Con_ClearNotify ();
}

/*
================
Con_Clear_f
================
*/
static void Con_Clear_f (void)
{
	int	i;
	for (i = 0; i < CON_TEXTSIZE; i++)
		con->text[i] = ' ';
}


/*
================
Con_ClearNotify
================
*/
void Con_ClearNotify (void)
{
	int		i;

	for (i = 0; i < NUM_CON_TIMES; i++)
		con_times[i] = 0;
}


/*
================
Con_MessageMode_f
================
*/
static void Con_MessageMode_f (void)
{
	if (cls.state != ca_active || cls.demoplayback)
		return;
	chat_team = false;
	Key_SetDest (key_message);
}

/*
================
Con_MessageMode2_f
================
*/
static void Con_MessageMode2_f (void)
{
	if (cls.state != ca_active || cls.demoplayback)
		return;
	chat_team = true;
	Key_SetDest (key_message);
}


/*
================
Con_CheckResize

If the line width has changed, reformat the buffer.
================
*/
void Con_CheckResize (void)
{
	int	i, j, width, oldwidth, oldtotallines, numlines, numchars;
	short	tbuf[CON_TEXTSIZE];

	width = (vid.width >> 3) - 2;

	if (width == con_linewidth)
		return;

	if (width < 1)			// video hasn't been initialized yet
	{
		width = 38;
		con_linewidth = width;
		con_totallines = CON_TEXTSIZE / con_linewidth;
		Con_Clear_f();
	}
	else
	{
		oldwidth = con_linewidth;
		con_linewidth = width;
		oldtotallines = con_totallines;
		con_totallines = CON_TEXTSIZE / con_linewidth;
		numlines = oldtotallines;

		if (con_totallines < numlines)
			numlines = con_totallines;

		numchars = oldwidth;

		if (con_linewidth < numchars)
			numchars = con_linewidth;

		memcpy (tbuf, con->text, CON_TEXTSIZE*sizeof(short));
		Con_Clear_f();

		for (i = 0; i < numlines; i++)
		{
			for (j = 0; j < numchars; j++)
			{
				con->text[(con_totallines - 1 - i) * con_linewidth + j] =
						tbuf[((con->current - i + oldtotallines) % oldtotallines) * oldwidth + j];
			}
		}

		Con_ClearNotify ();
	}

	con->current = con_totallines - 1;
	con->display = con->current;
}


/*
================
Con_Init
================
*/
void Con_Init (void)
{
	con = (console_t *) Hunk_AllocName (sizeof(console_t), "con_main");
	con_linewidth = -1;
	Con_CheckResize ();

	Con_Printf ("Console initialized.\n");

	Cvar_RegisterVariable (&con_notifytime);

	Cmd_AddCommand ("toggleconsole", Con_ToggleConsole_f);
	Cmd_AddCommand ("messagemode", Con_MessageMode_f);
	Cmd_AddCommand ("messagemode2", Con_MessageMode2_f);
	Cmd_AddCommand ("clear", Con_Clear_f);

	con_initialized = true;
}


/*
===============
Con_Linefeed
===============
*/
static void Con_Linefeed (void)
{
	int	i, j;

	con->x = 0;
	if (con->display == con->current)
		con->display++;
	con->current++;
	j = (con->current%con_totallines) * con_linewidth;
	for (i = 0; i < con_linewidth; i++)
		con->text[i+j] = ' ';
}

/*
================
Con_Print

Handles cursor positioning, line wrapping, etc
All console printing must go through this in order to be logged to disk
If no console is visible, the notify window will pop up.
================
*/
static void Con_Print (const char *txt)
{
	int		y;
	int		c, l;
	static int	cr;
	int		mask;
	qboolean	boundary;

	if (txt[0] == 1)
	{
		mask = 256;		// go to colored text
		S_LocalSound ("misc/comm.wav");	// play talk wav
		txt++;
	}
	else if (txt[0] == 2)
	{
		mask = 256;		// go to colored text
		txt++;
	}
	else
		mask = 0;

	boundary = true;

	while ( (c = (byte)*txt) )
	{
		if (c <= ' ')
		{
			boundary = true;
		}
		else if (boundary)
		{
			// count word length
			for (l = 0; l < con_linewidth; l++)
				if (txt[l] <= ' ')
					break;

			// word wrap
			if (l != con_linewidth && (con->x + l > con_linewidth))
				con->x = 0;

			boundary = false;
		}

		txt++;

		if (cr)
		{
			con->current--;
			cr = false;
		}

		if (!con->x)
		{
			Con_Linefeed ();
		// mark time for transparent overlay
			if (con->current >= 0)
				con_times[con->current % NUM_CON_TIMES] = realtime;
		}

		switch (c)
		{
		case '\n':
			con->x = 0;
			break;

		case '\r':
			con->x = 0;
			cr = 1;
			break;

		default:	// display character and advance
			y = con->current % con_totallines;
			con->text[y*con_linewidth+con->x] = c | mask | con_ormask;
			con->x++;
			if (con->x >= con_linewidth)
				con->x = 0;
			break;
		}
	}
}


/*
================
CON_Printf
Prepare the message to be printed and
send it to the proper handlers.
================
*/
void CON_Printf (unsigned int flags, const char *fmt, ...)
{
	va_list		argptr;
	char		msg[MAX_PRINTMSG];
	static qboolean	inupdate;

	if (flags & _PRINT_DEVEL && !developer.integer)
	{
		if (con_debuglog & LOG_DEVEL)	/* full logging */
		{
			va_start (argptr, fmt);
			q_vsnprintf (msg, sizeof(msg), fmt, argptr);
			va_end (argptr);
			LOG_Print (msg);
		}
		return;
	}

	va_start (argptr, fmt);
	q_vsnprintf (msg, sizeof(msg), fmt, argptr);
	va_end (argptr);

	Sys_PrintTerm (msg);	// echo to the terminal
	if (con_debuglog)
		LOG_Print (msg);

	if (flags & _PRINT_TERMONLY || !con_initialized)
		return;

	if (cls.state == ca_dedicated)
		return;		// no graphics mode

// write it to the scrollable buffer
	Con_Print (msg);

	if (flags & _PRINT_SAFE)
		return;	// safe: doesn't update the screen

// update the screen immediately if the console is displayed
	if (cls.signon != SIGNONS && !scr_disabled_for_loading )
	{
	// protect against infinite loop if SCR_UpdateScreen
	// itself calls Con_Printf
		if (!inupdate)
		{
			inupdate = true;
			SCR_UpdateScreen ();
			inupdate = false;
		}
	}
}


/*
==================
Con_ShowList

Tyrann's ShowList ported by S.A.:
Prints a given list to the console with columnized formatting
==================
*/
void Con_ShowList (int cnt, const char **list)
{
	const char	*s;
	char		*line;
	int	i, j, max_len, len, cols, rows;

	// Lay them out in columns
	max_len = 0;
	for (i = 0; i < cnt; ++i)
	{
		len = (int) strlen(list[i]);
		if (len > max_len)
			max_len = len;
	}

	line = (char *) Z_Malloc(con_linewidth + 1, Z_MAINZONE);
	cols = con_linewidth / (max_len + 2);
	rows = cnt / cols + 1;

	// Looks better if we have a few rows before spreading out
	if (rows < 5)
	{
		cols = cnt / 5 + 1;
		rows = cnt / cols + 1;
	}

	for (i = 0; i < rows; ++i)
	{
		line[0] = '\0';
		for (j = 0; j < cols; ++j)
		{
			if (j * rows + i >= cnt)
				break;
			s = list[j * rows + i];
			len = (int) strlen(s);

			q_strlcat(line, s, con_linewidth+1);
			if (j < cols - 1)
			{
				while (len < max_len)
				{
					q_strlcat(line, " ", con_linewidth+1);
					len++;
				}
				q_strlcat(line, "  ", con_linewidth+1);
			}
		}

		if (line[0] != '\0')
			Con_Printf("%s\n", line);
	}

	Z_Free(line);
}


/*
==============================================================================

DRAWING

==============================================================================
*/


/*
================
Con_DrawInput

The input line scrolls horizontally if typing goes beyond the right edge
================
*/
static void Con_DrawInput (void)
{
	int		i, y;
	size_t		pos;
	char	editlinecopy[MAXCMDLINE], *text;

	if (Key_GetDest() != key_console && !con_forcedup)
		return;		// don't draw anything

	pos = q_strlcpy(editlinecopy, key_lines[edit_line], sizeof(editlinecopy));
	text = editlinecopy;

// fill out remainder with spaces
	for ( ; pos < MAXCMDLINE; ++pos)
		text[pos] = ' ';

// add the cursor frame
	if ((int)(realtime * con_cursorspeed) & 1)	// cursor is visible
		text[key_linepos] = (key_insert) ? 11 : 95; // underscore for overwrite mode, square for insert

//	prestep if horizontally scrolling
	if (key_linepos >= con_linewidth)
		text += 1 + key_linepos - con_linewidth;

// draw it
	y = con_vislines - 22;
	for (i = 0; i < con_linewidth; i++)
		Draw_Character ((i + 1)<<3, y, text[i]);
}


/*
================
Con_DrawNotify

Draws the last few lines of output transparently over the game top
================
*/
void Con_DrawNotify (void)
{
	int	i, x, v;
	const short	*text;
	float	time;

	v = 0;
	for (i = con->current-NUM_CON_TIMES+1; i <= con->current; i++)
	{
		if (i < 0)
			continue;
		time = con_times[i % NUM_CON_TIMES];
		if (time == 0)
			continue;
		time = realtime - time;
		if (time > con_notifytime.value)
			continue;
		text = con->text + (i % con_totallines)*con_linewidth;

		if (scr_viewsize.integer < 100)
			clearnotify = 0;
		scr_copytop = 1;

		for (x = 0; x < con_linewidth; x++)
			Draw_Character ((x+1)<<3, v, text[x]);

		v += 8;
	}

	if (Key_GetDest() == key_message)
	{
		const char	*s;

		if (scr_viewsize.integer < 100)
			clearnotify = 0;
		scr_copytop = 1;

		if (chat_team)
		{
			Draw_String (8, v, "say_team:");
			x = 11;
		}
		else
		{
			Draw_String (8, v, "say:");
			x = 6;
		}

		s = Key_GetChatBuffer();
		i = Key_GetChatMsgLen();
		if (i > (vid.width>>3) - x - 1)
			s += i - (vid.width>>3) + x + 1;

		while (*s)
		{
			Draw_Character (x<<3, v, *s);
			s++;
			x++;
		}

		Draw_Character (x<<3, v, 10 + ((int)(realtime*con_cursorspeed)&1));
		v += 8;
	}

	if (v > con_notifylines)
		con_notifylines = v;
}

/*
================
Con_DrawConsole

Draws the console with the solid background
================
*/
void Con_DrawConsole (int lines)
{
	int		i, x, y;
	int		row, rows;
	const short	*text;

	if (lines <= 0)
		return;

// draw the background
	Draw_ConsoleBackground (lines);

// draw the text
	con_vislines = lines;

// changed to line things up better
	rows = (lines-22)>>3;		// rows of text to draw

	y = lines - 30;

// draw from the bottom up
	if (con->display != con->current)
	{
	// draw arrows to show the buffer is backscrolled
		for (x = 0; x < con_linewidth; x += 4)
			Draw_Character ( (x+1)<<3, y, '^');

		y -= 8;
		rows--;
	}

	row = con->display;
	for (i = 0; i < rows; i++, y -= 8, row--)
	{
		if (row < 0)
			break;
		if (con->current - row >= con_totallines)
			break;		// past scrollback wrap point

		text = con->text + (row % con_totallines)*con_linewidth;

		for (x = 0; x < con_linewidth; x++)
			Draw_Character ( (x+1)<<3, y, text[x]);
	}

// draw the input prompt, user text, and cursor if desired
	Con_DrawInput ();
}


/*
==================
Con_NotifyBox
==================
*/
void Con_NotifyBox (const char *text)
{
	double		t1, t2;

// during startup for sound / cd warnings
	Con_Printf("\n\n\35\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\37\n");

	Con_Printf ("%s", text);

	Con_Printf ("Press a key.\n");
	Con_Printf("\35\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\37\n");

	key_count = -2;		// wait for a key down and up
	Key_SetDest (key_console);

	do
	{
		t1 = Sys_DoubleTime ();
		SCR_UpdateScreen ();
		Sys_SendKeyEvents ();
		t2 = Sys_DoubleTime ();
		realtime += t2-t1;	// make the cursor blink
	} while (key_count < 0);

	Con_Printf ("\n");
	Key_SetDest (key_game);
	realtime = 0;		// put the cursor back to invisible
}

