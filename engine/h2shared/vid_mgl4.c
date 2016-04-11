/*
 * vid_mgl4.c -- Win32 video driver using SciTech MGL-4.05
 * This is X86-WIN32 only: No WIN64 because no 64 bit MGL.
 * $Id$
 *
 * Copyright (C) 1996-1997  Id Software, Inc.
 * Copyright (C) 1997-1998  Raven Software Corp.
 * Original MGL3 to MGL4 port by: Pa3PyX <pa3pyx87405703@icqmail.com>
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
#include "winquake.h"
#include "bgmusic.h"
#include "cdaudio.h"
#include "cfgfile.h"
#include <mmsystem.h>
#include "d_local.h"
#include "resource.h"
#include <ddraw.h>
#if defined(_MSC_VER)
#pragma warning(disable:4229)	/* mgraph gets this */
#endif	/* _MSC_VER */
#include <mgraph.h>

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


/* New variables (Pa3PyX) */
static LONG_PTR		mgl_wnd_proc;
static MGL_surfaceAccessFlagsType	mgldcAccessMode = MGL_NO_ACCESS,
					memdcAccessMode = MGL_NO_ACCESS;
static int				mgldcWidth = 0, memdcWidth = 0;

byte		globalcolormap[VID_GRADES*256], lastglobalcolor = 0;
byte		*lastsourcecolormap = NULL;

HWND		mainwindow;
qboolean	DDActive;
qboolean	msg_suppress_1 = false;

static qboolean	dibonly;

static int	DIBWidth, DIBHeight;
static RECT	WindowRect;
static LONG	WindowStyle, ExWindowStyle;

int		window_center_x, window_center_y, window_x, window_y, window_width, window_height;
RECT		window_rect;

static DEVMODE	gdevmode;
static qboolean	startwindowed = false;
static qboolean	firstupdate = true;
static qboolean	vid_initialized = false, vid_palettized;
static int	lockcount;
static int	vid_fulldib_on_focus_mode;
static qboolean	force_minimized, is_mode0x13, force_mode_set;
static int	vid_stretched, enable_mouse;
static qboolean	palette_changed, syscolchg, vid_mode_set, pal_is_nostatic;
static HICON	hIcon;

viddef_t	vid;		// global video state
qboolean	in_mode_set;

// 0 is MODE_WINDOWED, 3 is MODE_FULLSCREEN_DEFAULT
static	cvar_t	vid_mode = {"vid_mode", "0", CVAR_NONE};
static	cvar_t	_vid_default_mode_win = {"_vid_default_mode_win", "3", CVAR_ARCHIVE};
// compatibility with dos version:
static	cvar_t	_vid_default_mode = {"_vid_default_mode", "0", CVAR_ARCHIVE};
// compatibility with gl version:
static	cvar_t	vid_config_glx = {"vid_config_glx", "640", CVAR_ARCHIVE};
static	cvar_t	vid_config_fscr  = {"vid_config_fscr", "1", CVAR_ARCHIVE};
static	cvar_t	vid_config_gly = {"vid_config_gly", "480", CVAR_ARCHIVE};

static	cvar_t	vid_wait = {"vid_wait", "-1", CVAR_ARCHIVE};	// autodetect
/* Pa3PyX: vid_maxpages: new variable: maximum number of video pages to use.
   The more, the less chance there is to flicker, the better, but a lot of
   VESA drivers are buggy and report more video pages than there actually are.
   Thus, we limit this number to 3 by default (this was hardcoded before),
   and then let the user pick whatever value they wish. */
static	cvar_t	vid_maxpages = {"vid_maxpages", "3", CVAR_ARCHIVE};
/* Pa3PyX: vid_nopageflip now has meaning (was defunct in previous versions)
   and defaults to 1. Reason: page flipping with direct linear framebuffer
   access was fast for Quake which did not READ anything from frame buffer.
   Hexen II does that A LOT, on transparent surfaces and the likes. Writing
   directly into LFB is fast, but reading directly from it is generally
   extremely slow - hence huge slowdowns on translucencies. Thus, we now
   default to software buffering in VESA and DirectDraw modes, just like in
   DIB, but with the exception that we can copy from software buffer to LFB
   directly once the frame is drawn. This makes VESA/DirectDraw modes slightly
   faster than DIB. DIB still remains the default for now, for compatibility
   purposes (because it's only slightly slower, and VESA modes are usually
   more trouble than they are worth, especially in Windows) */
static	cvar_t	vid_nopageflip = {"vid_nopageflip", "1", CVAR_ARCHIVE};
static	cvar_t	vid_config_x = {"vid_config_x", "800", CVAR_ARCHIVE};
static	cvar_t	vid_config_y = {"vid_config_y", "600", CVAR_ARCHIVE};
static	cvar_t	vid_stretch_by_2 = {"vid_stretch_by_2", "1", CVAR_ARCHIVE};
static	cvar_t	vid_fullscreen_mode = {"vid_fullscreen_mode", "3", CVAR_ARCHIVE};
static	cvar_t	vid_windowed_mode = {"vid_windowed_mode", "0", CVAR_ARCHIVE};

cvar_t		_enable_mouse = {"_enable_mouse", "0", CVAR_ARCHIVE};

typedef struct {
	int		width;
	int		height;
} lmode_t;

static lmode_t lowresmodes[] = {
	{320, 200},
	{320, 240},
	{400, 300},
	{512, 384},
};

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

static int	driver = grDETECT, mode;

static qboolean	useWinDirect = false;
static qboolean	useDirectDraw= false;

static MGLDC	*mgldc = NULL, *memdc = NULL, *dibdc = NULL, *windc = NULL;

typedef struct {
	modestate_t	type;
	int			width;
	int			height;
	int			modenum;
	int			mode13;
	int			stretched;
	int			dib;
	int			fullscreen;
	int			bpp;
	int			halfscreen;
	char		modedesc[MAX_DESC];
} vmode_t;

static vmode_t	modelist[MAX_MODE_LIST];
static int		nummodes;

static int		aPage;	// Current active display page
static int		vPage;	// Current visible display page

/* Pa3PyX: in MGL 4.05, thare are three values for waitVRT

	triple buffer (0),
	wait for vertical retrace (1),
	and don't wait (2).

   Triple buffering does not always work. So we will again
   use vid_wait console variable (like in Quake DOS version)
   for the user to supply desired page flipping mode for their
   graphics hardware. Like in Quake, "0" would mean no wait,
   "1" wait for vertical retrace, "2" triple buffer, and now
   "-1" will mean autodetect.
*/
static MGL_waitVRTFlagType	waitVRT		= MGL_waitVRT,
				defaultVRT	= MGL_waitVRT;

static vmode_t	badmode;

//static byte	backingbuf[48*24];
static byte	backingbuf[48*48];

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

