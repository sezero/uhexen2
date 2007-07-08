/*
	console.c
	in-game console and chat message buffer handling

	$Header: /home/ozzie/Download/0000/uhexen2/hexenworld/Client/console.c,v 1.28 2007-07-08 11:55:34 sezero Exp $
*/

#include "quakedef.h"
#include "debuglog.h"


console_t	con_main;
console_t	con_chat;
console_t	*con;			// point to either con_main or con_chat

qboolean	con_initialized;

static int	con_linewidth;		// characters across screen
static int	con_vislines;
int		con_notifylines;	// scan lines to clear for notify lines
int		con_totallines;		// total lines in console scrollback
static float	con_cursorspeed = 4;
int		con_ormask;

static	cvar_t	con_notifytime = {"con_notifytime", "3", CVAR_NONE};	//seconds

#define	NUM_CON_TIMES 4
static float	con_times[NUM_CON_TIMES];	// realtime time the line was generated
						// for transparent notify lines

extern	char	key_lines[32][MAXCMDLINE];
extern	int		edit_line;
extern	int		key_linepos;
extern	int		key_insert;
extern qboolean		mousestate_sa;


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
	// activate mouse when in console in
	// case it is disabled somewhere else
	mousestate_sa = false;
	IN_ActivateMouse ();

	Key_ClearTyping ();

	if (key_dest == key_console)
	{
		if (cls.state == ca_active)
			key_dest = key_game;
	}
	else
	{
		key_dest = key_console;
	}
	Con_ClearNotify ();
}

/*
================
Con_ToggleChat_f
================
*/
static void Con_ToggleChat_f (void)
{
	Key_ClearTyping ();

	if (key_dest == key_console)
	{
		if (cls.state == ca_active)
			key_dest = key_game;
	}
	else
		key_dest = key_console;

	Con_ClearNotify ();
}

/*
================
Con_Clear_f
================
*/
static void Con_Clear_f (void)
{
	memset (con_main.text, ' ', CON_TEXTSIZE);
	memset (con_main.text_attr, 0, CON_TEXTSIZE);
	memset (con_chat.text, ' ', CON_TEXTSIZE);
	memset (con_chat.text_attr, 0, CON_TEXTSIZE);
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
	chat_team = false;
	key_dest = key_message;
}

/*
================
Con_MessageMode2_f
================
*/
static void Con_MessageMode2_f (void)
{
	chat_team = true;
	key_dest = key_message;
}

/*
================
Con_Resize

================
*/
static void Con_Resize (console_t *cons)
{
	int		i, j, width, oldwidth, oldtotallines, numlines, numchars;
	char	tbuf[CON_TEXTSIZE], tbuf_attr[CON_TEXTSIZE];

	width = (vid.width >> 3) - 2;

	if (width == con_linewidth)
		return;

	if (width < 1)			// video hasn't been initialized yet
	{
		width = 38;
		con_linewidth = width;
		con_totallines = CON_TEXTSIZE / con_linewidth;
		memset (cons->text, ' ', CON_TEXTSIZE);
		memset (cons->text_attr, 0, CON_TEXTSIZE);
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

		memcpy (tbuf, cons->text, CON_TEXTSIZE);
		memcpy (tbuf_attr, cons->text_attr, CON_TEXTSIZE);
		memset (cons->text, ' ', CON_TEXTSIZE);
		memset (cons->text_attr, 0, CON_TEXTSIZE);

		for (i = 0; i < numlines; i++)
		{
			for (j = 0; j < numchars; j++)
			{
				cons->text[(con_totallines - 1 - i) * con_linewidth + j] =
						tbuf[((cons->current - i + oldtotallines) % oldtotallines) * oldwidth + j];
				cons->text_attr[(con_totallines - 1 - i) * con_linewidth + j] =
						tbuf_attr[((cons->current - i + oldtotallines) % oldtotallines) * oldwidth + j];
			}
		}

		Con_ClearNotify ();
	}

	cons->current = con_totallines - 1;
	cons->display = cons->current;
}


/*
================
Con_CheckResize

If the line width has changed, reformat the buffer.
================
*/
void Con_CheckResize (void)
{
	Con_Resize (&con_main);
	Con_Resize (&con_chat);
}


