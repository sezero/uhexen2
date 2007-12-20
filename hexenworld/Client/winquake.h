/*
	winquake.h
	Win32-specific Quake header file

	$Id: winquake.h,v 1.31 2007-12-20 21:36:00 sezero Exp $
*/

#if !defined(PLATFORM_WINDOWS)
#error winquake.h is only meant to be used on Windows
#endif

#ifndef __WINQUAKE_H
#define __WINQUAKE_H

#ifdef _MSC_VER
#pragma warning( disable : 4229 )  /* mgraph gets this */
#endif
#include <windows.h>
#include <winsock.h>

#include <mmsystem.h>

/* required compatibility versions for directx components */
#define	DIRECTDRAW_VERSION	0x0300
#define	DIRECTSOUND_VERSION	0x0300
#define	DIRECTINPUT_VERSION	0x0300

#if !defined(__cplusplus) && !defined(CINTERFACE)
#define	CINTERFACE	/* for directx macros. */
#endif

extern	HINSTANCE	global_hInstance;
extern	int			global_nCmdShow;

#ifndef WM_MOUSEWHEEL
#define	WM_MOUSEWHEEL		0x020A
#endif
/* IntelliMouse explorer buttons: These are ifdef'ed out for < Win2000
   in the Feb. 2001 version of MS's platform SDK, but we need them for
   compilation. */
#ifndef WM_XBUTTONDOWN
#define	WM_XBUTTONDOWN		0x020B
#define	WM_XBUTTONUP		0x020C
#endif
#ifndef MK_XBUTTON1
#define	MK_XBUTTON1		0x0020
#define	MK_XBUTTON2		0x0040
#endif


extern qboolean			DDActive;

extern HWND			mainwindow;
extern qboolean		ActiveApp, Minimized;

extern qboolean		Win95, Win95old, WinNT;

extern int		window_center_x, window_center_y;
extern RECT		window_rect;

#if !defined(NO_SPLASHES)
extern HWND		hwnd_dialog;
#endif

void S_BlockSound (void);
void S_UnblockSound (void);

LONG CDAudio_MessageHandler (HWND, UINT, WPARAM, LPARAM);

#endif	/* __WINQUAKE_H */

