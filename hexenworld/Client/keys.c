/*
	keys.c
	key up events are sent even if in console mode

	$Id: keys.c,v 1.38 2007-11-11 13:17:44 sezero Exp $
*/

#include "quakedef.h"
#ifdef PLATFORM_WINDOWS
#include <windows.h>
#endif

char	key_lines[32][MAXCMDLINE];
int		key_linepos;
static qboolean	shift_down = false;
int		key_lastpress;
int		key_insert;	// insert key toggle

int		edit_line = 0;
static int	history_line = 0;

keydest_t	key_dest;

int		key_count;		// incremented every key event

char	*keybindings[256];
static qboolean	consolekeys[256];	// if true, can't be rebound while in console
static qboolean	menubound[256];		// if true, can't be rebound while in menu
static int	keyshift[256];		// key to map to if shift held down in console
static int	key_repeats[256];	// if > 1, it is autorepeating
static qboolean	keyreserved[256];	// hardcoded, can't be rebound by the user
static qboolean	keydown[256];

typedef struct
{
	const char	*name;
	int		keynum;
} keyname_t;

static keyname_t keynames[] =
{
	{"TAB", K_TAB},
	{"ENTER", K_ENTER},
	{"ESCAPE", K_ESCAPE},
	{"SPACE", K_SPACE},
	{"BACKSPACE", K_BACKSPACE},
	{"UPARROW", K_UPARROW},
	{"DOWNARROW", K_DOWNARROW},
	{"LEFTARROW", K_LEFTARROW},
	{"RIGHTARROW", K_RIGHTARROW},

	{"ALT", K_ALT},
	{"CTRL", K_CTRL},
	{"SHIFT", K_SHIFT},
	
	{"F1", K_F1},
	{"F2", K_F2},
	{"F3", K_F3},
	{"F4", K_F4},
	{"F5", K_F5},
	{"F6", K_F6},
	{"F7", K_F7},
	{"F8", K_F8},
	{"F9", K_F9},
	{"F10", K_F10},
	{"F11", K_F11},
	{"F12", K_F12},

	{"INS", K_INS},
	{"DEL", K_DEL},
	{"PGDN", K_PGDN},
	{"PGUP", K_PGUP},
	{"HOME", K_HOME},
	{"END", K_END},

	{"MOUSE1", K_MOUSE1},
	{"MOUSE2", K_MOUSE2},
	{"MOUSE3", K_MOUSE3},
	{"MWHEELUP", K_MWHEELUP},
	{"MWHEELDOWN", K_MWHEELDOWN},
	{"MOUSE4", K_MOUSE4},
	{"MOUSE5", K_MOUSE5},

	{"JOY1", K_JOY1},
	{"JOY2", K_JOY2},
	{"JOY3", K_JOY3},
	{"JOY4", K_JOY4},

	{"AUX1", K_AUX1},
	{"AUX2", K_AUX2},
	{"AUX3", K_AUX3},
	{"AUX4", K_AUX4},
	{"AUX5", K_AUX5},
	{"AUX6", K_AUX6},
	{"AUX7", K_AUX7},
	{"AUX8", K_AUX8},
	{"AUX9", K_AUX9},
	{"AUX10", K_AUX10},
	{"AUX11", K_AUX11},
	{"AUX12", K_AUX12},
	{"AUX13", K_AUX13},
	{"AUX14", K_AUX14},
	{"AUX15", K_AUX15},
	{"AUX16", K_AUX16},
	{"AUX17", K_AUX17},
	{"AUX18", K_AUX18},
	{"AUX19", K_AUX19},
	{"AUX20", K_AUX20},
	{"AUX21", K_AUX21},
	{"AUX22", K_AUX22},
	{"AUX23", K_AUX23},
	{"AUX24", K_AUX24},
	{"AUX25", K_AUX25},
	{"AUX26", K_AUX26},
	{"AUX27", K_AUX27},
	{"AUX28", K_AUX28},
	{"AUX29", K_AUX29},
	{"AUX30", K_AUX30},
	{"AUX31", K_AUX31},
	{"AUX32", K_AUX32},

	{"PAUSE", K_PAUSE},

	{"SEMICOLON", ';'},	// because a raw semicolon seperates commands

	{NULL,		0}
};

/*
==============================================================================

			LINE TYPING INTO THE CONSOLE

==============================================================================
*/

