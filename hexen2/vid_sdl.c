/*
	vid_sdl.c
	SDL video driver
	Select window size and mode and init SDL in SOFTWARE mode.

	$Id: vid_sdl.c,v 1.52 2006-07-04 21:03:17 sezero Exp $

	Changed by S.A. 7/11/04, 27/12/04
	Options are now: -fullscreen | -window, -height , -width
	Currently bpp is 8 bit , and it seems fairly hardwired at this depth

	Changed by O.S 7/01/06
	- Added video modes enumeration via SDL
	- Added video mode changing on the fly.
*/

#include "quakedef.h"
#include "d_local.h"
#include "sdl_inc.h"

#define MIN_WIDTH		320
//#define MIN_HEIGHT		200
#define MIN_HEIGHT		240

unsigned char	vid_curpal[256*3];
unsigned short	d_8to16table[256];
unsigned	d_8to24table[256];

byte globalcolormap[VID_GRADES*256], lastglobalcolor = 0;
byte *lastsourcecolormap = NULL;

//intermission screen cache reference (to flush on video mode switch)
extern cache_user_t	*intermissionScreen;

static qboolean	vid_initialized = false;
static int	lockcount;
qboolean	in_mode_set;
static int	enable_mouse;
static qboolean	palette_changed;

static int	num_fmodes;
static int	num_wmodes;
static int	*nummodes;
//static int	bpp = 8;
static const SDL_VideoInfo	*vid_info;
static SDL_Surface	*screen;
static qboolean	vid_menu_fs;
static qboolean	fs_toggle_works = true;

viddef_t	vid;		// global video state
// cvar vid_mode must be set before calling VID_SetMode, VID_ChangeVideoMode or VID_Restart_f
static cvar_t	vid_mode = {"vid_mode", "0", CVAR_NONE};
static cvar_t	vid_config_glx = {"vid_config_glx", "640", CVAR_ARCHIVE};
static cvar_t	vid_config_gly = {"vid_config_gly", "480", CVAR_ARCHIVE};
static cvar_t	vid_config_swx = {"vid_config_swx", "320", CVAR_ARCHIVE};
static cvar_t	vid_config_swy = {"vid_config_swy", "240", CVAR_ARCHIVE};
static cvar_t	vid_config_fscr= {"vid_config_fscr", "0", CVAR_ARCHIVE};

static cvar_t	vid_showload = {"vid_showload", "1", CVAR_NONE};

cvar_t		_enable_mouse = {"_enable_mouse", "1", CVAR_ARCHIVE};

static int	vid_default = -1;	// modenum of 320x240 as a safe default
static int	vid_modenum = -1;	// current video mode, set after mode setting succeeds
static int	vid_maxwidth = 640, vid_maxheight = 480;
modestate_t	modestate = MS_UNINIT;

static byte		*vid_surfcache;
static int		vid_surfcachesize;
static int		VID_highhunkmark;

typedef struct {
	modestate_t	type;
	int			width;
	int			height;
	int			modenum;
	int			fullscreen;
	int			bpp;
	int			halfscreen;
	char		modedesc[13];
} vmode_t;

typedef struct {
	int	width;
	int	height;
} stdmode_t;

#define RES_640X480	3
static const stdmode_t	std_modes[] = {
// NOTE: keep this list in order
	{320, 240},	// 0
	{400, 300},	// 1
	{512, 384},	// 2
	{640, 480},	// 3 == RES_640X480, this is our default, below
			//		this is the lowresmodes region.
			//		either do not change its order,
			//		or change the above define, too
	{800,  600},	// 4, RES_640X480 + 1
	{1024, 768}	// 5, RES_640X480 + 2
};

#define MAX_MODE_LIST	30
#define MAX_STDMODES	(sizeof(std_modes) / sizeof(std_modes[0]))
#define NUM_LOWRESMODES	(RES_640X480)
static vmode_t	fmodelist[MAX_MODE_LIST+1];	// list of enumerated fullscreen modes
static vmode_t	wmodelist[MAX_STDMODES +1];	// list of standart 4:3 windowed modes
static vmode_t	*modelist;	// modelist in use, points to one of the above lists

static int VID_SetMode (int modenum, unsigned char *palette);

void VID_MenuDraw (void);
void VID_MenuKey (int key);

// window manager stuff
#if defined(H2W)
#	define WM_TITLEBAR_TEXT	"HexenWorld"
#	define WM_ICON_TEXT	"HexenWorld"
#elif defined(H2MP)
#	define WM_TITLEBAR_TEXT	"Hexen II+"
#	define WM_ICON_TEXT	"HEXEN2MP"
#else
#	define WM_TITLEBAR_TEXT	"Hexen II"
#	define WM_ICON_TEXT	"HEXEN2"
#endif

//====================================

