/*
 * in_sdl.c -- SDL game input code
 * $Id$
 *
 * Copyright (C) 2001  contributors of the Anvil of Thyrion project
 * Joystick code taken from the darkplaces project with modifications
 * to make it work in uHexen2: Copyright (C) 2006-2011 Forest Hale
 * Copyright (C) 2005-2012  Steven Atkinson, O.Sezer, Sander van Dijk
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

#include "sdl_inc.h"
#include "quakedef.h"


static qboolean	prev_gamekey;

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

static int FilterMouseEvents(const SDL_Event *event)
{
	switch (event->type)
	{
	case SDL_MOUSEMOTION:
	case SDL_MOUSEBUTTONDOWN:
	case SDL_MOUSEBUTTONUP:
		return 0;
	}

	return 1;
}

/* joystick support: */
static	SDL_Joystick	*joy_id = NULL;
static	int		joy_available;

static	cvar_t	in_joystick = {"joystick", "0", CVAR_ARCHIVE};		/* enable/disable joystick */

static	cvar_t	joy_index = {"joy_index", "0", CVAR_NONE};		/* joystick to use when have multiple */
static	cvar_t	joy_axisforward = {"joy_axisforward", "1", CVAR_NONE};	/* axis for forward/backward movement */
static	cvar_t	joy_axisside = {"joy_axisside", "0", CVAR_NONE};	/* axis for right/left movement */
static	cvar_t	joy_axisup = {"joy_axisup", "-1", CVAR_NONE};		/* axis for up/down movement */
static	cvar_t	joy_axispitch = {"joy_axispitch", "3", CVAR_NONE};	/* axis for looking up/down" */
static	cvar_t	joy_axisyaw = {"joy_axisyaw", "2", CVAR_NONE};		/* axis for looking right/left */
/* joy_axisroll & co. (for tilting head right/left??): Nope. */
static	cvar_t	joy_deadzoneforward = {"joy_deadzoneforward", "0", CVAR_NONE};	/* deadzone tolerance, suggested 0 to 0.01 */
static	cvar_t	joy_deadzoneside = {"joy_deadzoneside", "0", CVAR_NONE};	/* deadzone tolerance */
static	cvar_t	joy_deadzoneup = {"joy_deadzoneup", "0", CVAR_NONE};		/* deadzone tolerance */
static	cvar_t	joy_deadzonepitch = {"joy_deadzonepitch", "0", CVAR_NONE};	/* deadzone tolerance */
static	cvar_t	joy_deadzoneyaw = {"joy_deadzoneyaw", "0", CVAR_NONE};		/* deadzone tolerance */
static	cvar_t	joy_sensitivityforward = {"joy_sensitivityforward", "-1", CVAR_NONE};	/* movement multiplier */
static	cvar_t	joy_sensitivityside = {"joy_sensitivityside", "1", CVAR_NONE};		/* movement multiplier */
static	cvar_t	joy_sensitivityup = {"joy_sensitivityup", "1", CVAR_NONE};		/* movement multiplier */
static	cvar_t	joy_sensitivitypitch = {"joy_sensitivitypitch", "1", CVAR_NONE};	/* movement multiplier */
static	cvar_t	joy_sensitivityyaw = {"joy_sensitivityyaw", "-1", CVAR_NONE};		/* movement multiplier */

/* hack to generate uparrow/leftarrow etc. key events
 * for axes if the stick driver isn't generating them. */
/* might be useful for menu navigation etc. */
#define JOY_KEYEVENT_FOR_AXES 0 /* not for now */
#if (JOY_KEYEVENT_FOR_AXES)
static	cvar_t	joy_axiskeyevents = {"joy_axiskeyevents", "0", CVAR_ARCHIVE};
static	cvar_t	joy_axiskeyevents_deadzone = {"joy_axiskeyevents_deadzone", "0.5", CVAR_ARCHIVE};

/* joystick axes state */
#define	MAX_JOYSTICK_AXES	16
typedef struct
{
	float oldmove;
	float move;
	double keytime;
} joy_axiscache_t;
static joy_axiscache_t joy_axescache[MAX_JOYSTICK_AXES];
#endif /* JOY_KEYEVENT_FOR_AXES */

/* HACKITY HACKITY HACK: since joystick trackballs convert to mousemove,
 * make IN_ActivateMouse()/IN_DeactivateMouse() to affect the trackball: */
