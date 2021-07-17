/* in_win.c -- windows 95 mouse and joystick code
 * 02/21/97 JCB Added extended DirectInput code to support
 * external controllers
 *
 * Copyright (C) 1996-2001  Id Software, Inc.
 * Copyright (C) 2005-2012  O.Sezer <sezero@users.sourceforge.net>
 * Several bits from darkplaces and the ioquake3 projects
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
#include "winquake.h"
#include <mmsystem.h>
#include <dinput.h>

#include "config.h"

#ifdef XINPUT
#include "in_joy_xinput.h"
#endif

// mouse variables
static cvar_t	m_filter = {"m_filter", "0", CVAR_NONE};

static int		mouse_oldbuttonstate;
static POINT		current_pos;
static int		mouse_x, mouse_y, old_mouse_x, old_mouse_y, mx_accum, my_accum;

static qboolean	restore_spi;
static int		originalmouseparms[3], newmouseparms[3] = {0, 0, 0};
			/* [0] threshold to double movement (only if accel level is >= 1)
			 * [1] threshold to quadruple movement (only if accel level is >= 2)
			 * [2] maximum level of acceleration (0 = off)
			 * maximum acceleration was set to 1 in the original engine:
			 * disabling it for better operation on Windows XP and newer.
			 * players can change their sensitivity setting if necessary. */

static qboolean	mouseactive;
static qboolean	mouseinitialized;
static qboolean	mouseparmsvalid, mouseactivatetoggle;
static qboolean	mouseshowtoggle = 1;

/* do NOT include the wheel enums below */
static int buttonremap[] =
{
	K_MOUSE1,
	K_MOUSE2,
	K_MOUSE3,
	K_MOUSE4,
	K_MOUSE5
};
#define	NUM_MOUSEBUTTONS	(sizeof(buttonremap) / sizeof(buttonremap[0]))

/* DirectInput mouse control: */
qboolean			dinput_init;
static qboolean			dinput_acquired;
static unsigned int		mstate_di;

static LPDIRECTINPUT		g_pdi;
static LPDIRECTINPUTDEVICE	g_pMouse;

#define	DINPUT_BUFFERSIZE		16

#if defined(DX_DLSYM)	/* dynamic loading of dinput symbols */

#undef	DEFINE_GUID
#define	DEFINE_GUID(name, l, w1, w2,   b1, b2, b3, b4, b5, b6, b7, b8)	\
	const GUID name =						\
			{ l, w1, w2, { b1, b2, b3, b4, b5, b6, b7, b8 } }

DEFINE_GUID(GUID_SysMouse,   0x6F1D2B60,0xD5A0,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00);
DEFINE_GUID(GUID_XAxis,   0xA36D02E0,0xC9F3,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00);
DEFINE_GUID(GUID_YAxis,   0xA36D02E1,0xC9F3,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00);
DEFINE_GUID(GUID_ZAxis,   0xA36D02E2,0xC9F3,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00);

typedef struct MYDATA {
	LONG	lX;		// X axis goes here
	LONG	lY;		// Y axis goes here
	LONG	lZ;		// Z axis goes here
	BYTE	bButtonA;	// One button goes here
	BYTE	bButtonB;	// Another button goes here
	BYTE	bButtonC;	// Another button goes here
	BYTE	bButtonD;	// Another button goes here
} MYDATA;

static DIOBJECTDATAFORMAT rgodf[] =
{
	{ &GUID_XAxis,	FIELD_OFFSET(MYDATA, lX),	DIDFT_AXIS | DIDFT_ANYINSTANCE,			0,},
	{ &GUID_YAxis,	FIELD_OFFSET(MYDATA, lY),	DIDFT_AXIS | DIDFT_ANYINSTANCE,			0,},
	{ &GUID_ZAxis,	FIELD_OFFSET(MYDATA, lZ),	0x80000000 | DIDFT_AXIS | DIDFT_ANYINSTANCE,	0,},
	{ 0,		FIELD_OFFSET(MYDATA, bButtonA),	DIDFT_BUTTON | DIDFT_ANYINSTANCE,		0,},
	{ 0,		FIELD_OFFSET(MYDATA, bButtonB),	DIDFT_BUTTON | DIDFT_ANYINSTANCE,		0,},
	{ 0,		FIELD_OFFSET(MYDATA, bButtonC),	0x80000000 | DIDFT_BUTTON | DIDFT_ANYINSTANCE,	0,},
	{ 0,		FIELD_OFFSET(MYDATA, bButtonD),	0x80000000 | DIDFT_BUTTON | DIDFT_ANYINSTANCE,	0,},
};