/*
================
ClearAllStates
================
*/
static void ClearAllStates (void)
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
	//if ((host_parms.memsize - tbuffersize + SURFCACHE_SIZE_AT_320X200 +
	//	 0x10000 * 3) < MINIMUM_MEMORY)
	// Pa3PyX: using hopefully better estimation now
	// if total memory < needed surface cache + (minimum operational memory
	// less surface cache for 320x200 and typical hunk state after init)
	if (host_parms.memsize < tbuffersize + 0x180000 + 0xC00000)
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
	//if ((host_parms.memsize - tbuffersize + SURFCACHE_SIZE_AT_320X200 +
	//	 0x10000 * 3) < MINIMUM_MEMORY)
	// Pa3PyX: using hopefully better estimation now
	// Experimentation: the heap should have at least 12.0 megs
	// remaining (after init) after setting video mode, otherwise
	// it's Hunk_Alloc failures and cache thrashes upon level load
	if (host_parms.memsize < tbuffersize + 0x180000 + 0xC00000)
	{
		return false;		// not enough memory for mode
	}

	return true;
}

static void VID_SetIcon (void)
{
	SDL_Surface *icon;
	SDL_Color color;
	Uint8 *ptr;
	int i, mask;
#	include "xbm_icon.h"

	icon = SDL_CreateRGBSurface(SDL_SWSURFACE, HOT_ICON_WIDTH, HOT_ICON_HEIGHT, 8, 0, 0, 0, 0);
	if (icon == NULL)
		return;	/* oh well... */
	SDL_SetColorKey(icon, SDL_SRCCOLORKEY, 0);

	color.r = 255;
	color.g = 255;
	color.b = 255;
	SDL_SetColors(icon, &color, 0, 1);	/* just in case */
	color.r = 128;
	color.g = 0;
	color.b = 0;
	SDL_SetColors(icon, &color, 1, 1);

	ptr = (Uint8 *)icon->pixels;
	for (i = 0; i < sizeof(HOT_ICON_bits); i++)
	{
		for (mask = 1; mask != 0x100; mask <<= 1) {
			*ptr = (HOT_ICON_bits[i] & mask) ? 1 : 0;
			ptr++;
		}		
	}

	SDL_WM_SetIcon(icon, NULL);
	SDL_FreeSurface(icon);
}

static int sort_modes (const void *arg1, const void *arg2)
{
	const SDL_Rect *a1, *a2;
	a1 = *(SDL_Rect **) arg1;
	a2 = *(SDL_Rect **) arg2;

	if (a1->w == a2->w)
		return a1->h - a2->h;	// lowres-to-highres
	//	return a2->h - a1->h;	// highres-to-lowres
	else
		return a1->w - a2->w;	// lowres-to-highres
	//	return a2->w - a1->w;	// highres-to-lowres
}

