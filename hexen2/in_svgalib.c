/*
	in_svgalib.c:	Linux SVGALIB specific input driver.
	from quake1 source with minor adaptations for uhexen2.

	$Id: in_svgalib.c,v 1.5 2008-04-02 13:32:02 sezero Exp $

	Copyright (C) 1996-1997  Id Software, Inc.

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

	See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to:

		Free Software Foundation, Inc.
		59 Temple Place - Suite 330
		Boston, MA  02111-1307, USA

*/

#include "q_stdinc.h"

#include "vga.h" /* vga_getmousetype() */
#include "vgakeyboard.h"
#include "vgamouse.h"

#include "quakedef.h"


static qboolean	mouse_initialized = false;
static qboolean	kbd_initialized = false;

static unsigned char	scantokey[128];

/*
static int	mouserate = MOUSE_DEFAULTSAMPLERATE;
*/
static char	default_mouse[] = "/dev/mouse";
static int	mouse_buttonstate;
static int	mouse_oldbuttonstate;
static float	mouse_x, mouse_y;
static float	old_mouse_x, old_mouse_y;
static int	mx, my;

cvar_t	m_filter = {"m_filter", "0", CVAR_NONE};


static void Force_CenterView_f (void)
{
	cl.viewangles[PITCH] = 0;
}


static void keyhandler (int scancode, int state)
{
	int	sc;

	sc = scancode & 0x7f;
//	Con_Printf("scancode=%x (%d%s)\n", scancode, sc, (scancode & 0x80) ? "+128" : "");
	Key_Event(scantokey[sc], state == KEY_EVENTPRESS);
}

static void mousehandler (int buttonstate, int dx, int dy, int dz,
					int drx, int dry, int drz)
{
	mouse_buttonstate = buttonstate;
	mx += dx;
	my += dy;
	if (drx > 0)
	{
		Key_Event (K_MWHEELUP, true);
		Key_Event (K_MWHEELUP, false);
	}
	else if (drx < 0)
	{
		Key_Event (K_MWHEELDOWN, true);
		Key_Event (K_MWHEELDOWN, false);
	}
}