#define NUM_OBJECTS (sizeof(rgodf) / sizeof(rgodf[0]))

static DIDATAFORMAT	diformat =
{
	sizeof(DIDATAFORMAT),		// this structure
	sizeof(DIOBJECTDATAFORMAT),	// size of object data format
	DIDF_RELAXIS,			// absolute axis coordinates
	sizeof(MYDATA),			// device data size
	NUM_OBJECTS,			// number of objects
	rgodf,				// and here they are
};

static HINSTANCE	hInstDI;
static HRESULT (WINAPI *pDirectInputCreate)(HINSTANCE hinst, DWORD dwVersion, LPDIRECTINPUT *lplpDirectInput, LPUNKNOWN punkOuter);

#else	/* ! DX_DLSYM : we're linked to dinput */

#define	diformat			c_dfDIMouse
#define	pDirectInputCreate		DirectInputCreateA

#endif	/* DX_DLSYM */

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

enum _ControlList
{
	AxisNada = 0, AxisForward, AxisLook, AxisSide, AxisTurn, AxisUp
};

static DWORD	dwAxisFlags[JOY_MAX_AXES] =
{
	JOY_RETURNX, JOY_RETURNY, JOY_RETURNZ, JOY_RETURNR, JOY_RETURNU, JOY_RETURNV
};

static DWORD	dwAxisMap[JOY_MAX_AXES];
static DWORD	dwControlMap[JOY_MAX_AXES];
static PDWORD	pdwRawValue[JOY_MAX_AXES];

// none of these cvars are saved over a session
// this means that advanced controller configuration needs to be executed
// each time.  this avoids any problems with getting back to a default usage
// or when changing from one controller to another.  this way at least something
// works.
static	cvar_t	in_joystick = {"joystick", "0", CVAR_ARCHIVE};
static	cvar_t	joy_name = {"joyname", "joystick", CVAR_NONE};
static	cvar_t	joy_advanced = {"joyadvanced", "0", CVAR_NONE};
static	cvar_t	joy_advaxisx = {"joyadvaxisx", "0", CVAR_NONE};
static	cvar_t	joy_advaxisy = {"joyadvaxisy", "0", CVAR_NONE};
static	cvar_t	joy_advaxisz = {"joyadvaxisz", "0", CVAR_NONE};
static	cvar_t	joy_advaxisr = {"joyadvaxisr", "0", CVAR_NONE};
static	cvar_t	joy_advaxisu = {"joyadvaxisu", "0", CVAR_NONE};
static	cvar_t	joy_advaxisv = {"joyadvaxisv", "0", CVAR_NONE};
static	cvar_t	joy_forwardthreshold = {"joyforwardthreshold", "0.15", CVAR_NONE};
static	cvar_t	joy_sidethreshold = {"joysidethreshold", "0.15", CVAR_NONE};
static	cvar_t	joy_upthreshold = {"joyupthreshold", "0.15", CVAR_NONE};
static	cvar_t	joy_pitchthreshold = {"joypitchthreshold", "0.15", CVAR_NONE};
static	cvar_t	joy_yawthreshold = {"joyyawthreshold", "0.15", CVAR_NONE};
static	cvar_t	joy_forwardsensitivity = {"joyforwardsensitivity", "-1.0", CVAR_NONE};
static	cvar_t	joy_sidesensitivity = {"joysidesensitivity", "-1.0", CVAR_NONE};
static	cvar_t	joy_upsensitivity = {"joyupsensitivity", "-1.0", CVAR_NONE};
static	cvar_t	joy_pitchsensitivity = {"joypitchsensitivity", "1.0", CVAR_NONE};
static	cvar_t	joy_yawsensitivity = {"joyyawsensitivity", "-1.0", CVAR_NONE};
static	cvar_t	joy_wwhack1 = {"joywwhack1", "0.0", CVAR_NONE};
static	cvar_t	joy_wwhack2 = {"joywwhack2", "0.0", CVAR_NONE};