static qboolean	trackballactive = false;

/* translate hat events into keypresses. the 4
 * highest buttons are used for the first hat */
static int hat_keys[16] = {
	K_AUX29, K_AUX30, K_AUX31, K_AUX32,
	K_AUX25, K_AUX26, K_AUX27, K_AUX28,
	K_AUX21, K_AUX22, K_AUX23, K_AUX24,
	K_AUX17, K_AUX18, K_AUX19, K_AUX20
};
static Uint32	oldhats = 0;	/* old hat state */

/* forward-referenced functions */
static void IN_StartupJoystick (void);
static void IN_JoyMove (usercmd_t *cmd);
static void IN_JoyTrackballMove (int *ballx, int *bally); /* adds to x/y args */
static void IN_JoyHatMove (void);
static float IN_JoystickGetAxis (int axis, float sensitivity, float deadzone);
static void IN_Callback_JoyEnable (cvar_t *var);
static void IN_Callback_JoyIndex (cvar_t *var);


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
	if (mouseinitialized) {
	    if (!mouseactivatetoggle) {
		if (_enable_mouse.integer /*|| (modestate != MS_WINDOWED)*/)
		{
			mouseactivatetoggle = true;
			mouseactive = true;
			SDL_WM_GrabInput (SDL_GRAB_ON);
			SDL_SetEventFilter(NULL);
		}
	    }
	}

	trackballactive = true; /* HACK... */

	if (mouseinitialized)
		SDL_GetRelativeMouseState (NULL, NULL);
	IN_JoyTrackballMove (NULL, NULL);
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
		mouseactivatetoggle = false;
		mouseactive = false;
		SDL_WM_GrabInput (SDL_GRAB_OFF);
		SDL_SetEventFilter(FilterMouseEvents);
	    }
	}

	trackballactive = false; /* HACK... */
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
		SDL_SetEventFilter(FilterMouseEvents);
		return;
	}

	mouseinitialized = true;
	if (_enable_mouse.integer /*|| (modestate != MS_WINDOWED)*/)
	{
		mouseactivatetoggle = true;
		mouseactive = true;
		SDL_WM_GrabInput (SDL_GRAB_ON);
		SDL_SetEventFilter(NULL);
		SDL_GetRelativeMouseState (NULL, NULL);
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
===========
IN_Init
===========
*/
void IN_Init (void)
{
	/* mouse variables */
	Cvar_RegisterVariable (&m_filter);
	/* joystick variables */
	Cvar_RegisterVariable (&in_joystick);
	Cvar_RegisterVariable (&joy_index);
	Cvar_RegisterVariable (&joy_axisforward);
	Cvar_RegisterVariable (&joy_axisside);
	Cvar_RegisterVariable (&joy_axisup);
	Cvar_RegisterVariable (&joy_axispitch);
	Cvar_RegisterVariable (&joy_axisyaw);
	Cvar_RegisterVariable (&joy_deadzoneforward);
	Cvar_RegisterVariable (&joy_deadzoneside);
	Cvar_RegisterVariable (&joy_deadzoneup);
	Cvar_RegisterVariable (&joy_deadzonepitch);
	Cvar_RegisterVariable (&joy_deadzoneyaw);
	Cvar_RegisterVariable (&joy_sensitivityforward);
	Cvar_RegisterVariable (&joy_sensitivityside);
	Cvar_RegisterVariable (&joy_sensitivityup);
	Cvar_RegisterVariable (&joy_sensitivitypitch);
	Cvar_RegisterVariable (&joy_sensitivityyaw);
#if (JOY_KEYEVENT_FOR_AXES)
	Cvar_RegisterVariable (&joy_axiskeyevents);
	Cvar_RegisterVariable (&joy_axiskeyevents_deadzone);
#endif

	Cvar_SetCallback (&in_joystick, IN_Callback_JoyEnable);
	Cvar_SetCallback (&joy_index, IN_Callback_JoyIndex);

	Cmd_AddCommand ("force_centerview", Force_CenterView_f);

	IN_StartupMouse ();
	IN_StartupJoystick ();

	prev_gamekey = Key_IsGameKey();
	SDL_EnableUNICODE (!prev_gamekey);
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

	if (joy_id)
		SDL_JoystickClose(joy_id);
	joy_id = NULL;
	joy_available = 0;
	if (SDL_WasInit(SDL_INIT_JOYSTICK))
		SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
}

/*
===========
IN_ReInit
===========
*/
void IN_ReInit (void)
{
	IN_StartupMouse ();

	prev_gamekey = Key_IsGameKey();
	SDL_EnableUNICODE (!prev_gamekey);
	SDL_EnableKeyRepeat (SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL*2);

	/* no need for joystick to reinit */
}


/*
===========
IN_MouseMove
===========
*/
static void IN_MouseMove (usercmd_t *cmd, int mx, int my)
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
		old_mouse_x = old_mouse_y = 0;
		SDL_GetRelativeMouseState (NULL, NULL);
	}
	IN_JoyTrackballMove (NULL, NULL);

	/* JOY AXES ??? */
}

