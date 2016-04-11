/*
 * vid_win.c -- Win32 video driver
 * $Id$
 *
 * Adapted from Quake2 and from an initial work by MH with many
 * modifications to make it work in Hexen II: Hammer of Thyrion.
 * Uses only DIB sections/GDI.
 *
 * - TODO: Add back more low resolutions.
 * - TODO: Add DDRAW (see Quake2)
 * - TODO: Better video mode management? Early config reading?
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

#include "quakedef.h"
#include "bgmusic.h"
#include "cdaudio.h"
#include "cfgfile.h"
#include "winquake.h"
#include <mmsystem.h>
#include "d_local.h"
#include "resource.h"

#if defined(H2W)
#define WM_CLASSNAME	"HexenWorld"
#define WM_WINDOWNAME	"HexenWorld"
#else
#define WM_CLASSNAME	"HexenII"
#define WM_WINDOWNAME	"HexenII"
#endif

#define MAX_MODE_LIST	64
#define MAX_DESC	13
#define VID_ROW_SIZE	3


byte		globalcolormap[VID_GRADES*256], lastglobalcolor = 0;
byte		*lastsourcecolormap = NULL;

HWND		mainwindow;
qboolean	DDActive;
qboolean	msg_suppress_1 = false;

static int	DIBWidth, DIBHeight;
static RECT	WindowRect;
static LONG	WindowStyle, ExWindowStyle;

int		window_center_x, window_center_y, window_x, window_y, window_width, window_height;
RECT		window_rect;

static DEVMODE	gdevmode;
static qboolean	startwindowed = false;
static qboolean	firstupdate = true;
static qboolean	vid_initialized = false, vid_palettized;
static int	vid_fulldib_on_focus_mode;
static qboolean	force_minimized, force_mode_set;
static int	enable_mouse;
static qboolean	palette_changed, vid_mode_set;
static HICON	hIcon;

viddef_t	vid;		// global video state
qboolean	in_mode_set;

// 0 is MODE_WINDOWED, 3 is MODE_FULLSCREEN_DEFAULT
static	cvar_t	vid_mode = {"vid_mode", "0", CVAR_NONE};
static	cvar_t	_vid_default_mode_win = {"_vid_default_mode_win", "3", CVAR_ARCHIVE};
// compatibility with dos version:
static	cvar_t	_vid_default_mode = {"_vid_default_mode", "0", CVAR_ARCHIVE};

static	cvar_t	vid_config_x = {"vid_config_x", "800", CVAR_ARCHIVE};
static	cvar_t	vid_config_y = {"vid_config_y", "600", CVAR_ARCHIVE};
static	cvar_t	vid_fullscreen_mode = {"vid_fullscreen_mode", "3", CVAR_ARCHIVE};
static	cvar_t	vid_windowed_mode = {"vid_windowed_mode", "0", CVAR_ARCHIVE};

cvar_t		_enable_mouse = {"_enable_mouse", "0", CVAR_ARCHIVE};

static int	vid_modenum = NO_MODE;
static int	vid_testingmode, vid_realmode;
static double	vid_testendtime;
static int	vid_default = MODE_WINDOWED;
static int	windowed_default;

modestate_t	modestate = MS_UNINIT;

static byte		*vid_surfcache;
static int	vid_surfcachesize;
static int	VID_highhunkmark;

static unsigned char	vid_curpal[256*3];	/* save for mode changes */

unsigned short	d_8to16table[256];
unsigned int	d_8to24table[256];

typedef struct {
	modestate_t	type;
	int			width;
	int			height;
	int			modenum;
	int			fullscreen;
	char		modedesc[MAX_DESC];
} vmode_t;

static vmode_t	modelist[MAX_MODE_LIST];
static int		nummodes;

static vmode_t	badmode;

//static byte	backingbuf[48*24];
static byte	backingbuf[48*48];

typedef union _dibinfo
{
    struct {
	BITMAPINFOHEADER	header;
	RGBQUAD			acolors[256];
    };
    BITMAPINFO bi;
} dibinfo_t;

static HGDIOBJ previously_selected_GDI_obj = NULL;
static HBITMAP hDIBSection;
static void *pDIBBase = NULL;
static HDC hdcDIBSection = NULL;
static HDC maindc = NULL;

static void VID_MenuDraw (void);
static void VID_MenuKey (int key);

static qboolean VID_SetMode (int modenum, unsigned char *palette);
static void AppActivate(BOOL fActive, BOOL minimize);
static LRESULT WINAPI MainWndProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


/*
================
VID_UpdateWindowStatus
================
*/
static void VID_UpdateWindowStatus (void)
{
	window_rect.left = window_x;
	window_rect.top = window_y;
	window_rect.right = window_x + window_width;
	window_rect.bottom = window_y + window_height;
	window_center_x = (window_rect.left + window_rect.right) / 2;
	window_center_y = (window_rect.top + window_rect.bottom) / 2;

	IN_UpdateClipCursor ();
}


/*
================
ClearAllStates
================
*/
static void ClearAllStates (void)
{
	Key_ClearStates ();
	IN_ClearStates ();
}


/*
================
VID_CheckAdequateMem
================
*/
static qboolean VID_CheckAdequateMem (int width, int height)
{
#if 0	/* MH says there will always be enough */
	int		tbuffersize;

	tbuffersize = width * height * sizeof (*d_pzbuffer);

	tbuffersize += D_SurfaceCacheForRes (width, height);

// see if there's enough memory, allowing for the normal mode 0x13 pixel,
// z, and surface buffers
	//if ((host_parms->memsize - tbuffersize + SURFCACHE_SIZE_AT_320X200 +
	//	 0x10000 * 3) < MINIMUM_MEMORY)
	// Pa3PyX: using hopefully better estimation now
	// Experimentation: the heap should have at least 12.0 megs
	// remaining (after init) after setting video mode, otherwise
	// it's Hunk_Alloc failures and cache thrashes upon level load
	if (host_parms->memsize < tbuffersize + 0x180000 + 0xC00000)
	{
		return false;		// not enough memory for mode
	}
#endif

	return true;
}


/*
================
VID_AllocBuffers
================
*/
static qboolean VID_AllocBuffers (int width, int height)
{
	int		tsize, tbuffersize;

	tbuffersize = width * height * sizeof (*d_pzbuffer);

	tsize = D_SurfaceCacheForRes (width, height);

	tbuffersize += tsize;

#if 0	/* as in VID_CheckAdequateMem() above. */
// see if there's enough memory, allowing for the normal mode 0x13 pixel,
// z, and surface buffers
	//if ((host_parms->memsize - tbuffersize + SURFCACHE_SIZE_AT_320X200 +
	//	 0x10000 * 3) < MINIMUM_MEMORY)
	// Pa3PyX: using hopefully better estimation now
	// if total memory < needed surface cache + (minimum operational memory
	// less surface cache for 320x200 and typical hunk state after init)
	if (host_parms->memsize < tbuffersize + 0x180000 + 0xC00000)
	{
		Con_SafePrintf ("Not enough memory for video mode\n");
		return false;		// not enough memory for mode
	}
#endif

	vid_surfcachesize = tsize;

	if (d_pzbuffer)
	{
		D_FlushCaches ();
		Hunk_FreeToHighMark (VID_highhunkmark);
		d_pzbuffer = NULL;
	}

	VID_highhunkmark = Hunk_HighMark ();

	d_pzbuffer = (short *) Hunk_HighAllocName (tbuffersize, "video");

	vid_surfcache = (byte *)d_pzbuffer +
			width * height * sizeof (*d_pzbuffer);

	return true;
}


/*
=================
VID_Windowed_f
=================
*/
static void VID_Windowed_f (void)
{
	VID_SetMode (vid_windowed_mode.integer, vid_curpal);
}


/*
=================
VID_Fullscreen_f
=================
*/
static void VID_Fullscreen_f (void)
{
	VID_SetMode (vid_fullscreen_mode.integer, vid_curpal);
}


static void VID_ShutdownDIB (void)
{
	if (hdcDIBSection)
	{
		SelectObject (hdcDIBSection, previously_selected_GDI_obj);
		DeleteDC (hdcDIBSection);
		hdcDIBSection = NULL;
	}

	if (hDIBSection)
	{
		DeleteObject (hDIBSection);
		hDIBSection = NULL;
		pDIBBase = NULL;
	}

	if (maindc)
	{
		// if maindc exists mainwindow must also be valid
		ReleaseDC (mainwindow, maindc);
		maindc = NULL;
	}
}


