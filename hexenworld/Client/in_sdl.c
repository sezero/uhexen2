// in_win.c -- windows 95 mouse and joystick code
// 02/21/97 JCB Added extended DirectInput code to support external controllers.

/*
 * $Header: /home/ozzie/Download/0000/uhexen2/hexenworld/Client/in_sdl.c,v 1.3 2004-12-04 02:05:57 sezero Exp $
 */

#include "SDL.h"
#include "quakedef.h"

// mouse variables
cvar_t	m_filter = {"m_filter","0"};

int			mouse_buttons;
int			mouse_oldbuttonstate;
//POINT		current_pos;
int			mouse_x, mouse_y, old_mouse_x, old_mouse_y, mx_accum, my_accum;
extern cvar_t	vid_mode, _windowed_mouse;
#define MODE_FULLSCREEN_DEFAULT 3

extern qboolean	in_mode_set;
static qboolean	restore_spi;
static int		originalmouseparms[3], newmouseparms[3] = {0, 0, 1};
static qboolean	mouseactive;
qboolean		mouseinitialized;
static qboolean	mouseparmsvalid, mouseactivatetoggle;
static qboolean	mouseshowtoggle = 1;

qboolean grab = 1;

// joystick defines and variables
// where should defines be moved?
#define JOY_ABSOLUTE_AXIS	0x00000000		// control like a joystick
#define JOY_RELATIVE_AXIS	0x00000010		// control like a mouse, spinner, trackball
#define	JOY_MAX_AXES		6				// X, Y, Z, R, U, V
#define JOY_AXIS_X			0
#define JOY_AXIS_Y			1
#define JOY_AXIS_Z			2
#define JOY_AXIS_R			3
#define JOY_AXIS_U			4
#define JOY_AXIS_V			5
#if 0
enum _ControlList
{
	AxisNada = 0, AxisForward, AxisLook, AxisSide, AxisTurn
};
DWORD	dwAxisFlags[JOY_MAX_AXES] =
{
	JOY_RETURNX, JOY_RETURNY, JOY_RETURNZ, JOY_RETURNR, JOY_RETURNU, JOY_RETURNV
};
DWORD	dwAxisMap[JOY_MAX_AXES];
DWORD	dwControlMap[JOY_MAX_AXES];
PDWORD	pdwRawValue[JOY_MAX_AXES];
#endif

// none of these cvars are saved over a session
// this means that advanced controller configuration needs to be executed
// each time.  this avoids any problems with getting back to a default usage
// or when changing from one controller to another.  this way at least something
// works.
cvar_t	in_joystick = {"joystick","1"};
cvar_t	joy_name = {"joyname", "joystick"};
cvar_t	joy_advanced = {"joyadvanced", "0"};
cvar_t	joy_advaxisx = {"joyadvaxisx", "0"};
cvar_t	joy_advaxisy = {"joyadvaxisy", "0"};
cvar_t	joy_advaxisz = {"joyadvaxisz", "0"};
cvar_t	joy_advaxisr = {"joyadvaxisr", "0"};
cvar_t	joy_advaxisu = {"joyadvaxisu", "0"};
cvar_t	joy_advaxisv = {"joyadvaxisv", "0"};
cvar_t	joy_forwardthreshold = {"joyforwardthreshold", "0.15"};
cvar_t	joy_sidethreshold = {"joysidethreshold", "0.15"};
cvar_t	joy_pitchthreshold = {"joypitchthreshold", "0.15"};
cvar_t	joy_yawthreshold = {"joyyawthreshold", "0.15"};
cvar_t	joy_forwardsensitivity = {"joyforwardsensitivity", "-1.0"};
cvar_t	joy_sidesensitivity = {"joysidesensitivity", "-1.0"};
cvar_t	joy_pitchsensitivity = {"joypitchsensitivity", "1.0"};
cvar_t	joy_yawsensitivity = {"joyyawsensitivity", "-1.0"};
cvar_t	joy_wwhack1 = {"joywwhack1", "0.0"};
cvar_t	joy_wwhack2 = {"joywwhack2", "0.0"};

qboolean	joy_avail, joy_advancedinit, joy_haspov;
#if 0
DWORD		joy_oldbuttonstate, joy_oldpovstate;

int			joy_id;
DWORD		joy_flags;
DWORD		joy_numbuttons;

static JOYINFOEX	ji;
#endif

// forward-referenced functions
void IN_StartupJoystick (void);
void Joy_AdvancedUpdate_f (void);
void IN_JoyMove (usercmd_t *cmd);

/*
===========
Force_CenterView_f
===========
*/
void Force_CenterView_f (void)
{
	cl.viewangles[PITCH] = 0;
}