/*
===========
IN_Move
===========
*/
void IN_Move (usercmd_t *cmd)
{
	int	x, y;
	qboolean app_active;

	if (cl.v.cameramode)
	{
	/* stuck in a different camera so don't move */
		memset (cmd, 0, sizeof(*cmd));
	/* ignore any mouse movements in camera mode */
		IN_DiscardMove ();
		return;
	}

	app_active = ((SDL_GetAppState() & SDL_APPACTIVE) != 0);
	x = 0;
	y = 0;

	if (mouseactive)
		SDL_GetRelativeMouseState(&x, &y);
	if (app_active && trackballactive)
		IN_JoyTrackballMove (&x, &y);
	if (x != 0 || y != 0)
		IN_MouseMove (cmd, x, y);

	if (app_active)
		IN_JoyMove (cmd);
}


/*
===============
IN_StartupJoystick
===============
*/
static void IN_StartupJoystick (void)
{
	int	i;

	if (safemode || COM_CheckParm ("-nojoy"))
		return;

	if (SDL_InitSubSystem(SDL_INIT_JOYSTICK) < 0)
	{
		Con_Printf("Couldn't init SDL joystick: %s\n", SDL_GetError());
		return;
	}

	joy_available = SDL_NumJoysticks();
	if (joy_available == 0)
	{
		SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
		Con_Printf ("No joystick devices found\n");
		return;
	}

	Con_Printf ("SDL_Joystick: %d devices are reported:\n", joy_available);
	for (i = 0; i < joy_available; i++)
	{
		Con_Printf("#%d: \"%s\"\n", i, SDL_JoystickName(i));
	}

	trackballactive = true;
	if (in_joystick.integer)
		IN_Callback_JoyIndex(&joy_index);
}

static void IN_Callback_JoyEnable (cvar_t *var)
{
	if (var->integer)
		IN_Callback_JoyIndex(&joy_index);
	else
	{
		if (joy_id)
		{
			SDL_JoystickClose(joy_id);
			joy_id = NULL;
		}
	}
}

static void IN_Callback_JoyIndex (cvar_t *var)
{
	int idx = var->integer;

#if (JOY_KEYEVENT_FOR_AXES)
	memset (joy_axescache, 0, MAX_JOYSTICK_AXES * sizeof(joy_axiscache_t));
#endif
	oldhats = 0;
	if (joy_id)
	{
		SDL_JoystickClose(joy_id);
		joy_id = NULL;
	}
	if (idx < 0 || idx >= joy_available)
	{
		Con_Printf ("joystick #%d doesn't exist\n", idx);
	}
	else if (in_joystick.integer)
	{
		joy_id = SDL_JoystickOpen(idx);
		if (joy_id == NULL)
		{
			Con_Printf("joystick #%d open failed: %s\n", idx, SDL_GetError());
		}
		else
		{
			int numaxes, numbtns, numballs, numhats;
			Con_Printf("joystick open ");
			Con_Printf("#%d: \"%s\"\n", idx, SDL_JoystickName(idx));
			numaxes = SDL_JoystickNumAxes(joy_id);
			numbtns = SDL_JoystickNumButtons(joy_id);
			numballs= SDL_JoystickNumBalls(joy_id);
			numhats = SDL_JoystickNumHats(joy_id);
			Con_Printf(" %d axes, %d buttons, %d balls, %d hats\n",
					numaxes, numbtns, numballs, numhats);
			IN_JoyTrackballMove (NULL, NULL);
			if (numhats > 4) numhats = 4;
			for (idx = 0; idx < numhats; ++idx)
				((Uint8 *)&oldhats)[idx] = SDL_JoystickGetHat(joy_id, idx);
			for (idx = 0; idx < numaxes; ++idx)
				SDL_JoystickGetAxis(joy_id, idx);
		}
	}
}

