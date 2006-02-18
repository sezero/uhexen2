/*
	linquake.h
	Unix-specific Quake header file

	$Id: linquake.h,v 1.4 2006-02-18 08:51:11 sezero Exp $
*/

#include "linux_inc.h"

void IN_ShowMouse (void);
void IN_DeactivateMouse (void);
void IN_HideMouse (void);
void IN_ActivateMouse (void);

extern cvar_t		_enable_mouse;

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.3  2005/06/01 14:10:43  sezero
 * removed win32-only net function prototypes from linquake.h
 * and updated linux_inc.h. also moved win32-only ipx netcode
 *
 * Revision 1.2  2005/05/21 17:04:17  sezero
 * - revived -nomouse that "disables mouse no matter what"
 * - renamed _windowed_mouse to _enable_mouse which is our intention,
 *   that is, dynamically disabling/enabling mouse while in game
 * - old code had many oversights/leftovers that prevented mouse being
 *   really disabled in fullscreen mode. fixed and cleaned-up here
 * - even in windowed mode, when mouse was disabled, mouse buttons and
 *   the wheel got processed. fixed it here.
 * - mouse cursor is never shown while the game is alive, regardless
 *   of mouse being enabled/disabled (I never liked an ugly pointer
 *   around while playing) Its only intention would be to be able to
 *   use the desktop, and for that see, the grab notes below
 * - if mouse is disabled, it is un-grabbed in windowed mode. Note: One
 *   can always use the keyboard shortcut CTRL-G for grabbing-ungrabbing
 *   the mouse regardless of mouse being enabled/disabled.
 * - ToggleFullScreenSA() used to update vid_mode but always forgot
 *   modestate. It now updates modestate as well.
 * - Now that IN_ActivateMouse() and IN_DeactivateMouse() are fixed,
 *   IN_ActivateMouseSA() and IN_DeactivateMouseSA() are redundant and
 *   are removed. BTW, I added a new qboolean mousestate_sa (hi Steve)
 *   which keeps track of whether we intentionally disabled the mouse.
 * - mouse disabling in fullscreen mode (in the absence of -nomouse
 *   arg) is not allowed in this patch, but this is done by a if 1/if 0
 *   conditional compilation. Next patch will change all those if 1 to
 *   if 0, and voila!, we can fully disable/enable mouse in fullscreen.
 * - moved modestate enums/defines to vid.h so that they can be used
 *   by other code properly.
 *
 * Revision 1.1  2005/05/20 15:27:37  sezero
 * separated winquake.h into winquake.h and linquake.h
 * changed all occurances of winquake.h to quakeinc.h,
 * which includes the correct header
 *
 *
 * 2005/05/20 12:29:37  sezero
 * leftovers after common.c sync-1
 *
 * 2004/12/18 13:44:12  sezero
 * Clean-up and kill warnings 1:
 * Kill two pragmas that are ignored anyway.
 *
 * 2004/12/04 19:51:43  sezero
 * Kill more warnings (add a forward declaration of
 * struct sockaddr in winquake.h)
 *
 * 2004/11/28 00:08:26  sezero
 * Initial import of AoT 1.2.0 code
 *
 * 2001/12/02 04:59:43  theoddone33
 * Fix nvidia extention problem and a whole bunch of other stuff too apparently
 *
 * 5     7/17/97 2:00p Rjohnson
 * Added a security means to control the running of the game
 * 
 * 4     3/07/97 2:34p Rjohnson
 * Id Updates
 */