static void VID_PrepareModes (SDL_Rect **sdl_modes)
{
	int	i, j;
	qboolean	have_mem, not_multiple;

	num_fmodes = 0;
	num_wmodes = 0;

	// Add the standart 4:3 modes to the windowed modes list
	// In an unlikely case that we receive no fullscreen modes,
	// this will be our modes list (kind of...)
	for (i = 0; i < MAX_STDMODES; i++)
	{
		have_mem = VID_CheckAdequateMem(std_modes[i].width, std_modes[i].height);
		if (!have_mem)
			break;
		wmodelist[num_wmodes].width = std_modes[i].width;
		wmodelist[num_wmodes].height = std_modes[i].height;
		wmodelist[num_wmodes].halfscreen = 0;
		wmodelist[num_wmodes].fullscreen = 0;
		wmodelist[num_wmodes].bpp = 8;
		sprintf (wmodelist[num_wmodes].modedesc,"%d x %d",std_modes[i].width,std_modes[i].height);
		num_wmodes++;
	}

	// disaster scenario #1: no fullscreen modes. bind to the
	// windowed modes list. limit it to 640x480 max. because
	// we don't know the desktop dimensions
	if (sdl_modes == (SDL_Rect **)0)
	{
no_fmodes:
		Con_Printf ("No fullscreen video modes available\n");
		if (num_wmodes > RES_640X480)
			num_wmodes = RES_640X480 + 1;
		modelist = (vmode_t *)wmodelist;
		nummodes = &num_wmodes;
		vid_default = 0;
		Cvar_SetValue ("vid_config_swx", modelist[vid_default].width);
		Cvar_SetValue ("vid_config_swy", modelist[vid_default].height);
		return;
	}

	// another disaster scenario (#2)
	if (sdl_modes == (SDL_Rect **)-1)
	{	// Really should NOT HAVE happened! this return value is
		// for windowed modes!  Since this means all resolutions
		// are supported, use our standart modes as modes list.
		Con_Printf ("Unexpectedly received -1 from SDL_ListModes\n");
		vid_maxwidth = MAXWIDTH;
		vid_maxheight = MAXHEIGHT;
	//	num_fmodes = -1;
		num_fmodes = num_wmodes;
		nummodes = &num_wmodes;
		modelist = (vmode_t *)wmodelist;
		vid_default = 0;
		Cvar_SetValue ("vid_config_swx", modelist[vid_default].width);
		Cvar_SetValue ("vid_config_swy", modelist[vid_default].height);
		return;
	}

#if 0
	// print the un-processed modelist as reported by SDL
	for (j = 0; sdl_modes[j]; ++j)
	{
		Con_Printf ("%d x %d\n", sdl_modes[j]->w, sdl_modes[j]->h);
	}
	Con_Printf ("Total %d entries\n", j);
#endif

	// count the entries
	j = 0;
	while ( sdl_modes[j] )
		j++;

	// sort the original list from low-res to high-res
	// so that the low resolutions take priority
	qsort(sdl_modes, j, sizeof *sdl_modes, sort_modes);

	for (i = 0; sdl_modes[i] && num_fmodes < MAX_MODE_LIST; ++i)
	{
		// avoid multiple listings of the same dimension
		not_multiple = true;
		for (j = 0; j < num_fmodes; ++j)
		{
			if (fmodelist[j].width == sdl_modes[i]->w && fmodelist[j].height == sdl_modes[i]->h)
			{
				not_multiple = false;
				break;
			}
		}

		// automatically strip-off resolutions that we
		// don't have enough memory for
		have_mem = VID_CheckAdequateMem(sdl_modes[i]->w, sdl_modes[i]->h);

		// avoid resolutions < 320x240
		if (not_multiple && have_mem && sdl_modes[i]->w >= MIN_WIDTH && sdl_modes[i]->h >= MIN_HEIGHT)
		{
			fmodelist[num_fmodes].width = sdl_modes[i]->w;
			fmodelist[num_fmodes].height = sdl_modes[i]->h;
			// FIXME: look at vid_win.c and learn how to
			// really functionalize the halfscreen field.
			fmodelist[num_fmodes].halfscreen = 0;
			fmodelist[num_fmodes].fullscreen = 1;
			fmodelist[num_fmodes].bpp = 8;
			sprintf (fmodelist[num_fmodes].modedesc,"%d x %d",sdl_modes[i]->w,sdl_modes[i]->h);
			num_fmodes++;
		}
	}

	if (!num_fmodes)
		goto no_fmodes;

	// At his point, we have a list of valid fullscreen modes:
	// Let's bind to it and use it for windowed modes, as well.
	// The only downside is that if SDL doesn't report any low
	// resolutions to us, we shall not have any for windowed
	// rendering where they would be perfectly legitimate...
	// Since our fullscreen/windowed toggling is instant and
	// doesn't require a vid_restart, switching lists won't be
	// feasible, either. The -width/-height commandline args
	// remain as the user's trusty old friends here.
	nummodes = &num_fmodes;
	modelist = (vmode_t *)fmodelist;

	vid_maxwidth = fmodelist[num_fmodes-1].width;
	vid_maxheight = fmodelist[num_fmodes-1].height;

	// see if we have 320x240 among the available modes
	for (i = 0; i < num_fmodes; i++)
	{
		if (fmodelist[i].width == 320 && fmodelist[i].height == 240)
		{
			vid_default = i;
			break;
		}
	}

	if (vid_default < 0)
	{
		// 320x240 not found among the supported dimensions
		// set default to the lowest resolution reported
		vid_default = 0;
	}

	// limit the windowed (standart) modes list to desktop dimensions
	for (i = 0; i < num_wmodes; i++)
	{
		if (wmodelist[i].width > vid_maxwidth || wmodelist[i].height > vid_maxheight)
			break;
	}
	if (i < num_wmodes)
		num_wmodes = i;

	Cvar_SetValue ("vid_config_swx", modelist[vid_default].width);
	Cvar_SetValue ("vid_config_swy", modelist[vid_default].height);
}

static void VID_ListModes_f (void)
{
	int	i;

	Con_Printf ("Maximum allowed mode: %d x %d\n", vid_maxwidth, vid_maxheight);
	Con_Printf ("Windowed modes enabled:\n");
	for (i = 0; i < num_wmodes; i++)
		Con_Printf ("%2d:  %u x %u\n", i, wmodelist[i].width, wmodelist[i].height);
	Con_Printf ("Fullscreen modes enumerated:");
	if (num_fmodes)
	{
		Con_Printf ("\n");
		for (i = 0; i < num_fmodes; i++)
			Con_Printf ("%2d:  %u x %u\n", i, fmodelist[i].width, fmodelist[i].height);
	}
	else
	{
		Con_Printf (" None\n");
	}
}

static void VID_NumModes_f (void)
{
	Con_Printf ("%d video modes in current list\n", *nummodes);
}

static void VID_ShowInfo_f (void)
{
	Con_Printf ("Video info:\n"
			"BitsPerPixel: %d,\n"
			"Rmask : %u, Gmask : %u, Bmask : %u\n"
			"Rshift: %u, Gshift: %u, Bshift: %u\n"
			"Rloss : %u, Gloss : %u, Bloss : %u\n"
			"alpha : %u, colorkey: %u\n",
			vid_info->vfmt->BitsPerPixel,
			vid_info->vfmt->Rmask, vid_info->vfmt->Gmask, vid_info->vfmt->Bmask,
			vid_info->vfmt->Rshift, vid_info->vfmt->Gshift, vid_info->vfmt->Bshift,
			vid_info->vfmt->Rloss, vid_info->vfmt->Gloss, vid_info->vfmt->Bloss,
			vid_info->vfmt->alpha, vid_info->vfmt->colorkey	);
}

