/*
	input.h
	external (non-keyboard) input devices

	$Id$
*/

#ifndef __HX2_INPUT_H
#define __HX2_INPUT_H

void IN_Init (void);
void IN_ReInit (void);

void IN_Shutdown (void);

void IN_Commands (void);
/* for devices to add button commands on the script buffer */

void IN_Move (usercmd_t *cmd);
/* add additional movement on top of the keyboard move cmd */

void IN_SendKeyEvents (void);
/* used as a callback for Sys_SendKeyEvents() by some drivers */

void IN_ClearStates (void);

#define IN_Accumulate()		do {} while (0)

void IN_ActivateMouse (void);
void IN_DeactivateMouse (void);
void IN_ShowMouse (void);
void IN_HideMouse (void);

#if defined(PLATFORM_WINDOWS)
#undef IN_Accumulate
void IN_Accumulate (void);	/* accumulate winmouse movements during frame updates */
void IN_UpdateClipCursor (void);	/* clip the mouse cursor to the window rectangle */
void IN_MouseEvent (int mstate);		/* called from the window procedure */
void IN_SetQuakeMouseState (void);		/* used by Scitech MGL video driver */
void IN_RestoreOriginalMouseState (void);	/* used by Scitech MGL video driver */
#endif	/* PLATFORM_WINDOWS */

#endif	/* __HX2_INPUT_H */

