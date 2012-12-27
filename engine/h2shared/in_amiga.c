/*
 * in_amiga.c -- Amiga Intuition game input code
 * $Id$
 *
 * Copyright (C) 2005-2010 Mark Olsen <bigfoot@private.dk>
 * Copyright (C) 2012 Szilárd Biró <col.lawrence@gmail.com>
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

#include <devices/input.h>
#ifdef __AMIGA__
#include <newmouse.h>
#else
#include <devices/rawkeycodes.h>
#endif
#include <intuition/intuition.h>
#include <proto/intuition.h>
#include <proto/exec.h>
#include <proto/keymap.h>
#ifdef __AROS__
#include <SDI/SDI_interrupt.h>
#elif defined __AMIGA__
#include <SDI_interrupt.h>
#endif
#if defined __MORPHOS__ || defined __AMIGA__
#include <intuition/intuitionbase.h>
#endif

#include "quakedef.h"

extern struct Window *window;
static struct Interrupt InputHandler;
static struct MsgPort *inputport;
static struct IOStdReq *inputreq;
static UWORD *pointermem;

static int in_dograb;
static int mx;
static int my;
#define MAXIMSGS 32
static struct InputEvent imsgs[MAXIMSGS];
static int imsglow;
static int imsghigh;

/* mouse variables */
static cvar_t	m_filter = {"m_filter", "0", CVAR_NONE};

static int	mouse_x, mouse_y, old_mouse_x, old_mouse_y;

static qboolean	mouseactive = false;
static qboolean	mouseinitialized = false;
static qboolean	mouseactivatetoggle = false;
/*static qboolean	mouseshowtoggle = true;*/

static unsigned char keyconv[] =
{
	'`',			/* 0 */
	'1',
	'2',
	'3',
	'4',
	'5',
	'6',
	'7',
	'8',
	'9',
	'0',			/* 10 */
	'-',
	'=',
	0,
	0,
	K_KP_INS,
	'q',
	'w',
	'e',
	'r',
	't',			/* 20 */
	'y',
	'u',
	'i',
	'o',
	'p',
	'[',
	']',
	0,
	K_KP_END,
	K_KP_DOWNARROW,		/* 30 */
	K_KP_PGDN,
	'a',
	's',
	'd',
	'f',
	'g',
	'h',
	'j',
	'k',
	'l',			/* 40 */
	';',
	'\'',
	'\\',
	0,
	K_KP_LEFTARROW,
	K_KP_5,
	K_KP_RIGHTARROW,
	'<',
	'z',
	'x',			/* 50 */
	'c',
	'v',
	'b',
	'n',
	'm',
	',',
	'.',
	'/',
	0,
	K_KP_DEL,			/* 60 */
	K_KP_HOME,
	K_KP_UPARROW,
	K_KP_PGUP,
	' ',
	K_BACKSPACE,
	K_TAB,
	K_KP_ENTER,
	K_ENTER,
	K_ESCAPE,
	K_DEL,			/* 70 */
	K_INS,
	K_PGUP,
	K_PGDN,
	K_KP_MINUS,
	K_F11,
	K_UPARROW,
	K_DOWNARROW,
	K_RIGHTARROW,
	K_LEFTARROW,
	K_F1,			/* 80 */
	K_F2,
	K_F3,
	K_F4,
	K_F5,
	K_F6,
	K_F7,
	K_F8,
	K_F9,
	K_F10,
	0,			/* 90 */
	0,
	K_KP_SLASH,
	0,
	K_KP_PLUS,
	0,
	K_SHIFT,
	K_SHIFT,
	0,
	K_CTRL,
	K_ALT,			/* 100 */
	K_ALT,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	K_PAUSE,		/* 110 */
	K_F12,
	K_HOME,
	K_END,
	0,
	0,
	0,
	0,
	0,
	0,
	0,			/* 120 */
	0,
	K_MWHEELUP,
	K_MWHEELDOWN,
	0,
	0,
	0,
	0,
	0,
	0,
	0,			/* 130 */
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,			/* 140 */
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,			/* 150 */
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,			/* 160 */
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,			/* 170 */
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,			/* 180 */
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,			/* 190 */
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,			/* 200 */
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,			/* 210 */
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,			/* 220 */
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,			/* 230 */
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,			/* 240 */
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,			/* 250 */
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0
};