static qboolean CheckForCommand (void)
{
	char	command[128];
	char	*s;
	int		i;

	s = key_lines[edit_line]+1;

	memset (command, 0, sizeof(command));
	for (i = 0; i < 127; i++)
	{
		if (s[i] <= ' ')
			break;
		else
			command[i] = s[i];
	}

	command[i] = 0;

	return Cmd_CheckCommand (command);
}

static void CompleteCommand (void)
{
	const char	*matches[MAX_MATCHES];
	char		*s, stmp[MAXCMDLINE];
	qboolean	editing, partial;
	int	count = 0, i, j;

	if (key_linepos < 2)
		return;

	editing = false;
	partial = true;

	if (strlen(key_lines[edit_line]+1) >= key_linepos)
	{
		editing = true;
		// make a copy of the text starting from the
		// cursor position (see below)
		q_strlcpy(stmp, key_lines[edit_line]+key_linepos, sizeof(stmp));
	}

	s = key_lines[edit_line]+1;
	// complete the text only up to the cursor position:
	// bash style. cut off the rest for now.
	// 2005-12-15: actually no harm in trimming when not
	// in edit mode as well
	//if (editing)
		s[key_linepos-1] = 0;

	// skip the leading whitespace and command markers
	while (*s)
	{
		if (*s != '\\' && *s != '/' && *s > ' ')
			break;
		s++;
	}

	// if the remainder line has no length or has
	// spaces in it, don't bother
	if (!*s || strstr(s," "))
		goto finish;

	// store the length of the relevant partial
	j = strlen(s);

	// start checking for matches, finally...
	count += ListCommands(s, matches, count);
	count += ListCvars(s, matches, count);
	count += ListAlias(s, matches, count);

	if (count)
	{
		// do not do a full auto-complete
		// unless there is only one match
		if (count == 1)
		{
			key_lines[edit_line][1] = '/';
			q_strlcpy (key_lines[edit_line]+2, matches[0], MAXCMDLINE-2);
			key_linepos = strlen(matches[0])+2;
		//	q_strlcpy (key_lines[edit_line]+1, matches[0], MAXCMDLINE-1);
		//	key_linepos = strlen(matches[0])+1;
			key_lines[edit_line][key_linepos] = ' ';
			key_linepos++;
		}
		else
		{
			// more than one match, sort and list all of them
			qsort (matches, count, sizeof(char *), COM_StrCompare);
			Con_Printf("\n");
#if 0
			// plain listing
			for (i = 0; i < count && i < MAX_MATCHES; i++)
				Con_Printf ("%s\n", matches[i]);
			Con_Printf("\n%d matches found\n\n", count);
#else
			// S.A.: columnize the listing.
			Con_Printf("%d possible completions:\n\n", count);
			Con_ShowList (count, (const char**) matches);
			Con_Printf("\n");
#endif

			// cycle throgh all matches and see
			// if there is a partial completion
			while (partial)
			{
				for (i = 1; i < count && i < MAX_MATCHES; i++)
				{
				//	if (memcmp (matches[0], matches[i], j+1))
					if (strncmp(matches[0], matches[i], j+1))
						partial = false;
				}

				if (partial)
					j++;
			}

			if (j > strlen(s))	// found a partial match
			{
				key_lines[edit_line][1] = '/';
				strncpy (key_lines[edit_line]+2, matches[0], j);
				key_linepos = j+2;
			//	strncpy (key_lines[edit_line]+1, matches[0], j);
			//	key_linepos = j+1;
			}
		}

		key_lines[edit_line][key_linepos] = 0;
	}
finish:
	if (editing)
	{
		// put back the remainder of the original text
		// which was lost after the trimming
		q_strlcpy (key_lines[edit_line]+key_linepos, stmp, MAXCMDLINE-key_linepos);
	}
}