static void VID_CreateDIB (int width, int height, unsigned char *palette)
{
	dibinfo_t   dibheader;
	BITMAPINFO *pbmiDIB = &dibheader.bi;
	int i;

	maindc = GetDC (mainwindow);
	memset (&dibheader, 0, sizeof (dibheader));

	// fill in the bitmap info
	pbmiDIB->bmiHeader.biSize		= sizeof (BITMAPINFOHEADER);
	pbmiDIB->bmiHeader.biWidth		= width;
	pbmiDIB->bmiHeader.biHeight		= height;
	pbmiDIB->bmiHeader.biPlanes		= 1;
	pbmiDIB->bmiHeader.biBitCount		= 8;
	pbmiDIB->bmiHeader.biCompression	= BI_RGB;
	pbmiDIB->bmiHeader.biSizeImage		= 0;
	pbmiDIB->bmiHeader.biXPelsPerMeter	= 0;
	pbmiDIB->bmiHeader.biYPelsPerMeter	= 0;
	pbmiDIB->bmiHeader.biClrUsed		= 256;
	pbmiDIB->bmiHeader.biClrImportant	= 256;

	// fill in the palette
	for (i = 0; i < 256; i++)
	{
	// d_8to24table isn't filled in yet, so this is just for testing
		dibheader.acolors[i].rgbRed   = palette[i * 3];
		dibheader.acolors[i].rgbGreen = palette[i * 3 + 1];
		dibheader.acolors[i].rgbBlue  = palette[i * 3 + 2];
	}

	// create the DIB section
	hDIBSection = CreateDIBSection (maindc, pbmiDIB, DIB_RGB_COLORS,
					&pDIBBase, NULL, 0);
	if (hDIBSection == NULL)
		Sys_Error ("DIB_Init() - CreateDIBSection failed\n");

	// set video buffers
	if (pbmiDIB->bmiHeader.biHeight > 0)
	{
		// bottom-up
		vid.buffer = (pixel_t *)pDIBBase + (height - 1) * width;
		vid.rowbytes = -width;
	}
	else
	{
		// top-down
		vid.buffer = (pixel_t *)pDIBBase;
		vid.rowbytes = vid.width;
	}

	vid.conbuffer = vid.direct = vid.buffer;
	vid.conrowbytes = vid.rowbytes;

	// clear the buffer
	if (height < 0)		// negative height was sent for top-down bitmap
		memset (pDIBBase, 0xff, width * -height);
	else	memset (pDIBBase, 0xff, width * height);

	if ((hdcDIBSection = CreateCompatibleDC (maindc)) == NULL)
		Sys_Error ("DIB_Init() - CreateCompatibleDC failed\n");

	if ((previously_selected_GDI_obj = SelectObject (hdcDIBSection, hDIBSection)) == NULL)
		Sys_Error ("DIB_Init() - SelectObject failed\n");
}


static void VID_RegisterWndClass (HINSTANCE hInstance)
{
	WNDCLASS	wc;

	wc.style		= CS_OWNDC;
	wc.lpfnWndProc		= MainWndProc;
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= 0;
	wc.hInstance		= hInstance;
	wc.hIcon		= 0;
	wc.hCursor		= LoadCursor (NULL, IDC_ARROW);
	wc.hbrBackground	= NULL;
	wc.lpszMenuName		= 0;
	wc.lpszClassName	= WM_CLASSNAME;

	if (!RegisterClass(&wc))
		Sys_Error ("Couldn't register main window class");
}

static void VID_InitModes (HINSTANCE hInstance)
{
	HDC		hdc;

	hIcon = LoadIcon (hInstance, MAKEINTRESOURCE (IDI_ICON2));

	/* Register the frame class */
	VID_RegisterWndClass(hInstance);

	modelist[0].type = MS_WINDOWED;
	modelist[0].width = 320;
	modelist[0].height = 240;
	q_strlcpy (modelist[0].modedesc, "320x240", MAX_DESC);
	modelist[0].modenum = MODE_WINDOWED;
	modelist[0].fullscreen = 0;

	modelist[1].type = MS_WINDOWED;
	modelist[1].width = 640;
	modelist[1].height = 480;
	q_strlcpy (modelist[1].modedesc, "640x480", MAX_DESC);
	modelist[1].modenum = MODE_WINDOWED + 1;
	modelist[1].fullscreen = 0;

	modelist[2].type = MS_WINDOWED;
	modelist[2].width = 800;
	modelist[2].height = 600;
	q_strlcpy (modelist[2].modedesc, "800x600", MAX_DESC);
	modelist[2].modenum = MODE_WINDOWED + 2;
	modelist[2].fullscreen = 0;

// automatically stretch the default mode up if > 640x480 desktop resolution
	hdc = GetDC(NULL);

	if ((GetDeviceCaps(hdc, HORZRES) > 800) && !COM_CheckParm("-noautostretch"))
	{
		vid_default = MODE_WINDOWED + 2;
	}
	else if ((GetDeviceCaps(hdc, HORZRES) > 640) && !COM_CheckParm("-noautostretch"))
	{
		vid_default = MODE_WINDOWED + 1;
	}
	else
	{
		vid_default = MODE_WINDOWED;
	}

	windowed_default = vid_default;

	ReleaseDC(NULL,hdc);

	nummodes = 3;	// reserve space for windowed mode
}


/*
=================
VID_GetDisplayModes
=================
*/
static void VID_GetDisplayModes (void)
{
	DEVMODE	devmode;
	int		i, modenum, existingmode, originalnummodes, lowestres;
	BOOL	status;

// enumerate > 8 bpp modes
	originalnummodes = nummodes;
	modenum = 0;
	lowestres = 99999;

	do
	{
		status = EnumDisplaySettings (NULL, modenum, &devmode);

		if ((devmode.dmPelsWidth <= MAXWIDTH) &&
			(devmode.dmPelsHeight <= MAXHEIGHT) &&
			(devmode.dmPelsWidth >= 640) &&
			(devmode.dmPelsHeight >= 480) &&
			(nummodes < MAX_MODE_LIST))
		{
			devmode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

			if (ChangeDisplaySettings (&devmode, CDS_TEST | CDS_FULLSCREEN) ==
								DISP_CHANGE_SUCCESSFUL)
			{
				modelist[nummodes].type = MS_FULLDIB;
				modelist[nummodes].width = devmode.dmPelsWidth;
				modelist[nummodes].height = devmode.dmPelsHeight;
				modelist[nummodes].modenum = 0;
				modelist[nummodes].fullscreen = 1;
				q_snprintf (modelist[nummodes].modedesc, MAX_DESC, "%dx%d",
						(int)devmode.dmPelsWidth, (int)devmode.dmPelsHeight);

				// see if the mode is already there
				// (same dimensions but different refresh rate)
				for (i = originalnummodes, existingmode = 0; i < nummodes; i++)
				{
					if ((modelist[nummodes].width == modelist[i].width) &&
						(modelist[nummodes].height == modelist[i].height))
					{
						existingmode = 1;
						break;
					}
				}

				// if it's not add it to the list
				if (!existingmode)
				{
					if (modelist[nummodes].width < lowestres)
						lowestres = modelist[nummodes].width;

					nummodes++;
				}
			}
		}

		modenum++;
	} while (status);

	if (nummodes != originalnummodes)
		vid_default = MODE_FULLSCREEN_DEFAULT;
	else
	{
		Cvar_SetValueQuick (&_vid_default_mode_win, vid_default);
		Con_SafePrintf ("No fullscreen DIB modes found\n");
	}
}


/*
=================
VID_NumModes
=================
*/
static int VID_NumModes (void)
{
	return nummodes;
}


/*
=================
VID_GetModePtr
=================
*/
static vmode_t *VID_GetModePtr (int modenum)
{

	if ((modenum >= 0) && (modenum < nummodes))
		return &modelist[modenum];
	else
		return &badmode;
}


/*
=================
VID_CheckModedescFixup
=================
*/
static void VID_CheckModedescFixup (int modenum)
{
	int		x, y;

	if (modenum == MODE_SETTABLE_WINDOW)
	{
		x = vid_config_x.integer;
		y = vid_config_y.integer;
		q_snprintf (modelist[modenum].modedesc, MAX_DESC, "%dx%d", x, y);
		modelist[modenum].width = x;
		modelist[modenum].height = y;
	}
}


/*
=================
VID_GetModeDescriptionMemCheck
=================
*/
static const char *VID_GetModeDescriptionMemCheck (int modenum)
{
	const char	*pinfo;
	vmode_t		*pv;

	if ((modenum < 0) || (modenum >= nummodes))
		return NULL;

	VID_CheckModedescFixup (modenum);

	pv = VID_GetModePtr (modenum);
	pinfo = pv->modedesc;

	if (VID_CheckAdequateMem (pv->width, pv->height))
	{
		return pinfo;
	}
	else
	{
		return NULL;
	}
}


