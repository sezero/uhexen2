/*
	in_sdl.c
	SDL game input code

	$Id$

	Copyright (C) 2001  contributors of the Anvil of Thyrion project
	Copyright (C) 2005-2011  Steven Atkinson, O.Sezer

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
		51 Franklin St, Fifth Floor,
		Boston, MA  02110-1301  USA
*/

#include "sdl_inc.h"
#include "quakedef.h"


static keydest_t	prev_key_dest;

/* mouse variables */
static cvar_t	m_filter = {"m_filter", "0", CVAR_NONE};

static int	mouse_x, mouse_y, old_mouse_x, old_mouse_y;

static qboolean	mouseactive = false;
static qboolean	mouseinitialized = false;
static qboolean	mouseactivatetoggle = false;
static qboolean	mouseshowtoggle = true;

static int buttonremap[] =
{
	K_MOUSE1,
	K_MOUSE3,	/* right button		*/
	K_MOUSE2,	/* middle button	*/
	K_MWHEELUP,
	K_MWHEELDOWN,
	K_MOUSE4,
	K_MOUSE5
};

#define	USE_JOYSTICK	0 /* disabled until we finish joystick support */
#if USE_JOYSTICK
static	cvar_t	in_joystick = {"joystick", "1", CVAR_ARCHIVE};

#define	MAX_JOYSTICKS	8
static	SDL_Joystick	*joy_id[MAX_JOYSTICKS];
static	int		joy_available;

/* forward-referenced functions */
static void IN_StartupJoystick (void);
static void IN_JoyMove (usercmd_t *cmd);
#endif	/* USE_JOYSTICK */

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
/* no need to check mouseinitialized here */
	if (!mouseshowtoggle)
	{
		SDL_ShowCursor(1);
		mouseshowtoggle = true;
	}
}


/*
===========
IN_HideMouse
===========
*/
void IN_HideMouse (void)
{
/* no need to check mouseinitialized here */
	if (mouseshowtoggle)
	{
		SDL_ShowCursor (0);
		mouseshowtoggle = false;
	}
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
	if (!mouseinitialized)
		return;

	if (!mouseactivatetoggle)
#if 0	/* change to 1 if dont want to disable mouse in fullscreen */
		if ((modestate != MS_WINDOWED) || _enable_mouse.integer)
#else
		if (_enable_mouse.integer)
#endif
		{
			mouseactivatetoggle = true;
			mouseactive = true;
			SDL_WM_GrabInput (SDL_GRAB_ON);
		}

	/* nuke events from when mouse was disabled: */
	SDL_PumpEvents ();
	SDL_GetRelativeMouseState (NULL, NULL);
}

/*
===========
IN_DeactivateMouse
===========
*/
void IN_DeactivateMouse (void)
{
	if (!mouseinitialized)
		return;
	if (mouseactivatetoggle)
	{
		mouseactivatetoggle = false;
		mouseactive = false;
		SDL_WM_GrabInput (SDL_GRAB_OFF);
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
		SDL_WM_GrabInput (SDL_GRAB_OFF);
		return;
	}

	mouseinitialized = true;

	/*if (mouseactivatetoggle)*/
#if 0	/* change to 1 if dont want to disable mouse in fullscreen */
	if ((modestate != MS_WINDOWED) || _enable_mouse.integer)
#else
	if (_enable_mouse.integer)
#endif
		IN_ActivateMouse ();
}


/*
===========
IN_Init
===========
*/
void IN_Init (void)
{
	/* mouse variables */
	Cvar_RegisterVariable (&m_filter);
#if USE_JOYSTICK
	/* joystick variables */
	Cvar_RegisterVariable (&in_joystick);
#endif	/* USE_JOYSTICK */

	Cmd_AddCommand ("force_centerview", Force_CenterView_f);

	IN_StartupMouse ();
#if USE_JOYSTICK
	IN_StartupJoystick ();
#endif	/* USE_JOYSTICK */

	SDL_EnableUNICODE (0); /* frame updates will change this as key_dest changes */
	SDL_EnableKeyRepeat (SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL*2);
}