static int VID_SetMode (int modenum, unsigned char *palette)
{
	Uint32 flags;
	int	is_fullscreen;

	in_mode_set = true;

	//flush the intermission screen if it's cached (Pa3PyX)
	if (intermissionScreen && intermissionScreen->data)
		Cache_Free(intermissionScreen);

	if (screen)
		SDL_FreeSurface(screen);

	flags = (SDL_SWSURFACE|SDL_HWPALETTE);
	if ((int)vid_config_fscr.value)
		flags |= SDL_FULLSCREEN;

	// Set the mode
	screen = SDL_SetVideoMode(modelist[modenum].width, modelist[modenum].height, modelist[modenum].bpp, flags);
	if (!screen)
		return false;

	// initial success. adjust vid vars.
	vid.height = vid.conheight = modelist[modenum].height;
	vid.width = vid.conwidth = modelist[modenum].width;
	vid.buffer = vid.conbuffer = vid.direct = screen->pixels;
	vid.rowbytes = vid.conrowbytes = screen->pitch;
	vid.numpages = 1;
	vid.aspect = ((float)vid.height / (float)vid.width) * (320.0 / 240.0);

	if (!VID_AllocBuffers (vid.width, vid.height))
		return false;

	D_InitCaches (vid_surfcache, vid_surfcachesize);

	// real success. set vid_modenum properly.
	vid_modenum = modenum;
	is_fullscreen = (screen->flags & SDL_FULLSCREEN) ? 1 : 0;
	modestate = (is_fullscreen) ? MS_FULLDIB : MS_WINDOWED;
	Cvar_SetValue ("vid_config_swx", modelist[vid_modenum].width);
	Cvar_SetValue ("vid_config_swy", modelist[vid_modenum].height);
	Cvar_SetValue ("vid_config_fscr", is_fullscreen);

	IN_HideMouse ();

	ClearAllStates();

	VID_SetPalette (palette);

	// setup the window manager stuff
	VID_SetIcon();
	SDL_WM_SetCaption(WM_TITLEBAR_TEXT, WM_ICON_TEXT);

	Con_SafePrintf ("Video Mode: %dx%dx%d\n", vid.width, vid.height, modelist[modenum].bpp);

	in_mode_set = false;
	vid.recalc_refdef = 1;

	return true;
}

//
// VID_ChangeVideoMode
// intended only as a callback for VID_Restart_f
//
static void VID_ChangeVideoMode(int newmode)
{
	int		stat, temp;

	if (!screen)
		return;

	temp = scr_disabled_for_loading;
	scr_disabled_for_loading = true;
	CDAudio_Pause ();
	MIDI_Pause(2);
	S_ClearBuffer ();

	stat = VID_SetMode (newmode, vid_curpal);
	if (!stat)
	{
		if (vid_modenum == newmode)
			Sys_Error ("Couldn't set video mode: %s", SDL_GetError());

		// failed setting mode, probably due to insufficient
		// memory. go back to previous mode.
		Cvar_SetValue ("vid_mode", vid_modenum);
		stat = VID_SetMode (vid_modenum, vid_curpal);
		if (!stat)
			Sys_Error ("Couldn't set video mode: %s", SDL_GetError());
	}

	CDAudio_Resume (); 
	MIDI_Pause(1);
	scr_disabled_for_loading = temp;
}