/*
===========
IN_UpdateClipCursor
===========
*/
void IN_UpdateClipCursor (void)
{
// FIXME - figure out what this does and un-stub it - DDOI
#if 0
	if (mouseinitialized && mouseactive)
		ClipCursor (&window_rect);
#endif
}


/*
===========
IN_ShowMouse
===========
*/
void IN_ShowMouse (void)
{

	if (!mouseshowtoggle)
	{
		SDL_ShowCursor(1);
		mouseshowtoggle = 1;
	}
}


/*
===========
IN_HideMouse
===========
*/
void IN_HideMouse (void)
{

	if (mouseshowtoggle)
	{
		SDL_ShowCursor (0);
		mouseshowtoggle = 0;
	}
}


/*
===========
IN_ActivateMouse
===========
*/

void IN_ActivateMouseSA (void)
{
	// S.A's hack to activate mouse
	if ((int)_windowed_mouse.value || (int)vid_mode.value == MODE_FULLSCREEN_DEFAULT)
		IN_ActivateMouse ();
}

void IN_ActivateMouse (void)
{

	mouseactivatetoggle = true;

	if (mouseinitialized)
	{
//		if (mouseparmsvalid)
//			restore_spi = SystemParametersInfo (SPI_SETMOUSE, 0, newmouseparms, 0);

//		SetCursorPos (window_center_x, window_center_y);

		mouseactive = true;

//		SetCapture (mainwindow);
		if (grab)
			SDL_WM_GrabInput (SDL_GRAB_ON);
//		ClipCursor (&window_rect);
	}
}


/*
===========
IN_SetQuakeMouseState
===========
*/
void IN_SetQuakeMouseState (void)
{
	if (mouseactivatetoggle)
		IN_ActivateMouse ();
}


/*
===========
IN_DeactivateMouse
===========
*/
void IN_DeactivateMouseSA (void)
{
	// don't worry if fullscreen - S.A.
	if ((int)vid_mode.value != MODE_FULLSCREEN_DEFAULT)
		IN_DeactivateMouse ();
}

void IN_DeactivateMouse (void)
{

	mouseactivatetoggle = false;

	if (mouseinitialized)
	{
//		if (restore_spi)
//			SystemParametersInfo (SPI_SETMOUSE, 0, originalmouseparms, 0);

		mouseactive = false;

//		ClipCursor (NULL);
		SDL_WM_GrabInput (SDL_GRAB_OFF);
//		ReleaseCapture ();
	}
}


/*
===========
IN_RestoreOriginalMouseState
===========
*/
void IN_RestoreOriginalMouseState (void)
{
	if (mouseactivatetoggle)
	{
		IN_DeactivateMouse ();
		mouseactivatetoggle = true;
	}

// try to redraw the cursor so it gets reinitialized, because sometimes it
// has garbage after the mode switch
// not applicable on Linux - DDOI	
//	ShowCursor (TRUE);
//	ShowCursor (FALSE);
}


/*
===========
IN_StartupMouse
===========
*/
void IN_StartupMouse (void)
{
	// Should be a NOP in Linux, with this exception - DDOI
	mouseinitialized = true;
	mouse_buttons = 3;
	
#if 0	
	HDC			hdc;

	if ( COM_CheckParm ("-nomouse") ) 
		return; 

	mouseinitialized = true;
	mouseparmsvalid = SystemParametersInfo (SPI_GETMOUSE, 0, originalmouseparms, 0);

	if (mouseparmsvalid)
	{
		if ( COM_CheckParm ("-noforcemspd") ) 
			newmouseparms[2] = originalmouseparms[2];

		if ( COM_CheckParm ("-noforcemaccel") ) 
		{
			newmouseparms[0] = originalmouseparms[0];
			newmouseparms[1] = originalmouseparms[1];
		}

		if ( COM_CheckParm ("-noforcemparms") ) 
		{
			newmouseparms[0] = originalmouseparms[0];
			newmouseparms[1] = originalmouseparms[1];
			newmouseparms[2] = originalmouseparms[2];
		}
	}

	mouse_buttons = 3;

	if (mouse_buttons > 3)
		mouse_buttons = 3;

#endif
// if a fullscreen video mode was set before the mouse was initialized,
// set the mouse state appropriately
	if (mouseactivatetoggle)
		IN_ActivateMouse ();
}