/*
====================
Key_Console

Interactive line editing and console scrollback
====================
*/
static void Key_Console (int key)
{
	int		i, history_line_last;
#ifdef PLATFORM_WINDOWS
	HANDLE	th;
	char	*clipText, *textCopied;
#endif
	if (key == K_ENTER)
	{	// backslash text are commands, else chat
		if (key_lines[edit_line][1] == '\\' || key_lines[edit_line][1] == '/')
			Cbuf_AddText (key_lines[edit_line]+2);	// skip the >
		else if (CheckForCommand())
			Cbuf_AddText (key_lines[edit_line]+1);	// valid command
		else
		{	// convert to a chat message
			if (cls.state >= ca_connected)
				Cbuf_AddText ("say ");
			Cbuf_AddText (key_lines[edit_line]+1);	// skip the >
		}

		Cbuf_AddText ("\n");
		Con_Printf ("%s\n",key_lines[edit_line]);
		edit_line = (edit_line + 1) & 31;
		history_line = edit_line;
		key_lines[edit_line][0] = ']';
		key_lines[edit_line][1] = 0;	// null terminate
		key_linepos = 1;
		if (cls.state == ca_disconnected)
			SCR_UpdateScreen ();	// force an update, because the command
								// may take some time
		return;
	}

	if (key == K_TAB)
	{	// command completion
		CompleteCommand ();
		return;
	}

	// left arrow will just move left one w/o earsing, backspace will
	// actually erase charcter
	if (key == K_LEFTARROW)
	{
		if (key_linepos > 1)
			key_linepos--;
		return;
	}

	if (key == K_BACKSPACE)	// delete char before cursor
	{
		if (key_linepos > 1)
		{
			strcpy(key_lines[edit_line] + key_linepos - 1, key_lines[edit_line] + key_linepos);
			key_linepos--;
		}
		return;
	}

	if (key == K_DEL)	// delete char on cursor
	{
		if (key_linepos < strlen(key_lines[edit_line]))
			strcpy(key_lines[edit_line] + key_linepos, key_lines[edit_line] + key_linepos + 1);
		return;
	}

	// if we're at the end, get one character from previous line,
	// otherwise just go right one
	if (key == K_RIGHTARROW)
	{
		if (strlen(key_lines[edit_line]) == key_linepos)
		{
			if (strlen(key_lines[(edit_line + 31) & 31]) <= key_linepos)
				return;	// no character to get
			key_lines[edit_line][key_linepos] = key_lines[(edit_line + 31) & 31][key_linepos];
			key_linepos++;
			key_lines[edit_line][key_linepos] = 0;
		}
		else
			key_linepos++;
		return;
	}

	if (key == K_INS)
	{	// toggle insert mode
		key_insert ^= 1;
		return;
	}

	if (key == K_UPARROW)
	{
		history_line_last = history_line;
		do
		{
			history_line = (history_line - 1) & 31;
		} while (history_line != edit_line && !key_lines[history_line][1]);

		if (history_line == edit_line)
			history_line = history_line_last;

		strcpy(key_lines[edit_line], key_lines[history_line]);
		key_linepos = strlen(key_lines[edit_line]);
		return;
	}

	if (key == K_DOWNARROW)
	{
		if (history_line == edit_line)
			return;

		do
		{
			history_line = (history_line + 1) & 31;
		}
		while (history_line != edit_line && !key_lines[history_line][1]);

		if (history_line == edit_line)
		{
			key_lines[edit_line][0] = ']';
			key_lines[edit_line][1] = 0;
			key_linepos = 1;
		}
		else
		{
			strcpy(key_lines[edit_line], key_lines[history_line]);
			key_linepos = strlen(key_lines[edit_line]);
		}
		return;
	}

	if (key == K_PGUP || key == K_MWHEELUP)
	{
		con->display -= 2;
		return;
	}

	if (key == K_PGDN || key == K_MWHEELDOWN)
	{
		con->display += 2;
		if (con->display > con->current)
			con->display = con->current;
		return;
	}

	if (key == K_HOME)
	{
		if (keydown[K_CTRL])
			con->display = con->current - con_totallines + 10;
		else
			key_linepos = 1;

		return;
	}

	if (key == K_END)
	{
		if (keydown[K_CTRL])
			con->display = con->current;
		else
			key_linepos = strlen(key_lines[edit_line]);

		return;
	}

#ifdef PLATFORM_WINDOWS
	if ((key == 'V' || key == 'v') && GetKeyState(VK_CONTROL) < 0)
	{
		if (OpenClipboard(NULL))
		{
			th = GetClipboardData(CF_TEXT);
			if (th)
			{
				clipText = (char *) GlobalLock(th);
				if (clipText)
				{
					textCopied = (char *) Z_Malloc(GlobalSize(th)+1, Z_MAINZONE);
					strcpy(textCopied, clipText);
					/* Substitute a NULL for every token */
					strtok(textCopied, "\n\r\b");
					i = strlen(textCopied);
					if (i + key_linepos >= MAXCMDLINE)
						i = MAXCMDLINE-key_linepos;
					if (i > 0)
					{
						textCopied[i] = 0;
						strcat(key_lines[edit_line], textCopied);
						key_linepos += i;
					}
					Z_Free(textCopied);
				}
				GlobalUnlock(th);
			}
			CloseClipboard();
			return;
		}
	}
#endif

	if (key < 32 || key > 127)
		return;	// non printable

	if (key_linepos < MAXCMDLINE-1)
	{
		// check insert mode
		if (key_insert)
		{	// can't do strcpy to move string to right
			i = strlen(key_lines[edit_line]) - 1;
			if (i == 254)
				i--;
			for ( ; i >= key_linepos; i--)
				key_lines[edit_line][i + 1] = key_lines[edit_line][i];
		}
		// only null terminate if at the end
		i = key_lines[edit_line][key_linepos];
		key_lines[edit_line][key_linepos] = key;
		key_linepos++;
		if (!i)
			key_lines[edit_line][key_linepos] = 0;
	}
}