static void IN_JoyTrackballMove (int *ballx, int *bally)
{
	int	i, numballs;
	int	x, y;

	if (!joy_id)
		return;

	numballs = SDL_JoystickNumBalls(joy_id);
	for (i = 0; i < numballs; i++)
	{
		SDL_JoystickGetBall(joy_id, i, &x, &y);
		if (ballx) *ballx += x;
		if (bally) *bally += y;
	}
}

static void IN_JoyHatMove (void)
{
	int	i, k, numhats;
	Uint32	hats;

	if (!joy_id)	return;
	numhats = SDL_JoystickNumHats(joy_id);
	if (!numhats)	return;
	if (numhats > 4) numhats = 4;
	hats = 0;
	for (i = 0; i < numhats; ++i)
		((Uint8 *)&hats)[i] = SDL_JoystickGetHat(joy_id, i);
	if (hats == oldhats)
		return;

	for (i = 0; i < numhats; i++ )
	{
		if (((Uint8 *)&hats)[i] == ((Uint8 *)&oldhats)[i])
			continue;
		k = 4 * i;
		/* release event */
		switch (((Uint8 *)&oldhats)[i])
		{
		case SDL_HAT_UP:
			Key_Event(hat_keys[k + 0], false);
			break;
		case SDL_HAT_RIGHT:
			Key_Event(hat_keys[k + 1], false);
			break;
		case SDL_HAT_DOWN:
			Key_Event(hat_keys[k + 2], false);
			break;
		case SDL_HAT_LEFT:
			Key_Event(hat_keys[k + 3], false);
			break;
		case SDL_HAT_RIGHTUP:
			Key_Event(hat_keys[k + 0], false);
			Key_Event(hat_keys[k + 1], false);
			break;
		case SDL_HAT_RIGHTDOWN:
			Key_Event(hat_keys[k + 2], false);
			Key_Event(hat_keys[k + 1], false);
			break;
		case SDL_HAT_LEFTUP:
			Key_Event(hat_keys[k + 0], false);
			Key_Event(hat_keys[k + 3], false);
			break;
		case SDL_HAT_LEFTDOWN:
			Key_Event(hat_keys[k + 2], false);
			Key_Event(hat_keys[k + 3], false);
			break;
		default:
			break;
		}
		/* press event */
		switch (((Uint8 *)&hats)[i])
		{
		case SDL_HAT_UP:
			Key_Event(hat_keys[k + 0], true);
			break;
		case SDL_HAT_RIGHT:
			Key_Event(hat_keys[k + 1], true);
			break;
		case SDL_HAT_DOWN:
			Key_Event(hat_keys[k + 2], true);
			break;
		case SDL_HAT_LEFT:
			Key_Event(hat_keys[k + 3], true);
			break;
		case SDL_HAT_RIGHTUP:
			Key_Event(hat_keys[k + 0], true);
			Key_Event(hat_keys[k + 1], true);
			break;
		case SDL_HAT_RIGHTDOWN:
			Key_Event(hat_keys[k + 2], true);
			Key_Event(hat_keys[k + 1], true);
			break;
		case SDL_HAT_LEFTUP:
			Key_Event(hat_keys[k + 0], true);
			Key_Event(hat_keys[k + 3], true);
			break;
		case SDL_HAT_LEFTDOWN:
			Key_Event(hat_keys[k + 2], true);
			Key_Event(hat_keys[k + 3], true);
			break;
		default:
			break;
		}
	}

	oldhats = hats;	/* save hat state */
}

static float IN_JoystickGetAxis (int axis, float sensitivity, float deadzone)
{
	float value;
	if (axis < 0 || axis >= SDL_JoystickNumAxes(joy_id))
		return 0; /* no such axis on this joystick */
	value = SDL_JoystickGetAxis(joy_id, axis) * (1.0f / 32767.0f);
	if (value < -1)		value = -1;
	else if (value > 1)	value =  1;
	if (fabs(value) < deadzone)
		return 0; /* within deadzone around center */
	return value * sensitivity;
}