/*
===========
IN_Init
===========
*/
void IN_Init (void)
{
	// mouse variables
	Cvar_RegisterVariable (&m_filter);

	// joystick variables
	Cvar_RegisterVariable (&in_joystick);
	Cvar_RegisterVariable (&joy_name);
	Cvar_RegisterVariable (&joy_advanced);
	Cvar_RegisterVariable (&joy_advaxisx);
	Cvar_RegisterVariable (&joy_advaxisy);
	Cvar_RegisterVariable (&joy_advaxisz);
	Cvar_RegisterVariable (&joy_advaxisr);
	Cvar_RegisterVariable (&joy_advaxisu);
	Cvar_RegisterVariable (&joy_advaxisv);
	Cvar_RegisterVariable (&joy_forwardthreshold);
	Cvar_RegisterVariable (&joy_sidethreshold);
	Cvar_RegisterVariable (&joy_pitchthreshold);
	Cvar_RegisterVariable (&joy_yawthreshold);
	Cvar_RegisterVariable (&joy_forwardsensitivity);
	Cvar_RegisterVariable (&joy_sidesensitivity);
	Cvar_RegisterVariable (&joy_pitchsensitivity);
	Cvar_RegisterVariable (&joy_yawsensitivity);
	Cvar_RegisterVariable (&joy_wwhack1);
	Cvar_RegisterVariable (&joy_wwhack2);

	Cmd_AddCommand ("force_centerview", Force_CenterView_f);
	Cmd_AddCommand ("joyadvancedupdate", Joy_AdvancedUpdate_f);

	IN_StartupMouse ();
	IN_StartupJoystick ();

	SDL_EnableUNICODE(1);
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
}


/*
===========
IN_MouseEvent
===========
*/
void IN_MouseEvent (int mstate)
{
	int		i;

	if (mouseactive)
	{
	// perform button actions
		for (i=0 ; i<=mouse_buttons ; i++)
		{
			if ( (mstate & SDL_BUTTON(i+1)) &&
				!(mouse_oldbuttonstate & SDL_BUTTON(i+1)) )
			{
				Key_Event (K_MOUSE1 + i, true);
			}

			if ( !(mstate & SDL_BUTTON(i+1)) &&
				(mouse_oldbuttonstate & SDL_BUTTON(i+1)) )
			{
					Key_Event (K_MOUSE1 + i, false);
			}
		}	
			
		mouse_oldbuttonstate = mstate;
	}
}


/*
===========
IN_MouseMove
===========
*/
void IN_MouseMove (usercmd_t *cmd)
{
	int		mx, my;
//	HDC	hdc;


//	if (sv_player->v.cameramode)	// Stuck in a different camera, don't move
//		return;

//	GetCursorPos (&current_pos);

//	mx = current_pos.x - window_center_x + mx_accum;
//	my = current_pos.y - window_center_y + my_accum;
	// This replaces these ^^^^ - DDOI
	SDL_GetRelativeMouseState(&mx,&my);

	mx_accum = 0;
	my_accum = 0;

	if (m_filter.value)
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

// add mouse X/Y movement to cmd
	if ( (in_strafe.state & 1) || (lookstrafe.value && (in_mlook.state & 1) ))
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
		if ((in_strafe.state & 1) && noclip_anglehack)
			cmd->upmove -= m_forward.value * mouse_y;
		else
			cmd->forwardmove -= m_forward.value * mouse_y;
	}

	if (cl.idealroll == 0) // Did keyboard set it already??
	{
		if ((mouse_x <0) && (cl.v.movetype==MOVETYPE_FLY))
			cl.idealroll=-10;
		else if ((mouse_x >0) && (cl.v.movetype==MOVETYPE_FLY))
			cl.idealroll=10;
	}

// if the mouse has moved, force it to the center, so there's room to move
	if (mx || my)
	{
	// I'll give it a whirl without this and see how she does - DDOI
	//	SetCursorPos (window_center_x, window_center_y);
	}
}


/*
===========
IN_Move
===========
*/
void IN_Move (usercmd_t *cmd)
{
	if (cl.v.cameramode)	// Stuck in a different camera so don't move
	{
		memset (cmd, 0, sizeof(*cmd));
		return;
	}

	if (mouseactive)
	{
		IN_MouseMove (cmd);
	}

	// FIXME - Stub - DDOI
	//if (ActiveApp)
	//	IN_JoyMove (cmd);
}


/*
===========
IN_Accumulate
===========
*/
void IN_Accumulate (void)
{
	// FIXME - Un-stub if needed! - DDOI
#if 0
	int		mx, my;
	HDC	hdc;

	if (mouseactive)
	{
		GetCursorPos (&current_pos);

		mx_accum += current_pos.x - window_center_x;
		my_accum += current_pos.y - window_center_y;

	// force the mouse to the center, so there's room to move
		SetCursorPos (window_center_x, window_center_y);
	}
#endif
}