/*
=================
VID_GetModeDescription
=================
*/
static const char *VID_GetModeDescription (int modenum)
{
	const char	*pinfo;
	vmode_t		*pv;

	if ((modenum < 0) || (modenum >= nummodes))
		return NULL;

	VID_CheckModedescFixup (modenum);

	pv = VID_GetModePtr (modenum);
	pinfo = pv->modedesc;
	return pinfo;
}


/*
=================
VID_GetModeDescription2

Tacks on "windowed" or "fullscreen"
=================
*/
static const char *VID_GetModeDescription2 (int modenum)
{
	static char	pinfo[40];
	vmode_t		*pv;

	if ((modenum < 0) || (modenum >= nummodes))
		return NULL;

	VID_CheckModedescFixup (modenum);

	pv = VID_GetModePtr (modenum);

	if (modelist[modenum].type == MS_FULLSCREEN)
	{
		q_snprintf(pinfo, sizeof(pinfo), "%s fullscreen", pv->modedesc);
	}
	else if (modelist[modenum].type == MS_FULLDIB)
	{
		q_snprintf(pinfo, sizeof(pinfo), "%s fullscreen", pv->modedesc);
	}
	else
	{
		q_snprintf(pinfo, sizeof(pinfo), "%s windowed", pv->modedesc);
	}

	return pinfo;
}


// KJB: Added this to return the mode driver name in description for console

static const char *VID_GetExtModeDescription (int modenum)
{
	static char	pinfo[40];
	vmode_t		*pv;

	if ((modenum < 0) || (modenum >= nummodes))
		return NULL;

	VID_CheckModedescFixup (modenum);

	pv = VID_GetModePtr (modenum);
	if (modelist[modenum].type == MS_FULLDIB)
	{
		q_snprintf(pinfo, sizeof(pinfo), "%s fullscreen", pv->modedesc);
	}
	else
	{
		q_snprintf(pinfo, sizeof(pinfo), "%s windowed", pv->modedesc);
	}

	return pinfo;
}


static void VID_DestroyWindow (void)
{
	if (modestate == MS_FULLDIB)
		ChangeDisplaySettings (NULL, CDS_FULLSCREEN);

	VID_ShutdownDIB ();
}


static void CenterWindow (HWND hWndCenter, int width, int height)
{
	int	CenterX, CenterY;

	CenterX = (GetSystemMetrics(SM_CXSCREEN) - width) / 2;
	CenterY = (GetSystemMetrics(SM_CYSCREEN) - height) / 2;
	if (CenterX > 2*CenterY)
		CenterX >>= 1;	// dual screen?
	if (CenterX < 0)
		CenterX = 0;
	if (CenterY < 0)
		CenterY = 0;
	SetWindowPos (hWndCenter, NULL, CenterX, CenterY, 0, 0,
			SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW | SWP_DRAWFRAME);
}

static qboolean VID_SetWindowedMode (int modenum)
{
	int		lastmodestate;

	VID_CheckModedescFixup (modenum);

	lastmodestate = modestate;

	VID_DestroyWindow ();

	WindowRect.top = WindowRect.left = 0;

	WindowRect.right = modelist[modenum].width;
	WindowRect.bottom = modelist[modenum].height;

	DIBWidth = modelist[modenum].width;
	DIBHeight = modelist[modenum].height;

	WindowStyle = WS_OVERLAPPED | WS_BORDER | WS_CAPTION | WS_SYSMENU |
			  WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	ExWindowStyle = 0;
	AdjustWindowRectEx(&WindowRect, WindowStyle, FALSE, 0);

// the first time we're called to set the mode, create the window we'll use
// for the rest of the session
	if (!vid_mode_set)
	{
		mainwindow = CreateWindowEx (
			 ExWindowStyle,
			 WM_CLASSNAME,
			 WM_WINDOWNAME,
			 WindowStyle,
			 0, 0,
			 WindowRect.right - WindowRect.left,
			 WindowRect.bottom - WindowRect.top,
			 NULL,
			 NULL,
			 global_hInstance,
			 NULL);

		if (!mainwindow)
			Sys_Error ("Couldn't create DIB window");

		vid_mode_set = true;
	}
	else
	{
		SetWindowLongPtr (mainwindow, GWL_STYLE, WindowStyle | WS_VISIBLE);
		SetWindowLongPtr (mainwindow, GWL_EXSTYLE, ExWindowStyle);
	}

	if (!SetWindowPos (mainwindow,
				NULL,
				0, 0,
				WindowRect.right - WindowRect.left,
				WindowRect.bottom - WindowRect.top,
				SWP_NOCOPYBITS | SWP_NOZORDER | SWP_HIDEWINDOW))
	{
		Sys_Error ("Couldn't resize DIB window");
	}

// position and show the DIB window
	CenterWindow(mainwindow, WindowRect.right - WindowRect.left,
				 WindowRect.bottom - WindowRect.top);

	if (force_minimized)
		ShowWindow (mainwindow, SW_MINIMIZE);
	else
		ShowWindow (mainwindow, SW_SHOWDEFAULT);

	UpdateWindow (mainwindow);

	modestate = MS_WINDOWED;
	vid_fulldib_on_focus_mode = 0;

	vid.numpages = 1;
	vid.maxwarpwidth = WARP_WIDTH;
	vid.maxwarpheight = WARP_HEIGHT;
	vid.height = vid.conheight = DIBHeight;
	vid.width = vid.conwidth = DIBWidth;
	vid.aspect = ((float)vid.height / (float)vid.width) * (320.0 / 240.0);

	SendMessage (mainwindow, WM_SETICON, (WPARAM)TRUE, (LPARAM)hIcon);
	SendMessage (mainwindow, WM_SETICON, (WPARAM)FALSE, (LPARAM)hIcon);

	return true;
}