//============================================================================

qboolean	chat_team;
char		chat_buffer[MAXCMDLINE];
int			chat_bufferlen = 0;

static void Key_Message (int key)
{
	if (key == K_ENTER)
	{
		if (chat_team)
			Cbuf_AddText ("say_team \"");
		else
			Cbuf_AddText ("say \"");
		Cbuf_AddText(chat_buffer);
		Cbuf_AddText("\"\n");

		key_dest = key_game;
		chat_bufferlen = 0;
		chat_buffer[0] = 0;
		return;
	}

	if (key == K_ESCAPE)
	{
		key_dest = key_game;
		chat_bufferlen = 0;
		chat_buffer[0] = 0;
		return;
	}

	if (key < 32 || key > 127)
		return;	// non printable

	if (key == K_BACKSPACE)
	{
		if (chat_bufferlen)
			chat_buffer[--chat_bufferlen] = 0;
		return;
	}

	if (chat_bufferlen == sizeof(chat_buffer) - 1)
		return; // all full

	chat_buffer[chat_bufferlen++] = key;
	chat_buffer[chat_bufferlen] = 0;
}

//============================================================================


/*
===================
Key_StringToKeynum

Returns a key number to be used to index keybindings[] by looking at
the given string.  Single ascii characters return themselves, while
the K_* names are matched up.
===================
*/
static int Key_StringToKeynum (const char *str)
{
	keyname_t	*kn;

	if (!str || !str[0])
		return -1;
	if (!str[1])
		return str[0];

	for (kn = keynames; kn->name; kn++)
	{
		if (!q_strcasecmp(str,kn->name))
			return kn->keynum;
	}
	return -1;
}

/*
===================
Key_KeynumToString

Returns a string (either a single ascii char, or a K_* name) for the
given keynum.
FIXME: handle quote special (general escape sequence?)
===================
*/
const char *Key_KeynumToString (int keynum)
{
	keyname_t	*kn;
	static	char	tinystr[2];

	if (keynum == -1)
		return "<KEY NOT FOUND>";
	if (keynum > 32 && keynum < 127)
	{	// printable ascii
		tinystr[0] = keynum;
		tinystr[1] = 0;
		return tinystr;
	}

	for (kn = keynames; kn->name; kn++)
	{
		if (keynum == kn->keynum)
			return kn->name;
	}

	return "<UNKNOWN KEYNUM>";
}


/*
===================
Key_SetBinding
===================
*/
void Key_SetBinding (int keynum, const char *binding)
{
	if (keynum == -1)
		return;
	if (keyreserved[keynum])
		return;

// free old bindings
	if (keybindings[keynum])
	{
		Z_Free (keybindings[keynum]);
		keybindings[keynum] = NULL;
	}

// allocate memory for new binding
	if (binding)
	{
		keybindings[keynum] = (char *) Z_Malloc(strlen(binding) + 1, Z_MAINZONE);
		strcpy(keybindings[keynum], binding);
	}
}

/*
===================
Key_Unbind_f
===================
*/
static void Key_Unbind_f (void)
{
	int		b;

	if (Cmd_Argc() != 2)
	{
		Con_Printf ("unbind <key> : remove commands from a key\n");
		return;
	}

	b = Key_StringToKeynum (Cmd_Argv(1));
	if (b == -1)
	{
		Con_Printf ("\"%s\" isn't a valid key\n", Cmd_Argv(1));
		return;
	}

	Key_SetBinding (b, NULL);
}

