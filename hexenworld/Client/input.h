/*
	input.h
	external (non-keyboard) input devices

	$Id: input.h,v 1.5 2007-03-14 21:03:33 sezero Exp $
*/

#ifndef __HX2_INPUT_H
#define __HX2_INPUT_H

void IN_Init (void);

void IN_Shutdown (void);

void IN_Commands (void);
// oportunity for devices to stick commands on the script buffer

void IN_Move (usercmd_t *cmd);
// add additional movement on top of the keyboard move cmd

void IN_ModeChanged (void);
// called whenever screen dimensions change

void IN_ClearStates (void);
// restores all button and position states to defaults


// for window manager events
void IN_ShowMouse (void);
void IN_DeactivateMouse (void);
void IN_HideMouse (void);
void IN_ActivateMouse (void);

extern	cvar_t		_enable_mouse;

void IN_SendKeyEvents (void);

#endif	/* __HX2_INPUT_H */