/*
===================
IN_ClearStates
===================
*/
void IN_ClearStates (void)
{

	if (mouseactive)
	{
		mx_accum = 0;
		my_accum = 0;
		mouse_oldbuttonstate = 0;
	}
}


/* 
=============== 
IN_StartupJoystick 
=============== 
*/  
void IN_StartupJoystick (void) 
{ 
// FIXME - Stubbed for now - DDOI
#if 0	
	int			i, numdevs;
	JOYCAPS		jc;
	MMRESULT	mmr;
 
 	// assume no joystick
	joy_avail = false; 

	// abort startup if user requests no joystick
	if ( COM_CheckParm ("-nojoy") ) 
		return; 
 
	// verify joystick driver is present
	if ((numdevs = joyGetNumDevs ()) == 0)
	{
		Con_Printf ("\njoystick not found -- driver not present\n\n");
		return;
	}

	// cycle through the joystick ids for the first valid one
	for (joy_id=0 ; joy_id<numdevs ; joy_id++)
	{
		memset (&ji, 0, sizeof(ji));
		ji.dwSize = sizeof(ji);
		ji.dwFlags = JOY_RETURNCENTERED;

		if ((mmr = joyGetPosEx (joy_id, &ji)) == JOYERR_NOERROR)
			break;
	} 

	// abort startup if we didn't find a valid joystick
	if (mmr != JOYERR_NOERROR)
	{
		Con_Printf ("\njoystick not found -- no valid joysticks (%x)\n\n", mmr);
		return;
	}

	// get the capabilities of the selected joystick
	// abort startup if command fails
	memset (&jc, 0, sizeof(jc));
	if ((mmr = joyGetDevCaps (joy_id, &jc, sizeof(jc))) != JOYERR_NOERROR)
	{
		Con_Printf ("\njoystick not found -- invalid joystick capabilities (%x)\n\n", mmr); 
		return;
	}

	// save the joystick's number of buttons and POV status
	joy_numbuttons = jc.wNumButtons;
	joy_haspov = jc.wCaps & JOYCAPS_HASPOV;

	// old button and POV states default to no buttons pressed
	joy_oldbuttonstate = joy_oldpovstate = 0;

	// mark the joystick as available and advanced initialization not completed
	// this is needed as cvars are not available during initialization
	Con_Printf ("\njoystick found\n\n", mmr); 
	joy_avail = true; 
	joy_advancedinit = false;
#endif
}


/*
===========
RawValuePointer
===========
*/
#if 0
PDWORD RawValuePointer (int axis)
{
	switch (axis)
	{
	case JOY_AXIS_X:
		return &ji.dwXpos;
	case JOY_AXIS_Y:
		return &ji.dwYpos;
	case JOY_AXIS_Z:
		return &ji.dwZpos;
	case JOY_AXIS_R:
		return &ji.dwRpos;
	case JOY_AXIS_U:
		return &ji.dwUpos;
	case JOY_AXIS_V:
		return &ji.dwVpos;
	}
}
#endif


/*
===========
Joy_AdvancedUpdate_f
===========
*/
void Joy_AdvancedUpdate_f (void)
{
#if 0
	// called once by IN_ReadJoystick and by user whenever an update is needed
	// cvars are now available
	int	i;
	DWORD dwTemp;

	// initialize all the maps
	for (i = 0; i < JOY_MAX_AXES; i++)
	{
		dwAxisMap[i] = AxisNada;
		dwControlMap[i] = JOY_ABSOLUTE_AXIS;
		pdwRawValue[i] = RawValuePointer(i);
	}

	if( joy_advanced.value == 0.0)
	{
		// default joystick initialization
		// 2 axes only with joystick control
		dwAxisMap[JOY_AXIS_X] = AxisTurn;
		// dwControlMap[JOY_AXIS_X] = JOY_ABSOLUTE_AXIS;
		dwAxisMap[JOY_AXIS_Y] = AxisForward;
		// dwControlMap[JOY_AXIS_Y] = JOY_ABSOLUTE_AXIS;
	}
	else
	{
		if (strcmp (joy_name.string, "joystick") != 0)
		{
			// notify user of advanced controller
			Con_Printf ("\n%s configured\n\n", joy_name.string);
		}

		// advanced initialization here
		// data supplied by user via joy_axisn cvars
		dwTemp = (DWORD) joy_advaxisx.value;
		dwAxisMap[JOY_AXIS_X] = dwTemp & 0x0000000f;
		dwControlMap[JOY_AXIS_X] = dwTemp & JOY_RELATIVE_AXIS;
		dwTemp = (DWORD) joy_advaxisy.value;
		dwAxisMap[JOY_AXIS_Y] = dwTemp & 0x0000000f;
		dwControlMap[JOY_AXIS_Y] = dwTemp & JOY_RELATIVE_AXIS;
		dwTemp = (DWORD) joy_advaxisz.value;
		dwAxisMap[JOY_AXIS_Z] = dwTemp & 0x0000000f;
		dwControlMap[JOY_AXIS_Z] = dwTemp & JOY_RELATIVE_AXIS;
		dwTemp = (DWORD) joy_advaxisr.value;
		dwAxisMap[JOY_AXIS_R] = dwTemp & 0x0000000f;
		dwControlMap[JOY_AXIS_R] = dwTemp & JOY_RELATIVE_AXIS;
		dwTemp = (DWORD) joy_advaxisu.value;
		dwAxisMap[JOY_AXIS_U] = dwTemp & 0x0000000f;
		dwControlMap[JOY_AXIS_U] = dwTemp & JOY_RELATIVE_AXIS;
		dwTemp = (DWORD) joy_advaxisv.value;
		dwAxisMap[JOY_AXIS_V] = dwTemp & 0x0000000f;
		dwControlMap[JOY_AXIS_V] = dwTemp & JOY_RELATIVE_AXIS;
	}

	// compute the axes to collect from DirectInput
	joy_flags = JOY_RETURNCENTERED | JOY_RETURNBUTTONS | JOY_RETURNPOV;
	for (i = 0; i < JOY_MAX_AXES; i++)
	{
		if (dwAxisMap[i] != AxisNada)
		{
			joy_flags |= dwAxisFlags[i];
		}
	}
#endif
}


