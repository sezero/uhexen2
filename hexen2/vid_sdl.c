/*
   vid_sdl.c
   SDL video driver
   Select window size and mode and init SDL in SOFTWARE mode.

   $Id: vid_sdl.c,v 1.25 2005-05-21 17:10:58 sezero Exp $

   Changed by S.A. 7/11/04, 27/12/04

   Options are now:
     -fullscreen | -window, -height , -width 
   Currently bpp is 8 bit , and it seems fairly hardwired at this depth
   Interactive Video Modes switching have been disabled and we shall not
   put it back.

   The modes which are used are the traditional 0 and 3, but have redefined
   MODE_FULLSCREEN_DEFAULT from 3 to 1 to be a boolean:

	vid_mode = MODE_WINDOWED  0 || MODE_FULLSCREEN_DEFAULT 1

   For clarity I have removed the variable "windowed_default" and replaced
   it with MODE_WINDOWED. Functionality is the same.

   Changed by O.S 8/02/05
   - Removed cvar _vid_default_mode_win
   - Removed all mode descriptions
   - Removed all nummodes and VID_NumModes stuff
   - Removed all VID_GetXXX and VID_DescXXX stuff
   - Removed firstupdate things (VID_Update() cleanup)

*/

#include "quakedef.h"
#include "quakeinc.h"
#include "d_local.h"
#include "SDL.h"

#define MAX_MODE_LIST	30
#define VID_ROW_SIZE	3

SDL_Surface	*screen;
byte globalcolormap[VID_GRADES*256], lastglobalcolor = 0;
byte *lastsourcecolormap = NULL;

static qboolean	vid_initialized = false, vid_palettized = true;
static int	lockcount;
qboolean	in_mode_set, is_mode0x13;
static int	vid_stretched, enable_mouse;
static qboolean	palette_changed;

viddef_t	vid;		// global video state

// Note that 0 is MODE_WINDOWED and 1 is MODE_FULLSCREEN_DEFAULT
cvar_t		vid_mode = {"vid_mode","0", false};
cvar_t		_vid_default_mode = {"_vid_default_mode","0", true};
cvar_t		vid_config_x = {"vid_config_x","800", true};
cvar_t		vid_config_y = {"vid_config_y","600", true};
cvar_t		vid_stretch_by_2 = {"vid_stretch_by_2","1", true};
cvar_t		_enable_mouse = {"_enable_mouse","1", true};
cvar_t		vid_showload = {"vid_showload", "1"};

int		vid_modenum = NO_MODE;
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

void VID_MenuDraw (void);
void VID_MenuKey (int key);
void VID_SetGamma(float value);
void VID_SetGamma_f(void);


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
		 0x10000 * 3) < MINIMUM_MEMORY)
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
		 0x10000 * 3) < MINIMUM_MEMORY)
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
	else
		Con_SafePrintf ("Video Mode: %d x %d x %d\n", vid.width, vid.height, modelist[modenum].bpp);

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

// S.A. did this Cvar_SetValue & "if 0" 
// It's pretty messy in here. Dunno much about it, probably alot of it
// could go as it's used in dynamically changing/testing video modes

	Cvar_SetValue ("vid_mode", (float)modenum);

	if (modenum == vid_modenum)
		return true;

	// so Con_Printfs don't mess us up by forcing vid and snd updates
	temp = scr_disabled_for_loading;
	scr_disabled_for_loading = true;
	in_mode_set = true;

	if (vid_modenum == NO_MODE)
		original_mode = windowed_default;
	else
		original_mode = vid_modenum;

	/* Set either the fullscreen or windowed mode */

	if (modelist[modenum].type == MS_WINDOWED)
		stat = VID_SetWindowedMode(modenum);
	else
		stat = VID_SetFullscreenMode(modenum);

	IN_HideMouse ();

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

	VID_SetPalette (palette);

#if defined(H2W)
	SDL_WM_SetCaption("HexenWorld", "HexenWorld");
#elif defined(H2MP)
	SDL_WM_SetCaption("Portal of Praevus", "PRAEVUS");
#else
	SDL_WM_SetCaption("Hexen II", "HEXEN2");
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