static void VID_Restart_f (void)
{
	if ((int)vid_mode.value < 0 || (int)vid_mode.value >= *nummodes)
	{
		Con_Printf ("Bad video mode %d\n", (int)vid_mode.value);
		Cvar_SetValue ("vid_mode", vid_modenum);
		return;
	}

	Con_Printf ("Re-initializing video:\n");
	VID_ChangeVideoMode ((int)vid_mode.value);
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
===================
VID_EarlyReadConfig

performs an early read of config.cfg. a temporary
solution until we merge a better cvar system.
===================
*/
void	VID_EarlyReadConfig (void)
{
	FILE	*cfg_file;
	char	buff[1024], tmp[256];
	int		i, j, len;
	char *read_vars[] = {
		"vid_config_fscr",
		"vid_config_swx",
		"vid_config_swy",
		NULL
	};

	len = COM_FOpenFile ("config.cfg", &cfg_file, true);
	if (!cfg_file)
		return;

	do {
		fgets(buff, sizeof(buff), cfg_file);
		if (!feof(cfg_file))
		{
			len = strlen(buff);
			buff[len-1] = '\0';

			for (i = 0; read_vars[i]; i++)
			{
				if (strstr(buff, va("%s \"",read_vars[i])) == buff)
				{
					j = strlen(read_vars[i]);
					memset (tmp, 0, sizeof(tmp));

				// we expect a line in the format that Cvar_WriteVariables
				// writes to the config file. if the user screws it up
				// by editing it, it is his fault.
				// the first +2 is for the separating space and the initial
				// quotation mark. the -3 is the first 2 plus the finishing
				// quotation mark.
					memcpy (tmp, buff+j+2, len-j-3);
					tmp[len-j-4] = '\0';
					Cvar_Set (read_vars[i], tmp);
					break;
				}
			}
		}
	} while (!feof(cfg_file));

	fclose (cfg_file);
}

static void VID_LockCvars (void)
{
	// prevent the early-read cvar values to get overwritten by the
	// actual final read of config.cfg (which will be the case when
	// commandline overrides were used):  mark them read only until
	// Host_Init() completely finishes its job. this is a temporary
	// solution until we adopt a better init sequence employing the
	// +set arguments like those in quake2 and quake3.
	vid_config_fscr.flags |= CVAR_ROM;
	vid_config_swx.flags |= CVAR_ROM;
	vid_config_swy.flags |= CVAR_ROM;
}

static void VID_UnlockCvars (void)
{
	// to be called from Host_Init() after execing
	// hexen.rc and flushing the command buffer:
	// remove the r/o bit from the relevant cvars.
	vid_config_fscr.flags &= ~CVAR_ROM;
	vid_config_swx.flags &= ~CVAR_ROM;
	vid_config_swy.flags &= ~CVAR_ROM;
}

void VID_PostInitFix (void)
{
	VID_UnlockCvars ();
}

/*
===================
VID_Init
===================
*/
void	VID_Init (unsigned char *palette)
{
	int		width, height, i, temp;
	SDL_Rect	**enumlist;

	temp = scr_disabled_for_loading;
	scr_disabled_for_loading = true;

	Cvar_RegisterVariable (&vid_config_fscr);
	Cvar_RegisterVariable (&vid_config_swy);
	Cvar_RegisterVariable (&vid_config_swx);
	Cvar_RegisterVariable (&vid_config_gly);
	Cvar_RegisterVariable (&vid_config_glx);
	Cvar_RegisterVariable (&vid_mode);
	Cvar_RegisterVariable (&_enable_mouse);
	Cvar_RegisterVariable (&vid_showload);

	Cmd_AddCommand ("vid_showinfo", &VID_ShowInfo_f);
	Cmd_AddCommand ("vid_listmodes", &VID_ListModes_f);
	Cmd_AddCommand ("vid_nummodes", &VID_NumModes_f);
	Cmd_AddCommand ("vid_restart", &VID_Restart_f);

	// init sdl
	// the first check is actually unnecessary
	if ( (SDL_WasInit(SDL_INIT_AUDIO)) == 0 )
		if (SDL_Init(SDL_INIT_VIDEO) < 0)
			Sys_Error("VID: Couldn't load SDL: %s", SDL_GetError());

	// this will contain the "best bpp" for the current display
	// make sure to re-retrieve it if you ever re-init sdl_video
	vid_info = SDL_GetVideoInfo();

	// retrieve the list of fullscreen modes
	enumlist = SDL_ListModes(NULL, SDL_SWSURFACE|SDL_HWPALETTE|SDL_FULLSCREEN);
	// prepare the modelists, find the actual modenum for vid_default
	VID_PrepareModes(enumlist);

	// set vid_mode to our safe default first
	Cvar_SetValue ("vid_mode", vid_default);

	// perform an early read of config.cfg
	VID_EarlyReadConfig();

	// windowed mode is default
	// see if the user wants fullscreen
	if (COM_CheckParm("-f") || COM_CheckParm("-fullscreen") || COM_CheckParm("--fullscreen"))
	{
		Cvar_SetValue("vid_config_fscr", 1);
	}
	else if (COM_CheckParm("-w") || COM_CheckParm("-window") || COM_CheckParm("--window"))
	{
		Cvar_SetValue("vid_config_fscr", 0);
	}

	if ((int)vid_config_fscr.value && !num_fmodes) // FIXME: see below, as well
		Sys_Error ("No fullscreen modes available at this color depth");

	width = (int)vid_config_swx.value;
	height = (int)vid_config_swy.value;

	// user is always right ...
	if (COM_CheckParm("-width"))
	{	// FIXME: this part doesn't know about a disaster case
		// like we aren't reported any fullscreen modes.
		width = atoi(com_argv[COM_CheckParm("-width")+1]);

		if (COM_CheckParm("-height"))
			height = atoi(com_argv[COM_CheckParm("-height")+1]);
		else	// proceed with 4/3 ratio
			height = 3 * width / 4;
	}

	// user requested a mode either from the config or from the
	// command line
	// scan existing modes to see if this is already available
	// if not, add this as the last "valid" video mode and set
	// vid_mode to it only if it doesn't go beyond vid_maxwidth
	i = 0;
	while (i < *nummodes)
	{
		if (modelist[i].width == width && modelist[i].height == height)
			break;
		i++;
	}
	if (i < *nummodes)
	{
		Cvar_SetValue ("vid_mode", i);
	}
	else if ( (width <= vid_maxwidth && width >= MIN_WIDTH &&
		   height <= vid_maxheight && height >= MIN_HEIGHT) ||
		  COM_CheckParm("-force") )
	{
		modelist[*nummodes].width = width;
		modelist[*nummodes].height = height;
		modelist[*nummodes].halfscreen = 0;
		modelist[*nummodes].fullscreen = 1;
		modelist[*nummodes].bpp = 8;
		sprintf (modelist[*nummodes].modedesc,"%d x %d (user mode)",width,height);
		Cvar_SetValue ("vid_mode", *nummodes);
		(*nummodes)++;	// ugly, I know. but works
	}
	else
	{
		Con_Printf ("ignoring invalid -width and/or -height arguments\n");
	}

	vid.maxwarpwidth = WARP_WIDTH;
	vid.maxwarpheight = WARP_HEIGHT;
	vid.colormap = host_colormap;
	vid.fullbright = 256 - LittleLong (*((int *)vid.colormap + 2048));

	i = VID_SetMode((int)vid_mode.value, palette);
	if ( !i )
	{
		if ((int)vid_mode.value == vid_default)
			Sys_Error ("Couldn't set video mode: %s", SDL_GetError());

		// just one more try before dying
		Con_Printf ("Couldn't set video mode %d\n"
			    "Trying the default mode\n", (int)vid_mode.value);
		//Cvar_SetValue("vid_config_fscr", 0);
		Cvar_SetValue ("vid_mode", vid_default);
		i = VID_SetMode(vid_default, palette);
		if ( !i )
			Sys_Error ("Couldn't set video mode: %s", SDL_GetError());
	}

	// set the rom bit on the early-read
	// cvars until Host_Init() is finished
	VID_LockCvars ();

	scr_disabled_for_loading = temp;
	vid_initialized = true;

	vid_menudrawfn = VID_MenuDraw;
	vid_menukeyfn = VID_MenuKey;
}


void	VID_Shutdown (void)
{
	if (vid_initialized)
	{
		if (screen != NULL && lockcount > 0)
			SDL_UnlockSurface (screen);

		vid_initialized = 0;
		SDL_Quit();
	}
}


/*
================
FlipScreen
================
*/
static void FlipScreen(vrect_t *rects)
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

	if (palette_changed)
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
//	these bits from quakeforge
	Uint8      *offset;

	if (!screen)
		return;
	if (x < 0)
		x = screen->w + x - 1;
	offset = (Uint8 *) screen->pixels + y * screen->pitch + x;
	while (height--)
	{
		memcpy (offset, pbitmap, width);
		offset += screen->pitch;
		pbitmap += width;
	}
}