/*
===========
IN_Commands
===========
*/
void IN_Commands (void)
{
#if 0
	int		i, key_index;
	DWORD	buttonstate, povstate;

	if (!joy_avail)
	{
		return;
	}

	if (cls.state != ca_connected || cls.signon != SIGNONS)
	{
		if( joy_advancedinit != true )
		{
			Joy_AdvancedUpdate_f();
			joy_advancedinit = true;
		}

		IN_ReadJoystick ();
	}
	
	// loop through the joystick buttons
	// key a joystick event or auxillary event for higher number buttons for each state change
	buttonstate = ji.dwButtons;
	for (i=0 ; i < joy_numbuttons ; i++)
	{
		if ( (buttonstate & (1<<i)) && !(joy_oldbuttonstate & (1<<i)) )
		{
			key_index = (i < 4) ? K_JOY1 : K_AUX1;
			Key_Event (key_index + i, true);
		}

		if ( !(buttonstate & (1<<i)) && (joy_oldbuttonstate & (1<<i)) )
		{
			key_index = (i < 4) ? K_JOY1 : K_AUX1;
			Key_Event (key_index + i, false);
		}
	}
	joy_oldbuttonstate = buttonstate;

	if (joy_haspov)
	{
		// convert POV information into 4 bits of state information
		// this avoids any potential problems related to moving from one
		// direction to another without going through the center position
		povstate = 0;
		if(ji.dwPOV != JOY_POVCENTERED)
		{
			if (ji.dwPOV == JOY_POVFORWARD)
				povstate |= 0x01;
			if (ji.dwPOV == JOY_POVRIGHT)
				povstate |= 0x02;
			if (ji.dwPOV == JOY_POVBACKWARD)
				povstate |= 0x04;
			if (ji.dwPOV == JOY_POVLEFT)
				povstate |= 0x08;
		}
		// determine which bits have changed and key an auxillary event for each change
		for (i=0 ; i < 4 ; i++)
		{
			if ( (povstate & (1<<i)) && !(joy_oldpovstate & (1<<i)) )
			{
				Key_Event (K_AUX29 + i, true);
			}

			if ( !(povstate & (1<<i)) && (joy_oldpovstate & (1<<i)) )
			{
				Key_Event (K_AUX29 + i, false);
			}
		}
		joy_oldpovstate = povstate;
	}
#endif
}


/* 
=============== 
IN_ReadJoystick
=============== 
*/  
qboolean IN_ReadJoystick (void)
{
#if 0
	memset (&ji, 0, sizeof(ji));
	ji.dwSize = sizeof(ji);
	ji.dwFlags = joy_flags;

	if (joyGetPosEx (joy_id, &ji) == JOYERR_NOERROR)
	{
		// this is a hack -- there is a bug in the Logitech WingMan Warrior DirectInput Driver
		// rather than having 32768 be the zero point, they have the zero point at 32668
		// go figure -- anyway, now we get the full resolution out of the device
		if (joy_wwhack1.value != 0.0)
		{
			ji.dwUpos += 100;
		}
		return true;
	}
	else
	{
		// read error occurred
		// turning off the joystick seems too harsh for 1 read error,\
		// but what should be done?
		// Con_Printf ("IN_ReadJoystick: no response\n");
		// joy_avail = false;
		return false;
	}
#endif
	return false;	// <--- Just a stub - DDOI
}


