/*
	winquake.h
	Win32-specific Quake header file

	$Id: winquake.h,v 1.18 2006-03-24 17:34:25 sezero Exp $
*/

#if !defined(_WIN32)
#error winquake.h is only meant to be used on Windows
#endif

#ifdef _MSC_VER
#pragma warning( disable : 4229 )  // mgraph gets this
#endif
#include <windows.h>
#include <winsock.h>	// for LCC
#include <ctype.h>

#ifndef SERVERONLY
#include <ddraw.h>
#include <dsound.h>
#include <mmsystem.h>	// for LCC
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

void IN_RestoreOriginalMouseState (void);
void IN_SetQuakeMouseState (void);
void IN_MouseEvent (int mstate);
void IN_Accumulate (void);

extern qboolean	winsock_lib_initialized;

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
int (PASCAL FAR *psetsockopt)(SOCKET s, int level, int optname, const char FAR * optval, int optlen);
int (PASCAL FAR *precvfrom)(SOCKET s, char FAR * buf, int len, int flags, struct sockaddr FAR *from, int FAR * fromlen);
int (PASCAL FAR *psendto)(SOCKET s, const char FAR * buf, int len, int flags, const struct sockaddr FAR *to, int tolen);
int (PASCAL FAR *pclosesocket)(SOCKET s);
int (PASCAL FAR *pgethostname)(char FAR * name, int namelen);
struct hostent FAR * (PASCAL FAR *pgethostbyname)(const char FAR * name);
struct hostent FAR * (PASCAL FAR *pgethostbyaddr)(const char FAR * addr, int len, int type);
int (PASCAL FAR *pgetsockname)(SOCKET s, struct sockaddr FAR *name, int FAR * namelen);

