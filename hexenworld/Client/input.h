/*
	input.h
	external (non-keyboard) input devices

	$Id: input.h,v 1.6 2007-07-17 14:04:03 sezero Exp $
*/

#ifndef __HX2_INPUT_H
#define __HX2_INPUT_H

void IN_Init (void);

void IN_Shutdown (void);

void IN_Commands (void);
// oportunity for devices to stick commands on the script buffer

void IN_Move (usercmd_t *cmd);
// add additional movement on top of the keyboard move cmd


// for window manager events
void IN_ShowMouse (void);
void IN_DeactivateMouse (void);
void IN_HideMouse (void);
void IN_ActivateMouse (void);

extern	cvar_t		_enable_mouse;

void IN_SendKeyEvents (void);

#if defined(PLATFORM_WINDOWS)
void IN_MouseEvent (int mstate);
void IN_ClearStates (void);
void IN_Accumulate (void);
void IN_SetQuakeMouseState (void);
void IN_RestoreOriginalMouseState (void);
void IN_UpdateClipCursor (void);
#else	/* not used by SDL input driver */
#define IN_MouseEvent(E)
#define IN_ClearStates()
#define IN_Accumulate()
#define IN_SetQuakeMouseState()
#define IN_RestoreOriginalMouseState()
#define IN_UpdateClipCursor()
#endif	/* PLATFORM_WINDOWS */

#endif	/* __HX2_INPUT_H */