static qboolean VID_SetFullDIBMode (int modenum)
{
	int		lastmodestate;

	VID_DestroyWindow ();

	gdevmode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;
	gdevmode.dmPelsWidth = modelist[modenum].width;
	gdevmode.dmPelsHeight = modelist[modenum].height;
	gdevmode.dmSize = sizeof (gdevmode);

	if (ChangeDisplaySettings (&gdevmode, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
		Sys_Error ("Couldn't set fullscreen DIB mode");

	lastmodestate = modestate;
	modestate = MS_FULLDIB;
	vid_fulldib_on_focus_mode = modenum;

	WindowRect.top = WindowRect.left = 0;

	WindowRect.right = modelist[modenum].width;
	WindowRect.bottom = modelist[modenum].height;

	DIBWidth = modelist[modenum].width;
	DIBHeight = modelist[modenum].height;

	WindowStyle = WS_POPUP | WS_SYSMENU | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	ExWindowStyle = 0;
	AdjustWindowRectEx(&WindowRect, WindowStyle, FALSE, 0);

	SetWindowLongPtr (mainwindow, GWL_STYLE, WindowStyle | WS_VISIBLE);
	SetWindowLongPtr (mainwindow, GWL_EXSTYLE, ExWindowStyle);

	if (!SetWindowPos (mainwindow,
				NULL,
				0, 0,
				WindowRect.right - WindowRect.left,
				WindowRect.bottom - WindowRect.top,
				SWP_NOCOPYBITS | SWP_NOZORDER))
	{
		Sys_Error ("Couldn't resize DIB window");
	}

// position and show the DIB window
	SetWindowPos (mainwindow, HWND_TOPMOST, 0, 0, 0, 0,
				  SWP_NOSIZE | SWP_SHOWWINDOW | SWP_DRAWFRAME);
	ShowWindow (mainwindow, SW_SHOWDEFAULT);
	UpdateWindow (mainwindow);

	vid.numpages = 1;
	vid.maxwarpwidth = WARP_WIDTH;
	vid.maxwarpheight = WARP_HEIGHT;
	vid.height = vid.conheight = DIBHeight;
	vid.width = vid.conwidth = DIBWidth;
	vid.aspect = ((float)vid.height / (float)vid.width) * (320.0 / 240.0);

// needed because we're not getting WM_MOVE messages fullscreen on NT
	window_x = 0;
	window_y = 0;

	return true;
}


static void VID_RestoreOldMode (int original_mode)
{
	static qboolean	inerror = false;

	if (inerror)
		return;

	in_mode_set = false;
	inerror = true;

// make sure mode set happens (video mode changes)
	vid_modenum = original_mode - 1;

	if (!VID_SetMode (original_mode, vid_curpal))
	{
		vid_modenum = MODE_WINDOWED - 1;

		if (!VID_SetMode (windowed_default, vid_curpal))
			Sys_Error ("Can't set any video mode");
	}

	inerror = false;
}

static qboolean VID_SetMode (int modenum, unsigned char *palette)
{
	int		original_mode, temp;
	qboolean	status;
	MSG		msg;
	HDC		hdc;

	while ((modenum >= nummodes) || (modenum < 0))
	{
		if (vid_modenum == NO_MODE)
		{
			if (modenum == vid_default)
			{
				modenum = windowed_default;
			}
			else
			{
				modenum = vid_default;
			}

			Cvar_SetValueQuick (&vid_mode, (float)modenum);
		}
		else
		{
			Cvar_SetValueQuick (&vid_mode, (float)vid_modenum);
			return false;
		}
	}

	if (!force_mode_set && (modenum == vid_modenum))
		return true;

// so Con_Printfs don't mess us up by forcing vid and snd updates
	temp = scr_disabled_for_loading;
	scr_disabled_for_loading = true;
	in_mode_set = true;

	CDAudio_Pause ();
	S_ClearBuffer ();

	if (vid_modenum == NO_MODE)
		original_mode = windowed_default;
	else
		original_mode = vid_modenum;

	// Set either the fullscreen or windowed mode
	if (modelist[modenum].type == MS_WINDOWED)
	{
		if (_enable_mouse.integer)
		{
			status = VID_SetWindowedMode(modenum);
			IN_ActivateMouse ();
			IN_HideMouse ();
		}
		else
		{
			IN_DeactivateMouse ();
			IN_ShowMouse ();
			status = VID_SetWindowedMode(modenum);
		}
	}
	else
	{
		status = VID_SetFullDIBMode(modenum);
		IN_ActivateMouse ();
		IN_HideMouse ();
	}

//	VID_CreateDIB(DIBWidth,-DIBHeight, palette);	// top-down
	VID_CreateDIB(DIBWidth, DIBHeight, palette);	// bottom-up

	window_width = vid.width;
	window_height = vid.height;
	VID_UpdateWindowStatus ();
	CDAudio_Resume ();
	scr_disabled_for_loading = temp;

	if (!status)
	{
		VID_RestoreOldMode (original_mode);
		return false;
	}

// now we try to make sure we get the focus on the mode switch, because
// sometimes in some systems we don't.  We grab the foreground, then
// finish setting up, pump all our messages, and sleep for a little while
// to let messages finish bouncing around the system, then we put
// ourselves at the top of the z order, then grab the foreground again,
// Who knows if it helps, but it probably doesn't hurt
	if (!force_minimized)
		SetForegroundWindow (mainwindow);

	hdc = GetDC(NULL);

	if (GetDeviceCaps(hdc, RASTERCAPS) & RC_PALETTE)
		vid_palettized = true;
	else
		vid_palettized = false;

	VID_SetPalette (palette);

	ReleaseDC(NULL,hdc);

	vid_modenum = modenum;
	Cvar_SetValueQuick (&vid_mode, (float)vid_modenum);

	if (!VID_AllocBuffers (vid.width, vid.height))
	{
	// couldn't get memory for this mode; try to fall back to previous mode
		VID_RestoreOldMode (original_mode);
		return false;
	}

	D_InitCaches (vid_surfcache, vid_surfcachesize);

	while (PeekMessage (&msg, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage (&msg);
		DispatchMessage (&msg);
	}

	Sleep (100);

	if (!force_minimized)
	{
		SetWindowPos (mainwindow, HWND_TOP, 0, 0, 0, 0,
				  SWP_DRAWFRAME | SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW |
				  SWP_NOCOPYBITS);

		SetForegroundWindow (mainwindow);
	}

// fix the leftover Alt from any Alt-Tab or the like that switched us away
	ClearAllStates ();

	if (!msg_suppress_1)
		Con_SafePrintf ("%s\n", VID_GetModeDescription (vid_modenum));

	VID_SetPalette (palette);

	in_mode_set = false;
	vid.recalc_refdef = 1;

	return true;
}

void VID_LockBuffer (void)
{
/* nothing. */
}

void VID_UnlockBuffer (void)
{
/* nothing. */
}


void	VID_SetPalette (unsigned char *palette)
{
	int		i;
	RGBQUAD		colors[256];
	unsigned char *pal = palette;

	if (!Minimized)
	{
		if (hdcDIBSection)
		{
			// incoming palette is 3 component
			for (i = 0; i < 256; i++, pal += 3)
			{
				colors[i].rgbRed   = pal[0];
				colors[i].rgbGreen = pal[1];
				colors[i].rgbBlue  = pal[2];
				colors[i].rgbReserved = 0;
			}

			colors[0].rgbRed = 0;
			colors[0].rgbGreen = 0;
			colors[0].rgbBlue = 0;
			colors[255].rgbRed = 0xff;
			colors[255].rgbGreen = 0xff;
			colors[255].rgbBlue = 0xff;

			if (SetDIBColorTable (hdcDIBSection, 0, 256, colors) == 0)
			{
				Con_SafePrintf ("DIB_SetPalette() - SetDIBColorTable failed\n");
			}
		}
	}

	if (palette != vid_curpal)
		memcpy(vid_curpal, palette, sizeof (vid_curpal));
}


void	VID_ShiftPalette (unsigned char *palette)
{
	VID_SetPalette (palette);
}


/*
=================
VID_DescribeCurrentMode_f
=================
*/
static void VID_DescribeCurrentMode_f (void)
{
	Con_Printf ("%s\n", VID_GetExtModeDescription (vid_modenum));
}


/*
=================
VID_NumModes_f
=================
*/
static void VID_NumModes_f (void)
{
	if (nummodes == 1)
		Con_Printf ("%d video mode is available\n", nummodes);
	else
		Con_Printf ("%d video modes are available\n", nummodes);
}


/*
=================
VID_DescribeMode_f
=================
*/
static void VID_DescribeMode_f (void)
{
	int		modenum;

	modenum = atoi (Cmd_Argv(1));

	Con_Printf ("%s\n", VID_GetExtModeDescription (modenum));
}


/*
=================
VID_DescribeModes_f
=================
*/
static void VID_DescribeModes_f (void)
{
	int		i, lnummodes;
	const char	*pinfo;
	qboolean	na;
	vmode_t		*pv;

	na = false;

	lnummodes = VID_NumModes ();

	for (i = 0; i < lnummodes; i++)
	{
		pv = VID_GetModePtr (i);
		pinfo = VID_GetExtModeDescription (i);

		if (VID_CheckAdequateMem (pv->width, pv->height))
		{
			Con_Printf ("%2d: %s\n", i, pinfo);
		}
		else
		{
			Con_Printf ("**: %s\n", pinfo);
			na = true;
		}
	}

	if (na)
	{
		Con_Printf ("\n[**: not enough system RAM for mode]\n");
	}
}


/*
=================
VID_TestMode_f
=================
*/
static void VID_TestMode_f (void)
{
	int		modenum;
	double	testduration;

	if (!vid_testingmode)
	{
		modenum = atoi (Cmd_Argv(1));

		if (VID_SetMode (modenum, vid_curpal))
		{
			vid_testingmode = 1;
			testduration = atof (Cmd_Argv(2));
			if (testduration == 0)
				testduration = 5.0;
			vid_testendtime = realtime + testduration;
		}
	}
}


/*
=================
VID_Minimize_f
=================
*/
static void VID_Minimize_f (void)
{
// we only support minimizing windows; if you're fullscreen,
// switch to windowed first
	if (modestate == MS_WINDOWED)
		ShowWindow (mainwindow, SW_MINIMIZE);
}


/*
=================
VID_ForceMode_f
=================
*/
static void VID_ForceMode_f (void)
{
	int		modenum;

	if (!vid_testingmode)
	{
		modenum = atoi (Cmd_Argv(1));

		force_mode_set = true;
		VID_SetMode (modenum, vid_curpal);
		force_mode_set = false;
	}
}


void	VID_Init (unsigned char *palette)
{
	int		i, bestmatch, bestmatchmetric, t, dr, dg, db;
	byte		*ptmp;
	const char	*read_vars[] = {
				"_vid_default_mode_win" };
#define num_readvars	( sizeof(read_vars)/sizeof(read_vars[0]) )

	Cvar_RegisterVariable (&vid_mode);
	Cvar_RegisterVariable (&_vid_default_mode);
	Cvar_RegisterVariable (&_vid_default_mode_win);
	Cvar_RegisterVariable (&vid_config_x);
	Cvar_RegisterVariable (&vid_config_y);
	Cvar_RegisterVariable (&_enable_mouse);
	Cvar_RegisterVariable (&vid_fullscreen_mode);
	Cvar_RegisterVariable (&vid_windowed_mode);

	Cmd_AddCommand ("vid_testmode", VID_TestMode_f);
	Cmd_AddCommand ("vid_nummodes", VID_NumModes_f);
	Cmd_AddCommand ("vid_describecurrentmode", VID_DescribeCurrentMode_f);
	Cmd_AddCommand ("vid_describemode", VID_DescribeMode_f);
	Cmd_AddCommand ("vid_describemodes", VID_DescribeModes_f);
	Cmd_AddCommand ("vid_forcemode", VID_ForceMode_f);
	Cmd_AddCommand ("vid_windowed", VID_Windowed_f);
	Cmd_AddCommand ("vid_fullscreen", VID_Fullscreen_f);
	Cmd_AddCommand ("vid_minimize", VID_Minimize_f);

// perform an early read of config.cfg
	CFG_ReadCvars (read_vars, num_readvars);

	VID_InitModes (global_hInstance);
	VID_GetDisplayModes ();

	vid.maxwarpwidth = WARP_WIDTH;
	vid.maxwarpheight = WARP_HEIGHT;
	vid.colormap = host_colormap;
	vid.fullbright = 256 - LittleLong (*((int *)vid.colormap + 2048));
	vid_testingmode = 0;

// GDI doesn't let us remap palette index 0, so we'll remap color
// mappings from that black to another one
	bestmatchmetric = 256*256*3;
	bestmatch = 0;	// FIXME - uninitialized, guessing 0...

	for (i = 1; i < 256; i++)
	{
		dr = palette[0] - palette[i*3];
		dg = palette[1] - palette[i*3+1];
		db = palette[2] - palette[i*3+2];

		t = (dr * dr) + (dg * dg) + (db * db);

		if (t < bestmatchmetric)
		{
			bestmatchmetric = t;
			bestmatch = i;

			if (t == 0)
				break;
		}
	}

	for (i = 0, ptmp = vid.colormap; i < (1 << (VID_CBITS + 8)); i++, ptmp++)
	{
		if (*ptmp == 0)
			*ptmp = bestmatch;
	}

	if (COM_CheckParm("-startwindowed") || COM_CheckParm("-window") || COM_CheckParm("-w"))
	{
		startwindowed = true;
		Cvar_SetValueQuick (&_vid_default_mode_win, windowed_default);
		vid_default = windowed_default;
	}

	if (_vid_default_mode_win.integer < 0 || _vid_default_mode_win.integer >= nummodes)
		Cvar_SetValueQuick (&_vid_default_mode_win, windowed_default);
	Cvar_LockVar ("_vid_default_mode_win");	/* so that config.cfg doesn't break -window */

#if !defined(NO_SPLASHES)
	if (hwnd_dialog)
	{
		DestroyWindow (hwnd_dialog);
		hwnd_dialog = NULL;
	}
#endif	/* ! NO_SPLASHES */

	/* set default fullscreen mode only if
	 * setting the windowed default fails: */
	if (!VID_SetMode(MODE_WINDOWED, palette))
	{
		force_mode_set = true;
		VID_SetMode(vid_default, palette);
		force_mode_set = false;
	}

	vid_initialized = true;
	vid_realmode = vid_modenum;
	VID_SetPalette (palette);
	vid_menudrawfn = VID_MenuDraw;
	vid_menukeyfn = VID_MenuKey;
	q_strlcpy (badmode.modedesc, "Bad mode", MAX_DESC);
}


void	VID_Shutdown (void)
{
	if (vid_initialized)
	{
		if (modestate == MS_FULLDIB)
			ChangeDisplaySettings (NULL, CDS_FULLSCREEN);

		PostMessage (HWND_BROADCAST, WM_PALETTECHANGED, (WPARAM)mainwindow, (LPARAM)0);
		PostMessage (HWND_BROADCAST, WM_SYSCOLORCHANGE, (WPARAM)0, (LPARAM)0);

		AppActivate(false, false);

		VID_DestroyWindow ();

#if !defined(NO_SPLASHES)
		if (hwnd_dialog)
		{
			DestroyWindow (hwnd_dialog);
			hwnd_dialog = NULL;
		}
#endif	/* ! NO_SPLASHES */

		if (mainwindow)
			DestroyWindow(mainwindow);

		vid_testingmode = 0;
		vid_initialized = 0;
	}
}


/*
================
FlipScreen
================
*/
static void FlipScreen (vrect_t *rects)
{
	if (hdcDIBSection)
	{
		int numrects = 0;

		while (rects)
		{
			BitBlt (maindc,
				rects->x, rects->y,
				rects->x + rects->width,
				rects->y + rects->height,
				hdcDIBSection,
				rects->x, rects->y,
				SRCCOPY);

			numrects++;
			rects = rects->pnext;
		}
	}
}


void VID_Update (vrect_t *rects)
{
	vrect_t		rect;

	if (!vid_palettized && palette_changed)
	{
		palette_changed = false;
		rect.x = 0;
		rect.y = 0;
		rect.width = vid.width;
		rect.height = vid.height;
		rect.pnext = NULL;
		rects = &rect;
	}

	if (firstupdate && host_initialized)
	{
		firstupdate = false;
		Cvar_SetValueQuick (&vid_mode, _vid_default_mode_win.integer);
	}

	// We've drawn the frame; copy it to the screen
	FlipScreen (rects);

	if (vid_testingmode)
	{
		if (realtime >= vid_testendtime)
		{
			VID_SetMode (vid_realmode, vid_curpal);
			vid_testingmode = 0;
		}
	}
	else
	{
		if (vid_mode.integer != vid_realmode)
		{
			VID_SetMode (vid_mode.integer, vid_curpal);
			Cvar_SetValueQuick (&vid_mode, vid_modenum);
							// so if mode set fails, we don't keep on
							//  trying to set it
			vid_realmode = vid_modenum;
		}
	}

// handle the mouse state when windowed if that's changed
	if (modestate == MS_WINDOWED)
	{
		if (_enable_mouse.integer != enable_mouse)
		{
			if (_enable_mouse.integer)
			{
				IN_ActivateMouse ();
				IN_HideMouse ();
			}
			else
			{
				IN_DeactivateMouse ();
				IN_ShowMouse ();
			}

			enable_mouse = _enable_mouse.integer;
		}
	}
}


/*
================
D_BeginDirectRect
================
*/
void D_BeginDirectRect (int x, int y, byte *pbitmap, int width, int height)
{
	int		i, j, reps, repshift;
	vrect_t		rect;

	if (!vid_initialized)
		return;

	if (vid.aspect > 1.5)
	{
		reps = 2;
		repshift = 1;
	}
	else
	{
		reps = 1;
		repshift = 0;
	}

//	if (vid.numpages == 1)
//	{
		VID_LockBuffer ();

		if (!vid.direct)
			Sys_Error ("NULL vid.direct pointer");

		for (i = 0; i < (height << repshift); i += reps)
		{
			for (j = 0; j < reps; j++)
			{
				memcpy (&backingbuf[(i + j) * width] /* &backingbuf[(i + j) * 24] */,
						vid.direct + x + ((y << repshift) + i + j) * vid.rowbytes,
						width);
				memcpy (vid.direct + x + ((y << repshift) + i + j) * vid.rowbytes,
						&pbitmap[(i >> repshift) * width],
						width);
			}
		}

		VID_UnlockBuffer ();

		rect.x = x;
		rect.y = y;
		rect.width = width;
		rect.height = height << repshift;
		rect.pnext = NULL;

		FlipScreen (&rect);
//	}
//	else
//	{
//	}
}


#ifndef H2W
/* unused in hexenworld */
void D_ShowLoadingSize (void)
{
#if defined(DRAW_PROGRESSBARS)
	static int prev_perc;
	int		cur_perc;
	vrect_t		rect;
	viddef_t	save_vid;	/* global video state */

	if (!vid_initialized)
		return;

	cur_perc = loading_stage * 100;
	if (total_loading_size)
		cur_perc += current_loading_size * 100 / total_loading_size;
	if (cur_perc == prev_perc)
		return;
	prev_perc = cur_perc;

	save_vid = vid;
//	if (vid.numpages == 1)
//	{
		VID_LockBuffer ();

		if (!vid.direct)
			Sys_Error ("NULL vid.direct pointer");

		vid.buffer = vid.direct;

		SCR_DrawLoading();

		VID_UnlockBuffer ();

#if 1	/* original code */
		rect.x = 0;
		rect.y = 0;
		rect.width = vid.width;
#else	/* Pa3PyX: tweaking sizes - faster redraw */
		rect.x = (vid.width >> 1) - 100;
		rect.y = 0;
		rect.width = 200;
#endif
		rect.height = 112;
		rect.pnext = NULL;

		FlipScreen (&rect);
//	}
//	else
//	{
//	}

	vid = save_vid;
#endif	/* !DRAW_PROGRESSBARS */
}
#endif	/* ! H2W */


/*
================
D_EndDirectRect
================
*/
void D_EndDirectRect (int x, int y, int width, int height)
{
	int		i, j, reps, repshift;
	vrect_t		rect;

	if (!vid_initialized)
		return;

	if (vid.aspect > 1.5)
	{
		reps = 2;
		repshift = 1;
	}
	else
	{
		reps = 1;
		repshift = 0;
	}

//	if (vid.numpages == 1)
//	{
		VID_LockBuffer ();

		if (!vid.direct)
			Sys_Error ("NULL vid.direct pointer");

		for (i = 0; i < (height << repshift); i += reps)
		{
			for (j = 0; j < reps; j++)
			{
				memcpy (vid.direct + x + ((y << repshift) + i + j) * vid.rowbytes,
						&backingbuf[(i + j) * width] /* &backingbuf[(i + j) * 24] */,
						width);
			}
		}

		VID_UnlockBuffer ();

		rect.x = x;
		rect.y = y;
		rect.width = width;
		rect.height = height << repshift;
		rect.pnext = NULL;

		FlipScreen (&rect);
//	}
//	else
//	{
//	}
}


//==========================================================================

/*
=======
MapKey

Map from windows to quake keynums
=======
*/
static byte scantokey[128] =
{
//	0        1       2       3       4       5       6       7
//	8        9       A       B       C       D       E       F
	0  ,    27,     '1',    '2',    '3',    '4',    '5',    '6',
	'7',    '8',    '9',    '0',    '-',    '=', K_BACKSPACE, 9,	// 0
	'q',    'w',    'e',    'r',    't',    'y',    'u',    'i',
	'o',    'p',    '[',    ']',  K_ENTER, K_CTRL,  'a',    's',	// 1
	'd',    'f',    'g',    'h',    'j',    'k',    'l',    ';',
	'\'',   '`',  K_SHIFT,  '\\',   'z',    'x',    'c',    'v',	// 2
	'b',    'n',    'm',    ',',    '.',    '/',  K_SHIFT, K_KP_STAR,
	K_ALT,  ' ',     0 ,    K_F1,   K_F2,   K_F3,   K_F4,  K_F5,	// 3
	K_F6,  K_F7,   K_F8,    K_F9,  K_F10, K_PAUSE,   0 , K_HOME,
	K_UPARROW,K_PGUP,K_KP_MINUS,K_LEFTARROW,K_KP_5,K_RIGHTARROW,K_KP_PLUS,K_END,	// 4
	K_DOWNARROW,K_PGDN,K_INS,K_DEL,   0 ,    0 ,     0 ,  K_F11,
	K_F12,   0 ,     0 ,     0 ,      0 ,    0 ,     0 ,     0 ,	// 5
	0  ,     0 ,     0 ,     0 ,      0 ,    0 ,     0 ,     0 ,
	0  ,     0 ,     0 ,     0 ,      0 ,    0 ,     0 ,     0 ,	// 6
	0  ,     0 ,     0 ,     0 ,      0 ,    0 ,     0 ,     0 ,
	0  ,     0 ,     0 ,     0 ,      0 ,    0 ,     0 ,     0	// 7
};

#if 0	/* not used */
static byte shiftscantokey[128] =
{
//	0       1       2       3       4       5       6       7
//	8       9       A       B       C       D       E       F
	0  ,    27,     '!',    '@',    '#',    '$',    '%',    '^',
	'&',    '*',    '(',    ')',    '_',    '+', K_BACKSPACE, 9,	// 0
	'Q',    'W',    'E',    'R',    'T',    'Y',    'U',    'I',
	'O',    'P',    '{',    '}',  K_ENTER, K_CTRL,  'A',    'S',	// 1
	'D',    'F',    'G',    'H',    'J',    'K',    'L',    ':',
	'"' ,    '~', K_SHIFT,  '|',    'Z',    'X',    'C',    'V',	// 2
	'B',    'N',    'M',    '<',    '>',    '?',  K_SHIFT, K_KP_STAR,
	K_ALT,  ' ',     0 ,    K_F1,   K_F2,   K_F3,   K_F4,  K_F5,	// 3
	K_F6,  K_F7,   K_F8,    K_F9,  K_F10, K_PAUSE,   0 , K_HOME,
	K_UPARROW,K_PGUP,K_KP_MINUS,K_LEFTARROW,K_KP_5,K_RIGHTARROW,K_KP_PLUS,K_END,	// 4
	K_DOWNARROW,K_PGDN,K_INS,K_DEL,   0 ,    0 ,     0 ,  K_F11,
	K_F12,   0 ,     0 ,     0 ,      0 ,    0 ,     0 ,     0 ,	// 5
	0  ,     0 ,     0 ,     0 ,      0 ,    0 ,     0 ,     0 ,
	0  ,     0 ,     0 ,     0 ,      0 ,    0 ,     0 ,     0 ,	// 6
	0  ,     0 ,     0 ,     0 ,      0 ,    0 ,     0 ,     0 ,
	0  ,     0 ,     0 ,     0 ,      0 ,    0 ,     0 ,     0	// 7
};
#endif

static int MapKey (int key)
{
	int result = (key >> 16) & 255;

	if (result > 127)
		return 0;
	result = scantokey[result];

	if (key & (1 << 24)) /* extended */
	{
		switch (result)
		{
		case K_PAUSE:
			return	(Key_IsGameKey()) ? K_KP_NUMLOCK : 0;
		case K_ENTER:
			return	(Key_IsGameKey()) ? K_KP_ENTER : K_ENTER;
		case '/':
			return	(Key_IsGameKey()) ? K_KP_SLASH : '/';
		}
	}
	else /* standart */
	{
		switch (result)
		{
		case K_KP_STAR:
			return	(Key_IsGameKey()) ? K_KP_STAR : '*';
		case K_KP_PLUS:
			return	(Key_IsGameKey()) ? K_KP_PLUS : '+';
		case K_KP_MINUS:
			return	(Key_IsGameKey()) ? K_KP_MINUS : '-';
		case K_HOME:
			return	(Key_IsGameKey()) ? K_KP_HOME :
				(GetKeyState(VK_NUMLOCK) & 0x01) ? '7' : K_HOME;
		case K_UPARROW:
			return	(Key_IsGameKey()) ? K_KP_UPARROW :
				(GetKeyState(VK_NUMLOCK) & 0x01) ? '8' : K_UPARROW;
		case K_PGUP:
			return	(Key_IsGameKey()) ? K_KP_PGUP :
				(GetKeyState(VK_NUMLOCK) & 0x01) ? '9' : K_PGUP;
		case K_LEFTARROW:
			return	(Key_IsGameKey()) ? K_KP_LEFTARROW :
				(GetKeyState(VK_NUMLOCK) & 0x01) ? '4' : K_LEFTARROW;
		case K_KP_5:
			return	(Key_IsGameKey()) ? K_KP_5 : '5';
		case K_RIGHTARROW:
			return	(Key_IsGameKey()) ? K_KP_RIGHTARROW :
				(GetKeyState(VK_NUMLOCK) & 0x01) ? '6' : K_RIGHTARROW;
		case K_END:
			return	(Key_IsGameKey()) ? K_KP_END :
				(GetKeyState(VK_NUMLOCK) & 0x01) ? '1' : K_END;
		case K_DOWNARROW:
			return	(Key_IsGameKey()) ? K_KP_DOWNARROW :
				(GetKeyState(VK_NUMLOCK) & 0x01) ? '2' : K_DOWNARROW;
		case K_PGDN:
			return	(Key_IsGameKey()) ? K_KP_PGDN :
				(GetKeyState(VK_NUMLOCK) & 0x01) ? '3' : K_PGDN;
		case K_INS:
			return	(Key_IsGameKey()) ? K_KP_INS :
				(GetKeyState(VK_NUMLOCK) & 0x01) ? '0' : K_INS;
		case K_DEL:
			return	(Key_IsGameKey()) ? K_KP_DEL :
				(GetKeyState(VK_NUMLOCK) & 0x01) ? '.' : K_DEL;
		}
	}

	return result;
}


static void AppActivate (BOOL fActive, BOOL minimize)
/****************************************************************************
*
* Function:	AppActivate
* Parameters:	fActive - True if app is activating
*
* Description:	If the application is activating, then swap the system
*		into SYSPAL_NOSTATIC mode so that our palettes will display
*		correctly.
*
****************************************************************************/
{
	HDC			hdc;
	int			i;
	qboolean		t;
	static qboolean	sound_active;

	ActiveApp = fActive;

// messy, but it seems to work
	if (vid_fulldib_on_focus_mode)
	{
		Minimized = minimize;

		if (Minimized)
			ActiveApp = false;
	}

	if (vid_initialized)
	{
	// yield the palette if we're losing the focus
		hdc = GetDC(NULL);

		if (!Minimized)
			VID_SetPalette (vid_curpal);

		scr_fullupdate = 0;

		ReleaseDC(NULL,hdc);
	}

// enable/disable sound on focus gain/loss
	if (!ActiveApp && sound_active)
	{
		S_BlockSound ();
		sound_active = false;
	}
	else if (ActiveApp && !sound_active)
	{
		S_UnblockSound ();
		sound_active = true;
	}

// minimize/restore fulldib windows/mouse-capture normal windows on demand
	if (!in_mode_set)
	{
		if (ActiveApp)
		{
			if (vid_fulldib_on_focus_mode)
			{
				if (vid_initialized)
				{
					msg_suppress_1 = true;	// don't want to see normal mode set message
					VID_SetMode (vid_fulldib_on_focus_mode, vid_curpal);
					msg_suppress_1 = false;

					t = in_mode_set;
					in_mode_set = true;
					AppActivate (true, false);
					in_mode_set = t;
				}

				IN_ActivateMouse ();
				IN_HideMouse ();
			}
			else if (modestate == MS_WINDOWED && _enable_mouse.integer)
			{
			// with winmouse, we may fail having our
			// window back from the iconified state. yuck...
				if (dinput_init)
				{
					IN_ActivateMouse ();
					IN_HideMouse ();
				}
			}
		}

		if (!ActiveApp)
		{
			if (modestate == MS_FULLDIB)
			{
				if (vid_initialized)
				{
					force_minimized = true;
					i = vid_fulldib_on_focus_mode;
					msg_suppress_1 = true;	// don't want to see normal mode set message
					VID_SetMode (windowed_default, vid_curpal);
					msg_suppress_1 = false;
					vid_fulldib_on_focus_mode = i;
					force_minimized = false;

				// we never seem to get WM_ACTIVATE inactive from this mode set, so we'll
				// do it manually
					t = in_mode_set;
					in_mode_set = true;
					AppActivate (false, true);
					in_mode_set = t;
				}

				IN_DeactivateMouse ();
				IN_ShowMouse ();
			}
			else if (modestate == MS_WINDOWED && _enable_mouse.integer)
			{
				IN_DeactivateMouse ();
				IN_ShowMouse ();
			}
		}
	}
}


/*
================
VID_HandlePause
================
*/
void VID_HandlePause (qboolean paused)
{
	if (modestate == MS_WINDOWED && _enable_mouse.integer)
	{
		if (paused)
		{
			IN_DeactivateMouse ();
			IN_ShowMouse ();
		}
		else
		{
			IN_ActivateMouse ();
			IN_HideMouse ();
		}
	}
}


void VID_ToggleFullscreen (void)
{
}


/*
===================================================================

MAIN WINDOW

===================================================================
*/

static int	MWheelAccumulator;
static UINT	uMSG_MOUSEWHEEL = 0;
extern cvar_t	mwheelthreshold;

/* main window procedure */
static LRESULT WINAPI MainWndProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT	ret = 0;
	int	fActive, fMinimized, temp;
	HDC		hdc;
	PAINTSTRUCT	ps;

	if (uMSG_MOUSEWHEEL && uMsg == uMSG_MOUSEWHEEL)
	{
		/* Win95/WinNT-3.51 code using MSH_MOUSEWHEEL, see:
		 * http://msdn.microsoft.com/en-us/library/ms645617.aspx */
		if (mwheelthreshold.integer >= 1)
		{
			MWheelAccumulator += (int) wParam;
			while (MWheelAccumulator >= mwheelthreshold.integer)
			{
				Key_Event(K_MWHEELUP, true);
				Key_Event(K_MWHEELUP, false);
				MWheelAccumulator -= mwheelthreshold.integer;
			}
			while (MWheelAccumulator <= -mwheelthreshold.integer)
			{
				Key_Event(K_MWHEELDOWN, true);
				Key_Event(K_MWHEELDOWN, false);
				MWheelAccumulator += mwheelthreshold.integer;
			}
		}
		return DefWindowProc (hWnd, uMsg, wParam, lParam);
	}

	switch (uMsg)
	{
	case WM_CREATE:
		if (Win95)
		{
			uMSG_MOUSEWHEEL = RegisterWindowMessage("MSWHEEL_ROLLMSG");
			if (!uMSG_MOUSEWHEEL)
				Con_SafePrintf ("couldn't register mousewheel\n");
		}
		break;

	case WM_SYSCOMMAND:
		// Check for maximize being hit
		switch (wParam & ~0x0F)
		{
		case SC_MAXIMIZE:
		// if minimized, bring up as a window before going fullscreen,
		// so MGL will have the right state to restore
			if (Minimized)
			{
				force_mode_set = true;
				VID_SetMode (vid_modenum, vid_curpal);
				force_mode_set = false;
			}
			VID_SetMode (vid_fullscreen_mode.integer, vid_curpal);
			break;

		case SC_SCREENSAVE:
		case SC_MONITORPOWER:
			if (modestate != MS_WINDOWED)
			{
			// don't call DefWindowProc() because we don't want to start
			// the screen saver fullscreen
				break;
			}
		// fall through windowed and allow the screen saver to start
		default:
			if (!in_mode_set)
				S_BlockSound ();
			ret = DefWindowProc (hWnd, uMsg, wParam, lParam);
			if (!in_mode_set)
				S_UnblockSound ();
		}
		break;

	case WM_MOVE:
		window_x = (int) LOWORD(lParam);
		window_y = (int) HIWORD(lParam);
		VID_UpdateWindowStatus ();
		break;

	case WM_SIZE:
		Minimized = false;
		if (!(wParam & SIZE_RESTORED))
		{
			if (wParam & SIZE_MINIMIZED)
				Minimized = true;
		}
		break;

	case WM_SYSCHAR:
		// keep Alt-Space from happening
		break;

	case WM_ACTIVATE:
		fActive = LOWORD(wParam);
		fMinimized = (BOOL) HIWORD(wParam);
		AppActivate(!(fActive == WA_INACTIVE), fMinimized);
		// fix the leftover Alt from any Alt-Tab or the like that switched us away
		ClearAllStates ();
		if (!in_mode_set)
			VID_SetPalette(vid_curpal);
		break;

	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		if (!in_mode_set && host_initialized)
			SCR_UpdateWholeScreen ();
		EndPaint(hWnd, &ps);
		break;

	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		if (in_mode_set)
			break;
		Key_Event (MapKey(lParam), true);
		break;

	case WM_KEYUP:
	case WM_SYSKEYUP:
		if (in_mode_set)
			break;
		Key_Event (MapKey(lParam), false);
		break;

	// this is complicated because Win32 seems to pack multiple mouse
	// events into one update sometimes, so we always check all states
	// and look for events
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_XBUTTONDOWN:
	case WM_XBUTTONUP:
	case WM_MOUSEMOVE:
		if (in_mode_set)
			break;

		temp = 0;

		if (wParam & MK_LBUTTON)
			temp |= 1;

		if (wParam & MK_RBUTTON)
			temp |= 2;

		if (wParam & MK_MBUTTON)
			temp |= 4;

		// intellimouse explorer
		if (wParam & MK_XBUTTON1)
			temp |= 8;

		if (wParam & MK_XBUTTON2)
			temp |= 16;

		IN_MouseEvent (temp);

		break;

	case WM_MOUSEWHEEL:
		if (in_mode_set)
			return 0;
		if ((short) HIWORD(wParam) > 0)
		{
			Key_Event(K_MWHEELUP, true);
			Key_Event(K_MWHEELUP, false);
		}
		else
		{
			Key_Event(K_MWHEELDOWN, true);
			Key_Event(K_MWHEELDOWN, false);
		}
		return 0;

	case WM_DISPLAYCHANGE:
		if (!in_mode_set && (modestate == MS_WINDOWED) && !vid_fulldib_on_focus_mode)
		{
			force_mode_set = true;
			VID_SetMode (vid_modenum, vid_curpal);
			force_mode_set = false;
		}
		break;

	case WM_CLOSE:
		if (in_mode_set)
			break;
		// this causes Close in the right-click task bar menu not to
		// work, but right now bad things happen if Close is handled
		// in that case (garbage and a crash on Win95)
		if (MessageBox (mainwindow, "Are you sure you want to quit?", "Confirm Exit",
					MB_YESNO | MB_SETFOREGROUND | MB_ICONQUESTION) == IDYES)
		{
			Sys_Quit ();
		}
		break;

	case MM_MCINOTIFY:
#if !defined(_NO_CDAUDIO)
		ret = CDAudio_MessageHandler (hWnd, uMsg, wParam, lParam);
#endif	/* ! _NO_CDAUDIO */
		break;

	default:
		/* pass all unhandled messages to DefWindowProc */
		ret = DefWindowProc (hWnd, uMsg, wParam, lParam);
		break;
	}

	/* return 1 if handled message, 0 if not */
	return ret;
}


//========================================================
// Video menu stuff
//========================================================

static int	vid_line, vid_wmodes;

typedef struct
{
	int		modenum;
	const char	*desc;
	int		iscur;
	int		width;
} modedesc_t;

#define MAX_COLUMN_SIZE		5
#define MODE_AREA_HEIGHT	(MAX_COLUMN_SIZE + 6)
#define MAX_MODEDESCS		(MAX_COLUMN_SIZE * 3)

static modedesc_t	modedescs[MAX_MODEDESCS];
static const char	no_desc[] = "N/A";

/*
================
VID_MenuDraw
================
*/
static void VID_MenuDraw (void)
{
	const char	*ptr;
	int			lnummodes, i, j, k, column, row, dup, dupmode;
	char		temp[100];
	vmode_t		*pv;
	modedesc_t	tmodedesc;

	ScrollTitle("gfx/menu/title7.lmp");

	for (i = 0; i < 3; i++)
	{
		ptr = VID_GetModeDescriptionMemCheck (i);
		modedescs[i].modenum = modelist[i].modenum;
		modedescs[i].desc = ptr ? ptr : no_desc;
		modedescs[i].iscur = 0;

		if (vid_modenum == i)
			modedescs[i].iscur = 1;
	}

	vid_wmodes = 3;
	lnummodes = VID_NumModes ();

	for (i = 3; i < lnummodes; i++)
	{
		ptr = VID_GetModeDescriptionMemCheck (i);
		pv = VID_GetModePtr (i);

	// we only have room for 15 fullscreen modes, so don't allow
	// 360-wide modes, because if there are 5 320-wide modes and
	// 5 360-wide modes, we'll run out of space
		if (ptr && ((pv->width != 360) || COM_CheckParm("-allow360")))
		{
			dup = 0;

			for (j = 3; j < vid_wmodes; j++)
			{
				if (!strcmp (modedescs[j].desc, ptr))
				{
					dup = 1;
					dupmode = j;
					break;
				}
			}

			if (dup || (vid_wmodes < MAX_MODEDESCS))
			{
				if (!dup || COM_CheckParm("-noforcevga"))
				{
					if (dup)
					{
						k = dupmode;
					}
					else
					{
						k = vid_wmodes;
					}

					modedescs[k].modenum = i;
					modedescs[k].desc = ptr;
					modedescs[k].iscur = 0;
					modedescs[k].width = pv->width;

					if (i == vid_modenum)
						modedescs[k].iscur = 1;

					if (!dup)
						vid_wmodes++;
				}
			}
		}
	}

// sort the modes on width (to handle picking up oddball dibonly modes
// after all the others)
	for (i = 3; i< (vid_wmodes-1); i++)
	{
		for (j = (i+1); j < vid_wmodes; j++)
		{
			if (modedescs[i].width > modedescs[j].width)
			{
				tmodedesc = modedescs[i];
				modedescs[i] = modedescs[j];
				modedescs[j] = tmodedesc;
			}
		}
	}


	M_Print (13*8, 60, "Windowed Modes");

	column = 16;
	row = 60+2*8;

	for (i = 0; i < 3; i++)
	{
		if (modedescs[i].iscur)
			M_PrintWhite (column, row, modedescs[i].desc);
		else
			M_Print (column, row, modedescs[i].desc);

		column += 13*8;
	}

	if (vid_wmodes > 3)
	{
		M_Print (12*8, 60+4*8, "Fullscreen Modes");

		column = 16;
		row = 60+6*8;

		for (i = 3; i < vid_wmodes; i++)
		{
			if (modedescs[i].iscur)
				M_PrintWhite (column, row, modedescs[i].desc);
			else
				M_Print (column, row, modedescs[i].desc);

			column += 13*8;

			if (((i - 3) % VID_ROW_SIZE) == (VID_ROW_SIZE - 1))
			{
				column = 16;
				row += 8;
			}
		}
	}

// line cursor
	if (vid_testingmode)
	{
		q_snprintf (temp, sizeof(temp), "TESTING %s",
				modedescs[vid_line].desc);
		M_Print (13*8, 60 + MODE_AREA_HEIGHT * 8 + 8*4, temp);
		M_Print (9*8, 60 + MODE_AREA_HEIGHT * 8 + 8*6,
				"Please wait 5 seconds...");
	}
	else
	{
		M_Print (9*8, 60 + MODE_AREA_HEIGHT * 8,
				"Press Enter to set mode");
		M_Print (6*8, 60 + MODE_AREA_HEIGHT * 8 + 8*1,
				"T to test mode for 5 seconds");
		ptr = VID_GetModeDescription2 (vid_modenum);

		if (ptr)
		{
			q_snprintf (temp, sizeof(temp), "D to set default: %s", ptr);
			M_Print (2*8, 60 + MODE_AREA_HEIGHT * 8 + 8*2, temp);
		}

		ptr = VID_GetModeDescription2 (_vid_default_mode_win.integer);

		if (ptr)
		{
			q_snprintf (temp, sizeof(temp), "Current default: %s", ptr);
			M_Print (3*8, 60 + MODE_AREA_HEIGHT * 8 + 8*5, temp);
		}

		M_Print (15*8, 60 + MODE_AREA_HEIGHT * 8 + 8*6,
				"Esc to exit");

		row = 60 + 2*8 + (vid_line / VID_ROW_SIZE) * 8;
		column = 8 + (vid_line % VID_ROW_SIZE) * 13*8;

		if (vid_line >= 3)
			row += 3*8;

		M_DrawCharacter (column, row, 12+((int)(realtime*4)&1));
	}
}


/*
================
VID_MenuKey
================
*/
static void VID_MenuKey (int key)
{
	if (vid_testingmode)
		return;

	switch (key)
	{
	case K_ESCAPE:
		S_LocalSound ("raven/menu1.wav");
		M_Menu_Options_f ();
		break;

	case K_LEFTARROW:
		S_LocalSound ("raven/menu1.wav");
		vid_line = ((vid_line / VID_ROW_SIZE) * VID_ROW_SIZE) + ((vid_line + 2) % VID_ROW_SIZE);
		if (vid_line >= vid_wmodes)
			vid_line = vid_wmodes - 1;
		break;

	case K_RIGHTARROW:
		S_LocalSound ("raven/menu1.wav");
		vid_line = ((vid_line / VID_ROW_SIZE) * VID_ROW_SIZE) + ((vid_line + 4) % VID_ROW_SIZE);
		if (vid_line >= vid_wmodes)
			vid_line = (vid_line / VID_ROW_SIZE) * VID_ROW_SIZE;
		break;

	case K_UPARROW:
		S_LocalSound ("raven/menu1.wav");
		vid_line -= VID_ROW_SIZE;
		if (vid_line < 0)
		{
			vid_line += ((vid_wmodes + (VID_ROW_SIZE - 1)) / VID_ROW_SIZE) * VID_ROW_SIZE;
			while (vid_line >= vid_wmodes)
				vid_line -= VID_ROW_SIZE;
		}
		break;

	case K_DOWNARROW:
		S_LocalSound ("raven/menu1.wav");
		vid_line += VID_ROW_SIZE;
		if (vid_line >= vid_wmodes)
		{
			vid_line -= ((vid_wmodes + (VID_ROW_SIZE - 1)) / VID_ROW_SIZE) * VID_ROW_SIZE;
			while (vid_line < 0)
				vid_line += VID_ROW_SIZE;
		}
		break;

	case K_ENTER:
		S_LocalSound ("raven/menu1.wav");
		VID_SetMode (modedescs[vid_line].modenum, vid_curpal);
		break;

	case 'T':
	case 't':
		S_LocalSound ("raven/menu1.wav");
	// have to set this before setting the mode because WM_PAINT
	// happens during the mode set and does a VID_Update, which
	// checks vid_testingmode
		vid_testingmode = 1;
		vid_testendtime = realtime + 5.0;
		if (!VID_SetMode (modedescs[vid_line].modenum, vid_curpal))
			vid_testingmode = 0;
		break;

	case 'D':
	case 'd':
		S_LocalSound ("raven/menu1.wav");
		firstupdate = false;
		Cvar_SetValueQuick (&_vid_default_mode_win, vid_modenum);
		break;

	default:
		break;
	}
}