/*
===========
Force_CenterView_f
===========
*/
static void Force_CenterView_f (void)
{
	cl.viewangles[PITCH] = 0;
}


/*
===========
IN_ShowMouse
===========
*/
void IN_ShowMouse (void)
{
	ClearPointer(window);
	//Con_Printf("IN_ShowMouseOK\n");
}


/*
===========
IN_HideMouse
===========
*/
void IN_HideMouse (void)
{
	if (pointermem)
		SetPointer(window, pointermem, 16, 1, 0, 0);
	//Con_Printf("IN_HideMouseOK\n");
}

/* ============================================================
   NOTES on enabling-disabling the mouse:
   - In windowed mode, mouse is temporarily disabled in main
     menu, so the un-grabbed pointer can be used for desktop
     This state is stored in menu_disabled_mouse as true
   - In fullscreen mode, we don't disable the mouse in menus,
     if we toggle windowed/fullscreen, the above state variable
     is used to correct this in VID_ToggleFullscreen()
   - In the console mode and in the options menu-group, mouse
     is not disabled, and menu_disabled_mouse is set to false
   - Starting a or connecting to a server activates the mouse
     and sets menu_disabled_mouse to false
   - Pausing the game disables (so un-grabs) the mouse, unpausing
     activates it. We don't play with menu_disabled_mouse in
     such cases
*/

/*
===========
IN_ActivateMouse
===========
*/
void IN_ActivateMouse (void)
{
	if (mouseinitialized) {
	    if (!mouseactivatetoggle) {
			if (_enable_mouse.integer /*|| (modestate != MS_WINDOWED)*/)
			{
				IN_HideMouse();
				mouseactivatetoggle = true;
				mouseactive = true;
				in_dograb = 1;
			}
	    }
	}
}

/*
===========
IN_DeactivateMouse
===========
*/
void IN_DeactivateMouse (void)
{
	if (mouseinitialized) {
	    if (mouseactivatetoggle) {
			IN_ShowMouse();
			mouseactivatetoggle = false;
			mouseactive = false;
			in_dograb = 0;
	    }
	}
}

/*
===========
IN_StartupMouse
===========
*/
static void IN_StartupMouse (void)
{
/*	IN_HideMouse ();*/
	if (safemode || COM_CheckParm ("-nomouse"))
	{
		/*in_dograb = 0;*/
		IN_DeactivateMouse();
		return;
	}

	mouseinitialized = true;
	if (_enable_mouse.integer /*|| (modestate != MS_WINDOWED)*/)
	{
		/*mouseactivatetoggle = true;
		mouseactive = true;
		in_dograb = 1;*/
		IN_ActivateMouse();
	}
}


/*
===================
IN_ClearStates
===================
*/
void IN_ClearStates (void)
{
}

/*
===================
IN_KeyboardHandler
===================
*/
#ifdef __MORPHOS__
// MorphOS SDI handler macros are messed up
static struct InputEvent *IN_KeyboardHandlerFunc(void);

static struct EmulLibEntry IN_KeyboardHandler =
{
	TRAP_LIB, 0, (void (*)(void))IN_KeyboardHandlerFunc
};