static void Key_Unbindall_f (void)
{
	int		i;

	for (i = 0; i < 256; i++)
		Key_SetBinding(i, NULL);
}


/*
===================
Key_Bind_f
===================
*/
static void Key_Bind_f (void)
{
	int			i, c, b;
	char		cmd[1024];

	c = Cmd_Argc();

	if (c != 2 && c != 3)
	{
		Con_Printf ("bind <key> [command] : attach a command to a key\n");
		return;
	}
	b = Key_StringToKeynum (Cmd_Argv(1));
	if (b == -1)
	{
		Con_Printf ("\"%s\" isn't a valid key\n", Cmd_Argv(1));
		return;
	}

	if (c == 2)
	{
		if (keybindings[b])
			Con_Printf ("\"%s\" = \"%s\"\n", Cmd_Argv(1), keybindings[b] );
		else
			Con_Printf ("\"%s\" is not bound\n", Cmd_Argv(1) );
		return;
	}

// copy the rest of the command line
	cmd[0] = 0;		// start out with a null string
	for (i = 2; i < c; i++)
	{
		q_strlcat (cmd, Cmd_Argv(i), sizeof(cmd));
		if (i != (c-1))
			q_strlcat (cmd, " ", sizeof(cmd));
	}

	Key_SetBinding (b, cmd);
}

/*
============
Key_WriteBindings

Writes lines containing "bind key value"
============
*/
void Key_WriteBindings (FILE *f)
{
	int		i;

	for (i = 0; i < 256; i++)
	{
		if (keybindings[i] && *keybindings[i])
			fprintf (f, "bind %s \"%s\"\n", Key_KeynumToString(i), keybindings[i]);
	}
}


/*
===================
Key_Init
===================
*/
void Key_Init (void)
{
	int		i;

	for (i = 0; i < 32; i++)
	{
		key_lines[i][0] = ']';
		key_lines[i][1] = 0;
	}
	key_linepos = 1;

	memset (consolekeys, 0, sizeof(consolekeys));
	memset (menubound, 0, sizeof(menubound));
	memset (keyreserved, 0, sizeof(keyreserved));

//
// init ascii characters in console mode
//
	for (i = 32; i < 128; i++)
		consolekeys[i] = true;
	consolekeys[K_ENTER] = true;
	consolekeys[K_TAB] = true;
	consolekeys[K_LEFTARROW] = true;
	consolekeys[K_RIGHTARROW] = true;
	consolekeys[K_UPARROW] = true;
	consolekeys[K_DOWNARROW] = true;
	consolekeys[K_BACKSPACE] = true;
	consolekeys[K_DEL] = true;
	consolekeys[K_INS] = true;
	consolekeys[K_HOME] = true;
	consolekeys[K_END] = true;
	consolekeys[K_PGUP] = true;
	consolekeys[K_PGDN] = true;
	consolekeys[K_SHIFT] = true;
	consolekeys[K_MWHEELUP] = true;
	consolekeys[K_MWHEELDOWN] = true;
	consolekeys['`'] = false;
	consolekeys['~'] = false;

	for (i = 0; i < 256; i++)
		keyshift[i] = i;
	for (i = 'a'; i <= 'z'; i++)
		keyshift[i] = i - 'a' + 'A';
	keyshift['1'] = '!';
	keyshift['2'] = '@';
	keyshift['3'] = '#';
	keyshift['4'] = '$';
	keyshift['5'] = '%';
	keyshift['6'] = '^';
	keyshift['7'] = '&';
	keyshift['8'] = '*';
	keyshift['9'] = '(';
	keyshift['0'] = ')';
	keyshift['-'] = '_';
	keyshift['='] = '+';
	keyshift[','] = '<';
	keyshift['.'] = '>';
	keyshift['/'] = '?';
	keyshift[';'] = ':';
	keyshift['\''] = '"';
	keyshift['['] = '{';
	keyshift[']'] = '}';
	keyshift['`'] = '~';
	keyshift['\\'] = '|';

	menubound[K_ESCAPE] = true;
	for (i = 0; i < 12; i++)
		menubound[K_F1+i] = true;

	memset (key_repeats, 0, sizeof(key_repeats));

//
// bind our reserved keys
//
	Key_SetBinding ('`', "toggleconsole");
	Key_SetBinding ('~', "toggleconsole");
	Key_SetBinding (K_PAUSE, "pause");
	keyreserved['`'] = true;
	keyreserved['~'] = true;
	keyreserved[K_PAUSE] = true;

//
// register our functions
//
	Cmd_AddCommand ("bind",Key_Bind_f);
	Cmd_AddCommand ("unbind",Key_Unbind_f);
	Cmd_AddCommand ("unbindall",Key_Unbindall_f);
}