/*
===========
IN_JoyMove
===========
*/
void IN_JoyMove (usercmd_t *cmd)
{
#if 0
	float	speed, aspeed;
	float	fAxisValue, fTemp;
	int		i;

	// complete initialization if first time in
	// this is needed as cvars are not available at initialization time
	if( joy_advancedinit != true )
	{
		Joy_AdvancedUpdate_f();
		joy_advancedinit = true;
	}

	// verify joystick is available and that the user wants to use it
	if (!joy_avail || !in_joystick.value)
	{
		return; 
	}
 
	// collect the joystick data, if possible
	if (IN_ReadJoystick () != true)
	{
		return;
	}

	if (in_speed.state & 1)
		speed = cl_movespeedkey.value;
	else
		speed = 1;
	aspeed = speed * host_frametime;

	// loop through the axes
	for (i = 0; i < JOY_MAX_AXES; i++)
	{
		// get the floating point zero-centered, potentially-inverted data for the current axis
		fAxisValue = (float) *pdwRawValue[i];
		// move centerpoint to zero
		fAxisValue -= 32768.0;

		if (joy_wwhack2.value != 0.0)
		{
			if (dwAxisMap[i] == AxisTurn)
			{
				// this is a special formula for the Logitech WingMan Warrior
				// y=ax^b; where a = 300 and b = 1.3
				// also x values are in increments of 800 (so this is factored out)
				// then bounds check result to level out excessively high spin rates
				fTemp = 300.0 * pow(abs(fAxisValue) / 800.0, 1.3);
				if (fTemp > 14000.0)
					fTemp = 14000.0;
				// restore direction information
				fAxisValue = (fAxisValue > 0.0) ? fTemp : -fTemp;
			}
		}

		// convert range from -32768..32767 to -1..1 
		fAxisValue /= 32768.0;

		switch (dwAxisMap[i])
		{
		case AxisForward:
			if ((joy_advanced.value == 0.0) && (in_mlook.state & 1))
			{
				// user wants forward control to become look control
				if (fastfabs(fAxisValue) > joy_pitchthreshold.value)
				{		
					// if mouse invert is on, invert the joystick pitch value
					// only absolute control support here (joy_advanced is false)
					if (m_pitch.value < 0.0)
					{
						cl.viewangles[PITCH] -= (fAxisValue * joy_pitchsensitivity.value) * aspeed * cl_pitchspeed.value;
					}
					else
					{
						cl.viewangles[PITCH] += (fAxisValue * joy_pitchsensitivity.value) * aspeed * cl_pitchspeed.value;
					}
					V_StopPitchDrift();
				}
				else
				{
					// no pitch movement
					// disable pitch return-to-center unless requested by user
					// *** this code can be removed when the lookspring bug is fixed
					// *** the bug always has the lookspring feature on
					if(lookspring.value == 0.0)
						V_StopPitchDrift();
				}
			}
			else
			{
				// user wants forward control to be forward control
				if (fastfabs(fAxisValue) > joy_forwardthreshold.value)
				{
//					cmd->forwardmove += (fAxisValue * joy_forwardsensitivity.value) * speed * cl_forwardspeed.value;
					cmd->forwardmove += (fAxisValue * joy_forwardsensitivity.value) * speed * 200;
				}
			}
			break;

		case AxisSide:
			if (fastfabs(fAxisValue) > joy_sidethreshold.value)
			{
//				cmd->sidemove += (fAxisValue * joy_sidesensitivity.value) * speed * cl_sidespeed.value;
				cmd->sidemove += (fAxisValue * joy_sidesensitivity.value) * speed * 225;
			}
			break;

		case AxisTurn:
			if ((in_strafe.state & 1) || (lookstrafe.value && (in_mlook.state & 1)))
			{
				// user wants turn control to become side control
				if (fastfabs(fAxisValue) > joy_sidethreshold.value)
				{
//					cmd->sidemove -= (fAxisValue * joy_sidesensitivity.value) * speed * cl_sidespeed.value;
					cmd->sidemove -= (fAxisValue * joy_sidesensitivity.value) * speed * 225;
				}
			}
			else
			{
				// user wants turn control to be turn control
				if (fastfabs(fAxisValue) > joy_yawthreshold.value)
				{
					if(dwControlMap[i] == JOY_ABSOLUTE_AXIS)
					{
						cl.viewangles[YAW] += (fAxisValue * joy_yawsensitivity.value) * aspeed * cl_yawspeed.value;
					}
					else
					{
						cl.viewangles[YAW] += (fAxisValue * joy_yawsensitivity.value) * speed * 180.0;
					}

				}
			}
			break;

		case AxisLook:
			if (in_mlook.state & 1)
			{
				if (fastfabs(fAxisValue) > joy_pitchthreshold.value)
				{
					// pitch movement detected and pitch movement desired by user
					if(dwControlMap[i] == JOY_ABSOLUTE_AXIS)
					{
						cl.viewangles[PITCH] += (fAxisValue * joy_pitchsensitivity.value) * aspeed * cl_pitchspeed.value;
					}
					else
					{
						cl.viewangles[PITCH] += (fAxisValue * joy_pitchsensitivity.value) * speed * 180.0;
					}
					V_StopPitchDrift();
				}
				else
				{
					// no pitch movement
					// disable pitch return-to-center unless requested by user
					// *** this code can be removed when the lookspring bug is fixed
					// *** the bug always has the lookspring feature on
					if(lookspring.value == 0.0)
						V_StopPitchDrift();
				}
			}
			break;

		default:
			break;
		}
	}

	// bounds check pitch
	if (cl.viewangles[PITCH] > 80.0)
		cl.viewangles[PITCH] = 80.0;
	if (cl.viewangles[PITCH] < -70.0)
		cl.viewangles[PITCH] = -70.0;
#endif
}