void	VID_Init (unsigned char *palette)
{
	int		i, bestmatch, bestmatchmetric, t, dr, dg, db;
	int		width,height;
	byte	*ptmp;

	if (SDL_Init(SDL_INIT_VIDEO)<0)
		Sys_Error("VID: Couldn't load SDL: %s", SDL_GetError());
		
	Cvar_RegisterVariable (&vid_mode);
	Cvar_RegisterVariable (&_vid_default_mode);
	Cvar_RegisterVariable (&vid_config_x);
	Cvar_RegisterVariable (&vid_config_y);
	Cvar_RegisterVariable (&vid_stretch_by_2);
	Cvar_RegisterVariable (&_enable_mouse);
	Cvar_RegisterVariable (&vid_showload);

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


	/*********************************
 	 * command line processing (S.A) *
 	 *********************************/

	// default mode is windowed
	vid_default = MODE_WINDOWED;

	// see if the user wants fullscreen
	if (COM_CheckParm("-fullscreen") || COM_CheckParm ("--fullscreen")
			|| COM_CheckParm ("-f") || COM_CheckParm ("-fs"))
	{
		vid_default = MODE_FULLSCREEN_DEFAULT;
	}

	if (COM_CheckParm("-width"))
	{
		width = atoi(com_argv[COM_CheckParm("-width")+1]);
		if (COM_CheckParm("-height")) {
			// we removed the old stuff, and now we need
			// some sanity check here...
			height = atoi(com_argv[COM_CheckParm("-height")+1]);
		} else {
			// we currently do "normal" modes with 4/3 ratio
			height = 3 * width / 4;
		}
	}
	else
	{
		// make the default sizes for software mode smaller
		if (vid_default == MODE_WINDOWED)
			width=400;
		else
			width=512;

		// we currently do "normal" modes with 4/3 ratio
		height = 3 * width / 4;
	}

	modelist[vid_default].width = width;
	modelist[vid_default].height = height;
	sprintf (modelist[vid_default].modedesc,"%dx%d",width,height);

	vid_initialized = true;

	VID_SetMode (vid_default, palette);

	VID_SetPalette (palette);	// Useless (?) - DDOI

	vid_menudrawfn = VID_MenuDraw;
	vid_menukeyfn = VID_MenuKey;
}