static void IN_InitKeyboard (void)
{
	int	i;

	for (i = 0; i < 128; i++)
		scantokey[i] = ' ';

	scantokey[  1] = K_ESCAPE;
	scantokey[  2] = '1';
	scantokey[  3] = '2';
	scantokey[  4] = '3';
	scantokey[  5] = '4';
	scantokey[  6] = '5';
	scantokey[  7] = '6';
	scantokey[  8] = '7';
	scantokey[  9] = '8';
	scantokey[ 10] = '9';
	scantokey[ 11] = '0';
	scantokey[ 12] = '-';
	scantokey[ 13] = '=';
	scantokey[ 14] = K_BACKSPACE;
	scantokey[ 15] = K_TAB;
	scantokey[ 16] = 'q';
	scantokey[ 17] = 'w';
	scantokey[ 18] = 'e';
	scantokey[ 19] = 'r';
	scantokey[ 20] = 't';
	scantokey[ 21] = 'y';
	scantokey[ 22] = 'u';
	scantokey[ 23] = 'i';
	scantokey[ 24] = 'o';
	scantokey[ 25] = 'p';
	scantokey[ 26] = '[';
	scantokey[ 27] = ']';
	scantokey[ 28] = K_ENTER;
	scantokey[ 29] = K_CTRL;	/* left		*/
	scantokey[ 30] = 'a';
	scantokey[ 31] = 's';
	scantokey[ 32] = 'd';
	scantokey[ 33] = 'f';
	scantokey[ 34] = 'g';
	scantokey[ 35] = 'h';
	scantokey[ 36] = 'j';
	scantokey[ 37] = 'k';
	scantokey[ 38] = 'l';
	scantokey[ 39] = ';';
	scantokey[ 40] = '\'';
	scantokey[ 41] = '`';
	scantokey[ 42] = K_SHIFT;	/* left		*/
	scantokey[ 43] = '\\';
	scantokey[ 44] = 'z';
	scantokey[ 45] = 'x';
	scantokey[ 46] = 'c';
	scantokey[ 47] = 'v';
	scantokey[ 48] = 'b';
	scantokey[ 49] = 'n';
	scantokey[ 50] = 'm';
	scantokey[ 51] = ',';
	scantokey[ 52] = '.';
	scantokey[ 53] = '/';
	scantokey[ 54] = K_SHIFT;	/* right	*/
	scantokey[ 55] = '*';		/* keypad	*/
	scantokey[ 56] = K_ALT;		/* left		*/
	scantokey[ 57] = ' ';
	/* 58 caps lock	*/
	scantokey[ 59] = K_F1;
	scantokey[ 60] = K_F2;
	scantokey[ 61] = K_F3;
	scantokey[ 62] = K_F4;
	scantokey[ 63] = K_F5;
	scantokey[ 64] = K_F6;
	scantokey[ 65] = K_F7;
	scantokey[ 66] = K_F8;
	scantokey[ 67] = K_F9;
	scantokey[ 68] = K_F10;
	/* 69 numlock	*/
	/* 70 scrollock	*/
	scantokey[ 71] = K_HOME;
	scantokey[ 72] = K_UPARROW;
	scantokey[ 73] = K_PGUP;
	scantokey[ 74] = '-';
	scantokey[ 75] = K_LEFTARROW;
	scantokey[ 76] = '5';
	scantokey[ 77] = K_RIGHTARROW;
	scantokey[ 79] = K_END;
	scantokey[ 78] = '+';
	scantokey[ 80] = K_DOWNARROW;
	scantokey[ 81] = K_PGDN;
	scantokey[ 82] = K_INS;
	scantokey[ 83] = K_DEL;
	/* 84 to 86 not used */
	scantokey[ 87] = K_F11;
	scantokey[ 88] = K_F12;
	/* 89 to 95 not used */
	scantokey[ 96] = K_ENTER;	/* keypad	*/
	scantokey[ 97] = K_CTRL;	/* right	*/
	scantokey[ 98] = '/';
	scantokey[ 99] = K_F12;		/* print screen, bind to screenshot by default */
	scantokey[100] = K_ALT;		/* right	*/

	scantokey[101] = K_PAUSE;	/* break	*/
	scantokey[102] = K_HOME;
	scantokey[103] = K_UPARROW;
	scantokey[104] = K_PGUP;
	scantokey[105] = K_LEFTARROW;
	scantokey[106] = K_RIGHTARROW;
	scantokey[107] = K_END;
	scantokey[108] = K_DOWNARROW;
	scantokey[109] = K_PGDN;
	scantokey[110] = K_INS;
	scantokey[111] = K_DEL;

	scantokey[119] = K_PAUSE;

	if (keyboard_init())
		Sys_Error("keyboard_init() failed");
	keyboard_seteventhandler(keyhandler);
	kbd_initialized = true;
}

static void IN_InitMouse (void)
{
	int	mtype;
	char	*mousedev;
	int	mouserate;

	mtype = vga_getmousetype();

	mousedev = default_mouse;
	if (getenv("MOUSEDEV"))
		mousedev = getenv("MOUSEDEV");
	if (COM_CheckParm("-mdev"))
		mousedev = com_argv[COM_CheckParm("-mdev")+1];

	mouserate = 1200;
	if (getenv("MOUSERATE"))
		mouserate = atoi(getenv("MOUSERATE"));
	if (COM_CheckParm("-mrate"))
		mouserate = atoi(com_argv[COM_CheckParm("-mrate")+1]);

/* vga_init() opens the mouse automoatically, closing it to ensure it
 * is opened the way we want it (from quakeforge.)
 * FIXME: should I check for a return value >= 0 from mouse_update()
 * before doing this?
 */
	mouse_close();

	if (mouse_init(mousedev, mtype, mouserate))
		Con_Printf("No mouse found\n");
	else
	{
		mouse_seteventhandler((void *) mousehandler);
		mouse_initialized = true;
	}
}

