/*
	winquake.h
	Win32-specific Quake header file

	$Id: winquake.h,v 1.17 2006-02-18 08:51:10 sezero Exp $
*/

#if !defined(_WIN32)
#error winquake.h is only meant to be used on Windows
#endif

#ifdef _MSC_VER
#pragma warning( disable : 4229 )  // mgraph gets this
#endif
#include <windows.h>
#include <winsock.h>	// for LCC
#include <mmsystem.h>	// for LCC
#include <ctype.h>

#ifndef SERVERONLY
#include <ddraw.h>
#include <dsound.h>
#endif

extern	HINSTANCE	global_hInstance;
extern	int			global_nCmdShow;

#ifndef WM_MOUSEWHEEL
#define	WM_MOUSEWHEEL		0x020A
#endif

#ifndef SERVERONLY
extern LPDIRECTDRAW		lpDD;
extern qboolean			DDActive;
extern LPDIRECTSOUND pDS;
extern LPDIRECTSOUNDBUFFER pDSBuf;

extern DWORD gSndBufSize;
//#define SNDBUFSIZE 65536
#endif

extern HWND			mainwindow;
extern qboolean		ActiveApp, Minimized;

extern qboolean	Win95, Win95old, WinNT;

void IN_ShowMouse (void);
void IN_DeactivateMouse (void);
void IN_HideMouse (void);
void IN_ActivateMouse (void);
void IN_RestoreOriginalMouseState (void);
void IN_SetQuakeMouseState (void);
void IN_MouseEvent (int mstate);

extern qboolean	winsock_lib_initialized;

extern cvar_t		_enable_mouse;

extern int		window_center_x, window_center_y;
extern RECT		window_rect;

#if !defined(NO_SPLASHES)
extern HWND		hwnd_dialog;
#endif

extern HANDLE	hinput, houtput;

void IN_UpdateClipCursor (void);

void S_BlockSound (void);
void S_UnblockSound (void);

struct sockaddr;
int (PASCAL FAR *pWSAStartup)(WORD wVersionRequired, LPWSADATA lpWSAData);
int (PASCAL FAR *pWSACleanup)(void);
int (PASCAL FAR *pWSAGetLastError)(void);
SOCKET (PASCAL FAR *psocket)(int af, int type, int protocol);
int (PASCAL FAR *pioctlsocket)(SOCKET s, long cmd, u_long FAR *argp);
int (PASCAL FAR *psetsockopt)(SOCKET s, int level, int optname,
							  const char FAR * optval, int optlen);
int (PASCAL FAR *precvfrom)(SOCKET s, char FAR * buf, int len, int flags,
							struct sockaddr FAR *from, int FAR * fromlen);
int (PASCAL FAR *psendto)(SOCKET s, const char FAR * buf, int len, int flags,
						  const struct sockaddr FAR *to, int tolen);
int (PASCAL FAR *pclosesocket)(SOCKET s);
int (PASCAL FAR *pgethostname)(char FAR * name, int namelen);
struct hostent FAR * (PASCAL FAR *pgethostbyname)(const char FAR * name);
struct hostent FAR * (PASCAL FAR *pgethostbyaddr)(const char FAR * addr,
												  int len, int type);
int (PASCAL FAR *pgetsockname)(SOCKET s, struct sockaddr FAR *name,
							   int FAR * namelen);

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.16  2006/01/17 20:10:43  sezero
 * missing parts of vid_win synchronization (remove Snd_AcquireBuffer and
 * Snd_ReleaseBuffer which was only used by the old vid_win.c of hexen2.)
 *
 * Revision 1.15  2006/01/12 12:34:38  sezero
 * added video modes enumeration via SDL. added on-the-fly video mode changing
 * partially based on the Pa3PyX hexen2 tree. TODO: make the game remember its
 * video settings, clean it up, fix it up...
 *
 * Revision 1.14  2006/01/07 09:54:29  sezero
 * cleanup and "static" stuff on the vid files
 *
 * Revision 1.13  2005/09/17 06:02:46  sezero
 * minor ifdef, syntax and defines clean-up
 *
 * Revision 1.12  2005/08/11 12:34:41  sezero
 * mgraph.h is only needed in vid_win.c, no need to include it globally in here
 *
 * Revision 1.11  2005/06/19 11:23:23  sezero
 * added wheelmouse support and conwidth support to hexen2. changed
 * hexenworld's default behavior of default 640 conwidth to main width
 * unless specified otherwise by the user. disabled startup splash
 * screens for now. sycned hexen2 and hexnworld's GL_Init_Functions().
 * disabled InitCommonControls()in gl_vidnt. moved RegisterWindowMessage
 * for uMSG_MOUSEWHEEL to in_win where it belongs. bumped MAXIMUM_WIN_MEMORY
 * to 32 MB. killed useless Sys_ConsoleInput in hwcl. several other sycning
 * and clean-up
 *
 * Revision 1.10  2005/06/17 16:24:33  sezero
 * win32 fixes and clean-ups
 *
 * Revision 1.9  2005/06/15 11:03:39  sezero
 * missing includes for LCC
 *
 * Revision 1.8  2005/05/21 22:15:47  sezero
 * forgot replacing windowed_mouse in winquake.h
 *
 * Revision 1.7  2005/05/21 17:04:17  sezero
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
 * Revision 1.6  2005/05/20 15:26:33  sezero
 * separated winquake.h into winquake.h and linquake.h
 * changed all occurances of winquake.h to quakeinc.h,
 * which includes the correct header
 *
 * Revision 1.5  2005/05/20 12:29:37  sezero
 * leftovers after common.c sync-1
 *
 * Revision 1.4  2004/12/18 13:44:12  sezero
 * Clean-up and kill warnings 1:
 * Kill two pragmas that are ignored anyway.
 *
 * Revision 1.3  2004/12/12 14:14:43  sezero
 * style changes to our liking
 *
 * Revision 1.2  2004/12/04 19:51:43  sezero
 * Kill more warnings (add a forward declaration of
 * struct sockaddr in winquake.h)
 *
 * Revision 1.1.1.1  2004/11/28 00:08:26  sezero
 * Initial import of AoT 1.2.0 code
 *
 * Revision 1.2  2001/12/02 04:59:43  theoddone33
 * Fix nvidia extention problem and a whole bunch of other stuff too apparently
 *
 * 5     7/17/97 2:00p Rjohnson
 * Added a security means to control the running of the game
 * 
 * 4     3/07/97 2:34p Rjohnson
 * Id Updates
 */
