/*
	console.c
	in-game console and chat message buffer handling

	$Header: /home/ozzie/Download/0000/uhexen2/hexen2/console.c,v 1.37 2007-09-14 14:10:00 sezero Exp $
*/

#include "quakedef.h"
#include "debuglog.h"


qboolean	con_initialized;

static int	con_linewidth;		// characters across screen
static int	con_vislines;
int		con_notifylines;	// scan lines to clear for notify lines
int		con_totallines;		// total lines in console scrollback
int		con_backscroll;		// lines up from bottom to display
static int	con_current;		// where next message will be printed
static int	con_x;			// offset in current line for next print
static short	*con_text = NULL;
static float	con_cursorspeed = 4;
qboolean 	con_forcedup;		// because no entities to refresh

static	cvar_t	con_notifytime = {"con_notifytime", "3", CVAR_NONE};	//seconds

#define	NUM_CON_TIMES 4
static float	con_times[NUM_CON_TIMES];	// realtime time the line was generated
						// for transparent notify lines

extern	char	key_lines[32][MAXCMDLINE];
extern	int		edit_line;
extern	int		key_linepos;
extern	int		key_insert;
extern qboolean		menu_disabled_mouse;

extern void M_Menu_Main_f (void);


/*
================
Con_ToggleConsole_f
================
*/
void Con_ToggleConsole_f (void)
{
	// activate mouse when in console in
	// case it is disabled somewhere else
	menu_disabled_mouse = false;
	IN_ActivateMouse ();

	if (key_dest == key_console)
	{
		if (cls.state == ca_connected)
		{
			key_dest = key_game;
			key_lines[edit_line][1] = 0;	// clear any typing
			key_linepos = 1;
		}
		else
		{
			M_Menu_Main_f ();
		}
	}
	else
	{
		key_dest = key_console;
	}

	SCR_EndLoadingPlaque ();
	memset (con_times, 0, sizeof(con_times));
}

/*
================
Con_Clear_f
================
*/
static void Con_Clear_f (void)
{
	short i;

	for (i = 0; i < CON_TEXTSIZE; i++)
		con_text[i] = ' ';
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
extern qboolean team_message;

static void Con_MessageMode_f (void)
{
	key_dest = key_message;
	team_message = false;
}


/*
================
Con_MessageMode2_f
================
*/
static void Con_MessageMode2_f (void)
{
	key_dest = key_message;
	team_message = true;
}


/*
================
Con_CheckResize

If the line width has changed, reformat the buffer.
================
*/
void Con_CheckResize (void)
{
	int		i, j, width, oldwidth, oldtotallines, numlines, numchars;
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

		memcpy (tbuf, con_text, CON_TEXTSIZE<<1);
		Con_Clear_f();

		for (i = 0; i < numlines; i++)
		{
			for (j = 0; j < numchars; j++)
			{
				con_text[(con_totallines - 1 - i) * con_linewidth + j] =
						tbuf[((con_current - i + oldtotallines) % oldtotallines) * oldwidth + j];
			}
		}

		Con_ClearNotify ();
	}

	con_backscroll = 0;
	con_current = con_totallines - 1;
}