/*
===========
IN_Shutdown
===========
*/
void IN_Shutdown (void)
{

	IN_DeactivateMouse ();
	IN_ShowMouse ();
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

	SDL_EnableUNICODE (0); /* frame updates will change this as key_dest changes */
	prev_key_dest = key_game;
	SDL_EnableKeyRepeat (SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL*2);
}


/*
===========
IN_MouseMove
===========
*/
static void IN_MouseMove (usercmd_t *cmd)
{
	int		mx, my;

	SDL_GetRelativeMouseState(&mx,&my);

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


/*
===========
IN_Move
===========
*/
void IN_Move (usercmd_t *cmd)
{
	if (cl.v.cameramode)	/* Stuck in a different camera so don't move */
	{
		memset (cmd, 0, sizeof(*cmd));
		return;
	}

	if (mouseactive)
		IN_MouseMove (cmd);

#if USE_JOYSTICK
	if (SDL_GetAppState() & SDL_APPACTIVE)
		IN_JoyMove (cmd);
#endif	/* USE_JOYSTICK */
}


#if USE_JOYSTICK
/*
===============
IN_StartupJoystick
===============
*/
static void IN_StartupJoystick (void)
{
	int		i, num_joys;
	SDL_Joystick	*j;

	memset (joy_id, 0, sizeof(joy_id));
	joy_available = 0;
	if (safemode || COM_CheckParm ("-nojoy"))
		return;

	if (SDL_InitSubSystem(SDL_INIT_JOYSTICK) < 0)
	{
		Con_Printf("Couldn't init SDL joystick: %s\n", SDL_GetError());
		return;
	}

	num_joys = SDL_NumJoysticks();
	if (num_joys < 1)
	{
		Con_Printf ("No joystick devices found\n");
		return;
	}
	Con_Printf ("SDL_Joystick: %d devices are reported\n", num_joys);

	for (i = 0; i < num_joys && joy_available < MAX_JOYSTICKS; i++)
	{
		j = SDL_JoystickOpen(i);
		if (j == NULL)
		{
			Con_Printf("joystick #%d: open failed: %s\n", i, SDL_GetError());
		}
		else
		{
			joy_id[joy_available++] = j;
			Con_Printf("joystick #%d: opened \"%s\" with %d axes, %d buttons, %d balls, %d hats\n",
					i, SDL_JoystickName(i), SDL_JoystickNumAxes(j), SDL_JoystickNumButtons(j),
					SDL_JoystickNumBalls(j), SDL_JoystickNumHats(j));
		}
	}

	if (!joy_available)
	{
		Con_Printf ("Unable to open any of the joystick devices\n");
	}
}


/*
===============
IN_ReadJoystick
===============
*/
static qboolean IN_ReadJoystick (void)
{
	return false;	/* to be coded... */
}

/*
===========
IN_JoyMove
===========
*/
static void IN_JoyMove (usercmd_t *cmd)
{
	if (!joy_available || !in_joystick.integer)
		return;

	if (IN_ReadJoystick () != true)
	{
		return;
	}

	/*
	if (in_speed.state & 1)
		speed = cl_movespeedkey.value;
	else
		speed = 1;
	aspeed = speed * host_frametime;
	*/

	/* loop through the axes */
		/* to be coded.. */

	/* bounds check pitch */
		/* to be coded.. */
}
#endif	/* USE_JOYSTICK */

/*
===========
IN_Commands
===========
*/
void IN_Commands (void)
{
#if USE_JOYSTICK
	if (!joy_available)
		return;

	if (cls.state != ca_connected || cls.signon != SIGNONS)
	{
		IN_ReadJoystick ();
	}

	/* loop through the joystick buttons */
		/* to be coded.. */

	/* key a joystick event or auxillary event for
	   higher number buttons for each state change */
		/* to be coded.. */
#endif	/* USE_JOYSTICK */
}


void IN_SendKeyEvents (void)
{
	SDL_Event event;
	int sym, state;
	int modstate;

	if (key_dest != prev_key_dest)
	{
		SDL_EnableUNICODE((key_dest == key_console || key_dest == key_message));
		Key_ClearStates();
		prev_key_dest = key_dest;
	}

	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case SDL_ACTIVEEVENT:
			if (event.active.state & (SDL_APPINPUTFOCUS|SDL_APPACTIVE))
			{
				if (event.active.gain)
				{
				/*	Sys_Printf("FOCUS GAIN\n");*/
					S_UnblockSound();
				}
				else
				{
				/*	Sys_Printf("FOCUS LOSS\n");*/
					S_BlockSound();
				}
			}
			break;

		case SDL_KEYDOWN:
			if ((event.key.keysym.sym == SDLK_RETURN) &&
			    (event.key.keysym.mod & KMOD_ALT))
			{
				VID_ToggleFullscreen();
				break;
			}
			else if ((event.key.keysym.sym == SDLK_ESCAPE) &&
				 (event.key.keysym.mod & KMOD_SHIFT))
			{
				Con_ToggleConsole_f();
				break;
			}
			else if ((event.key.keysym.sym == SDLK_g) &&
				 (event.key.keysym.mod & KMOD_CTRL))
			{
				SDL_WM_GrabInput( (SDL_WM_GrabInput (SDL_GRAB_QUERY) == SDL_GRAB_ON) ? SDL_GRAB_OFF : SDL_GRAB_ON );
				break;
			}

		case SDL_KEYUP:
			sym = event.key.keysym.sym;
			state = event.key.state;
			modstate = SDL_GetModState();

			switch (key_dest)
			{
			case key_game:
				if ((event.key.keysym.unicode != 0) || (modstate & KMOD_SHIFT))
				{	/* only use unicode for ~ and ` in game mode */
					if ((event.key.keysym.unicode & 0xFF80) == 0)
					{
						if (((event.key.keysym.unicode & 0x7F) == '`') ||
						    ((event.key.keysym.unicode & 0x7F) == '~') )
							sym = event.key.keysym.unicode & 0x7F;
					}
				}
				break;
			case key_message:
			case key_console:
				if ((event.key.keysym.unicode != 0) || (modstate & KMOD_SHIFT))
				{
#if defined(__QNX__)
					if ((sym == SDLK_BACKSPACE) || (sym == SDLK_RETURN))
						break;	/* S.A: fixes QNX weirdness */
#endif	/* __QNX__ */
					if ((event.key.keysym.unicode & 0xFF80) == 0)
						sym = event.key.keysym.unicode & 0x7F;
					/* else: it's an international character */
				}
			/*	printf("You pressed %s (%d) (%c)\n", SDL_GetKeyName(sym), sym, sym);*/
				break;
			default:
				break;
			}

			switch (sym)
			{
			case SDLK_DELETE:
				sym = K_DEL;
				break;
			case SDLK_BACKSPACE:
				sym = K_BACKSPACE;
				break;
			case SDLK_F1:
				sym = K_F1;
				break;
			case SDLK_F2:
				sym = K_F2;
				break;
			case SDLK_F3:
				sym = K_F3;
				break;
			case SDLK_F4:
				sym = K_F4;
				break;
			case SDLK_F5:
				sym = K_F5;
				break;
			case SDLK_F6:
				sym = K_F6;
				break;
			case SDLK_F7:
				sym = K_F7;
				break;
			case SDLK_F8:
				sym = K_F8;
				break;
			case SDLK_F9:
				sym = K_F9;
				break;
			case SDLK_F10:
				sym = K_F10;
				break;
			case SDLK_F11:
				sym = K_F11;
				break;
			case SDLK_F12:
				sym = K_F12;
				break;
			case SDLK_BREAK:
			case SDLK_PAUSE:
				sym = K_PAUSE;
				break;
			case SDLK_UP:
				sym = K_UPARROW;
				break;
			case SDLK_DOWN:
				sym = K_DOWNARROW;
				break;
			case SDLK_RIGHT:
				sym = K_RIGHTARROW;
				break;
			case SDLK_LEFT:
				sym = K_LEFTARROW;
				break;
			case SDLK_INSERT:
				sym = K_INS;
				break;
			case SDLK_HOME:
				sym = K_HOME;
				break;
			case SDLK_END:
				sym = K_END;
				break;
			case SDLK_PAGEUP:
				sym = K_PGUP;
				break;
			case SDLK_PAGEDOWN:
				sym = K_PGDN;
				break;
			case SDLK_RSHIFT:
			case SDLK_LSHIFT:
				sym = K_SHIFT;
				break;
			case SDLK_RCTRL:
			case SDLK_LCTRL:
				sym = K_CTRL;
				break;
			case SDLK_RALT:
			case SDLK_LALT:
				sym = K_ALT;
				break;
			case SDLK_KP0:
				if (modstate & KMOD_NUM)
					sym = K_INS;
				else
					sym = SDLK_0;
				break;
			case SDLK_KP1:
				if (modstate & KMOD_NUM)
					sym = K_END;
				else
					sym = SDLK_1;
				break;
			case SDLK_KP2:
				if (modstate & KMOD_NUM)
					sym = K_DOWNARROW;
				else
					sym = SDLK_2;
				break;
			case SDLK_KP3:
				if (modstate & KMOD_NUM)
					sym = K_PGDN;
				else
					sym = SDLK_3;
				break;
			case SDLK_KP4:
				if (modstate & KMOD_NUM)
					sym = K_LEFTARROW;
				else
					sym = SDLK_4;
				break;
			case SDLK_KP5:
				sym = SDLK_5;
				break;
			case SDLK_KP6:
				if (modstate & KMOD_NUM)
					sym = K_RIGHTARROW;
				else
					sym = SDLK_6;
				break;
			case SDLK_KP7:
				if (modstate & KMOD_NUM)
					sym = K_HOME;
				else
					sym = SDLK_7;
				break;
			case SDLK_KP8:
				if (modstate & KMOD_NUM)
					sym = K_UPARROW;
				else
					sym = SDLK_8;
				break;
			case SDLK_KP9:
				if (modstate & KMOD_NUM)
					sym = K_PGUP;
				else
					sym = SDLK_9;
				break;
			case SDLK_KP_PERIOD:
				if (modstate & KMOD_NUM)
					sym = K_DEL;
				else
					sym = SDLK_PERIOD;
				break;
			case SDLK_KP_DIVIDE:
				sym = SDLK_SLASH;
				break;
			case SDLK_KP_MULTIPLY:
				sym = SDLK_ASTERISK;
				break;
			case SDLK_KP_MINUS:
				sym = SDLK_MINUS;
				break;
			case SDLK_KP_PLUS:
				sym = SDLK_PLUS;
				break;
			case SDLK_KP_ENTER:
				sym = SDLK_RETURN;
				break;
			case SDLK_KP_EQUALS:
				sym = SDLK_EQUALS;
				break;
			case 178: /* the '²' key */
				sym = '~';
				break;
			}
			/* If we're not directly handled and still
			 * above 255, just force it to 0 */
			if (sym > 255)
				sym = 0;
			Key_Event (sym, state);
			break;

		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
			if (!mouseactive || in_mode_set)
				break;
			if (event.button.button < 1 ||
			    event.button.button > sizeof(buttonremap) / sizeof(buttonremap[0]))
			{
				Con_Printf ("Ignored event for mouse button %d\n",
							event.button.button);
				break;
			}
			Key_Event(buttonremap[event.button.button - 1], event.button.state == SDL_PRESSED);
			break;

		case SDL_MOUSEMOTION:
		/*	SDL_GetMouseState (NULL, NULL);*/
			break;

#if USE_JOYSTICK
		case SDL_JOYBUTTONDOWN:
		case SDL_JOYBUTTONUP:
			if (!in_joystick.value)
				break;
			if (event.jbutton.button > K_AUX32 - K_JOY1)
			{
				Con_Printf ("Ignored event for joystick button %d\n",
							event.jbutton.button);
				break;
			}
			Key_Event(K_JOY1 + event.jbutton.button, event.jbutton.state == SDL_PRESSED);
			break;

		case SDL_JOYAXISMOTION:
		case SDL_JOYHATMOTION:
		case SDL_JOYBALLMOTION:
		/* to be coded.. */
			break;
#endif	/* USE_JOYSTICK */

		case SDL_QUIT:
			CL_Disconnect ();
			Sys_Quit ();
			break;

		default:
			break;
		}
	}
}

