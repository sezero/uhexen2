/*
   vid_sdl.c
   SDL video driver
   Select window size and mode and init SDL in SOFTWARE mode.

   $Header: /home/ozzie/Download/0000/uhexen2/hexen2/vid_sdl.c,v 1.10 2005-01-01 21:43:47 sezero Exp $

   Changed by S.A. 7/11/04, 27/12/04

   Options are now:
     -fullscreen | -window, -height , -width 
   Currently bpp is 8 bit , and it seems fairly hardwired at this depth
   Interactive Video Modes have been disabled.

   The modes which are used are the traditional 0 and 3, but have redefined
   MODE_FULLSCREEN_DEFAULT from 3 to 1 to be a boolean:

	vid_mode = MODE_WINDOWED  0 || MODE_FULLSCREEN_DEFAULT 1

   For clarity I have removed the variable "windowed_default" and replaced
   it with MODE_WINDOWED. Functionality is the same.
*/

#include "quakedef.h"
#include "winquake.h"
#include "d_local.h"
#include "resource.h"
#include "SDL.h"

#define MAX_MODE_LIST	30
#define VID_ROW_SIZE	3

SDL_Surface	*screen;
byte globalcolormap[VID_GRADES*256], lastglobalcolor = 0;
byte *lastsourcecolormap = NULL;

int	window_center_x, window_center_y, window_x, window_y, window_width, window_height;
RECT	window_rect;

extern qboolean grab;
static qboolean	startwindowed = 0;
static int		firstupdate = 1;
static qboolean	vid_initialized = false, vid_palettized = true;
static int	lockcount;
qboolean	in_mode_set, is_mode0x13;
static int	vid_stretched, windowed_mouse;
static qboolean	palette_changed;

viddef_t	vid;				// global video state

#define MODE_WINDOWED			0
#define MODE_SETTABLE_WINDOW	2
#define NO_MODE					(MODE_WINDOWED - 1)
#define MODE_FULLSCREEN_DEFAULT	(MODE_WINDOWED + 1)

// Note that 0 is MODE_WINDOWED
cvar_t		vid_mode = {"vid_mode","0", false};
// Note that 0 is MODE_WINDOWED
cvar_t		_vid_default_mode = {"_vid_default_mode","0", true};
// Note that 1 is MODE_FULLSCREEN_DEFAULT
cvar_t		_vid_default_mode_win = {"_vid_default_mode_win","3", true};
cvar_t		vid_wait = {"vid_wait","0"};
cvar_t		vid_nopageflip = {"vid_nopageflip","0", true};
cvar_t		_vid_wait_override = {"_vid_wait_override", "0", true};
cvar_t		vid_config_x = {"vid_config_x","800", true};
cvar_t		vid_config_y = {"vid_config_y","600", true};
cvar_t		vid_stretch_by_2 = {"vid_stretch_by_2","1", true};
cvar_t		_windowed_mouse = {"_windowed_mouse","1", true};
cvar_t		vid_showload = {"vid_showload", "1"};

typedef struct {
	int		width;
	int		height;
} lmode_t;

lmode_t	lowresmodes[] = {
	{320, 200},
	{320, 240},
	{400, 300},
	{512, 384},
};

int		vid_modenum = NO_MODE;
int		vid_realmode;
double		vid_testendtime;
int		vid_default = MODE_WINDOWED;
static int	windowed_default;

modestate_t	modestate = MS_UNINIT;

static byte		*vid_surfcache;
static int		vid_surfcachesize;
static int		VID_highhunkmark;

unsigned char	vid_curpal[256*3];

unsigned short	d_8to16table[256];
unsigned	d_8to24table[256];

int	mode;

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
	char		modedesc[13];
} vmode_t;

static vmode_t	modelist[MAX_MODE_LIST];
static int		nummodes;

int		aPage;					// Current active display page
int		vPage;					// Current visible display page
int		waitVRT = true;			// True to wait for retrace on flip

static vmode_t	badmode;

void VID_MenuDraw (void);
void VID_MenuKey (int key);
void VID_SetGamma(float value);
void VID_SetGamma_f(void);


/*
================
VID_UpdateWindowStatus
================
*/
void VID_UpdateWindowStatus (void)
{

	window_rect.left = window_x;
	window_rect.top = window_y;
	window_rect.right = window_x + window_width;
	window_rect.bottom = window_y + window_height;
	window_center_x = (window_rect.left + window_rect.right) / 2;
	window_center_y = (window_rect.top + window_rect.bottom) / 2;

	//IN_UpdateClipCursor ();
}