static qboolean	joy_avail, joy_advancedinit, joy_haspov;
static DWORD	joy_oldbuttonstate, joy_oldpovstate;

static int		joy_id;
static DWORD	joy_flags;
static DWORD	joy_numbuttons;

static JOYINFOEX	ji;

// forward-referenced functions
static void IN_StartupJoystick (void);
static void Joy_AdvancedUpdate_f (void);
static void IN_JoyMove (usercmd_t *cmd);


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
IN_UpdateClipCursor
===========
*/
void IN_UpdateClipCursor (void)
{
	if (dinput_init)
		return;
	if (mouseinitialized && mouseactive)
	{
		ClipCursor (&window_rect);
	}
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
		ShowCursor (TRUE);
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
		ShowCursor (FALSE);
		mouseshowtoggle = 0;
	}
}


/*
===========
IN_ActivateMouse
===========
*/
void IN_ActivateMouse (void)
{

	mouseactivatetoggle = true;

	if (mouseinitialized && _enable_mouse.integer)
	{
		IN_HideMouse();
		if (dinput_init)
		{
			if (g_pMouse)
			{
				if (!dinput_acquired)
				{
					IDirectInputDevice_Acquire(g_pMouse);
					dinput_acquired = true;
				}
			}
			else
			{
				return;
			}
		}
		else
		{
			if (mouseparmsvalid)
				restore_spi = SystemParametersInfo (SPI_SETMOUSE, 0, newmouseparms, 0);

			SetCursorPos (window_center_x, window_center_y);
			SetCapture (mainwindow);
			ClipCursor (&window_rect);
		}

		mouseactive = true;
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
void IN_DeactivateMouse (void)
{

	mouseactivatetoggle = false;

	if (mouseinitialized)
	{
		IN_ShowMouse();
		if (dinput_init)
		{
			if (g_pMouse)
			{
				if (dinput_acquired)
				{
					IDirectInputDevice_Unacquire(g_pMouse);
					dinput_acquired = false;
				}
			}
		}
		else
		{
			if (restore_spi)
				SystemParametersInfo (SPI_SETMOUSE, 0, originalmouseparms, 0);

			ClipCursor (NULL);
			ReleaseCapture ();
		}

		mouseactive = false;
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
	ShowCursor (TRUE);
	ShowCursor (FALSE);
}


/*
===========
IN_InitDInput
===========
*/
static qboolean IN_InitDInput (void)
{
	HRESULT		hr;
	DIPROPDWORD	dipdw = {
		{
			sizeof(DIPROPDWORD),	// diph.dwSize
			sizeof(DIPROPHEADER),	// diph.dwHeaderSize
			0,			// diph.dwObj
			DIPH_DEVICE,		// diph.dwHow
		},
		DINPUT_BUFFERSIZE,		// dwData
	};

#if defined(DX_DLSYM)	/* dynamic loading of dinput symbols */
	if (!hInstDI)
	{
		hInstDI = LoadLibrary("dinput.dll");

		if (hInstDI == NULL)
		{
			Con_SafePrintf ("Couldn't load dinput.dll\n");
			return false;
		}
	}

	if (!pDirectInputCreate)
	{
		pDirectInputCreate = (HRESULT (WINAPI *)(HINSTANCE, DWORD, LPDIRECTINPUT *, LPUNKNOWN))
								GetProcAddress(hInstDI, "DirectInputCreateA");

		if (!pDirectInputCreate)
		{
			Con_SafePrintf ("Couldn't get DI proc addr\n");
			return false;
		}
	}
#endif	/* DX_DLSYM */

// register with DirectInput and get an IDirectInput to play with.
	hr = pDirectInputCreate(global_hInstance, DIRECTINPUT_VERSION, &g_pdi, NULL);

	if (FAILED(hr))
	{
		return false;
	}

// obtain an interface to the system mouse device.
#if defined(__cplusplus)
	hr = IDirectInput_CreateDevice(g_pdi, GUID_SysMouse, &g_pMouse, NULL);
#else
	hr = IDirectInput_CreateDevice(g_pdi, &GUID_SysMouse, &g_pMouse, NULL);
#endif

	if (FAILED(hr))
	{
		Con_SafePrintf ("Couldn't open DI mouse device\n");
		return false;
	}

// set the data format to "mouse format".
	hr = IDirectInputDevice_SetDataFormat(g_pMouse, &diformat);

	if (FAILED(hr))
	{
		Con_SafePrintf ("Couldn't set DI mouse format\n");
		return false;
	}

// set the cooperativity level.
	hr = IDirectInputDevice_SetCooperativeLevel(g_pMouse, mainwindow,
			DISCL_EXCLUSIVE | DISCL_FOREGROUND);

	if (FAILED(hr))
	{
		Con_SafePrintf ("Couldn't set DI coop level\n");
		return false;
	}

// set the buffer size to DINPUT_BUFFERSIZE elements.
// the buffer size is a DWORD property associated with the device
	hr = IDirectInputDevice_SetProperty(g_pMouse, DIPROP_BUFFERSIZE, &dipdw.diph);

	if (FAILED(hr))
	{
		Con_SafePrintf ("Couldn't set DI buffersize\n");
		return false;
	}

	Con_SafePrintf ("DirectInput initialized\n");
	return true;
}

static void IN_InitWinMouse (void)
{
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
}

/*
===========
IN_StartupMouse
===========
*/
static void IN_StartupMouse (void)
{
	if (safemode || COM_CheckParm ("-nomouse"))
		return;

	mouseinitialized = true;

	if (COM_CheckParm ("-dinput"))
	{
		dinput_init = IN_InitDInput ();
		if (!dinput_init)
			Con_SafePrintf ("DirectInput initialization failed\n");
	}

	if (!dinput_init)
	{
		IN_InitWinMouse ();
	}

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
	Cvar_RegisterVariable (&joy_upthreshold);
	Cvar_RegisterVariable (&joy_pitchthreshold);
	Cvar_RegisterVariable (&joy_yawthreshold);
	Cvar_RegisterVariable (&joy_forwardsensitivity);
	Cvar_RegisterVariable (&joy_sidesensitivity);
	Cvar_RegisterVariable (&joy_upsensitivity);
	Cvar_RegisterVariable (&joy_pitchsensitivity);
	Cvar_RegisterVariable (&joy_yawsensitivity);
	Cvar_RegisterVariable (&joy_wwhack1);
	Cvar_RegisterVariable (&joy_wwhack2);

	Cmd_AddCommand ("force_centerview", Force_CenterView_f);
	Cmd_AddCommand ("joyadvancedupdate", Joy_AdvancedUpdate_f);

	IN_StartupMouse ();
	IN_StartupJoystick ();
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

	dinput_init = mouseinitialized = false;
	if (g_pMouse)
	{
		IDirectInputDevice_Release(g_pMouse);
		g_pMouse = NULL;
	}
	if (g_pdi)
	{
		IDirectInput_Release(g_pdi);
		g_pdi = NULL;
	}
}

/*
===========
IN_ReInit
===========
*/
void IN_ReInit (void)
{
	if (safemode || COM_CheckParm ("-nomouse"))
		return;

	if (g_pMouse)
	{
		IDirectInputDevice_Release(g_pMouse);
		g_pMouse = NULL;
	}
	if (g_pdi)
	{
		IDirectInput_Release(g_pdi);
		g_pdi = NULL;
	}
	old_mouse_x = old_mouse_y = mx_accum = my_accum = 0;
	// we only need to re-initialize direct input.
	// if winmouse is active, nothing is necessary.
	if (!dinput_init)
		return;
	dinput_init = IN_InitDInput ();
	if (!dinput_init)
	{
		Con_SafePrintf ("DirectInput initialization failed\n");
		IN_InitWinMouse ();
	}
}


/*
===========
IN_MouseEvent
===========
*/
void IN_MouseEvent (int mstate)
{
	int		i;

	if (mouseactive && !dinput_init)
	{
	// perform button actions
		for (i = 0; i < (int)NUM_MOUSEBUTTONS; i++)
		{
			if ((mstate ^ mouse_oldbuttonstate) & (1<<i))
				Key_Event (buttonremap[i], (mstate & (1<<i)) != 0);
		}

		mouse_oldbuttonstate = mstate;
	}
}


/*
===========
IN_MouseMove
===========
*/
static void IN_MouseMove (usercmd_t *cmd)
{
	int		mx, my;
	int		i;

	if (!mouseactive)
		return;

	if (dinput_init)
	{
		DIDEVICEOBJECTDATA	od;
		DWORD			dwElements;
		HRESULT			hr;

		mx = 0;
		my = 0;

		for (;;)
		{
			dwElements = 1;

			hr = IDirectInputDevice_GetDeviceData(g_pMouse,
					sizeof(DIDEVICEOBJECTDATA), &od, &dwElements, 0);

			if ((hr == DIERR_INPUTLOST) || (hr == DIERR_NOTACQUIRED))
			{
				dinput_acquired = true;
				IDirectInputDevice_Acquire(g_pMouse);
				break;
			}

			/* Unable to read data or no data available */
			if (FAILED(hr) || dwElements == 0)
			{
				break;
			}

			/* Look at the element to see what happened */
			if ((LONG)od.dwOfs == DIMOFS_X)
			{
				mx += (LONG) od.dwData;
			}
			else if ((LONG)od.dwOfs == DIMOFS_Y)
			{
				my += (LONG) od.dwData;
			}
			else if ((LONG)od.dwOfs == DIMOFS_Z)
			{
				if ((LONG) od.dwData < 0)
				{
					Key_Event (K_MWHEELDOWN, true);
					Key_Event (K_MWHEELDOWN, false);
				}
				else if ((LONG) od.dwData > 0)
				{
					Key_Event (K_MWHEELUP, true);
					Key_Event (K_MWHEELUP, false);
				}
			}
			else if ((LONG)od.dwOfs == DIMOFS_BUTTON0)
			{
				if (od.dwData & 0x80)
					mstate_di |= 1;
				else
					mstate_di &= ~1;
			}
			else if ((LONG)od.dwOfs == DIMOFS_BUTTON1)
			{
				if (od.dwData & 0x80)
					mstate_di |= (1<<1);
				else
					mstate_di &= ~(1<<1);
			}
			else if ((LONG)od.dwOfs == DIMOFS_BUTTON2)
			{
				if (od.dwData & 0x80)
					mstate_di |= (1<<2);
				else
					mstate_di &= ~(1<<2);
			}
			else if ((LONG)od.dwOfs == DIMOFS_BUTTON3)
			{
				if (od.dwData & 0x80)
					mstate_di |= (1<<3);
				else
					mstate_di &= ~(1<<3);
			}
		}

	// perform button actions
		for (i = 0; i < (int)NUM_MOUSEBUTTONS; i++)
		{
			if ((mstate_di ^ mouse_oldbuttonstate) & (1<<i))
				Key_Event (buttonremap[i], (mstate_di & (1<<i)) != 0);
		}

		mouse_oldbuttonstate = mstate_di;
	}
	else
	{
		GetCursorPos (&current_pos);
		mx = current_pos.x - window_center_x + mx_accum;
		my = current_pos.y - window_center_y + my_accum;
		mx_accum = 0;
		my_accum = 0;
	}

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

// add mouse X/Y movement to cmd
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

	if (cl.idealroll == 0) // Did keyboard set it already??
	{
		if (cl.v.movetype == MOVETYPE_FLY)
		{
			if (mouse_x < 0)
				cl.idealroll = -10;
			else if (mouse_x > 0)
				cl.idealroll = 10;
		}
	}

	if (dinput_init)
		return;

// if the mouse has moved, force it to the center, so there's room to move
	if (mx || my)
	{
		SetCursorPos (window_center_x, window_center_y);
	}
}


static void IN_DiscardMove (void)
{
	if (mouseactive && !dinput_init)
	{
	/* nuke data collected by IN_Accumulate(): */
		old_mouse_x = old_mouse_y =
		mx_accum = my_accum =	0;
	}
	/* need I mess with the joystick here too? */
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

	if (ActiveApp && !Minimized)
	{
		IN_MouseMove (cmd);
		IN_JoyMove (cmd);
	}
}


/*
===========
IN_Accumulate
===========
*/
void IN_Accumulate (void)
{
	if (dinput_init)
		return;
	if (mouseactive)
	{
		GetCursorPos (&current_pos);

		mx_accum += current_pos.x - window_center_x;
		my_accum += current_pos.y - window_center_y;

	// force the mouse to the center, so there's room to move
		SetCursorPos (window_center_x, window_center_y);
	}
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
static void IN_StartupJoystick (void)
{
	int		numdevs;
	JOYCAPS		jc;
	MMRESULT	mmr;

	// assume no joystick
	joy_avail = false;
	mmr = ~JOYERR_NOERROR;	// shut up the compiler

	// abort startup if user requests no joystick
	if (safemode || COM_CheckParm ("-nojoy"))
		return;

	// verify joystick driver is present
	if ((numdevs = joyGetNumDevs ()) == 0)
	{
		Con_SafePrintf ("\njoystick not found -- driver not present\n\n");
		return;
	}

	// cycle through the joystick ids for the first valid one
	for (joy_id = 0; joy_id < numdevs; joy_id++)
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
		Con_SafePrintf ("\njoystick not found -- no valid joysticks (%x)\n\n", mmr);
		return;
	}

	// get the capabilities of the selected joystick
	// abort startup if command fails
	memset (&jc, 0, sizeof(jc));
	if ((mmr = joyGetDevCaps (joy_id, &jc, sizeof(jc))) != JOYERR_NOERROR)
	{
		Con_SafePrintf ("\njoystick not found -- invalid joystick capabilities (%x)\n\n", mmr);
		return;
	}

	// save the joystick's number of buttons and POV status
	joy_numbuttons = jc.wNumButtons;
	joy_haspov = jc.wCaps & JOYCAPS_HASPOV;

	// old button and POV states default to no buttons pressed
	joy_oldbuttonstate = joy_oldpovstate = 0;

	// mark the joystick as available and advanced initialization not completed
	// this is needed as cvars are not available during initialization

	joy_avail = true;
	joy_advancedinit = false;

	Con_SafePrintf ("\njoystick detected\n\n");
}


/*
===========
RawValuePointer
===========
*/
static PDWORD RawValuePointer (int axis)
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
	return NULL;
}


/*
===========
Joy_AdvancedUpdate_f
===========
*/
static void Joy_AdvancedUpdate_f (void)
{
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

	if (!joy_advanced.integer)
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
}


/*
===============
IN_ReadJoystick
===============
*/
static qboolean IN_ReadJoystick (void)
{
	memset (&ji, 0, sizeof(ji));
	ji.dwSize = sizeof(ji);
	ji.dwFlags = joy_flags;

	if (joyGetPosEx (joy_id, &ji) == JOYERR_NOERROR)
	{
		// this is a hack -- there is a bug in the Logitech WingMan Warrior DirectInput Driver
		// rather than having 32768 be the zero point, they have the zero point at 32668
		// go figure -- anyway, now we get the full resolution out of the device
		if (joy_wwhack1.integer)
		{
			ji.dwUpos += 100;
		}
		return true;
	}
	else
	{
		// read error occurred
		// turning off the joystick seems too harsh for 1 read error,
		// but what should be done?
		// Con_Printf ("%s: no response\n", __thisfunc__);
		// joy_avail = false;
		return false;
	}
}


/*
===========
IN_Commands
===========
*/
void IN_Commands (void)
{
	int		i, key_index;
	DWORD	buttonstate, povstate;

	if (!joy_avail || !in_joystick.integer)
	{
		return;
	}

#ifndef H2W /* FIXME: why not in HexenWorld ? */
	if (cls.state != ca_connected || cls.signon != SIGNONS)
	{
		if (joy_advancedinit != true)
		{
			Joy_AdvancedUpdate_f();
			joy_advancedinit = true;
		}

		IN_ReadJoystick ();
	}
#endif /* H2W */

	// loop through the joystick buttons
	// key a joystick event or auxillary event for higher number buttons for each state change
	buttonstate = ji.dwButtons;
	for (i = 0; i < (int)joy_numbuttons; i++)
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
		if (ji.dwPOV != JOY_POVCENTERED)
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
		for (i = 0; i < 4; i++)
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
}


/*
===========
IN_JoyMove
===========
*/
static void IN_JoyMove (usercmd_t *cmd)
{
	float	speed, aspeed;
	float	fAxisValue, fTemp;
	int		i;

	// complete initialization if first time in
	// this is needed as cvars are not available at initialization time
	if (joy_advancedinit != true)
	{
		Joy_AdvancedUpdate_f();
		joy_advancedinit = true;
	}

	// verify joystick is available and that the user wants to use it
	if (!joy_avail || !in_joystick.integer)
	{
		return;
	}

	// collect the joystick data, if possible
	if (IN_ReadJoystick () != true)
	{
		return;
	}

	// don't apply while paused, otherwise looking influences the camera
	if (cl.paused)
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

		if (joy_wwhack2.integer)
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
			if (!joy_advanced.integer && (in_mlook.state & 1))
			{
				// user wants forward control to become look control
				if (fabs(fAxisValue) > joy_pitchthreshold.value)
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
			}
			else
			{
				// user wants forward control to be forward control
				if (fabs(fAxisValue) > joy_forwardthreshold.value)
				{
				//	cmd->forwardmove += (fAxisValue * joy_forwardsensitivity.value) * speed * cl_forwardspeed.value;
					cmd->forwardmove += (fAxisValue * joy_forwardsensitivity.value) * speed * 200;
				}
			}
			break;

		case AxisSide:
			if (fabs(fAxisValue) > joy_sidethreshold.value)
			{
			//	cmd->sidemove += (fAxisValue * joy_sidesensitivity.value) * speed * cl_sidespeed.value;
				cmd->sidemove += (fAxisValue * joy_sidesensitivity.value) * speed * 225;
			}
			break;

		case AxisUp:
			if (fabs(fAxisValue) > joy_upthreshold.value)
			{
			//	cmd->upmove += (fAxisValue * joy_upsensitivity.value) * speed * cl_upspeed.value;
				cmd->upmove += (fAxisValue * joy_upsensitivity.value) * speed * 200;
			}
			break;

		case AxisTurn:
			if ((in_strafe.state & 1) || (lookstrafe.integer && (in_mlook.state & 1)))
			{
				// user wants turn control to become side control
				if (fabs(fAxisValue) > joy_sidethreshold.value)
				{
				//	cmd->sidemove -= (fAxisValue * joy_sidesensitivity.value) * speed * cl_sidespeed.value;
					cmd->sidemove -= (fAxisValue * joy_sidesensitivity.value) * speed * 225;
				}
			}
			else
			{
				// user wants turn control to be turn control
				if (fabs(fAxisValue) > joy_yawthreshold.value)
				{
					if (dwControlMap[i] == JOY_ABSOLUTE_AXIS)
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
				if (fabs(fAxisValue) > joy_pitchthreshold.value)
				{
					// pitch movement detected and pitch movement desired by user
					if (dwControlMap[i] == JOY_ABSOLUTE_AXIS)
					{
						cl.viewangles[PITCH] += (fAxisValue * joy_pitchsensitivity.value) * aspeed * cl_pitchspeed.value;
					}
					else
					{
						cl.viewangles[PITCH] += (fAxisValue * joy_pitchsensitivity.value) * speed * 180.0;
					}
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
}