static struct InputEvent *IN_KeyboardHandlerFunc()
{
	struct InputEvent *moo = (struct InputEvent *)REG_A0;
	struct inputdata *id = (struct inputdata *)REG_A1;
#else
HANDLERPROTO(IN_KeyboardHandler, struct InputEvent *, struct InputEvent *moo, APTR id)
{
#endif
	struct InputEvent *coin;

	ULONG screeninfront;

	if (!(window->Flags & WFLG_WINDOWACTIVE))
		return moo;

	coin = moo;

	if (window->WScreen)
	{
#ifdef __MORPHOS__
		if (IntuitionBase->LibNode.lib_Version > 50 || (IntuitionBase->LibNode.lib_Version == 50 && IntuitionBase->LibNode.lib_Revision >= 56))
			GetAttr(SA_Displayed, window->WScreen, &screeninfront);
		else
#endif
			screeninfront = (window->WScreen == IntuitionBase->FirstScreen);
	}
	else
		screeninfront = 1;

	do
	{
		// mouse buttons, mouse wheel and keyboard
		if ((coin->ie_Class == IECLASS_RAWKEY) || 
			((coin->ie_Class == IECLASS_RAWMOUSE || coin->ie_Class == IECLASS_NEWMOUSE) && coin->ie_Code != IECODE_NOBUTTON))
		{
			if ((imsghigh > imsglow && !(imsghigh == MAXIMSGS - 1 && imsglow == 0)) ||
				(imsghigh < imsglow && imsghigh != imsglow - 1) || 
				(imsglow == imsghigh))
			{
				CopyMem(coin, &imsgs[imsghigh], sizeof(imsgs[0]));
				imsghigh++;
				imsghigh %= MAXIMSGS;
			}

			if ((coin->ie_Class == IECLASS_RAWMOUSE || coin->ie_Class == IECLASS_NEWMOUSE) && in_dograb && screeninfront)
				coin->ie_Code = IECODE_NOBUTTON;
		}

		// mouse movement
		if ((coin->ie_Class == IECLASS_RAWMOUSE) && in_dograb && screeninfront)
		{
			mx += coin->ie_position.ie_xy.ie_x;
			my += coin->ie_position.ie_xy.ie_y;

			coin->ie_position.ie_xy.ie_x = 0;
			coin->ie_position.ie_xy.ie_y = 0;
		}
		
		coin = coin->ie_NextEvent;
	} while (coin);

	return moo;
}
//MakeHandlerPri(InputHandler, &IN_KeyboardHandler, "Hexen II input handler", NULL, 100);

/*
===========
IN_Init
===========
*/
void IN_Init (void)
{
	/* mouse variables */
	Cvar_RegisterVariable (&m_filter);

	Cmd_AddCommand ("force_centerview", Force_CenterView_f);

	pointermem = AllocVec(256, MEMF_CHIP | MEMF_CLEAR);

	IN_StartupMouse ();

	mx = my = imsglow = imsghigh = 0;

	if ((inputport = CreateMsgPort()))
	{
		if ((inputreq = CreateIORequest(inputport, sizeof(*inputreq))))
		{
			if (!OpenDevice("input.device", 0, (struct IORequest *)inputreq, 0))
			{
				InputHandler.is_Node.ln_Type = NT_INTERRUPT;
				InputHandler.is_Node.ln_Pri = 100;
				InputHandler.is_Node.ln_Name = "Hexen II input handler";
				InputHandler.is_Code = (void (*)())&IN_KeyboardHandler;
				inputreq->io_Data = (void *)&InputHandler;
				inputreq->io_Command = IND_ADDHANDLER;
				DoIO((struct IORequest *)inputreq);

				return;
			}
			DeleteIORequest(inputreq);
		}
		DeleteMsgPort(inputport);
	}

	Sys_Error ("Couldn't install input handler");
}

/*
===========
IN_Shutdown
===========
*/
void IN_Shutdown (void)
{
	if (inputreq)
	{
		inputreq->io_Data = (void *)&InputHandler;
		inputreq->io_Command = IND_REMHANDLER;
		DoIO((struct IORequest *)inputreq);

		CloseDevice((struct IORequest *)inputreq);
		DeleteIORequest(inputreq);
	}

	if (inputport)
	{
		DeleteMsgPort(inputport);
	}

	IN_DeactivateMouse ();
	/*IN_ShowMouse ();*/

	if (pointermem)
	{
		FreeVec(pointermem);
	}

	mouseinitialized = false;
}

/*
===========
IN_ReInit
===========
*/
void IN_ReInit (void)
{
	IN_StartupMouse ();
}


/*
===========
IN_MouseMove
===========
*/
static void IN_MouseMove (usercmd_t *cmd/*, int mx, int my*/)
{
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

	mouse_x *= sensitivity.value;
	mouse_y *= sensitivity.value;

/* add mouse X/Y movement to cmd */
	if ( (in_strafe.state & 1) || (lookstrafe.integer && (in_mlook.state & 1) ))
		cmd->sidemove += m_side.value * mouse_x;
	else
		cl.viewangles[YAW] -= m_yaw.value * mouse_x;

	if (in_mlook.state & 1)
	{
		if (mx || my)
			V_StopPitchDrift ();
	}

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

	if (cl.idealroll == 0) /* Did keyboard set it already?? */
	{
		if (cl.v.movetype == MOVETYPE_FLY)
		{
			if (mouse_x < 0)
				cl.idealroll = -10;
			else if (mouse_x > 0)
				cl.idealroll = 10;
		}
	}
}

static void IN_DiscardMove (void)
{
	if (mouseinitialized)
	{
		mx = my = old_mouse_x = old_mouse_y = 0;
	}
}

/*
===========
IN_Move
===========
*/
void IN_Move (usercmd_t *cmd)
{
	if (cl.v.cameramode)
	{
	/* stuck in a different camera so don't move */
		memset (cmd, 0, sizeof(*cmd));
	/* ignore any mouse movements in camera mode */
		IN_DiscardMove ();
		return;
	}

	if (mx != 0 || my != 0)
	{
		IN_MouseMove (cmd/*, x, y*/);
		mx = my = 0;
	}
}


/*
===========
IN_Commands
===========
*/
void IN_Commands (void)
{
	/* all button events handled by IN_SendKeyEvents() */
}


void IN_SendKeyEvents (void)
{
	struct IntuiMessage *intuimsg;
	int sym, state;
	int i;

	while ((intuimsg = (struct IntuiMessage *) GetMsg(window->UserPort)))
	{
		switch (intuimsg->Class)
		{
		case IDCMP_ACTIVEWINDOW:
			S_UnblockSound();
			break;
		case IDCMP_INACTIVEWINDOW:
			S_BlockSound();
			break;
		case IDCMP_CLOSEWINDOW:
			CL_Disconnect ();
			Sys_Quit ();
			break;
		}

		ReplyMsg((struct Message *)intuimsg);
	}

	while (imsglow != imsghigh)
	{
		sym = -1;
		state = -1;
		i = imsglow;
		imsglow++;
		imsglow %= MAXIMSGS;
		
		if (imsgs[i].ie_Class == IECLASS_RAWKEY)
		{
			switch (imsgs[i].ie_Code & ~IECODE_UP_PREFIX)
			{
#ifdef __AROS__
			case RAWKEY_NM_WHEEL_UP:
				sym = K_MWHEELUP;
				break;
			case RAWKEY_NM_WHEEL_DOWN:
				sym = K_MWHEELDOWN;
				break;
#endif
			default:
				state = !(imsgs[i].ie_Code & IECODE_UP_PREFIX);
				if (!Key_IsGameKey())
				{
					UBYTE bufascii;
					if (MapRawKey(&imsgs[i], &bufascii, sizeof(bufascii), NULL) > 0)
					{
						//Con_Printf("%d\n", bufascii);
						sym = (bufascii == 8) ? K_BACKSPACE	: bufascii;
					}
				}

				if (sym == -1)
					sym = keyconv[imsgs[i].ie_Code & ~IECODE_UP_PREFIX];
				//Con_Printf("ie_Code %d sym %d state %d\n", imsgs[i].ie_Code & ~IECODE_UP_PREFIX, sym, state);
			}
		}
		else if (imsgs[i].ie_Class == IECLASS_RAWMOUSE && mouseactive && !in_mode_set)
		{
			state = !(imsgs[i].ie_Code & IECODE_UP_PREFIX);

			switch (imsgs[i].ie_Code & ~IECODE_UP_PREFIX)
			{
			case IECODE_LBUTTON:
				sym = K_MOUSE1;
				break;
			case IECODE_RBUTTON:
				sym = K_MOUSE2;
				break;
			case IECODE_MBUTTON:
				sym = K_MOUSE3;
				break;
			}
		}
#if defined __MORPHOS__ || defined __AMIGA__
		else if (imsgs[i].ie_Class == IECLASS_NEWMOUSE && mouseactive && !in_mode_set)
		{
			switch (imsgs[i].ie_Code & ~IECODE_UP_PREFIX)
			{
			case NM_WHEEL_UP:
				sym = K_MWHEELUP;
				break;
			case NM_WHEEL_DOWN:
				sym = K_MWHEELDOWN;
				break;
			case NM_BUTTON_FOURTH:
				sym = K_MOUSE4;
				state = !(imsgs[i].ie_Code & IECODE_UP_PREFIX);
				break;
			}
		}
#endif
		if (sym != -1)
		{
			if (state != -1)
			{
				Key_Event(sym, state);
			}
			else
			{
				//Con_Printf("wheel!\n");
				/* the mouse wheel doesn't generate a key up event */
				Key_Event(sym, 1);
				Key_Event(sym, 0);
			}
		}
	}
}

