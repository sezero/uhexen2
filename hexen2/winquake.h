/*
	winquake.h
	Win32-specific Quake header file

	$Header: /home/ozzie/Download/0000/uhexen2/hexen2/winquake.h,v 1.4 2004-12-18 13:44:12 sezero Exp $
*/

#if defined _WIN32
// This is ignored on GNU
#pragma warning( disable : 4229 )  // mgraph gets this
#endif

#ifndef PLATFORM_UNIX
#include <windows.h>

#ifndef SERVERONLY
#include <ddraw.h>
#include <dsound.h>
#ifndef GLQUAKE
#include <mgraph.h>
#endif
#endif

extern	HINSTANCE	global_hInstance;
extern	int			global_nCmdShow;
#else
#include "linux_inc.h"
#endif // !PLATFORM_UNIX

#ifndef SERVERONLY

#ifndef PLATFORM_UNIX
extern LPDIRECTDRAW		lpDD;
extern qboolean			DDActive;
extern LPDIRECTDRAWSURFACE	lpPrimary;
extern LPDIRECTDRAWSURFACE	lpFrontBuffer;
extern LPDIRECTDRAWSURFACE	lpBackBuffer;
extern LPDIRECTDRAWPALETTE	lpDDPal;
extern LPDIRECTSOUND pDS;
extern LPDIRECTSOUNDBUFFER pDSBuf;

extern DWORD gSndBufSize;
//#define SNDBUFSIZE 65536

#endif // !PLATFORM_UNIX

void	VID_LockBuffer (void);
void	VID_UnlockBuffer (void);

void Snd_AcquireBuffer (void);
void Snd_ReleaseBuffer (void);

#endif

typedef enum {MS_WINDOWED, MS_FULLSCREEN, MS_FULLDIB, MS_FULLDIRECT, MS_UNINIT} modestate_t;

extern modestate_t	modestate;

extern HWND			mainwindow;
extern qboolean		ActiveApp, Minimized;

extern qboolean	Win32AtLeastV4, WinNT;
extern qboolean	LegitCopy;

int VID_ForceUnlockedAndReturnState (void);
void VID_ForceLockState (int lk);

void IN_ShowMouse (void);
void IN_DeactivateMouse (void);
void IN_HideMouse (void);
void IN_ActivateMouse (void);
void IN_RestoreOriginalMouseState (void);
void IN_SetQuakeMouseState (void);
void IN_MouseEvent (int mstate);

extern qboolean	winsock_lib_initialized;

extern cvar_t		_windowed_mouse;

extern int		window_center_x, window_center_y;
extern RECT		window_rect;

extern qboolean	mouseinitialized;
extern HWND		hwnd_dialog;

extern HANDLE	hinput, houtput;

void IN_UpdateClipCursor (void);
void CenterWindow(HWND hWndCenter, int width, int height, BOOL lefttopjustify);

void S_BlockSound (void);
void S_UnblockSound (void);

void VID_SetDefaultMode (void);

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