/*
================
Con_Init
================
*/
void Con_Init (void)
{
	con_text = (short *) Hunk_AllocName (CON_TEXTSIZE<<1, "context");
	Con_Clear_f();
	con_linewidth = -1;
	Con_CheckResize ();

	Con_Printf ("Console initialized.\n");

//
// register our commands
//
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
	int i,j;

	con_x = 0;
	con_current++;

	j = (con_current%con_totallines) * con_linewidth;
	for (i = 0; i < con_linewidth; i++)
		con_text[i+j] = ' ';
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

	con_backscroll = 0;

	if (txt[0] == 1)
	{
		mask = 256;		// go to colored text
		S_LocalSound ("misc/comm.wav");
	// play talk wav
		txt++;
	}
	else if (txt[0] == 2)
	{
		mask = 256;		// go to colored text
		txt++;
	}
	else
		mask = 0;

	while ( (c = *txt) )
	{
	// count word length
		for (l = 0; l < con_linewidth; l++)
			if ( txt[l] <= ' ')
				break;

	// word wrap
		if (l != con_linewidth && (con_x + l > con_linewidth) )
			con_x = 0;

		txt++;

		if (cr)
		{
			con_current--;
			cr = false;
		}

		if (!con_x)
		{
			Con_Linefeed ();
		// mark time for transparent overlay
			if (con_current >= 0)
				con_times[con_current % NUM_CON_TIMES] = realtime;
		}

		switch (c)
		{
		case '\n':
			con_x = 0;
			break;

		case '\r':
			con_x = 0;
			cr = 1;
			break;

		default:	// display character and advance
			y = con_current % con_totallines;
			con_text[y*con_linewidth+con_x] = c | mask;
			con_x++;
			if (con_x >= con_linewidth)
				con_x = 0;
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
			vsnprintf (msg, sizeof(msg), fmt, argptr);
			va_end (argptr);
			LOG_Print (msg);
		}
		return;
	}

	va_start (argptr, fmt);
	vsnprintf (msg, sizeof(msg), fmt, argptr);
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
	unsigned int	i, j, max_len, len, cols, rows;

	// Lay them out in columns
	max_len = 0;
	for (i = 0; i < cnt; ++i)
	{
		len = strlen(list[i]);
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
			len = strlen(s);

			Q_strlcat(line, s, con_linewidth+1);
			if (j < cols - 1)
			{
				while (len < max_len)
				{
					Q_strlcat(line, " ", con_linewidth+1);
					len++;
				}
				Q_strlcat(line, "  ", con_linewidth+1);
			}
		}

		if (strlen(line) != 0)
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
	int		y;
	int		i;
	char	editlinecopy[256], *text;

	if (key_dest != key_console && !con_forcedup)
		return;		// don't draw anything

	Q_strlcpy(editlinecopy, key_lines[edit_line], sizeof(editlinecopy));
	text = editlinecopy;

	y = strlen(text);

// fill out remainder with spaces
	for (i = y; i < 256; i++)
		text[i] = ' ';

// add the cursor frame
	if ((int)(realtime * con_cursorspeed) & 1)	// cursor is visible
		text[key_linepos] = 95 - 84 * key_insert; // underscore for overwrite mode, square for insert

//	prestep if horizontally scrolling
	if (key_linepos >= con_linewidth)
		text += 1 + key_linepos - con_linewidth;

// draw it
	y = con_vislines-16;

	for (i = 0; i < con_linewidth; i++)
		Draw_Character ( (i+1)<<3, con_vislines - 16, text[i]);

// remove cursor
	//key_lines[edit_line][key_linepos] = 0;
}


/*
================
Con_DrawNotify

Draws the last few lines of output transparently over the game top
================
*/
extern char chat_buffer[];
void Con_DrawNotify (void)
{
	int		x, v;
	short	*text;
	int		i;
	float	time;

	v = 0;
	for (i = con_current-NUM_CON_TIMES+1; i <= con_current; i++)
	{
		if (i < 0)
			continue;
		time = con_times[i % NUM_CON_TIMES];
		if (time == 0)
			continue;
		time = realtime - time;
		if (time > con_notifytime.value)
			continue;
		text = con_text + (i % con_totallines)*con_linewidth;

		clearnotify = 0;
		scr_copytop = 1;

		for (x = 0; x < con_linewidth; x++)
			Draw_Character ( (x+1)<<3, v, text[x]);

		v += 8;
	}

	if (key_dest == key_message)
	{
		clearnotify = 0;
		scr_copytop = 1;

		x = 0;

		Draw_String (8, v, "say:");
		while (chat_buffer[x])
		{
			Draw_Character ( (x+5)<<3, v, chat_buffer[x]);
			x++;
		}
		Draw_Character ( (x+5)<<3, v, 10+((int)(realtime*con_cursorspeed)&1));
		v += 8;
	}

	if (v > con_notifylines)
		con_notifylines = v;
}

/*
================
Con_DrawConsole

Draws the console with the solid background
The typing input line at the bottom should only be drawn if typing is allowed
================
*/
void Con_DrawConsole (int lines, qboolean drawinput)
{
	int				i, j, x, y;
	int				rows;
	short			*text;

	if (lines <= 0)
		return;

// draw the background
	Draw_ConsoleBackground (lines);

// draw the text
	con_vislines = lines;

	rows = (lines-16)>>3;		// rows of text to draw
	y = lines - 16 - (rows<<3);	// may start slightly negative

	for (i = con_current - rows + 1; i <= con_current; i++, y += 8)
	{
		j = i - con_backscroll;
		if (j < 0)
			j = 0;
		text = con_text + (j % con_totallines)*con_linewidth;

		for (x = 0; x < con_linewidth; x++)
			Draw_Character ( (x+1)<<3, y, text[x]);
	}

// draw the input prompt, user text, and cursor if desired
	if (drawinput)
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

	Con_Printf (text);

	Con_Printf ("Press a key.\n");
	Con_Printf("\35\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\37\n");

	key_count = -2;		// wait for a key down and up
	key_dest = key_console;

	do
	{
		t1 = Sys_DoubleTime ();
		SCR_UpdateScreen ();
		Sys_SendKeyEvents ();
		t2 = Sys_DoubleTime ();
		realtime += t2-t1;	// make the cursor blink
	} while (key_count < 0);

	Con_Printf ("\n");
	key_dest = key_game;
	realtime = 0;		// put the cursor back to invisible
}