void	VID_Shutdown (void)
{
	extern byte *transTable;	//from r_part
	extern byte *mainTransTable;	// in r_main.c

	if (vid_initialized)
	{
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

	// We've drawn the frame; copy it to the screen
	FlipScreen (rects);

	// handle the mouse state when windowed if that's changed
#if 0	// change to 1 if dont want to disable mouse in fullscreen
	if (modestate == MS_WINDOWED)
#endif
		if ((int)_enable_mouse.value != enable_mouse)
		{
			if (_enable_mouse.value)
				IN_ActivateMouse ();
			else
				IN_DeactivateMouse ();

			enable_mouse = (int)_enable_mouse.value;
		}
}


/*
================
D_BeginDirectRect
================
*/
void D_BeginDirectRect (int x, int y, byte *pbitmap, int width, int height)
{
}


#ifndef H2W
// unused in hexenworld
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
#endif


/*
================
D_EndDirectRect
================
*/
void D_EndDirectRect (int x, int y, int width, int height)
{
}

/*
============================
Gamma functions for UNIX/SDL
============================
*/

void VID_SetGamma(float value)
{
	SDL_SetGamma(value,value,value);
}

void VID_ApplyGamma (void)
{
	if ((v_gamma.value != 0)&&(v_gamma.value > (1/GAMMA_MAX)))
		VID_SetGamma(1/v_gamma.value);
	else
		VID_SetGamma(GAMMA_MAX);
}

void VID_SetGamma_f (void)
{
	float value = 1;

	value = atof (Cmd_Argv(1));

	if (value != 0) {
		if (value > (1/GAMMA_MAX))
			v_gamma.value = value;
		else
			v_gamma.value =  1/GAMMA_MAX;
	}

	/* if value==0 , just apply current settings.
	   this is usefull at startup */

	VID_ApplyGamma();
}

//==========================================================================
#ifndef H2W
/*
================
VID_HandlePause
================
*/
void VID_HandlePause (qboolean pause)
{
#if 0	// change to 1 if dont want to disable mouse in fullscreen
	if ((modestate == MS_WINDOWED) && _enable_mouse.value)
#else
	if (_enable_mouse.value)
#endif
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
#endif

#define MAX_COLUMN_SIZE		5
#define MODE_AREA_HEIGHT	(MAX_COLUMN_SIZE + 6)

/*
================
VID_MenuDraw
================ S.A.
*/

void VID_MenuDraw (void)
{
	ScrollTitle("gfx/menu/title7.lmp");

	M_Print (8*8, 4 + MODE_AREA_HEIGHT * 8 + 8*0,
			 "Select video modes");
	M_Print (8*8, 4 + MODE_AREA_HEIGHT * 8 + 8*1,
			 "from the command line:");
	M_Print (13*8, 4 + MODE_AREA_HEIGHT * 8 + 8*4,
			 "-window");
	M_Print (13*8, 4 + MODE_AREA_HEIGHT * 8 + 8*5,
			 "-fullscreen");
	M_Print (13*8, 4 + MODE_AREA_HEIGHT * 8 + 8*6,
			 "-width  <width>");
	M_Print (13*8, 4 + MODE_AREA_HEIGHT * 8 + 8*7,
			 "-height <height>");
}

/*
================
ToggleFullScreenSA
Handles switching between fullscreen/windowed modes
and brings the mouse to a proper state afterwards
================
*/
extern qboolean mousestate_sa;
void ToggleFullScreenSA ()
{
	if (SDL_WM_ToggleFullScreen(screen)==1) {
		Cvar_SetValue ("vid_mode", !vid_mode.value);
		modestate = (vid_mode.value) ? MODE_FULLSCREEN_DEFAULT : MODE_WINDOWED;
#if 0	// change to 1 if dont want to disable mouse in fullscreen
		if (vid_mode.value) {
			if (!_enable_mouse.value) {
				// activate the mouse if not in menus
				if (!mousestate_sa)
					IN_ActivateMouse();
			}
		} else {
			if (!_enable_mouse.value)
				IN_DeactivateMouse();
		}
#endif
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
	case K_ESCAPE:
	case K_ENTER:
		S_LocalSound ("raven/menu1.wav");
		M_Menu_Options_f ();
		break;
	default:
		break;
	}
}

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.24  2005/05/21 17:04:16  sezero
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
 * Revision 1.23  2005/05/21 12:55:31  sezero
 * more syncing of h2/hw vid_sdl
 *
 * Revision 1.22  2005/05/21 09:51:48  sezero
 * more syncing of h2/hw vid_sdl
 *
 * Revision 1.21  2005/05/21 09:17:19  sezero
 * synced wm caption code
 *
 * Revision 1.20  2005/05/21 08:56:11  sezero
 * MINIMUM_MEMORY_LEVELPAK was never used, switched to MINIMUM_MEMORY
 *
 * Revision 1.19  2005/05/20 15:26:33  sezero
 * separated winquake.h into winquake.h and linquake.h
 * changed all occurances of winquake.h to quakeinc.h,
 * which includes the correct header
 *
 * Revision 1.18  2005/04/30 09:59:17  sezero
 * Many things in gl_vidsdl.c, and *especially in vid_sdl.c, are there
 * for the dynamic video mode swithching which we removed a long time
 * ago (and we don't seem to put it back in any foreseeable future.)
 * Some stuff were there only to provide human readable descriptions in
 * the menu and I removed them in 1.2.3 or in 1.2.4. In this patch:
 * 1. Tried cleaning-up the remaining mess: There still were some
 *    windoze left-overs, unused variables/cvars, functions using those
 *    vars/cvars serving no purpose (especially those window_rect and
 *    window_center stuff, and more). I removed them as best as I could.
 *    There still are things in vid_sdl.c that I didn't fully understand,
 *    they are there, for now.
 * 2. The -window and -w cmdline args are now now removed: They actually
 *    did nothing, unless the user did some silly thing like using both
 *    -w and -f on the same cmdline.
 * 3. The two mode-setting functions (windowed and f/s) are made into one
 *    as VID_SDL_SetMode
 * 4. The -height arg now is functional *only* if used together -height.
 *    Since we only do the normal modes, I removed the width switch and
 *    calculated:  height = 3*width/4
 *    Issue: We need some sanity check in case of both -width and -height
 *    args are specified
 * 5. -bpp wasn't written into modenum[x].bpp, I did it here. As a side
 *    note, bpp doesn't affect anything, or my eyes are in more need of a
 *    doctor than I know: -bpp 8 / 16 / 32 give the same picture.
 * 6. The code calls VID_SetPalette very multiple times in gl_vidsdl.c.
 *    Why the hell is that?.. Something windoze spesific?  I unified them
 *    here in VID_Init: After VID_SetMode, VID_SetPalette is called first,
 *    and then 8-bit palette is activated if -paltex is specified.
 *    Note: I didn't touch vid_sdl.c in this manner, but DDOI (one of the
 *    guys during Dan's porting, perpahs) has a comment on a VID_SetPalette
 *    call being "Useless".
 * 7. Many whitespace clean-up as a bonus material ;)
 *
 * Revision 1.17  2005/04/09 22:16:02  sezero
 * Removed scankey[] and MapKey(), unused for SDL/PLATFORM_UNIX
 * Did the same unused/cosmetic clean-up in vid_sdl.c which was
 * done in gl_vidsdl.c
 *
 * Revision 1.16  2005/03/13 16:00:17  sezero
 * added back console video mode reporting
 * also removed the non-functional findbpp
 *
 * Revision 1.15  2005/02/12 13:24:28  sezero
 * missing scrolltitle in the software version of VID_MenuDraw()
 *
 * Revision 1.14  2005/02/11 08:33:55  sezero
 * Kill sound/music messing in vid code: S_Init() and CDAudio_Init() are
 * called AFTER than VID_Init(), so this should be correct. See hexen2/host.c
 * for reasoning on Win32.
 *
 * Revision 1.13  2005/02/08 21:12:36  sezero
 * video cleanup:
 * * removed cvar _vid_default_mode_win
 * * removed all mode descriptions
 * * removed all nummodes and VID_NumModes stuff
 * * removed all VID_GetXXX and VID_DescXXX stuff
 * * beautified the Video Modes menu screen a little
 * * removed firstupdate things (VID_Update() cleanup)
 *
 * Revision 1.12  2005/02/06 15:03:10  sezero
 * move resource.h to ./win_stuff/
 *
 * Revision 1.11  2005/01/12 11:59:11  sezero
 * remove some commented-out win32-stuff
 *
 * Revision 1.10  2005/01/01 21:43:47  sezero
 * prototypes clean-up
 *
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