/*
===================
Key_Event

Called by the system between frames for both key up and key down events
Should NOT be called during an interrupt!
===================
*/
extern int			m_state;
#define	m_none	0		// enumerated type from menu.c
void Key_Event (int key, qboolean down)
{
	char	*kb;
	char	cmd[1024];

	keydown[key] = down;

	if (!down)
		key_repeats[key] = 0;

	key_lastpress = key;
	key_count++;
	if (key_count <= 0)
	{
		return;		// just catching keys for Con_NotifyBox
	}

// update auto-repeat status
	if (down)
	{
		/* Pause key doesn't generate a scancode when released,
		 * never increment its auto-repeat status.
		 */
		if (key != K_PAUSE)
			key_repeats[key]++;

		/*
		if (key != K_BACKSPACE 
			&& key != K_PGUP 
			&& key != K_PGDN
			&& key_repeats[key] > 1)
		{
			return;	// ignore most autorepeats
		}
		*/

		if (key_repeats[key] > 1)
		{
			// ignore autorepeats unless chatting or in console
			if (key_dest == key_console)
				goto autorep0;
			if (key_dest == key_message)
				goto autorep0;
			// hack to allow autorepeat in forcedup console:
			if (cls.state != ca_active && m_state == m_none)
				goto autorep0;
			return;
		}

		if (key >= 200 && !keybindings[key])
			Con_Printf ("%s is unbound, hit F4 to set.\n", Key_KeynumToString (key) );
	}

autorep0:

	if (key == K_SHIFT)
		shift_down = down;

//
// handle escape specialy, so the user can never unbind it
//
	if (key == K_ESCAPE)
	{
		if (!down)
			return;
		switch (key_dest)
		{
		case key_message:
			Key_Message (key);
			break;
		case key_menu:
			M_Keydown (key);
			break;
		case key_game:
		case key_console:
			M_ToggleMenu_f ();
			break;
		default:
			Sys_Error ("Bad key_dest");
		}
		return;
	}

//
// key up events only generate commands if the game key binding is
// a button command (leading + sign).  These will occur even in console mode,
// to keep the character from continuing an action started before a console
// switch.  Button commands include the kenum as a parameter, so multiple
// downs can be matched with ups
//
	if (!down)
	{
		kb = keybindings[key];
		if (kb && kb[0] == '+')
		{
			sprintf (cmd, "-%s %i\n", kb+1, key);
			Cbuf_AddText (cmd);
		}
		if (keyshift[key] != key)
		{
			kb = keybindings[keyshift[key]];
			if (kb && kb[0] == '+')
			{
				sprintf (cmd, "-%s %i\n", kb+1, key);
				Cbuf_AddText (cmd);
			}
		}
		return;
	}

//
// during demo playback, most keys bring up the main menu
//
	if (cls.demoplayback && down && consolekeys[key] && key_dest == key_game)
	{
		M_ToggleMenu_f ();
		return;
	}

//
// if not a consolekey, send to the interpreter no matter what mode is
//
	if ( (key_dest == key_menu && menubound[key])
		|| (key_dest == key_console && !consolekeys[key])
		|| (key_dest == key_game && ( cls.state == ca_active || !consolekeys[key] )) )
	{
		kb = keybindings[key];
		if (kb)
		{
			if (kb[0] == '+')
			{	// button commands add keynum as a parm
				sprintf (cmd, "%s %i\n", kb, key);
				Cbuf_AddText (cmd);
			}
			else
			{
				Cbuf_AddText (kb);
				Cbuf_AddText ("\n");
			}
		}
		return;
	}

	if (!down)
		return;		// other systems only care about key down events

	if (shift_down)
	{
		key = keyshift[key];
	}

	switch (key_dest)
	{
	case key_message:
		Key_Message (key);
		break;
	case key_menu:
		M_Keydown (key);
		break;

	case key_game:
	case key_console:
		Key_Console (key);
		break;
	default:
		Sys_Error ("Bad key_dest");
	}
}


/*
===================
Key_ClearStates
===================
*/
void Key_ClearStates (void)
{
	int		i;

	for (i = 0; i < 256; i++)
	{
		keydown[i] = false;
		key_repeats[i] = 0;
	}
}