static void initFatalError (void)
{
	MGL_exit();
	MGL_fatalError(MGL_errorMsg(MGL_result()));
	exit(EXIT_FAILURE);
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

static int VID_Suspend (MGLDC *dc, int flags)
{
	qboolean	s;

	if (flags & MGL_DEACTIVATE)
	{
		IN_RestoreOriginalMouseState ();
		CDAudio_Pause ();
		BGM_Pause ();

		// keep WM_PAINT from trying to redraw
		in_mode_set = true;
		block_drawing = true;
	}
	else if (flags & MGL_REACTIVATE)
	{
		IN_SetQuakeMouseState ();
		// fix the leftover Alt from any Alt-Tab or the like that switched us away
		ClearAllStates ();
		CDAudio_Resume ();
		BGM_Resume ();
		in_mode_set = false;

		block_drawing = false;
//		vid.recalc_refdef = 1;
		force_mode_set = true;
		s = msg_suppress_1;
		msg_suppress_1 = true;
		VID_Fullscreen_f();
		msg_suppress_1 = s;
		force_mode_set = false;
	}

	return 1;
}


static void registerAllDispDrivers (void)
{
	/* Event though these driver require WinDirect, we register
	 * them so that they will still be available even if DirectDraw
	 * is present and the user has disable the high performance
	 * WinDirect modes.
	 */
	MGL_registerDriver(MGL_VGA8NAME, VGA8_driver);
//	MGL_registerDriver(MGL_VGAXNAME, VGAX_driver);

	/* Register display drivers */
	if (useWinDirect)
	{
	//	MGL_registerDriver(MGL_SVGA8NAME, SVGA8_driver); // we don't want VESA 1.X drivers
		MGL_registerDriver(MGL_LINEAR8NAME, LINEAR8_driver);
		if (!COM_CheckParm ("-novbeaf"))
			MGL_registerDriver(MGL_ACCEL8NAME, ACCEL8_driver);
	}

	if (useDirectDraw)
	{
		MGL_registerDriver(MGL_DDRAW8NAME, DDRAW8_driver);
	}
}


static void registerAllMemDrivers (void)
{
	/* Register memory context drivers */
	MGL_registerDriver(MGL_PACKED8NAME, PACKED8_driver);
}


static void VID_InitMGLFull (HINSTANCE hInstance)
{
	int		i, xRes, yRes, bits, lowres, curmode, temp;
	int		lowstretchedres, stretchedmode, lowstretched;
	uchar		*m;

	if (COM_CheckParm("-usedirectdraw") || COM_CheckParm("-useddraw"))
		useDirectDraw = true;
	if (!WinNT && (COM_CheckParm("-usewindirect") || COM_CheckParm("-usevesa")))
		useWinDirect = true;

	// Initialise the MGL
	MGL_unregisterAllDrivers();
	registerAllDispDrivers();
	registerAllMemDrivers();
	MGL_detectGraph(&driver,&mode);
	m = MGL_availableModes();

	if (m[0] != 0xFF)
	{
		lowres = lowstretchedres = 99999;
		lowstretched = 0;
		curmode = 0;
		stretchedmode = 0;	// avoid compiler warning

	// find the lowest-res mode, or a mode we can stretch up to and get
	// lowest-res that way
		for (i = 0; m[i] != 0xFF; i++)
		{
			MGL_modeResolution(m[i], &xRes, &yRes,&bits);

			if ((bits == 8) &&
				(xRes <= MAXWIDTH) &&
				(yRes <= MAXHEIGHT) &&
				(curmode < MAX_MODE_LIST))
			{
				if (m[i] == grVGA_320x200x256)
					is_mode0x13 = true;

				if (!COM_CheckParm("-noforcevga"))
				{
					if (m[i] == grVGA_320x200x256)
					{
						mode = i;
						break;
					}
				}

				if (xRes < lowres)
				{
					lowres = xRes;
					mode = i;
				}

				if ((xRes < lowstretchedres) && ((xRes >> 1) >= 320))
				{
					lowstretchedres = xRes >> 1;
					stretchedmode = i;
				}
			}

			curmode++;
		}

	// if there's a mode we can stretch by 2 up to, thereby effectively getting
	// a lower-res mode than the lowest-res real but still at least 320x200, that
	// will be our default mode
		if (lowstretchedres < lowres)
		{
			mode = stretchedmode;
			lowres = lowstretchedres;
			lowstretched = 1;
		}

	// build the mode list, leaving room for the low-res stretched mode, if any
		nummodes++;		// leave room for default mode

		for (i = 0; m[i] != 0xFF; i++)
		{
			MGL_modeResolution(m[i], &xRes, &yRes,&bits);

			if ((bits == 8) &&
				(xRes <= MAXWIDTH) &&
				(yRes <= MAXHEIGHT) &&
				(nummodes < MAX_MODE_LIST))
			{
				if (i == mode)
				{
					if (lowstretched)
					{
						stretchedmode = nummodes;
						curmode = nummodes++;
					}
					else
					{
						curmode = MODE_FULLSCREEN_DEFAULT;
					}
				}
				else
				{
					curmode = nummodes++;
				}

				modelist[curmode].type = MS_FULLSCREEN;
				modelist[curmode].width = xRes;
				modelist[curmode].height = yRes;
				q_snprintf (modelist[curmode].modedesc, MAX_DESC, "%dx%d", xRes, yRes);

				if (m[i] == grVGA_320x200x256)
					modelist[curmode].mode13 = 1;
				else
					modelist[curmode].mode13 = 0;

				modelist[curmode].modenum = m[i];
				modelist[curmode].stretched = 0;
				modelist[curmode].dib = 0;
				modelist[curmode].fullscreen = 1;
				modelist[curmode].halfscreen = 0;
				modelist[curmode].bpp = 8;
			}
		}

		if (lowstretched)
		{
			modelist[MODE_FULLSCREEN_DEFAULT] = modelist[stretchedmode];
			modelist[MODE_FULLSCREEN_DEFAULT].stretched = 1;
			modelist[MODE_FULLSCREEN_DEFAULT].width >>= 1;
			modelist[MODE_FULLSCREEN_DEFAULT].height >>= 1;
			q_snprintf (modelist[MODE_FULLSCREEN_DEFAULT].modedesc, MAX_DESC, "%dx%d",
					 modelist[MODE_FULLSCREEN_DEFAULT].width,
					 modelist[MODE_FULLSCREEN_DEFAULT].height);
		}

		vid_default = MODE_FULLSCREEN_DEFAULT;

		temp = m[0];

		if (!MGL_init(&driver, &temp, ""))
		{
			initFatalError();
		}
	}

	MGL_setSuspendAppCallback(VID_Suspend);
}


/****************************************************************************
*
* Function:	createDisplayDC
* Returns:	Pointer to the MGL device context to use for the application
*
* Description:	Initialises the MGL and creates an appropriate display
*		device context to be used by the GUI. This creates and
*		apropriate device context depending on the system being
*		compile for, and should be the only place where system
*		specific code is required.
*
****************************************************************************/
static MGLDC *createDisplayDC (int forcemem)
{
	MGLDC		*dc;
	pixel_format_t	pf;
	int		npages;

	// Start the specified video mode
	if (!MGL_changeDisplayMode(mode))
		initFatalError();

	npages = MGL_availablePages(mode);

#if 0
	if (npages > 3)
		npages = 3;

	if (!COM_CheckParm ("-notriplebuf"))
	{
		if (npages > 2)
		{
			npages = 2;
		}
	}
#endif

	if ((dc = MGL_createDisplayDC(npages)) == NULL)
		return NULL;

	// Pa3PyX: check if the user wants to do page flips (default: no)
	if (!vid_nopageflip.integer && (vid_maxpages.integer > 1) &&
	    !forcemem && (MGL_surfaceAccessType(dc) == MGL_LINEAR_ACCESS) &&
	    (dc->mi.maxPage > 0))
	{
		// Page flipping used
		MGL_makeCurrentDC(dc);
		memdc = NULL;
		vid.numpages = dc->mi.maxPage + 1;
		if (vid.numpages > vid_maxpages.integer)
			vid.numpages = vid_maxpages.integer;
		MGL_setActivePage(dc, aPage = 1);
		MGL_setVisualPage(dc, vPage = 0, false);
	}
	else
	{
		// No page flipping
		memdc = MGL_createMemoryDC(MGL_sizex(dc)+1, MGL_sizey(dc)+1, 8, &pf);
		MGL_makeCurrentDC(memdc);
		/* Pa3PyX: No page flipping on blitted modes anymore
		   (no need to - we are drawing everything to system
		   memory first, then flushing it to screen in one blow.
		   so there is no visible overdraw, even though writing
		   directly to front buffer) */
		vid.numpages = 1;
		aPage = vPage = 0;
		MGL_setActivePage(dc, aPage);
		MGL_setVisualPage(dc, vPage, false);
	}
	/* Pa3PyX: these will be needed if we want to copy surface
	   to surface directly (faster, but need to make sure both
	   contexts are accessible in either virtualized or linear
	   mode (not MGL_NO_ACCESS == 0) */
	if (dc)
	{
		mgldcAccessMode = (MGL_surfaceAccessFlagsType) MGL_surfaceAccessType(dc);
		mgldcWidth = dc->mi.bytesPerLine * (dc->mi.bitsPerPixel / 8);
	}
	if (memdc)
	{
		memdcAccessMode = (MGL_surfaceAccessFlagsType) MGL_surfaceAccessType(memdc);
		memdcWidth = memdc->mi.bytesPerLine * (memdc->mi.bitsPerPixel / 8);
	}

	if (vid.numpages > 2)
		defaultVRT = MGL_tripleBuffer;
	else if (vid.numpages == 2)
		defaultVRT = MGL_waitVRT;	// regular double buffering/vsync
	else	// only 1 video page (aPage == vPage), software buffer, no need to sync
		defaultVRT = MGL_dontWait;

	return dc;
}


static void VID_RegisterWndClass (HINSTANCE hInstance)
{
	WNDCLASS	wc;

	wc.style		= 0;
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

static void VID_InitMGLDIB (HINSTANCE hInstance)
{
	HDC		hdc;

	hIcon = LoadIcon (hInstance, MAKEINTRESOURCE (IDI_ICON2));

	/* Register the frame class */
	VID_RegisterWndClass(hInstance);

	/* Find the size for the DIB window */
	/* Initialise the MGL for windowed operation */
	MGL_setAppInstance(hInstance);
	registerAllMemDrivers();
	MGL_initWindowed("");

	modelist[0].type = MS_WINDOWED;
	modelist[0].width = 320;
	modelist[0].height = 240;
	q_strlcpy (modelist[0].modedesc, "320x240", MAX_DESC);
	modelist[0].mode13 = 0;
	modelist[0].modenum = MODE_WINDOWED;
	modelist[0].stretched = 0;
	modelist[0].dib = 1;
	modelist[0].fullscreen = 0;
	modelist[0].halfscreen = 0;
	modelist[0].bpp = 8;

	modelist[1].type = MS_WINDOWED;
	modelist[1].width = 640;
	modelist[1].height = 480;
	q_strlcpy (modelist[1].modedesc, "640x480", MAX_DESC);
	modelist[1].mode13 = 0;
	modelist[1].modenum = MODE_WINDOWED + 1;
	modelist[1].stretched = 1;
	modelist[1].dib = 1;
	modelist[1].fullscreen = 0;
	modelist[1].halfscreen = 0;
	modelist[1].bpp = 8;

	modelist[2].type = MS_WINDOWED;
	modelist[2].width = 800;
	modelist[2].height = 600;
	q_strlcpy (modelist[2].modedesc, "800x600", MAX_DESC);
	modelist[2].mode13 = 0;
	modelist[2].modenum = MODE_WINDOWED + 2;
	modelist[2].stretched = 1;
	modelist[2].dib = 1;
	modelist[2].fullscreen = 0;
	modelist[2].halfscreen = 0;
	modelist[2].bpp = 8;

// automatically stretch the default mode up if > 640x480 desktop resolution
	hdc = GetDC(NULL);

	if ((GetDeviceCaps(hdc, HORZRES) > 640) && !COM_CheckParm("-noautostretch"))
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

	DDActive = false;
}


/*
=================
VID_InitFullDIB
=================
*/
static void VID_InitFullDIB (HINSTANCE hInstance)
{
	DEVMODE	devmode;
	int		i, j, modenum, existingmode, originalnummodes, lowestres;
	int		numlowresmodes, bpp, done;
	int		cstretch, istretch, mstretch;
	BOOL	status;

// enumerate 8 bpp modes
	originalnummodes = nummodes;
	modenum = 0;
	lowestres = 99999;

	do
	{
		status = EnumDisplaySettings (NULL, modenum, &devmode);

		if ((devmode.dmBitsPerPel == 8) &&
			(devmode.dmPelsWidth <= MAXWIDTH) &&
			(devmode.dmPelsHeight <= MAXHEIGHT) &&
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
				modelist[nummodes].mode13 = 0;
				modelist[nummodes].stretched = 0;
				modelist[nummodes].halfscreen = 0;
				modelist[nummodes].dib = 1;
				modelist[nummodes].fullscreen = 1;
				modelist[nummodes].bpp = devmode.dmBitsPerPel;
				q_snprintf (modelist[nummodes].modedesc, MAX_DESC, "%dx%d",
						(int)devmode.dmPelsWidth, (int)devmode.dmPelsHeight);

			// if the width is more than twice the height, reduce it by half because this
			// is probably a dual-screen monitor
				if (!COM_CheckParm("-noadjustaspect"))
				{
					if (modelist[nummodes].width > (modelist[nummodes].height << 1))
					{
						modelist[nummodes].width >>= 1;
						modelist[nummodes].halfscreen = 1;
						q_snprintf (modelist[nummodes].modedesc, MAX_DESC, "%dx%d",
								 modelist[nummodes].width,
								 modelist[nummodes].height);
					}
				}

				for (i = originalnummodes, existingmode = 0; i < nummodes; i++)
				{
					if ((modelist[nummodes].width == modelist[i].width) &&
						(modelist[nummodes].height == modelist[i].height))
					{
						existingmode = 1;
						break;
					}
				}

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

// see if any of them were actually settable; if so, this is our mode list,
// else enumerate all modes; our mode list is whichever ones are settable
// with > 8 bpp
	if (nummodes == originalnummodes)
	{
		modenum = 0;
		lowestres = 99999;

		Con_SafePrintf ("No 8-bpp fullscreen DIB modes found\n");

		do
		{
			status = EnumDisplaySettings (NULL, modenum, &devmode);

			if ((((devmode.dmPelsWidth <= MAXWIDTH) &&
				  (devmode.dmPelsHeight <= MAXHEIGHT)) ||
				 (!COM_CheckParm("-noadjustaspect") &&
				  (devmode.dmPelsWidth <= (MAXWIDTH*2)) &&
				  (devmode.dmPelsWidth > (devmode.dmPelsHeight*2)))) &&
				(nummodes < MAX_MODE_LIST) &&
				(devmode.dmBitsPerPel > 8))
			{
				devmode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

				if (ChangeDisplaySettings (&devmode, CDS_TEST | CDS_FULLSCREEN) ==
						DISP_CHANGE_SUCCESSFUL)
				{
					modelist[nummodes].type = MS_FULLDIB;
					modelist[nummodes].width = devmode.dmPelsWidth;
					modelist[nummodes].height = devmode.dmPelsHeight;
					modelist[nummodes].modenum = 0;
					modelist[nummodes].mode13 = 0;
					modelist[nummodes].stretched = 0;
					modelist[nummodes].halfscreen = 0;
					modelist[nummodes].dib = 1;
					modelist[nummodes].fullscreen = 1;
					modelist[nummodes].bpp = devmode.dmBitsPerPel;
					q_snprintf (modelist[nummodes].modedesc, MAX_DESC, "%dx%d",
							(int)devmode.dmPelsWidth, (int)devmode.dmPelsHeight);

				// if the width is more than twice the height, reduce it by half because this
				// is probably a dual-screen monitor
					if (!COM_CheckParm("-noadjustaspect"))
					{
						if (modelist[nummodes].width > (modelist[nummodes].height*2))
						{
							modelist[nummodes].width >>= 1;
							modelist[nummodes].halfscreen = 1;
							q_snprintf (modelist[nummodes].modedesc, MAX_DESC, "%dx%d",
									 modelist[nummodes].width,
									 modelist[nummodes].height);
						}
					}

					for (i = originalnummodes, existingmode = 0; i < nummodes; i++)
					{
						if ((modelist[nummodes].width == modelist[i].width) &&
							(modelist[nummodes].height == modelist[i].height))
						{
						// pick the lowest available bpp
							if (modelist[nummodes].bpp < modelist[i].bpp)
								modelist[i] = modelist[nummodes];

							existingmode = 1;
							break;
						}
					}

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
	}

// see if there are any low-res modes that aren't being reported
	numlowresmodes = sizeof(lowresmodes) / sizeof(lowresmodes[0]);
	bpp = 8;
	done = 0;

// first make sure the driver doesn't just answer yes to all tests
	devmode.dmBitsPerPel = 8;
	devmode.dmPelsWidth = 42;
	devmode.dmPelsHeight = 37;
	devmode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

	if (ChangeDisplaySettings (&devmode, CDS_TEST | CDS_FULLSCREEN) ==
						DISP_CHANGE_SUCCESSFUL)
	{
		done = 1;
	}

	while (!done)
	{
		for (j = 0; (j < numlowresmodes) && (nummodes < MAX_MODE_LIST); j++)
		{
			devmode.dmBitsPerPel = bpp;
			devmode.dmPelsWidth = lowresmodes[j].width;
			devmode.dmPelsHeight = lowresmodes[j].height;
			devmode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

			if (ChangeDisplaySettings (&devmode, CDS_TEST | CDS_FULLSCREEN) ==
								DISP_CHANGE_SUCCESSFUL)
			{
					modelist[nummodes].type = MS_FULLDIB;
					modelist[nummodes].width = devmode.dmPelsWidth;
					modelist[nummodes].height = devmode.dmPelsHeight;
					modelist[nummodes].modenum = 0;
					modelist[nummodes].mode13 = 0;
					modelist[nummodes].stretched = 0;
					modelist[nummodes].halfscreen = 0;
					modelist[nummodes].dib = 1;
					modelist[nummodes].fullscreen = 1;
					modelist[nummodes].bpp = devmode.dmBitsPerPel;
					q_snprintf (modelist[nummodes].modedesc, MAX_DESC, "%dx%d",
							(int)devmode.dmPelsWidth, (int)devmode.dmPelsHeight);

			// we only want the lowest-bpp version of each mode
				for (i = originalnummodes, existingmode = 0; i < nummodes; i++)
				{
					if ((modelist[nummodes].width == modelist[i].width)   &&
						(modelist[nummodes].height == modelist[i].height) &&
						(modelist[nummodes].bpp >= modelist[i].bpp))
					{
						existingmode = 1;
						break;
					}
				}

				if (!existingmode)
				{
					if (modelist[nummodes].width < lowestres)
						lowestres = modelist[nummodes].width;

					nummodes++;
				}
			}
		}

		switch (bpp)
		{
		case 8:
			bpp = 16;
			break;
		case 16:
			bpp = 32;
			break;
		case 32:
			bpp = 24;
			break;
		case 24:
			done = 1;
			break;
		}
	}

// now add the lowest stretch-by-2 pseudo-modes between 320-wide
// (inclusive) and lowest real res (not inclusive)
// don't bother if we have a real VGA mode 0x13 mode
	if (!is_mode0x13)
	{
		mstretch = 0;	// avoid compiler warning
		for (i = originalnummodes, cstretch = 0; i < nummodes; i++)
		{
			if (((modelist[i].width >> 1) < lowestres) &&
				((modelist[i].width >> 1) >= 320))
			{
				lowestres = modelist[i].width >> 1;
				cstretch = 1;
				mstretch = i;
			}
		}

		if ((nummodes + cstretch) > MAX_MODE_LIST)
			cstretch = MAX_MODE_LIST - nummodes;

		if (cstretch > 0)
		{
			for (i = (nummodes-1); i >= originalnummodes; i--)
				modelist[i+cstretch] = modelist[i];

			nummodes += cstretch;
			istretch = originalnummodes;

			modelist[istretch] = modelist[mstretch];
			modelist[istretch].width >>= 1;
			modelist[istretch].height >>= 1;
			modelist[istretch].stretched = 1;
			q_snprintf (modelist[istretch].modedesc, MAX_DESC, "%dx%d",
					 modelist[istretch].width, modelist[istretch].height);
		}
	}

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
	int		x, y, stretch;

	if (modenum == MODE_SETTABLE_WINDOW)
	{
		modelist[modenum].stretched = vid_stretch_by_2.integer;
		stretch = modelist[modenum].stretched;

		if (vid_config_x.integer < (320 << stretch))
			vid_config_x.integer = 320 << stretch;

		if (vid_config_y.integer < (200 << stretch))
			vid_config_y.integer = 200 << stretch;

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

	// stretched modes are half width/height. Pa3PyX
	if (VID_CheckAdequateMem (pv->width >> pv->stretched, pv->height >> pv->stretched))
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
	if (modelist[modenum].type == MS_FULLSCREEN)
	{
		q_snprintf(pinfo, sizeof(pinfo), "%s fullscreen %s", pv->modedesc,
				MGL_modeDriverName(pv->modenum));
	}
	else if (modelist[modenum].type == MS_FULLDIB)
	{
		q_snprintf(pinfo, sizeof(pinfo), "%s fullscreen DIB", pv->modedesc);
	}
	else
	{
		q_snprintf(pinfo, sizeof(pinfo), "%s windowed", pv->modedesc);
	}

	return pinfo;
}


static void DestroyDIBWindow (void)
{

	if (modestate == MS_WINDOWED)
	{
	// destroy the associated MGL DC's; the window gets reused
		if (windc)
			MGL_destroyDC(windc);
		if (dibdc)
			MGL_destroyDC(dibdc);
		windc = dibdc = NULL;
	}
}


static void DestroyFullscreenWindow (void)
{

	if (modestate == MS_FULLSCREEN)
	{
	// destroy the existing fullscreen mode and DC's
		if (mgldc)
			MGL_destroyDC (mgldc);
		if (memdc)
			MGL_destroyDC (memdc);
		mgldc = memdc = NULL;
	}
}


static void DestroyFullDIBWindow (void)
{
	if (modestate == MS_FULLDIB)
	{
		ChangeDisplaySettings (NULL, CDS_FULLSCREEN);

	// Destroy the fullscreen DIB window and associated MGL DC's
		if (windc)
			MGL_destroyDC(windc);
		if (dibdc)
			MGL_destroyDC(dibdc);
		windc = dibdc = NULL;
	}
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
	HDC		hdc;
	pixel_format_t	pf;
	qboolean	stretched;
	int		lastmodestate;

	VID_CheckModedescFixup (modenum);

	DDActive = false;
	lastmodestate = modestate;

	DestroyFullscreenWindow ();
	DestroyFullDIBWindow ();

	if (windc)
		MGL_destroyDC(windc);
	if (dibdc)
		MGL_destroyDC(dibdc);
	windc = dibdc = NULL;

// KJB: Signal to the MGL that we are going back to windowed mode
	if (!MGL_changeDisplayMode(grWINDOWED))
		initFatalError();

	WindowRect.top = WindowRect.left = 0;

	WindowRect.right = modelist[modenum].width;
	WindowRect.bottom = modelist[modenum].height;
	stretched = modelist[modenum].stretched;

	DIBWidth = modelist[modenum].width;
	DIBHeight = modelist[modenum].height;

	if (stretched)
	{
		DIBWidth >>= 1;
		DIBHeight >>= 1;
	}

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

	// tell MGL to use this window for fullscreen modes
		MGL_registerFullScreenWindow (mainwindow);

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

// because we have set the background brush for the window to NULL
// (to avoid flickering when re-sizing the window on the desktop),
// we clear the window to black when created, otherwise it will be
// empty while Quake starts up.
	hdc = GetDC(mainwindow);
	PatBlt(hdc, 0, 0, WindowRect.right, WindowRect.bottom, BLACKNESS);
	ReleaseDC(mainwindow, hdc);

	/* Create the MGL window DC and the MGL memory DC */
	if ((windc = MGL_createWindowedDC(mainwindow)) == NULL)
		MGL_fatalError("Unable to create Windowed DC!");

	if ((dibdc = MGL_createMemoryDC(DIBWidth, DIBHeight, 8, &pf)) == NULL)
		MGL_fatalError("Unable to create Memory DC!");

	MGL_makeCurrentDC(dibdc);

	vid.buffer = vid.conbuffer = vid.direct = (pixel_t *) dibdc->surface;
	vid.rowbytes = vid.conrowbytes = dibdc->mi.bytesPerLine;
	vid.numpages = 1;
	vid.maxwarpwidth = WARP_WIDTH;
	vid.maxwarpheight = WARP_HEIGHT;
	vid.height = vid.conheight = DIBHeight;
	vid.width = vid.conwidth = DIBWidth;
	vid.aspect = ((float)vid.height / (float)vid.width) * (320.0 / 240.0);

	vid_stretched = stretched;

	SendMessage (mainwindow, WM_SETICON, (WPARAM)TRUE, (LPARAM)hIcon);
	SendMessage (mainwindow, WM_SETICON, (WPARAM)FALSE, (LPARAM)hIcon);

	return true;
}


static qboolean VID_SetFullscreenMode (int modenum)
{
	DDActive = true;

	DestroyDIBWindow ();
	DestroyFullDIBWindow ();

	mode = modelist[modenum].modenum;

	// Destroy old DC's, resetting back to fullscreen mode
	if (mgldc)
		MGL_destroyDC (mgldc);
	if (memdc)
		MGL_destroyDC (memdc);
	mgldc = memdc = NULL;

//	if ((mgldc = createDisplayDC (modelist[modenum].stretched || vid_nopageflip.integer)) == NULL)
	if ((mgldc = createDisplayDC (modelist[modenum].stretched)) == NULL)
	{
		return false;
	}

	modestate = MS_FULLSCREEN;
	vid_fulldib_on_focus_mode = 0;

	vid.buffer = vid.conbuffer = vid.direct = NULL;
	vid.maxwarpwidth = WARP_WIDTH;
	vid.maxwarpheight = WARP_HEIGHT;
	DIBHeight = vid.height = vid.conheight = modelist[modenum].height;
	DIBWidth = vid.width = vid.conwidth = modelist[modenum].width;
	vid.aspect = ((float)vid.height / (float)vid.width) * (320.0 / 240.0);

	vid_stretched = modelist[modenum].stretched;

// needed because we're not getting WM_MOVE messages fullscreen on NT
	window_x = 0;
	window_y = 0;

// set the large icon, so the Quake icon will show up in the taskbar
	SendMessage (mainwindow, WM_SETICON, (WPARAM)1, (LPARAM)hIcon);
	SendMessage (mainwindow, WM_SETICON, (WPARAM)0, (LPARAM)hIcon);

// shouldn't be needed, but Kendall needs to let us get the activation
// message for this not to be needed on NT
	AppActivate (true, false);

/*	Pa3PyX: HACK: Override MGL's default wndproc by our own.
	1) We don't want to handle minimize/restore on fullscreen modes,
	especially in VESA modes and even VGA modes (which may cause
	Windows to stop responding or give a blue screen in many
	instances, since VESA modes access hardware directly);
	2) MGL has a bug that prevents us from using the ALT key in
	DirectDraw modes (WM_SYSKEYDOWN and WM_SYSKEYUP are not forwarded
	to the user eventproc).
*/
	mgl_wnd_proc = (LONG_PTR)GetWindowLongPtr(mainwindow, GWLP_WNDPROC);
	SetWindowLongPtr(mainwindow, GWLP_WNDPROC, (LONG_PTR)MainWndProc);

	return true;
}


static qboolean VID_SetFullDIBMode (int modenum)
{
	HDC		hdc;
	pixel_format_t	pf;
	int		lastmodestate;

	DDActive = false;

	DestroyFullscreenWindow ();
	DestroyDIBWindow ();

	if (windc)
		MGL_destroyDC(windc);
	if (dibdc)
		MGL_destroyDC(dibdc);
	windc = dibdc = NULL;

	// KJB: Signal to the MGL that we are going back to windowed mode
	if (!MGL_changeDisplayMode(grWINDOWED))
		initFatalError();

	gdevmode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
	gdevmode.dmBitsPerPel = modelist[modenum].bpp;
	gdevmode.dmPelsWidth = modelist[modenum].width << modelist[modenum].stretched << modelist[modenum].halfscreen;
	gdevmode.dmPelsHeight = modelist[modenum].height << modelist[modenum].stretched;
	gdevmode.dmSize = sizeof (gdevmode);

	if (ChangeDisplaySettings (&gdevmode, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
		Sys_Error ("Couldn't set fullscreen DIB mode");

	lastmodestate = modestate;
	modestate = MS_FULLDIB;
	vid_fulldib_on_focus_mode = modenum;

	WindowRect.top = WindowRect.left = 0;

	hdc = GetDC(NULL);

	WindowRect.right = modelist[modenum].width << modelist[modenum].stretched;
	WindowRect.bottom = modelist[modenum].height << modelist[modenum].stretched;

	ReleaseDC(NULL,hdc);

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

	// Because we have set the background brush for the window to NULL
	// (to avoid flickering when re-sizing the window on the desktop), we
	// clear the window to black when created, otherwise it will be
	// empty while Quake starts up.
	hdc = GetDC(mainwindow);
	PatBlt(hdc,0,0,WindowRect.right,WindowRect.bottom,BLACKNESS);
	ReleaseDC(mainwindow, hdc);

	/* Create the MGL window DC and the MGL memory DC */
	if ((windc = MGL_createWindowedDC(mainwindow)) == NULL)
		MGL_fatalError("Unable to create Fullscreen DIB DC!");

	if ((dibdc = MGL_createMemoryDC(DIBWidth, DIBHeight, 8, &pf)) == NULL)
		MGL_fatalError("Unable to create Memory DC!");

	MGL_makeCurrentDC(dibdc);

	vid.buffer = vid.conbuffer = vid.direct = (pixel_t *) dibdc->surface;
	vid.rowbytes = vid.conrowbytes = dibdc->mi.bytesPerLine;
	vid.numpages = 1;
	vid.maxwarpwidth = WARP_WIDTH;
	vid.maxwarpheight = WARP_HEIGHT;
	vid.height = vid.conheight = DIBHeight;
	vid.width = vid.conwidth = DIBWidth;
	vid.aspect = ((float)vid.height / (float)vid.width) * (320.0 / 240.0);

	vid_stretched = modelist[modenum].stretched;

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
	else if (modelist[modenum].type == MS_FULLDIB)
	{
		status = VID_SetFullDIBMode(modenum);
		IN_ActivateMouse ();
		IN_HideMouse ();
	}
	else
	{
		status = VID_SetFullscreenMode(modenum);
		IN_ActivateMouse ();
		IN_HideMouse ();
	}

	window_width = vid.width << vid_stretched;
	window_height = vid.height << vid_stretched;
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

	// Pa3PyX: set desired page flipping mode
	switch (vid_wait.integer)
	{
		case 0:	waitVRT = MGL_dontWait;
			break;
		case 1:	waitVRT = MGL_waitVRT;
			break;
		case 2:	waitVRT = MGL_tripleBuffer;
			break;
		default: waitVRT = defaultVRT;
	}

	// print number of video pages as well,
	// if page flipping is enabled. Pa3PyX
	if (!msg_suppress_1)
	{
		if (vid.numpages > 1)
			Con_SafePrintf("%s (hw buffer: %i pages)\n", VID_GetModeDescription(vid_modenum), vid.numpages);
		else
			Con_SafePrintf("%s (sw buffer)\n", VID_GetModeDescription(vid_modenum));
	}

	VID_SetPalette (palette);

	in_mode_set = false;
	vid.recalc_refdef = 1;

	return true;
}

void VID_LockBuffer (void)
{
	if (dibdc)
		return;

	lockcount++;

	if (lockcount > 1)
		return;

	MGL_beginDirectAccess();

	if (memdc)
	{
		// Update surface pointer for linear access modes
		vid.buffer = vid.conbuffer = vid.direct = (pixel_t *) memdc->surface;
		vid.rowbytes = vid.conrowbytes = memdc->mi.bytesPerLine;
	}
	else if (mgldc)
	{
		// Update surface pointer for linear access modes
		vid.buffer = vid.conbuffer = vid.direct = (pixel_t *) mgldc->surface;
		vid.rowbytes = vid.conrowbytes = mgldc->mi.bytesPerLine;
	}

	if (r_dowarp)
		d_viewbuffer = r_warpbuffer;
	else
		d_viewbuffer = vid.buffer;

	if (r_dowarp)
		screenwidth = WARP_WIDTH;
	else
		screenwidth = vid.rowbytes;
}
		
		
void VID_UnlockBuffer (void)
{
	if (dibdc)
		return;

	lockcount--;

	if (lockcount > 0)
		return;

	if (lockcount < 0)
		Sys_Error ("Unbalanced unlock");

	MGL_endDirectAccess();

// to turn up any unlocked accesses
	vid.buffer = vid.conbuffer = vid.direct = d_viewbuffer = NULL;

}


void	VID_SetPalette (unsigned char *palette)
{
	int		i;
	palette_t	pal[256];
	HDC		hdc;

	if (!Minimized)
	{
		palette_changed = true;

	// make sure we have the static colors if we're the active app
		hdc = GetDC(NULL);

		if (vid_palettized && ActiveApp)
		{
			if (GetSystemPaletteUse(hdc) == SYSPAL_STATIC)
			{
			// switch to SYSPAL_NOSTATIC and remap the colors
				SetSystemPaletteUse(hdc, SYSPAL_NOSTATIC);
				syscolchg = true;
				pal_is_nostatic = true;
			}
		}

		ReleaseDC(NULL,hdc);

		// Translate the palette values to an MGL palette array and
		// set the values.
		for (i = 0; i < 256; i++)
		{
			pal[i].red = palette[i*3];
			pal[i].green = palette[i*3+1];
			pal[i].blue = palette[i*3+2];
		}

		if (DDActive)
		{
			if (!mgldc)
				return;

			MGL_setPalette(mgldc,pal,256,0);
			MGL_realizePalette(mgldc,256,0,false);
			if (memdc)
				MGL_setPalette(memdc,pal,256,0);
		}
		else
		{
			if (!windc)
				return;

			MGL_setPalette(windc,pal,256,0);
			MGL_realizePalette(windc,256,0,false);
			if (dibdc)
			{
				MGL_setPalette(dibdc,pal,256,0);
				MGL_realizePalette(dibdc,256,0,false);
			}
		}
	}

	if (palette != vid_curpal)
		memcpy(vid_curpal, palette, sizeof(vid_curpal));

	if (syscolchg)
	{
		PostMessage (HWND_BROADCAST, WM_SYSCOLORCHANGE, (WPARAM)0, (LPARAM)0);
		syscolchg = false;
	}
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

		// stretched modes are half width/height. Pa3PyX
		if (VID_CheckAdequateMem (pv->width >> pv->stretched, pv->height >> pv->stretched))
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
	int		basenummodes;
	byte		*ptmp;
	const char	*read_vars[] = {
				"_vid_default_mode_win" };
#define num_readvars	( sizeof(read_vars)/sizeof(read_vars[0]) )

	Cvar_RegisterVariable (&vid_mode);
	Cvar_RegisterVariable (&vid_wait);
	Cvar_RegisterVariable (&vid_nopageflip);
	Cvar_RegisterVariable (&vid_maxpages);
	Cvar_RegisterVariable (&_vid_default_mode);
	Cvar_RegisterVariable (&_vid_default_mode_win);
	Cvar_RegisterVariable (&vid_config_x);
	Cvar_RegisterVariable (&vid_config_y);
	Cvar_RegisterVariable (&vid_config_fscr);
	Cvar_RegisterVariable (&vid_config_glx);
	Cvar_RegisterVariable (&vid_config_gly);
	Cvar_RegisterVariable (&vid_stretch_by_2);
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

	if (safemode || COM_CheckParm ("-dibonly"))
		dibonly = true;

	VID_InitMGLDIB (global_hInstance);

	basenummodes = nummodes;

	if (!dibonly)
		VID_InitMGLFull (global_hInstance);

// if there are no non-windowed modes, or only windowed and mode 0x13, then use
// fullscreen DIBs as well
	if (((nummodes == basenummodes) ||
		 ((nummodes == (basenummodes + 1)) && is_mode0x13)) &&
		!COM_CheckParm ("-nofulldib"))
	{
		VID_InitFullDIB (global_hInstance);
	}

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
		if (lockcount && !dibdc)
			MGL_endDirectAccess();

		if (modestate == MS_FULLDIB)
			ChangeDisplaySettings (NULL, CDS_FULLSCREEN);

		/* Pa3PyX: restore MGL's original event handling procedure so
		   that it can do any needed cleanup before we start minimizing
		   and closing windows */
		if (modestate == MS_FULLSCREEN && mainwindow && mgl_wnd_proc)
		{
			SetWindowLongPtr(mainwindow, GWLP_WNDPROC, mgl_wnd_proc);
		}

		PostMessage (HWND_BROADCAST, WM_PALETTECHANGED, (WPARAM)mainwindow, (LPARAM)0);
		PostMessage (HWND_BROADCAST, WM_SYSCOLORCHANGE, (WPARAM)0, (LPARAM)0);

		AppActivate(false, false);

		DestroyDIBWindow ();
		DestroyFullscreenWindow ();
		DestroyFullDIBWindow ();

#if !defined(NO_SPLASHES)
		if (hwnd_dialog)
		{
			DestroyWindow (hwnd_dialog);
			hwnd_dialog = NULL;
		}
#endif	/* ! NO_SPLASHES */

		if (mainwindow)
			DestroyWindow(mainwindow);

		MGL_exit();

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
	int		i;

	// Flip the surfaces

	if (DDActive)
	{
		if (mgldc)
		{
			if (memdc)
			{
				while (rects)
				{
					if (vid_stretched)
					{
						MGL_stretchBltCoord(mgldc, memdc,
									rects->x,
									rects->y,
									rects->x + rects->width,
									rects->y + rects->height,
									rects->x << 1,
									rects->y << 1,
									(rects->x + rects->width) << 1,
									(rects->y + rects->height) << 1);
					}
					else
					{
					// Pa3PyX: if we have linear/virtual
					// framebuffer access, it is much faster
					// to copy surface to surface directly
						if (mgldcAccessMode && memdcAccessMode)
						{
							for (i = 0; i < rects->height; i++)
							{
								memcpy(((byte *)mgldc->surface) + ((rects->y + i) * mgldcWidth + rects->x), ((byte *)memdc->surface) + ((rects->y + i) * memdcWidth + rects->x), rects->width);
							}
						}
						else
						{
							MGL_bitBltCoord(mgldc, memdc,
									rects->x, rects->y,
									(rects->x + rects->width),
									(rects->y + rects->height),
									rects->x, rects->y, MGL_REPLACE_MODE);
						}
					}

					rects = rects->pnext;
				}
			}

			if (vid.numpages > 1)
			{
				// We have a flipping surface, so do a hard page flip
				aPage = (aPage+1) % vid.numpages;
				vPage = (vPage+1) % vid.numpages;
				MGL_setActivePage(mgldc,aPage);
				MGL_setVisualPage(mgldc,vPage,waitVRT);
			}
		}
	}
	else
	{
		HDC	hdcScreen;

		hdcScreen = GetDC(mainwindow);

		if (windc && dibdc)
		{
			MGL_setWinDC(windc,hdcScreen);

			while (rects)
			{
				if (vid_stretched)
				{
					MGL_stretchBltCoord(windc,dibdc,
						rects->x, rects->y,
						rects->x + rects->width, rects->y + rects->height,
						rects->x << 1, rects->y << 1,
						(rects->x + rects->width) << 1,
						(rects->y + rects->height) << 1);
				}
				else
				{
					MGL_bitBltCoord(windc,dibdc,
						rects->x, rects->y,
						rects->x + rects->width, rects->y + rects->height,
						rects->x, rects->y, MGL_REPLACE_MODE);
				}

				rects = rects->pnext;
			}
		}

		ReleaseDC(mainwindow, hdcScreen);
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

	if (vid.numpages == 1)
	{
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
	}
	else
	{
	// unlock if locked
		if (lockcount > 0)
			MGL_endDirectAccess();

	// set the active page to the displayed page
		MGL_setActivePage (mgldc, vPage);

	// lock the screen
		MGL_beginDirectAccess ();

	// save from and draw to screen
		for (i = 0; i < (height << repshift); i += reps)
		{
			for (j = 0; j < reps; j++)
			{
				memcpy (&backingbuf[(i + j) * width] /* &backingbuf[(i + j) * 24] */,
						(byte *)mgldc->surface + x +
						 ((y << repshift) + i + j) * mgldc->mi.bytesPerLine,
						width);
				memcpy ((byte *)mgldc->surface + x +
						 ((y << repshift) + i + j) * mgldc->mi.bytesPerLine,
						&pbitmap[(i >> repshift) * width],
						width);
			}
		}

	// unlock the screen
		MGL_endDirectAccess ();

	// restore the original active page
		MGL_setActivePage (mgldc, aPage);

	// relock the screen if it was locked
		if (lockcount > 0)
			MGL_beginDirectAccess();
	}
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
	if (vid.numpages == 1)
	{
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
	}
	else
	{
	// unlock if locked
		if (lockcount > 0)
			MGL_endDirectAccess();

	// set the active page to the displayed page
		MGL_setActivePage (mgldc, vPage);

	// lock the screen
		MGL_beginDirectAccess ();

		vid.buffer = (byte *)mgldc->surface;
		vid.rowbytes = mgldc->mi.bytesPerLine;

		SCR_DrawLoading();

	// unlock the screen
		MGL_endDirectAccess ();

	// restore the original active page
		MGL_setActivePage (mgldc, aPage);

	// relock the screen if it was locked
		if (lockcount > 0)
			MGL_beginDirectAccess();
	}

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

	if (vid.numpages == 1)
	{
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
	}
	else
	{
	// unlock if locked
		if (lockcount > 0)
			MGL_endDirectAccess();

	// set the active page to the displayed page
		MGL_setActivePage (mgldc, vPage);

	// lock the screen
		MGL_beginDirectAccess ();

	// restore to the screen
		for (i = 0; i < (height << repshift); i += reps)
		{
			for (j = 0; j < reps; j++)
			{
				memcpy ((byte *)mgldc->surface + x +
						 ((y << repshift) + i + j) * mgldc->mi.bytesPerLine,
						&backingbuf[(i + j) * width] /* &backingbuf[(i + j) * 24] */,
						width);
			}
		}

	// unlock the screen
		MGL_endDirectAccess ();

	// restore the original active page
		MGL_setActivePage (mgldc, aPage);

	// relock the screen if it was locked
		if (lockcount > 0)
			MGL_beginDirectAccess();
	}
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

	MGL_appActivate(windc, ActiveApp);

	if (vid_initialized)
	{
	// yield the palette if we're losing the focus
		hdc = GetDC(NULL);

		if (GetDeviceCaps(hdc, RASTERCAPS) & RC_PALETTE)
		{
			if (ActiveApp)
			{
				if ((modestate == MS_WINDOWED) || (modestate == MS_FULLDIB))
				{
					if (GetSystemPaletteUse(hdc) == SYSPAL_STATIC)
					{
						// switch to SYSPAL_NOSTATIC and remap the colors
						SetSystemPaletteUse(hdc, SYSPAL_NOSTATIC);
						syscolchg = true;
						pal_is_nostatic = true;
					}
				}
			}
			else if (pal_is_nostatic)
			{
				if (GetSystemPaletteUse(hdc) == SYSPAL_NOSTATIC)
				{
				// switch back to SYSPAL_STATIC and the old mapping
					SetSystemPaletteUse(hdc, SYSPAL_STATIC);
					syscolchg = true;
				}

				pal_is_nostatic = false;
			}
		}

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
//	LRESULT	ret = 0;// Pa3PyX: ignore irrelevant messages in DDRAW/VESA/VGA modes
	LRESULT	ret = (LRESULT) DDActive;
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
		// Pa3PyX: Won't handle these in DDRAW/VESA/VGA modes
		if (DDActive)
			break;

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
		// Pa3PyX: Won't handle these in DDRAW/VESA/VGA modes
		if (DDActive)
			break;
		window_x = (int) LOWORD(lParam);
		window_y = (int) HIWORD(lParam);
		VID_UpdateWindowStatus ();
		break;

	case WM_SIZE:
		// Pa3PyX: Won't handle these in DDRAW/VESA/VGA modes
		if (DDActive)
			break;

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
		{
			if (windc)
				MGL_activatePalette(windc,true);

			VID_SetPalette(vid_curpal);
		}

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

	// KJB: Added these new palette functions
	case WM_PALETTECHANGED:
		if ((HWND)wParam == hWnd)
			break;
	/* Fall through to WM_QUERYNEWPALETTE */
	case WM_QUERYNEWPALETTE:
		hdc = GetDC(NULL);

		if (GetDeviceCaps(hdc, RASTERCAPS) & RC_PALETTE)
			vid_palettized = true;
		else
			vid_palettized = false;

		ReleaseDC(NULL,hdc);

		scr_fullupdate = 0;

		if (vid_initialized && !in_mode_set && windc && MGL_activatePalette(windc,false) && !Minimized)
		{
			VID_SetPalette (vid_curpal);
			InvalidateRect (mainwindow, NULL, false);

		// specifically required if WM_QUERYNEWPALETTE realizes a new palette
			ret = (LRESULT) TRUE;
		}
		break;

	case WM_DISPLAYCHANGE:
		// Pa3PyX: Won't handle these in DDRAW/VESA/VGA modes
		if (DDActive)
			break;
		if (!in_mode_set && (modestate == MS_WINDOWED) && !vid_fulldib_on_focus_mode)
		{
			force_mode_set = true;
			VID_SetMode (vid_modenum, vid_curpal);
			force_mode_set = false;
		}
		break;

	case WM_CLOSE:
		// Pa3PyX: Won't handle these in DDRAW/VESA/VGA modes
		if (DDActive)
			break;
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
		// Pa3PyX: Won't handle these in DDRAW/VESA/VGA modes
		if (DDActive)
			break;
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
	int		ismode13;
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
		modedescs[i].ismode13 = 0;
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
				if (!dup || !modedescs[dupmode].ismode13 || COM_CheckParm("-noforcevga"))
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
					modedescs[k].ismode13 = pv->mode13;
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