void IN_Init (void)
{
	Cmd_AddCommand ("force_centerview", Force_CenterView_f);
	Cvar_RegisterVariable (&m_filter);

	if (!COM_CheckParm("-nokbd"))
	{
		IN_InitKeyboard();
	}
	if (!(safemode || COM_CheckParm("-nomouse")))
	{
		IN_InitMouse();
	}
}

void IN_Shutdown (void)
{
	if (mouse_initialized)
	{
		mouse_close();
		mouse_initialized = false;
	}
	if (kbd_initialized)
	{
		keyboard_close();
		kbd_initialized = false;
	}
}

/*
===========
IN_SendKeyEvents
===========
*/
void IN_SendKeyEvents (void)
{
	if (kbd_initialized)
	{
		while (keyboard_update())
			;
	}
}

/*
===========
IN_Commands
===========
*/
void IN_Commands (void)
{
	if (mouse_initialized)
	{
		/* poll mouse values */
		while (mouse_update())
			;

		/* perform button actions */
		if ((mouse_buttonstate & MOUSE_LEFTBUTTON) && !(mouse_oldbuttonstate & MOUSE_LEFTBUTTON))
			Key_Event (K_MOUSE1, true);
		else if (!(mouse_buttonstate & MOUSE_LEFTBUTTON) && (mouse_oldbuttonstate & MOUSE_LEFTBUTTON))
			Key_Event (K_MOUSE1, false);

		if ((mouse_buttonstate & MOUSE_RIGHTBUTTON) && !(mouse_oldbuttonstate & MOUSE_RIGHTBUTTON))
			Key_Event (K_MOUSE2, true);
		else if (!(mouse_buttonstate & MOUSE_RIGHTBUTTON) && (mouse_oldbuttonstate & MOUSE_RIGHTBUTTON))
			Key_Event (K_MOUSE2, false);

		if ((mouse_buttonstate & MOUSE_MIDDLEBUTTON) && !(mouse_oldbuttonstate & MOUSE_MIDDLEBUTTON))
			Key_Event (K_MOUSE3, true);
		else if (!(mouse_buttonstate & MOUSE_MIDDLEBUTTON) && (mouse_oldbuttonstate & MOUSE_MIDDLEBUTTON))
			Key_Event (K_MOUSE3, false);

		mouse_oldbuttonstate = mouse_buttonstate;
	}
}

/*
===========
IN_Move
===========
*/
static void IN_MouseMove (usercmd_t *cmd)
{
	if (!mouse_initialized)
		return;

/* poll mouse values */
	while (mouse_update())
		;

	if (m_filter.integer)
	{
		mouse_x = (mx + old_mouse_x) * 0.5;
		mouse_y = (my + old_mouse_y) * 0.5;
	}
	else
	{
		mouse_x = mx;
		mouse_y = my;
	}
	old_mouse_x = mx;
	old_mouse_y = my;
	mx = my = 0;	/* clear for next update */

	mouse_x *= sensitivity.value;
	mouse_y *= sensitivity.value;

/* add mouse X/Y movement to cmd */
	if ( (in_strafe.state & 1) || (lookstrafe.integer && (in_mlook.state & 1) ))
		cmd->sidemove += m_side.value * mouse_x;
	else
		cl.viewangles[YAW] -= m_yaw.value * mouse_x;

	if (in_mlook.state & 1)
		V_StopPitchDrift ();

	if ( (in_mlook.state & 1) && !(in_strafe.state & 1))
	{
		cl.viewangles[PITCH] += m_pitch.value * mouse_y;
		if (cl.viewangles[PITCH] > 80)
			cl.viewangles[PITCH] = 80;
		if (cl.viewangles[PITCH] < -70)
			cl.viewangles[PITCH] = -70;
	}
	else
	{
		if ((in_strafe.state & 1) && (cl.v.movetype == MOVETYPE_NOCLIP))
			cmd->upmove -= m_forward.value * mouse_y;
		else
			cmd->forwardmove -= m_forward.value * mouse_y;
	}
}

void IN_Move (usercmd_t *cmd)
{
	IN_MouseMove(cmd);
}