void IN_SendKeyEvents (void)
{
        SDL_Event event;
        int sym, state;
        int modstate;

	//SDL_EnableUNICODE(1);

        while (SDL_PollEvent(&event))
        {
                switch (event.type) {

                        case SDL_KEYDOWN:
				if ((event.key.keysym.sym == SDLK_RETURN) &&
				    (event.key.keysym.mod & KMOD_ALT))
				{
					SDL_WM_ToggleFullScreen (SDL_GetVideoSurface());
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
				      {
					/* only use unicode for ~ and ` in game mode */
					if ((event.key.keysym.unicode & 0xFF80) == 0 ) 
					  {
					    if ( ((event.key.keysym.unicode & 0x7F) == '`') ||((event.key.keysym.unicode & 0x7F) == '~')) {
					      sym=event.key.keysym.unicode & 0x7F;
					      
					    }
					  }
					
				      }
				    break;
				  case key_message:
				  case key_console:
				    if ((event.key.keysym.unicode != 0) || (modstate & KMOD_SHIFT))
				      {
					if ((event.key.keysym.unicode & 0xFF80) == 0 )
					  sym=event.key.keysym.unicode & 0x7F;
					
					/* else: it's an international character */
				      }
				    //printf("You pressed %s (%d) (%c)\n",SDL_GetKeyName(sym),sym,sym);
				    break;
				  default:
				    break;
				  }

                                switch(sym)
                                {
                                        case SDLK_DELETE: sym = K_DEL; break;
                                        case SDLK_BACKSPACE: sym = K_BACKSPACE; break;
                                        case SDLK_F1: sym = K_F1; break;
                                        case SDLK_F2: sym = K_F2; break;
                                        case SDLK_F3: sym = K_F3; break;
                                        case SDLK_F4: sym = K_F4; break;
                                        case SDLK_F5: sym = K_F5; break;
                                        case SDLK_F6: sym = K_F6; break;
                                        case SDLK_F7: sym = K_F7; break;
                                        case SDLK_F8: sym = K_F8; break;
                                        case SDLK_F9: sym = K_F9; break;
                                        case SDLK_F10: sym = K_F10; break;
                                        case SDLK_F11: sym = K_F11; break;
                                        case SDLK_F12: sym = K_F12; break;
                                        case SDLK_BREAK:
                                        case SDLK_PAUSE: sym = K_PAUSE; break;
                                        case SDLK_UP: sym = K_UPARROW; break;
                                        case SDLK_DOWN: sym = K_DOWNARROW; break;
                                        case SDLK_RIGHT: sym = K_RIGHTARROW; break;
                                        case SDLK_LEFT: sym = K_LEFTARROW; break;
                                        case SDLK_INSERT: sym = K_INS; break;
                                        case SDLK_HOME: sym = K_HOME; break;
                                        case SDLK_END: sym = K_END; break;
                                        case SDLK_PAGEUP: sym = K_PGUP; break;
                                        case SDLK_PAGEDOWN: sym = K_PGDN; break;
                                        case SDLK_RSHIFT:
                                        case SDLK_LSHIFT: sym = K_SHIFT; break;
                                        case SDLK_RCTRL:
                                        case SDLK_LCTRL: sym = K_CTRL; break;
                                        case SDLK_RALT:
                                        case SDLK_LALT: sym = K_ALT; break;
                                        case SDLK_KP0:
                                                if(modstate & KMOD_NUM)
                                                        sym = K_INS;
                                                else
                                                        sym = SDLK_0;
                                                break;
                                        case SDLK_KP1:
                                                if(modstate & KMOD_NUM)
                                                        sym = K_END;
                                                else
                                                        sym = SDLK_1;
                                                break;
                                        case SDLK_KP2:
                                                if(modstate & KMOD_NUM)
                                                        sym = K_DOWNARROW;
                                                else
                                                        sym = SDLK_2;
                                                break;
                                        case SDLK_KP3:
                                                if(modstate & KMOD_NUM)
                                                        sym = K_PGDN;
                                                else
                                                        sym = SDLK_3;
                                                break;
                                        case SDLK_KP4:
                                                if(modstate & KMOD_NUM)
                                                        sym = K_LEFTARROW;
                                                else
                                                        sym = SDLK_4;
                                                break;
                                        case SDLK_KP5: sym = SDLK_5; break;
                                        case SDLK_KP6:
                                                if(modstate & KMOD_NUM)
                                                        sym = K_RIGHTARROW;
                                                else
                                                        sym = SDLK_6;
                                                break;
                                        case SDLK_KP7:
                                                if(modstate & KMOD_NUM)
                                                        sym = K_HOME;
                                                else
                                                        sym = SDLK_7;
                                                break;
                                        case SDLK_KP8:
                                                if(modstate & KMOD_NUM)
                                                        sym = K_UPARROW;
                                                else
                                                        sym = SDLK_8;
                                                break;
                                        case SDLK_KP9:
                                                if(modstate & KMOD_NUM)
                                                        sym = K_PGUP;
                                                else
                                                        sym = SDLK_9;
                                                break;
                                        case SDLK_KP_PERIOD:
                                                if(modstate & KMOD_NUM)
                                                        sym = K_DEL;
                                                else
                                                        sym = SDLK_PERIOD;
                                                break;
                                        case SDLK_KP_DIVIDE: sym = SDLK_SLASH; break;
                                        case SDLK_KP_MULTIPLY: sym = SDLK_ASTERISK; break;
                                        case SDLK_KP_MINUS: sym = SDLK_MINUS; break;
                                        case SDLK_KP_PLUS: sym = SDLK_PLUS; break;
                                        case SDLK_KP_ENTER: sym = SDLK_RETURN; break;
                                        case SDLK_KP_EQUALS: sym = SDLK_EQUALS; break;

				case 178: /* the '²' key */
				  //sym = 178; 
				  sym = '~';
                                }
                                // If we're not directly handled and still above
                                // 255 just force it to 0
                                if(sym > 255) sym = 0;
                                Key_Event(sym, state);
                                break;
                        case SDL_MOUSEBUTTONDOWN:
                        case SDL_MOUSEBUTTONUP:
                        case SDL_MOUSEMOTION:
                                if (!in_mode_set)
                                {
                                        IN_MouseEvent (SDL_GetMouseState(NULL,NULL));
                                }
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

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.2  2004/12/04 02:04:42  sezero
 * porting from hexen2 :  steve's mouse changes-1.
 *
 * Revision 1.1  2004/11/28 08:54:29  sezero
 * Initial revision
 *
 * Revision 1.2  2002/01/03 15:50:52  phneutre
 * unicode support
 *
 * Revision 1.1  2001/12/02 00:22:17  theoddone33
 * Initial client porting, software mode works
 *
 * Revision 1.3  2001/11/12 23:57:21  theoddone33
 * Alt-enter and ^G support
 *
 * Revision 1.2  2001/11/12 23:31:58  theoddone33
 * Some Loki-ish parameters and general cleanup/bugfixes.
 *
 * Revision 1.1.1.1  2001/11/09 17:04:15  theoddone33
 * Inital import
 *
 * 
 * 3     3/01/98 8:20p Jmonroe
 * removed the slow "quake" version of common functions
 * 
 * 2     2/04/98 12:53a Jmonroe
 * added fastfabs
 * 
 * 10    9/01/97 4:29a Rjohnson
 * Joystick fix
 * 
 * 9     8/27/97 12:11p Rjohnson
 * Hardcoded speeds
 * 
 * 8     8/26/97 11:38a Rlove
 * 
 * 7     7/15/97 1:59p Rjohnson
 * Fix for mouse and client
 * 
 * 6     3/25/97 11:28a Rlove
 * New camera entity
 * 
 * 5     3/07/97 1:46p Rjohnson
 * Id Updates
 */