#if !(JOY_KEYEVENT_FOR_AXES)
# define	IN_JoystickBlockDoubledKeyEvents(S)		(false)
#else
/* Joystick axis key events:  a sort of hack emulating Arrow keys for
 * joystick axes as some drives dont send such key events for them.
 * additionally, we should block drivers that do send arrow key events
 * to prevent double events.
 */
static inline void DEBUG_KeyeventForAxis (float move, int key_pos, int key_neg)
{
/*	Con_Printf("joy %s %f\n", Key_KeynumToString((move > 0) ? key_pos : key_neg), cl.time);*/
}

static void IN_JoystickKeyeventForAxis (int axis, int key_pos, int key_neg)
{
	double joytime;
	if (axis < 0 || axis >= SDL_JoystickNumAxes(joy_id))
		return; /* no such axis on this joystick */
	joytime = Sys_DoubleTime();
	/* no key event, continuous keydown event */
	if (joy_axescache[axis].move == joy_axescache[axis].oldmove)
	{
		if (joy_axescache[axis].move != 0 && joytime > joy_axescache[axis].keytime)
		{
			DEBUG_KeyeventForAxis (joy_axescache[axis].move, key_pos, key_neg);
			Key_Event((joy_axescache[axis].move > 0) ? key_pos : key_neg, true);
			joy_axescache[axis].keytime = joytime + 0.5 / 20;
		}
		return;
	}
	/* generate key up event */
	if (joy_axescache[axis].oldmove)
	{
		DEBUG_KeyeventForAxis (joy_axescache[axis].oldmove, key_pos, key_neg);
		Key_Event((joy_axescache[axis].oldmove > 0) ? key_pos : key_neg, false);
	}
	/* generate key down event */
	if (joy_axescache[axis].move)
	{
		DEBUG_KeyeventForAxis (joy_axescache[axis].move, key_pos, key_neg);
		Key_Event((joy_axescache[axis].move > 0) ? key_pos : key_neg, true);
		joy_axescache[axis].keytime = joytime + 0.5;
	}
}

static qboolean IN_JoystickBlockDoubledKeyEvents (int keycode)
{
	if (!joy_id || !joy_axiskeyevents.integer)
		return false;
	/* block keyevent if it's going to be provided by joystick keyevent system */
	if (keycode == K_UPARROW || keycode == K_DOWNARROW)
		if (IN_JoystickGetAxis(joy_axisforward.integer, 1, joy_axiskeyevents_deadzone.value) || joy_axescache[joy_axisforward.integer].move || joy_axescache[joy_axisforward.integer].oldmove)
			return true;
	if (keycode == K_RIGHTARROW || keycode == K_LEFTARROW)
		if (IN_JoystickGetAxis(joy_axisside.integer, 1, joy_axiskeyevents_deadzone.value) || joy_axescache[joy_axisside.integer].move || joy_axescache[joy_axisside.integer].oldmove)
			return true;
	return false;
}
#endif /* JOY_KEYEVENT_FOR_AXES */

