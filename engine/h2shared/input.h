/*
 * input.h -- external (non-keyboard) input devices
 * $Id$
 *
 * Copyright (C) 1996-1997  Id Software, Inc.
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