/*
================
ClearAllStates
================
*/
void ClearAllStates (void)
{
	int		i;
	
// send an up event for each key, to make sure the server clears them all
	for (i=0 ; i<256 ; i++)
	{
		Key_Event (i, false);
	}

	Key_ClearStates ();
	IN_ClearStates ();
}


/*
================
VID_CheckAdequateMem
================
*/
qboolean VID_CheckAdequateMem (int width, int height)
{
	int		tbuffersize;

	tbuffersize = width * height * sizeof (*d_pzbuffer);

	tbuffersize += D_SurfaceCacheForRes (width, height);

// see if there's enough memory, allowing for the normal mode 0x13 pixel,
// z, and surface buffers
	if ((host_parms.memsize - tbuffersize + SURFCACHE_SIZE_AT_320X200 +
		 0x10000 * 3) < minimum_memory)
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
qboolean VID_AllocBuffers (int width, int height)
{
	int		tsize, tbuffersize;

	tbuffersize = width * height * sizeof (*d_pzbuffer);

	tsize = D_SurfaceCacheForRes (width, height);

	tbuffersize += tsize;

// see if there's enough memory, allowing for the normal mode 0x13 pixel,
// z, and surface buffers
	if ((host_parms.memsize - tbuffersize + SURFCACHE_SIZE_AT_320X200 +
		 0x10000 * 3) < minimum_memory)
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

	d_pzbuffer = Hunk_HighAllocName (tbuffersize, "video");

	vid_surfcache = (byte *)d_pzbuffer +
			width * height * sizeof (*d_pzbuffer);
	
	return true;
}

/*
=================
VID_NumModes
=================
*/
int VID_NumModes (void)
{
	return nummodes;
}

	
/*
=================
VID_GetModePtr
=================
*/
vmode_t *VID_GetModePtr (int modenum)
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
void VID_CheckModedescFixup (int mode)
{
	int		x, y, stretch;

	if (mode == MODE_SETTABLE_WINDOW)
	{
		modelist[mode].stretched = (int)vid_stretch_by_2.value;
		stretch = modelist[mode].stretched;

		if (vid_config_x.value < (320 << stretch))
			vid_config_x.value = 320 << stretch;

		if (vid_config_y.value < (200 << stretch))
			vid_config_y.value = 200 << stretch;

		x = (int)vid_config_x.value;
		y = (int)vid_config_y.value;
		sprintf (modelist[mode].modedesc, "%dx%d", x, y);
		modelist[mode].width = x;
		modelist[mode].height = y;
	}
}


/*
=================
VID_GetModeDescriptionMemCheck
=================
*/
char *VID_GetModeDescriptionMemCheck (int mode)
{
	char		*pinfo;
	vmode_t		*pv;

	if ((mode < 0) || (mode >= nummodes))
		return NULL;

	VID_CheckModedescFixup (mode);

	pv = VID_GetModePtr (mode);
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
char *VID_GetModeDescription (int mode)
{
	char		*pinfo;
	vmode_t		*pv;

	if ((mode < 0) || (mode >= nummodes))
		return NULL;

	VID_CheckModedescFixup (mode);

	pv = VID_GetModePtr (mode);
	pinfo = pv->modedesc;
	return pinfo;
}


/*
=================
VID_GetModeDescription2

Tacks on "windowed" or "fullscreen"
=================
*/
char *VID_GetModeDescription2 (int mode)
{
	static char	pinfo[40];
	vmode_t		*pv;

	if ((mode < 0) || (mode >= nummodes))
		return NULL;

	VID_CheckModedescFixup (mode);

	pv = VID_GetModePtr (mode);

	if (modelist[mode].type == MS_FULLSCREEN)
	{
		sprintf(pinfo,"%s fullscreen", pv->modedesc);
	}
	else if (modelist[mode].type == MS_FULLDIB)
	{
		sprintf(pinfo,"%s fullscreen", pv->modedesc);
	}
	else
	{
		sprintf(pinfo, "%s windowed", pv->modedesc);
	}

	return pinfo;
}


// KJB: Added this to return the mode driver name in description for console

char *VID_GetExtModeDescription (int mode)
{
	static char	pinfo[40];
	vmode_t		*pv;

	if ((mode < 0) || (mode >= nummodes))
		return NULL;

	VID_CheckModedescFixup (mode);

	pv = VID_GetModePtr (mode);
	if (modelist[mode].type == MS_FULLSCREEN)
	{
		sprintf(pinfo,"%s fullscreen",pv->modedesc);
		sprintf(pinfo,"%s fullscreen",pv->modedesc);
	}
	else if (modelist[mode].type == MS_FULLDIB)
	{
		sprintf(pinfo,"%s fullscreen DIB", pv->modedesc);
	}
	else
	{
		sprintf(pinfo, "%s windowed", pv->modedesc);
	}

	return pinfo;
}

qboolean VID_SetWindowedMode (int modenum)
{
	Uint32 flags = (SDL_SWSURFACE|SDL_HWPALETTE);

	VID_CheckModedescFixup (modenum);

	modestate = MS_WINDOWED;

	vid.height = vid.conheight = modelist[modenum].height;
	vid.width = vid.conwidth = modelist[modenum].width;
	vid.maxwarpwidth = WARP_WIDTH;
	vid.maxwarpheight = WARP_HEIGHT;
	
	if (!(screen = SDL_SetVideoMode(vid.width, vid.height, modelist[modenum].bpp, flags)))
		return false;

	vid.buffer = vid.conbuffer = vid.direct = screen->pixels;
	vid.rowbytes = vid.conrowbytes = screen->pitch;
	vid.numpages = 1;
	vid.aspect = ((float)vid.height / (float)vid.width) *
				(320.0 / 240.0);

	vid_stretched = modelist[modenum].stretched;

	return true;
}


qboolean VID_SetFullscreenMode (int modenum)
{
	Uint32 flags = (SDL_SWSURFACE|SDL_HWPALETTE|SDL_FULLSCREEN);
	// (SDL_HWSURFACE|SDL_DOUBLEBUF|SDL_HWPALETTE|SDL_FULLSCREEN);

	mode = modelist[modenum].modenum;
	modestate = MS_FULLSCREEN;

	vid.height = vid.conheight = modelist[modenum].height;
	vid.width = vid.conwidth = modelist[modenum].width;

	// This doesn't work at 16 bpp.
	// Looks like there's a heap of programing around 256 colours S.A

	if (!(screen = SDL_SetVideoMode(vid.width, vid.height, 8, flags)))
		return false;

	vid.buffer = vid.conbuffer = vid.direct = screen->pixels;
	vid.rowbytes = vid.conrowbytes = screen->pitch;
	vid.maxwarpwidth = WARP_WIDTH;
	vid.maxwarpheight = WARP_HEIGHT;
	vid.aspect = ((float)vid.height / (float)vid.width) *
				(320.0 / 240.0);

	vid_stretched = modelist[modenum].stretched;

	return true;
}


qboolean VID_SetFullDIBMode (int modenum)
{
	// This should never get called anyway - DDOI
	return false;
}


void VID_RestoreOldMode (int original_mode)
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

void VID_SetDefaultMode (void)
{

	if (vid_initialized)
		VID_SetMode (0, vid_curpal);

	IN_DeactivateMouse ();
}

int VID_SetMode (int modenum, unsigned char *palette)
{
	int			original_mode, temp;
	qboolean		stat;

	// debug printf ("VID_SetMode modenum = %i\n",modenum);

// S.A. did this Cvar_SetValue & "if 0" 
// It's pretty messy in here. Dunno much about it, probably alot of it
// could go as it's used in dynamically changing/testing video modes

	Cvar_SetValue ("vid_mode", (float)modenum);

	// stuff removed S.A.

	if (modenum == vid_modenum)
		return true;

	// so Con_Printfs don't mess us up by forcing vid and snd updates
	temp = scr_disabled_for_loading;
	scr_disabled_for_loading = true;
	in_mode_set = true;

	//Snd_ReleaseBuffer ();
	CDAudio_Pause ();

	if (vid_modenum == NO_MODE)
		original_mode = windowed_default;
	else
		original_mode = vid_modenum;

	/* Set either the fullscreen or windowed mode */

	// debug printf ("!! modenum is %i\n\n",modenum);
	if (modelist[modenum].type == MS_WINDOWED)
	{
		if (_windowed_mouse.value)
		{
			stat = VID_SetWindowedMode(modenum);
			IN_ActivateMouse ();
			IN_HideMouse ();
		}
		else
		{
			IN_DeactivateMouse ();
			IN_ShowMouse ();
			stat = VID_SetWindowedMode(modenum);
		}
	}
	else if (modelist[modenum].type == MS_FULLDIB)
	{
		stat = VID_SetFullDIBMode(modenum);
		IN_ActivateMouse ();
		IN_HideMouse ();
	}
	else
	{
		stat = VID_SetFullscreenMode(modenum);
		IN_ActivateMouse ();
		IN_HideMouse ();
	}

	window_width = vid.width << vid_stretched;
	window_height = vid.height << vid_stretched;
	VID_UpdateWindowStatus ();

	CDAudio_Resume ();
	//Snd_AcquireBuffer ();
	scr_disabled_for_loading = temp;

	if (!stat)
	{
		VID_RestoreOldMode (original_mode);
		return false;
	}

	VID_SetPalette (palette);


	vid_modenum = modenum;
	// S.A.removed: Cvar_SetValue ("vid_mode", (float)vid_modenum);

	if (!VID_AllocBuffers (vid.width, vid.height))
	{
	// couldn't get memory for this mode; try to fall back to previous mode
		VID_RestoreOldMode (original_mode);
		return false;
	}

	D_InitCaches (vid_surfcache, vid_surfcachesize);

	ClearAllStates();

	if (!msg_suppress_1)
		Con_SafePrintf ("%s\n", VID_GetModeDescription (vid_modenum));

	VID_SetPalette (palette);

#ifdef H2MP
	SDL_WM_SetCaption("Portal of Praevus", "PRAEVUS");
#else
	SDL_WM_SetCaption("HexenII", "HEXEN2");
#endif

	in_mode_set = false;
	vid.recalc_refdef = 1;

	return true;
}

void VID_LockBuffer (void)
{
	lockcount++;

	if (lockcount > 1)
		return;

	SDL_LockSurface(screen);

	// Update surface pointer for linear access modes
	vid.buffer = vid.conbuffer = vid.direct = screen->pixels;
	vid.rowbytes = vid.conrowbytes = screen->pitch;

	if (r_dowarp)
		d_viewbuffer = r_warpbuffer;
	else
		d_viewbuffer = (void *)(byte *)vid.buffer;

	if (r_dowarp)
		screenwidth = WARP_WIDTH;
	else
		screenwidth = vid.rowbytes;

	if (lcd_x.value)
		screenwidth <<= 1;
}
		
		
void VID_UnlockBuffer (void)
{

	lockcount--;

	if (lockcount > 0)
		return;

	if (lockcount < 0)
		Sys_Error ("Unbalanced unlock");

	SDL_UnlockSurface (screen);

// to turn up any unlocked accesses
	//vid.buffer = vid.conbuffer = vid.direct = d_viewbuffer = NULL;
}


int VID_ForceUnlockedAndReturnState (void)
{
	int	lk;

	if (!lockcount)
		return 0;

	lk = lockcount;

	lockcount = 1;
	VID_UnlockBuffer ();

	return lk;
}


void VID_ForceLockState (int lk)
{

	if (lk)
	{
		lockcount = 0;
		VID_LockBuffer ();
	}

	lockcount = lk;
}


void	VID_SetPalette (unsigned char *palette)
{
        int i;
        SDL_Color colors[256];

	palette_changed = true;

	memcpy (vid_curpal, palette, sizeof(vid_curpal));

        for ( i=0; i<256; ++i )
        {
                colors[i].r = *palette++;
                colors[i].g = *palette++;
                colors[i].b = *palette++;
        }
        SDL_SetColors(screen, colors, 0, 256);

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
void VID_DescribeCurrentMode_f (void)
{
	Con_Printf ("%s\n", VID_GetExtModeDescription (vid_modenum));
}


/*
=================
VID_NumModes_f
=================
*/
void VID_NumModes_f (void)
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
void VID_DescribeMode_f (void)
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
void VID_DescribeModes_f (void)
{
	int			i, lnummodes;
	char		*pinfo;
	qboolean	na;
	vmode_t		*pv;

	na = false;

	lnummodes = VID_NumModes ();

	for (i=0 ; i<lnummodes ; i++)
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


void	VID_Init (unsigned char *palette)
{
	int		i, bestmatch, bestmatchmetric, t, dr, dg, db;
	int		basenummodes;
	int		width,height;
	byte	*ptmp;

	if (SDL_Init(SDL_INIT_VIDEO)<0)
		Sys_Error("VID: Couldn't load SDL: %s", SDL_GetError());
		
	Cvar_RegisterVariable (&vid_mode);
	Cvar_RegisterVariable (&vid_wait);
	Cvar_RegisterVariable (&vid_nopageflip);
	Cvar_RegisterVariable (&_vid_wait_override);
	Cvar_RegisterVariable (&_vid_default_mode);
	Cvar_RegisterVariable (&_vid_default_mode_win);
	Cvar_RegisterVariable (&vid_config_x);
	Cvar_RegisterVariable (&vid_config_y);
	Cvar_RegisterVariable (&vid_stretch_by_2);
	Cvar_RegisterVariable (&_windowed_mouse);
	Cvar_RegisterVariable (&vid_showload);

	Cmd_AddCommand ("vid_nummodes", VID_NumModes_f);
	Cmd_AddCommand ("vid_describecurrentmode", VID_DescribeCurrentMode_f);
	Cmd_AddCommand ("vid_describemode", VID_DescribeMode_f);
	Cmd_AddCommand ("vid_describemodes", VID_DescribeModes_f);
	Cmd_AddCommand ("vid_setgamma", VID_SetGamma_f);

	modelist[0].type = MS_WINDOWED;
	modelist[0].width = 320;
	modelist[0].height = 240;
	strcpy (modelist[0].modedesc, "320x240");
	modelist[0].mode13 = 0;
	modelist[0].modenum = MODE_WINDOWED;
	modelist[0].stretched = 0;
	modelist[0].dib = 1;
	modelist[0].fullscreen = 0;
	modelist[0].halfscreen = 0;
	modelist[0].bpp = 8;

	// mode[1] has been hacked to be like the (missing) mode[3] S.A.

	modelist[1].type = MS_FULLSCREEN;
	modelist[1].width = 640;
	modelist[1].height = 480;
	strcpy (modelist[1].modedesc, "640x480");
	modelist[1].mode13 = 0;
	modelist[1].modenum = MODE_FULLSCREEN_DEFAULT;
	modelist[1].stretched = 1;
	modelist[1].dib = 1;
	modelist[1].fullscreen = 1;
	modelist[1].halfscreen = 0;
	modelist[1].bpp = 8;

	// modelist[2-4] removed

	vid_default = MODE_WINDOWED;

	nummodes = 5;

	basenummodes = nummodes;

	vid.maxwarpwidth = WARP_WIDTH;
	vid.maxwarpheight = WARP_HEIGHT;
	vid.colormap = host_colormap;
	vid.fullbright = 256 - LittleLong (*((int *)vid.colormap + 2048));

// GDI doesn't let us remap palette index 0, so we'll remap color
// mappings from that black to another one
	bestmatchmetric = 256*256*3;

	for (i=1 ; i<256 ; i++)
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

	for (i=0, ptmp = vid.colormap ; i<(1<<(VID_CBITS+8)) ; i++, ptmp++)
	{
		if (*ptmp == 0)
			*ptmp = bestmatch;
	}

	// debug printf ("check -window\n");

	/*********************************
 	 * command line processing (S.A) *
 	 *********************************/

	grab = 1; // grab window, release it when main menu is drawn

	startwindowed = 1; // default is windowed
	vid_default = MODE_WINDOWED;

	if (COM_CheckParm("-fullscreen") || COM_CheckParm ("--fullscreen")
			|| COM_CheckParm ("-f") || COM_CheckParm ("-fs"))
	{
		startwindowed=0;
		vid_default = MODE_FULLSCREEN_DEFAULT;
	}

	if (COM_CheckParm("-width"))
	{
		width = atoi(com_argv[COM_CheckParm("-width")+1]);
	}
	else
	{
		// make the default sizes for software mode smaller
		if (vid_default == MODE_WINDOWED)
			width=400;
		else
			width=512;
	}

	if (COM_CheckParm("-height"))
	{
		height = atoi(com_argv[COM_CheckParm("-height")+1]);
	}
	else
	{
		switch (width) {
			case 800: height=600; break;
			case 640: height=480; break;
			case 512: height=384; break;
			case 400: height=300; break;
			case 320: height=240; break;
			default:  height=300;
		}
	}
	modelist[vid_default].width = width;
	modelist[vid_default].height = height;
	sprintf (modelist[vid_default].modedesc,"%dx%d",width,height);

	// printf ("SA vid_default = %i\n",vid_default);

	vid_initialized = true;

	VID_SetMode (vid_default, palette);

	vid_realmode = vid_modenum;

	VID_SetPalette (palette);	// Useless (?) - DDOI

	vid_menudrawfn = VID_MenuDraw;
	// vid_menudrawfn = NULL; S.A

	vid_menukeyfn = VID_MenuKey;

	strcpy (badmode.modedesc, "Bad mode");
}


void	VID_Shutdown (void)
{
#if 0
	HDC				hdc;
#endif
extern byte *transTable;	//from r_part
extern byte *mainTransTable; // in r_main.c

	if (vid_initialized)
	{
#if 0
		if (hwnd_dialog)
		{
			DestroyWindow (hwnd_dialog);
			hwnd_dialog=NULL;
		}

		if (modestate == MS_FULLDIB)
			ChangeDisplaySettings (NULL, CDS_FULLSCREEN);

		PostMessage (HWND_BROADCAST, WM_PALETTECHANGED, (WPARAM)mainwindow, (LPARAM)0);
		PostMessage (HWND_BROADCAST, WM_SYSCOLORCHANGE, (WPARAM)0, (LPARAM)0);

		AppActivate(false, false);
		DestroyDIBWindow ();
		DestroyFullscreenWindow ();
		DestroyFullDIBWindow ();
		MGL_exit();
#endif
		vid_initialized = 0;
		if (transTable)
		{
			free(transTable);
			transTable=NULL;
		}
		if (mainTransTable)
		{
			free(mainTransTable);
			mainTransTable=NULL;
		}
		SDL_Quit();
	}
}


/*
================
FlipScreen
================
*/
void FlipScreen(vrect_t *rects)
{
	while (rects) {
		SDL_UpdateRect (screen, rects->x, rects->y, rects->width,
				rects->height);
		rects = rects->pnext;
	}
}

void	VID_Update (vrect_t *rects)
{
	vrect_t	rect;

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

	if (firstupdate)
	{
		if ((_vid_default_mode_win.value != vid_default) &&
			(!startwindowed || (_vid_default_mode_win.value < MODE_FULLSCREEN_DEFAULT)))
		{
			firstupdate = 0;

			if ((_vid_default_mode_win.value < 0) ||
				(_vid_default_mode_win.value >= nummodes))
			{
				Cvar_SetValue ("_vid_default_mode_win", MODE_WINDOWED);
			}

			// S.A. Cvar_SetValue ("vid_mode", _vid_default_mode_win.value);
		}
	}

	// We've drawn the frame; copy it to the screen
	FlipScreen (rects);

	// handle the mouse state when windowed if that's changed
	if (modestate == MS_WINDOWED)
	{
		if ((int)_windowed_mouse.value != windowed_mouse)
		{
			if (_windowed_mouse.value)
			{
				IN_ActivateMouse ();
				IN_HideMouse ();
			}
			else
			{
				IN_DeactivateMouse ();
				IN_ShowMouse ();
			}

			windowed_mouse = (int)_windowed_mouse.value;
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
	// "if 0" removed
}


void D_ShowLoadingSize (void)
{
	vrect_t		rect;
	viddef_t	save_vid;	// global video state

	if (!vid_showload.value)
		return; 

	if (!vid_initialized)
		return;

	save_vid = vid;
	if (vid.numpages == 1)
	{
		VID_LockBuffer ();

		if (!vid.direct)
			Sys_Error ("NULL vid.direct pointer");

		vid.buffer = vid.direct;

		SCR_DrawLoading();

		VID_UnlockBuffer ();

		rect.x = 0;
		rect.y = 0;
		rect.width = vid.width;
		rect.height = 112;
		rect.pnext = NULL;

		FlipScreen (&rect);
	}
	else
	{

		vid.buffer = (byte *)screen->pixels;
		vid.rowbytes = screen->pitch;

		SCR_DrawLoading();

	}

	vid = save_vid;
}


/*
================
D_EndDirectRect
================
*/
void D_EndDirectRect (int x, int y, int width, int height)
{
	// "if 0" removed
}

/*
=================
VID_SetGamma

=================
*/

void VID_SetGamma(float value)
{
#ifdef PLATFORM_UNIX
  SDL_SetGamma(value,value,value);
#endif
}

void VID_ApplyGamma (void)
{
  if ((v_gamma.value != 0)&&(v_gamma.value > (1/GAMMA_MAX)))
    VID_SetGamma(1/v_gamma.value);

  else VID_SetGamma(GAMMA_MAX);

//  Con_Printf ("Gamma changed to %f\n", v_gamma.value);
}

void VID_SetGamma_f (void)
{
  float value = 1;

  value = atof (Cmd_Argv(1));

  if (value != 0) {
    if (value > (1/GAMMA_MAX))
      v_gamma.value = value;

    else v_gamma.value =  1/GAMMA_MAX;
  }

/* if value==0 , just apply current settings.
   this is usefull at startup */

  VID_ApplyGamma();

//  Con_Printf ("Gamma changed to %f\n", v_gamma.value);
}

//==========================================================================

byte        scantokey[128] = 
					{ 
//  0           1       2       3       4       5       6       7 
//  8           9       A       B       C       D       E       F 
	0  ,    27,     '1',    '2',    '3',    '4',    '5',    '6', 
	'7',    '8',    '9',    '0',    '-',    '=',    K_BACKSPACE, 9, // 0 
	'q',    'w',    'e',    'r',    't',    'y',    'u',    'i', 
	'o',    'p',    '[',    ']',    13 ,    K_CTRL,'a',  's',      // 1 
	'd',    'f',    'g',    'h',    'j',    'k',    'l',    ';', 
	'\'' ,    '`',    K_SHIFT,'\\',  'z',    'x',    'c',    'v',      // 2 
	'b',    'n',    'm',    ',',    '.',    '/',    K_SHIFT,'*', 
	K_ALT,' ',   0  ,    K_F1, K_F2, K_F3, K_F4, K_F5,   // 3 
	K_F6, K_F7, K_F8, K_F9, K_F10,  K_PAUSE,    0  , K_HOME, 
	K_UPARROW,K_PGUP,'-',K_LEFTARROW,'5',K_RIGHTARROW,'+',K_END, //4 
	K_DOWNARROW,K_PGDN,K_INS,K_DEL,0,0,             0,              K_F11, 
	K_F12,0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0,        // 5
	0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0, 
	0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0,        // 6 
	0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0, 
	0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0         // 7 
}; 

/*
=======
MapKey

Map from windows to quake keynums
=======
*/
int MapKey (int key)
{
	key = (key>>16)&255;
	if (key > 127)
		return 0;

	return scantokey[key];
}

/*
================
VID_HandlePause
================
*/
void VID_HandlePause (qboolean pause)
{
	if ((modestate == MS_WINDOWED) && _windowed_mouse.value)
	{
		// for consistency , don't show pointer S.A
		if (pause)
		{
			IN_DeactivateMouse ();
			// IN_ShowMouse ();
		}
		else
		{
			IN_ActivateMouse ();
			// IN_HideMouse ();
		}
	}
}

typedef struct
{
	int		modenum;
	char	*desc;
	int		iscur;
	int		ismode13;
	int		width;
} modedesc_t;

#define MAX_COLUMN_SIZE		5
#define MODE_AREA_HEIGHT	(MAX_COLUMN_SIZE + 6)
#define MAX_MODEDESCS		(MAX_COLUMN_SIZE*3)

/*
================
VID_MenuDraw
================ S.A.
*/

void VID_MenuDraw (void)
{
	M_Print (0*8, 4 + MODE_AREA_HEIGHT * 8 + 8*2,
			 "Select video modes from the command line");
	M_Print (13*8, 4 + MODE_AREA_HEIGHT * 8 + 8*4,
			 "-window");
	M_Print (13*8, 4 + MODE_AREA_HEIGHT * 8 + 8*5,
			 "-fullscreen");
	M_Print (13*8, 4 + MODE_AREA_HEIGHT * 8 + 8*6,
			 "-width  <width>");
	M_Print (13*8, 4 + MODE_AREA_HEIGHT * 8 + 8*7,
			 "-height <height>");
}

void ToggleFullScreenSA ()
{
	if (SDL_WM_ToggleFullScreen(screen)==1) {

		Cvar_SetValue ("vid_mode", !vid_mode.value);
	
	} else {
	    Con_Printf ("SDL_WM_ToggleFullScreen failed\n");
	}
}

/*
================
VID_MenuKey
================
*/
void VID_MenuKey (int key)
{
	switch (key)
	{
/*  unused // Toggle fullscreen/window modes S.A.
	case K_SPACE:
		ToggleFullScreenSA ();
		break;
*/
	case K_ESCAPE:
	case K_ENTER:
		S_LocalSound ("raven/menu1.wav");
		M_Menu_Options_f ();
		break;

	// disabled menu key bindings - S.A. 

	default:
		break;
	}
}

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.9  2004/12/29 19:49:40  sezero
 * From Steven (2004-12-29):
 * - Fullscreen/Windowed mode is now switchable. Seems to work good.
 * - Mouse can now be disabled for fullscreen modes.
 *
 * Revision 1.8  2004/12/18 14:20:40  sezero
 * Clean-up and kill warnings: 11
 * A lot of whitespace cleanups.
 *
 * Revision 1.7  2004/12/18 14:08:08  sezero
 * Clean-up and kill warnings 9:
 * Kill many unused vars.
 *
 * Revision 1.6  2004/12/12 20:01:44  sezero
 * fix another typo so that compilation doesn't bomb-out
 *
 * Revision 1.5  2004/12/12 14:29:35  sezero
 * make WM_SetCaption report PoP if running the mission pack
 *
 * Revision 1.4  2004/12/12 14:14:43  sezero
 * style changes to our liking
 *
 * Revision 1.3  2004/12/05 10:52:18  sezero
 * Sync with Steven, 2004-12-04 :
 *  Fix the "Old Mission" menu PoP
 *  Also release the windowed mouse on pause
 *  Heapsize is now 32768 default
 *  The final splash screens now centre the messages properly
 *  Add more mods to the video mods table
 *  Add the docs folder and update it
 *
 * Revision 1.2  2004/11/28 00:58:08  sezero
 *
 * Commit Steven's changes as of 2004.11.24:
 *
 * * Rewritten Help/Version message(s)
 * * Proper fullscreen mode(s) for OpenGL.
 * * Screen sizes are selectable with "-width" and "-height" options.
 * * Mouse grab in window modes , which is released when menus appear.
 * * Interactive video modes in software game disabled.
 * * Replaced Video Mode menu with a helpful message.
 * * New menu items for GL Glow, Chase mode, Draw Shadows.
 * * Changes to initial cvar_t variables:
 *      r_shadows, gl_other_glows, _windowed_mouse,
 *
 * Revision 1.1.1.1  2004/11/28 00:08:07  sezero
 * Initial import of AoT 1.2.0 code
 *
 * Revision 1.5  2002/01/03 16:14:50  phneutre
 * gamma correction fixes for software mode
 *
 * Revision 1.4  2001/12/13 13:40:35  phneutre
 * implemented gamma correction functions
 *
 * Revision 1.3  2001/12/13 13:16:27  phneutre
 * implement gamma correction functions
 *
 * Revision 1.2  2001/11/12 23:31:58  theoddone33
 * Some Loki-ish parameters and general cleanup/bugfixes.
 *
 * 4     4/01/98 6:43p Jmonroe
 * fixed boundschecker errors
 * 
 * 3     3/01/98 8:20p Jmonroe
 * removed the slow "quake" version of common functions
 * 
 * 2     1/18/98 8:06p Jmonroe
 * all of rick's patch code is in now
 * 
 * 18    9/23/97 8:56p Rjohnson
 * Updates
 * 
 * 17    8/27/97 12:09p Rjohnson
 * Disabled windirect
 * 
 * 16    8/24/97 11:11a Rjohnson
 * Changed default sounds
 * 
 * 15    8/19/97 10:35p Rjohnson
 * Fix for loading plaque
 * 
 * 14    8/18/97 12:03a Rjohnson
 * Added loading progress
 * 
 * 13    8/17/97 3:28p Rjohnson
 * Fix for color selection
 * 
 * 12    8/04/97 3:51p Rjohnson
 * Fix for the menus
 * 
 * 11    8/01/97 3:17p Rjohnson
 * Added new menu graphics
 * 
 * 10    7/28/97 2:44p Rjohnson
 * Fix for tinting
 * 
 * 9     7/24/97 4:37p Rjohnson
 * Color Tinting for models
 * 
 * 8     6/15/97 7:44p Rjohnson
 * Added new pause and loading graphics
 * 
 * 7     3/07/97 2:28p Rjohnson
 * Id Updates
 * 
 * 6     2/18/97 5:00p Rjohnson
 * Id Updates
 */