/*
===========
IN_JoyMove
===========
*/
static void IN_JoyMove (usercmd_t *cmd)
{
	float	speed, aspeed;
	float	value;
#if (JOY_KEYEVENT_FOR_AXES)
	int	i, numaxes;
#endif

	if (!joy_id)
		return;

	if (in_speed.state & 1)
		speed = cl_movespeedkey.value;
	else	speed = 1;
	aspeed = speed * host_frametime;

	/* axes */
	value  = IN_JoystickGetAxis(joy_axisforward.integer, joy_sensitivityforward.value, joy_deadzoneforward.value);
	value *= speed * 200;	/*cl_forwardspeed.value*/
	cmd->forwardmove += value;

	value  = IN_JoystickGetAxis(joy_axisside.integer, joy_sensitivityside.value, joy_deadzoneside.value);
	value *= speed * 225;	/*cl_sidespeed.value*/
	cmd->sidemove += value;

	value  = IN_JoystickGetAxis(joy_axisup.integer, joy_sensitivityup.value, joy_deadzoneup.value);
	value *= speed * 200;	/*cl_upspeed.value*/
	cmd->upmove += value;

	value  = IN_JoystickGetAxis(joy_axispitch.integer, joy_sensitivitypitch.value, joy_deadzonepitch.value);
	value *= aspeed * cl_pitchspeed.value;
	cl.viewangles[PITCH] += value;
	if (value) V_StopPitchDrift ();

	value  = IN_JoystickGetAxis(joy_axisyaw.integer, joy_sensitivityyaw.value, joy_deadzoneyaw.value);
	value *= aspeed * cl_yawspeed.value;
	cl.viewangles[YAW] += value;

	/* bounds check pitch */
	if (cl.viewangles[PITCH] > 80.0)
		cl.viewangles[PITCH] = 80.0;
	if (cl.viewangles[PITCH] < -70.0)
		cl.viewangles[PITCH] = -70.0;

	/* hats (pov): */
	IN_JoyHatMove ();

#if (JOY_KEYEVENT_FOR_AXES)
	/* cache state of axes to emulate button events for them */
	numaxes = SDL_JoystickNumAxes(joy_id);
	if (numaxes > MAX_JOYSTICK_AXES)
		numaxes = MAX_JOYSTICK_AXES;
	for (i = 0; i < numaxes; i++)
	{
		joy_axescache[i].oldmove = joy_axescache[i].move;
		joy_axescache[i].move = IN_JoystickGetAxis(i, 1, joy_axiskeyevents_deadzone.value);
	}
	/* run keyevents */
	if (joy_axiskeyevents.integer)
	{
		IN_JoystickKeyeventForAxis(joy_axisforward.integer, K_DOWNARROW, K_UPARROW);
		IN_JoystickKeyeventForAxis(joy_axisside.integer, K_RIGHTARROW, K_LEFTARROW);
	}
#endif /* JOY_KEYEVENT_FOR_AXES */
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
	SDL_Event event;
	int sym, state, modstate;
	qboolean gamekey;

	if ((gamekey = Key_IsGameKey()) != prev_gamekey)
	{
		prev_gamekey = gamekey;
		SDL_EnableUNICODE(!gamekey);
	}

	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case SDL_ACTIVEEVENT:
			if (event.active.state & (SDL_APPINPUTFOCUS|SDL_APPACTIVE))
			{
				if (event.active.gain)
					S_UnblockSound();
				else	S_BlockSound();
			}
			break;

		case SDL_KEYDOWN:
			if ((event.key.keysym.sym == SDLK_RETURN) &&
			    (event.key.keysym.mod & KMOD_ALT))
			{
				VID_ToggleFullscreen();
				break;
			}
			if ((event.key.keysym.sym == SDLK_ESCAPE) &&
			    (event.key.keysym.mod & KMOD_SHIFT))
			{
				Con_ToggleConsole_f();
				break;
			}
			if ((event.key.keysym.sym == SDLK_g) &&
			    (event.key.keysym.mod & KMOD_CTRL))
			{
				SDL_WM_GrabInput((SDL_WM_GrabInput (SDL_GRAB_QUERY) == SDL_GRAB_ON) ?
									  SDL_GRAB_OFF : SDL_GRAB_ON);
				break;
			}
		/* fallthrough */
		case SDL_KEYUP:
			sym = event.key.keysym.sym;
			state = event.key.state;
			modstate = SDL_GetModState();

			if (event.key.keysym.unicode != 0)
			{
				if ((event.key.keysym.unicode & 0xFF80) == 0)
				{
					int usym = event.key.keysym.unicode & 0x7F;
					if (modstate & KMOD_CTRL && usym < 32 && sym >= 32)
					{
						/* control characters */
						if (modstate & KMOD_SHIFT)
							usym += 64;
						else	usym += 96;
					}
#if defined(__APPLE__) && defined(__MACH__)
					if (sym == SDLK_BACKSPACE)
						usym = sym;	/* avoid change to SDLK_DELETE */
#endif	/* Mac OS X */
#if defined(__QNX__) || defined(__QNXNTO__)
					if (sym == SDLK_BACKSPACE || sym == SDLK_RETURN)
						usym = sym;	/* S.A: fixes QNX weirdness */
#endif	/* __QNX__ */
					/* only use unicode for ` and ~ in game mode */
					if (!gamekey || usym == '`' || usym == '~')
						sym = usym;
				}
				/* else: it's an international character */
			}
			/*printf("You pressed %s (%d) (%c)\n", SDL_GetKeyName(sym), sym, sym);*/

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
			case SDLK_RMETA:
			case SDLK_LMETA:
				sym = K_COMMAND;
				break;
			case SDLK_NUMLOCK:
				if (gamekey)
					sym = K_KP_NUMLOCK;
				else	sym = 0;
				break;
			case SDLK_KP0:
				if (gamekey)
					sym = K_KP_INS;
				else	sym = (modstate & KMOD_NUM) ? SDLK_0 : K_INS;
				break;
			case SDLK_KP1:
				if (gamekey)
					sym = K_KP_END;
				else	sym = (modstate & KMOD_NUM) ? SDLK_1 : K_END;
				break;
			case SDLK_KP2:
				if (gamekey)
					sym = K_KP_DOWNARROW;
				else	sym = (modstate & KMOD_NUM) ? SDLK_2 : K_DOWNARROW;
				break;
			case SDLK_KP3:
				if (gamekey)
					sym = K_KP_PGDN;
				else	sym = (modstate & KMOD_NUM) ? SDLK_3 : K_PGDN;
				break;
			case SDLK_KP4:
				if (gamekey)
					sym = K_KP_LEFTARROW;
				else	sym = (modstate & KMOD_NUM) ? SDLK_4 : K_LEFTARROW;
				break;
			case SDLK_KP5:
				if (gamekey)
					sym = K_KP_5;
				else	sym = SDLK_5;
				break;
			case SDLK_KP6:
				if (gamekey)
					sym = K_KP_RIGHTARROW;
				else	sym = (modstate & KMOD_NUM) ? SDLK_6 : K_RIGHTARROW;
				break;
			case SDLK_KP7:
				if (gamekey)
					sym = K_KP_HOME;
				else	sym = (modstate & KMOD_NUM) ? SDLK_7 : K_HOME;
				break;
			case SDLK_KP8:
				if (gamekey)
					sym = K_KP_UPARROW;
				else	sym = (modstate & KMOD_NUM) ? SDLK_8 : K_UPARROW;
				break;
			case SDLK_KP9:
				if (gamekey)
					sym = K_KP_PGUP;
				else	sym = (modstate & KMOD_NUM) ? SDLK_9 : K_PGUP;
				break;
			case SDLK_KP_PERIOD:
				if (gamekey)
					sym = K_KP_DEL;
				else	sym = (modstate & KMOD_NUM) ? SDLK_PERIOD : K_DEL;
				break;
			case SDLK_KP_DIVIDE:
				if (gamekey)
					sym = K_KP_SLASH;
				else	sym = SDLK_SLASH;
				break;
			case SDLK_KP_MULTIPLY:
				if (gamekey)
					sym = K_KP_STAR;
				else	sym = SDLK_ASTERISK;
				break;
			case SDLK_KP_MINUS:
				if (gamekey)
					sym = K_KP_MINUS;
				else	sym = SDLK_MINUS;
				break;
			case SDLK_KP_PLUS:
				if (gamekey)
					sym = K_KP_PLUS;
				else	sym = SDLK_PLUS;
				break;
			case SDLK_KP_ENTER:
				if (gamekey)
					sym = K_KP_ENTER;
				else	sym = SDLK_RETURN;
				break;
			case SDLK_KP_EQUALS:
				if (gamekey)
					sym = 0;
				else	sym = SDLK_EQUALS;
				break;
			case 178: /* the '²' key */
				sym = '~';
				break;
			default:
			/* If we are not directly handled and still above 255,
			 * just force it to 0. kill unsupported international
			 * characters, too.  */
				if ((sym >= SDLK_WORLD_0 && sym <= SDLK_WORLD_95) ||
									sym > 255)
					sym = 0;
				break;
			}
			if (!IN_JoystickBlockDoubledKeyEvents(sym))
				Key_Event(sym, state);
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

		case SDL_JOYBUTTONDOWN:
		case SDL_JOYBUTTONUP:
			if (in_mode_set)
				break;
			if (event.jbutton.button > K_AUX28 - K_JOY1)
			{
				Con_Printf ("Ignored event for joystick button %d\n",
							event.jbutton.button);
				break;
			}
			Key_Event(K_JOY1 + event.jbutton.button, event.jbutton.state == SDL_PRESSED);
			break;

		/* mouse/trackball motion handled by IN_MouseMove() */
		/* axes and hat (pov) motion handled by IN_JoyMove() */
		case SDL_MOUSEMOTION:
		case SDL_JOYBALLMOTION:
		case SDL_JOYHATMOTION:
		case SDL_JOYAXISMOTION:
			break;

		case SDL_QUIT:
			CL_Disconnect ();
			Sys_Quit ();
			break;

		default:
			break;
		}
	}
}