/*
================
D_EndDirectRect
================
*/
void D_EndDirectRect (int x, int y, int width, int height)
{
//	these bits from quakeforge
	if (!screen)
		return;
	if (x < 0)
		x = screen->w + x - 1;
	SDL_UpdateRect (screen, x, y, width, height);
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
============================
Gamma functions for UNIX/SDL
============================
*/
#if 0
static void VID_SetGamma(void)
{
	float value;

	if ((v_gamma.value != 0)&&(v_gamma.value > (1/GAMMA_MAX)))
		value = 1.0/v_gamma.value;
	else
		value = GAMMA_MAX;

	SDL_SetGamma(value,value,value);
}
#endif


//==========================================================================

/*
================
VID_HandlePause
================
*/
void VID_HandlePause (qboolean paused)
{
#if 0	// change to 1 if dont want to disable mouse in fullscreen
	if ((modestate == MS_WINDOWED) && _enable_mouse.value)
#else
	if (_enable_mouse.value)
#endif
	{
		// for consistency , don't show pointer S.A
		if (paused)
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


/*
================
VID_ToggleFullscreen
Handles switching between fullscreen/windowed modes
and brings the mouse to a proper state afterwards
================
*/
extern qboolean mousestate_sa;
void VID_ToggleFullscreen (void)
{
	int	is_fullscreen;

	if (!fs_toggle_works)
		return;
	if (!num_fmodes)
		return;
	if (!screen)
		return;

	S_ClearBuffer ();

	if (SDL_WM_ToggleFullScreen(screen)==1)
	{
		is_fullscreen = (screen->flags & SDL_FULLSCREEN) ? 1 : 0;
		Cvar_SetValue("vid_config_fscr", is_fullscreen);
		modestate = (is_fullscreen) ? MS_FULLDIB : MS_WINDOWED;
		if (is_fullscreen)
		{
			// activate mouse in fullscreen mode
			// in_sdl.c handles other non-moused cases
			if (mousestate_sa)
				IN_ActivateMouse();
		}
		else
		{	// windowed mode:
			// deactivate mouse if we are in menus
			if (mousestate_sa)
				IN_DeactivateMouse();
		}
		// update the video menu option
		vid_menu_fs = (modestate != MS_WINDOWED);
	}
	else
	{
		fs_toggle_works = false;
		Con_Printf ("SDL_WM_ToggleFullScreen failed\n");
	}
}


//========================================================
// Video menu stuff
//========================================================

#define MAX_ROWS		10
static int	modes_cursor = 0, modes_top = 0;
static qboolean	vid_cursor;	// 0 : resolution option
				// 1 : fullscreen option. switched by TAB key
static qboolean	want_fstoggle;
static qboolean	vid_menu_firsttime = true;

static void M_DrawYesNo (int x, int y, int on, int white)
{
	if (on)
	{
		if (white)
			M_PrintWhite (x, y, "yes");
		else
			M_Print (x, y, "yes");
	}
	else
	{
		if (white)
			M_PrintWhite (x, y, "no");
		else
			M_Print (x, y, "no");
	}
}

/*
================
VID_MenuDraw
================
*/
void VID_MenuDraw (void)
{
	int		i, y;

	if (vid_menu_firsttime)
	{	// settings for entering the menu first time
		vid_cursor = (num_fmodes) ? 0 : 1;
		vid_menu_fs = (modestate != MS_WINDOWED);
		modes_cursor = vid_modenum;
		if (modes_cursor < modes_top)
			modes_top = modes_cursor;
		else if (modes_cursor >= modes_top+MAX_ROWS)
			modes_top = modes_cursor - MAX_ROWS + 1;
		vid_menu_firsttime = false;
	}

	ScrollTitle("gfx/menu/title7.lmp");

	want_fstoggle = ( ((modestate == MS_WINDOWED) && vid_menu_fs) || ((modestate != MS_WINDOWED) && !vid_menu_fs) );

	M_Print (64, 72, "Press TAB to switch options");

	M_Print (64, 84, "Fullscreen: ");
	M_DrawYesNo (184, 84, vid_menu_fs, !want_fstoggle);

	if (modes_top)
		M_DrawCharacter (160, 92, 128);
	if (modes_top + MAX_ROWS < *nummodes)
		M_DrawCharacter (160, 92 + ((MAX_ROWS-1)*8), 129);

	M_Print (64, 76+16, "Resolution: ");
	for (i=0 ; (i < MAX_ROWS) && (i+modes_top < *nummodes); i++)
	{
		y = 92 + 8*i;

		if (i+modes_top == vid_modenum)
			M_PrintWhite (184, y, modelist[i+modes_top].modedesc);
		else
			M_Print (184, y, modelist[i+modes_top].modedesc);
	}

	if (vid_cursor)
		M_DrawCharacter (172, 92 + (modes_cursor-modes_top)*8, 12+((int)(realtime*4)&1));
	else
	{
		M_DrawCharacter (172, 84, 12+((int)(realtime*4)&1));
		// indicate an unset resolution selection
		// when we aren't in the scrolling list
		if (modes_cursor != vid_modenum)
			M_DrawCharacter (172, 92 + (modes_cursor-modes_top)*8, 13);
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
	case K_TAB:
		vid_cursor = !vid_cursor;
		return;

	case K_ESCAPE:
		M_Menu_Options_f ();
		return;

	case K_ENTER:
		if (modes_cursor != vid_modenum || want_fstoggle)
		{
			Cvar_SetValue("vid_mode", modes_cursor);
			Cvar_SetValue("vid_config_fscr", vid_menu_fs);
			VID_Restart_f();
		}
		return;

	case K_LEFTARROW:
	case K_RIGHTARROW:
	// fullscreen / windowed toggling
		if (vid_cursor == 1)
			return;	// resolution is on the scrolling list
		vid_menu_fs = !vid_menu_fs;
		if (fs_toggle_works)
			VID_ToggleFullscreen();
		return;

	case K_UPARROW:
		if (!vid_cursor)
			return;
		modes_cursor--;
		if (modes_cursor < 0)
			modes_cursor = *nummodes - 1;
		S_LocalSound ("raven/menu1.wav");
		break;

	case K_DOWNARROW:
		if (!vid_cursor)
			return;
		modes_cursor++;
		if (modes_cursor >= *nummodes)
			modes_cursor = 0;
		S_LocalSound ("raven/menu1.wav");
		break;

	default:
		return;
	}

	if (modes_cursor < modes_top)
		modes_top = modes_cursor;
	else if (modes_cursor >= modes_top+MAX_ROWS)
		modes_top = modes_cursor - MAX_ROWS + 1;
}

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.51  2006/07/04 16:23:30  sezero
 * enabled fullscreen/windowed switching through the menu system
 * in cases where instant SDL toggling doesn't work
 *
 * Revision 1.50  2006/05/19 11:32:54  sezero
 * misc clean-up
 *
 * Revision 1.49  2006/03/24 18:43:34  sezero
 * moved include macros for different xbm icons to a new xbm_icon.h header
 *
 * Revision 1.48  2006/03/24 17:34:20  sezero
 * includes cleanup
 *
 * Revision 1.47  2006/03/24 15:05:39  sezero
 * killed the archive, server and info members of the cvar structure.
 * the new flags member is now employed for all those purposes. also
 * made all non-globally used cvars static.
 *
 * Revision 1.46  2006/03/23 19:48:16  sezero
 * Updated the earl-read-cvars in VID_Init and VID_PostInitFix to use the
 * new CVAR_ROM flag: they look less clumsy now.
 *
 * Revision 1.45  2006/01/17 17:36:44  sezero
 * A quick'n'dirty patch for making the game to remember its video settings:
 * Essentially it does an early read of config.cfg while in VID_Init to
 * remember the settings (new procedure: VID_EarlyReadConfig). (new cvars:
 * vid_config_glx, vid_config_gly, vid_config_swx, vid_config_swy, and
 * vid_config_fscr). the commandline still acts as an override. then, it fixes
 * the cvar screw-up caused by the actual read of config.cfg by overwriting
 * the affected cvars with the running settings (new tiny procedure:
 * VID_PostInitFix, called from Host_Init).
 *
 * Implemented here are the screen dimensions, color bits (bpp, for win32,
 * cvar: vid_config_bpp), palettized textures and multisampling (fsaa, for
 * unix, cvars: vid_config_gl8bit and vid_config_fsaa) options with their
 * menu representations and cvar memorizations.
 *
 * This method can probably be also used to store/remember the conwidth
 * settings. Also applicable is the sound settings, such as the driver,
 * sampling rate, format, etc.
 *
 * Secondly, the patch sets the fullscreen cvar not by only looking at silly
 * values but by looking at the current SDL_Screen flags.
 *
 * Revision 1.44  2006/01/14 08:39:24  sezero
 * fixed the incorrect (mislead) usage of modestate values, although the result
 * doesn't change.
 *
 * Revision 1.43  2006/01/12 12:43:49  sezero
 * Created an sdl_inc.h with all sdl version requirements and replaced all
 * SDL.h and SDL_mixer.h includes with it. Made the source to compile against
 * SDL versions older than 1.2.6 without disabling multisampling. Multisampling
 * (fsaa) option is now decided at runtime. Minimum required SDL and SDL_mixer
 * versions are now 1.2.4. If compiled without midi, minimum SDL required is
 * 1.2.0. Added SDL_mixer version checking to sdl-midi with measures to prevent
 * relocation errors.
 *
 * Revision 1.42  2006/01/12 12:34:38  sezero
 * added video modes enumeration via SDL. added on-the-fly video mode changing
 * partially based on the Pa3PyX hexen2 tree. TODO: make the game remember its
 * video settings, clean it up, fix it up...
 *
 * Revision 1.41  2006/01/07 09:54:29  sezero
 * cleanup and "static" stuff on the vid files
 *
 * Revision 1.40  2005/12/11 18:30:08  sezero
 * updated the software sdl renderer: removed a lot of unnecessary
 * things which aren't of use in its current state with no mode
 * switching feature. it now follows the same style as in the sdlgl
 * version. placed D_BeginDirectRect and D_EndDirectRect bits taken
 * from quakeforge, so the rotating skull now works. the loading
 * plaque now works in fullscreen modes, which it didn't before.
 *
 * Revision 1.39  2005/12/04 11:19:18  sezero
 * gamma stuff update
 *
 * Revision 1.38  2005/10/24 22:54:48  sezero
 * fixed "bestmatch might be used uninitialized" warning
 *
 * Revision 1.37  2005/10/02 15:45:27  sezero
 * killed lcd_x and lcd_yaw (the stereoscopic stuff.) never tested, never used.
 *
 * Revision 1.36  2005/10/02 15:43:08  sezero
 * killed -Wshadow warnings
 *
 * Revision 1.35  2005/09/28 06:07:32  sezero
 * renamed ToggleFullScreenSA to VID_ToggleFullscreen which
 * actually is of VID_ class and now is easier to locate
 *
 * Revision 1.34  2005/09/20 21:19:45  sezero
 * Sys_Quit and Sys_Error clean-up: VID_SetDefaultMode, VID_ForceLockState and
 * VID_ForceUnlockedAndReturnState are history. Host_Shutdown is called before
 * quit/error messages. Placed SDL_UnlockSurface() and MGL_endDirectAccess()
 * to VID_Shutdown, just in case. Added the word "HexenWorld" to win32 version
 * of hexenworld error window label. Took care of some accidentally repeated
 * code. "Fatalized" and added extra linefeeds to the sys_error messages.
 *
 * Revision 1.33  2005/09/19 19:50:10  sezero
 * fixed those famous spelling errors
 *
 * Revision 1.32  2005/08/10 23:19:26  sezero
 * slight tweaks
 *
 * Revision 1.31  2005/07/30 11:36:42  sezero
 * support for window manager icon using xbm format data. stolen from
 * the quake2 project at icculus, code probably by relnev (Steven Fuller)
 *
 * Revision 1.30  2005/07/09 08:56:17  sezero
 * the transtable externs are now unused
 *
 * Revision 1.29  2005/07/09 07:29:40  sezero
 * use hunk instead of malloc
 *
 * Revision 1.28  2005/06/26 12:43:38  sezero
 * Added the intermissionScreen CacheFree code to software vid.
 * I have ignored this fragment for a long time due to the fact
 * we removed video mode switching, but here it is for possible
 * future use.
 *
 * Revision 1.27  2005/06/05 07:50:27  sezero
 * *** empty log message ***
 *
 * Revision 1.26  2005/06/03 13:25:29  sezero
 * Latest mouse fixes and clean-ups
 *
 * Revision 1.25  2005/05/21 17:10:58  sezero
 * re-enabled complete disabling/enabling of mousa in fullscreen
 * mode. (only replaced a bunch of if 1's to if 0's)
 *
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