/*
================
Con_Init
================
*/
void Con_Init (void)
{
	con = &con_main;
	con_linewidth = -1;
	Con_CheckResize ();

	Con_Printf ("Console initialized.\n");

//
// register our commands
//
	Cvar_RegisterVariable (&con_notifytime);

	Cmd_AddCommand ("toggleconsole", Con_ToggleConsole_f);
	Cmd_AddCommand ("togglechat", Con_ToggleChat_f);
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
	con->x = 0;
	if (con->display == con->current)
		con->display++;
	con->current++;
	memset (&con->text[(con->current%con_totallines)*con_linewidth], ' ', con_linewidth);
	memset (&con->text_attr[(con->current%con_totallines)*con_linewidth], 0, con_linewidth);
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

	if (txt[0] == 1 || txt[0] == 2)
	{
		mask = 128;		// go to colored text
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
		if (l != con_linewidth && (con->x + l > con_linewidth) )
			con->x = 0;

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
			con->text[y*con_linewidth+con->x] = c;
			con->text_attr[y*con_linewidth+con->x] = mask | con_ormask;
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
	int			temp = 0;
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

	if (flags & _PRINT_SAFE)
	{
		temp = scr_disabled_for_loading;
		scr_disabled_for_loading = true;
	}

// write it to the scrollable buffer
	Con_Print (msg);

// update the screen immediately if the console is displayed
	if (cls.state != ca_active)
	{
	// protect against infinite loop if something in SCR_UpdateScreen calls
	// Con_Printd
		if (!inupdate)
		{
			inupdate = true;
			SCR_UpdateScreen ();
			inupdate = false;
		}
	}

	if (flags & _PRINT_SAFE)
	{
		scr_disabled_for_loading = temp;
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
	unsigned	i, j, max_len, len, cols, rows;

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

	if (key_dest != key_console && cls.state == ca_active)
		return;		// don't draw anything (always draw if not active)

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
	y = con_vislines-22;

	for (i = 0; i < con_linewidth; i++)
		Draw_Character ( (i+1)<<3, con_vislines - 22, text[i]);

// remove cursor
	//key_lines[edit_line][key_linepos] = 0;
}


/*
================
Con_DrawNotify

Draws the last few lines of output transparently over the game top
================
*/
void Con_DrawNotify (void)
{
	int		x, v;
	char	*text;
	byte	*text_attr;
	int		i;
	float	time;
	char	*s;
	int		skip;

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
		text_attr = con->text_attr + (i % con_totallines)*con_linewidth;

		clearnotify = 0;
		scr_copytop = 1;

		for (x = 0; x < con_linewidth; x++)
			Draw_Character ( (x+1)<<3, v, text[x] + 256*text_attr[x]);

		v += 8;
	}

	if (key_dest == key_message)
	{
		clearnotify = 0;
		scr_copytop = 1;

		if (chat_team)
		{
			Draw_String (8, v, "say_team:");
			skip = 11;
		}
		else
		{
			Draw_String (8, v, "say:");
			skip = 5;
		}

		s = chat_buffer;
		if (chat_bufferlen > (vid.width>>3)-(skip+1))
			s += chat_bufferlen - ((vid.width>>3)-(skip+1));
		x = 0;
		while (s[x])
		{
			Draw_Character ( (x+skip)<<3, v, s[x]);
			x++;
		}
		Draw_Character ( (x+skip)<<3, v, 10+((int)(realtime*con_cursorspeed)&1));
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
	int				i, j, x, y, n;
	int				rows;
	char			*text;
	byte			*text_attr;
	int				row;
	char			dlbar[1024];

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
		text_attr = con->text_attr + (row % con_totallines)*con_linewidth;

		for (x = 0; x < con_linewidth; x++)
			Draw_Character ( (x+1)<<3, y, text[x] + 256*text_attr[x]);
	}

	// draw the download bar
	// figure out width
	if (cls.download)
	{
		if ((text = strrchr(cls.downloadname, '/')) != NULL)
			text++;
		else
			text = cls.downloadname;

		x = con_linewidth - ((con_linewidth * 7) / 40);
		y = x - strlen(text) - 8;
		i = con_linewidth/3;
		if (strlen(text) > i)
		{
			y = x - i - 11;
			strncpy(dlbar, text, i);
			dlbar[i] = 0;
			strcat(dlbar, "...");
		}
		else
		{
			strcpy(dlbar, text);
		}
		strcat(dlbar, ": ");
		i = strlen(dlbar);
		dlbar[i++] = '\x80';
		// where's the dot go?
		if (cls.downloadpercent == 0)
			n = 0;
		else
			n = y * cls.downloadpercent / 100;

		for (j = 0; j < y; j++)
			if (j == n)
				dlbar[i++] = '\x83';
			else
				dlbar[i++] = '\x81';

		dlbar[i++] = '\x82';
		dlbar[i] = 0;

		sprintf(dlbar + strlen(dlbar), " %02d%%", cls.downloadpercent);

		// draw it
		y = con_vislines-22 + 8;
		for (i = 0; i < strlen(dlbar); i++)
			Draw_Character ( (i+1)<<3, y, dlbar[i]);
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

