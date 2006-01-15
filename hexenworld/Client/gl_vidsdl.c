/*
	gl_vidsdl.c -- SDL GL vid component
	Select window size and mode and init SDL in GL mode.

	$Id: gl_vidsdl.c,v 1.90 2006-01-15 22:07:50 sezero Exp $

	Changed 7/11/04 by S.A.
	- Fixed fullscreen opengl mode, window sizes
	- Options are now: -fullscreen, -height, -width, -bpp
	- The "-mode" option has been removed

	Changed 7/01/06 by O.S
	- Added video modes enumeration via SDL
	- Added video mode changing on the fly.
*/

#include "quakedef.h"
#include "quakeinc.h"

#include "sdl_inc.h"
#include <dlfcn.h>
#include <unistd.h>

#define WARP_WIDTH		320
#define WARP_HEIGHT		200
#define MAXWIDTH		10000
#define MAXHEIGHT		10000
#define MIN_WIDTH		320
//#define MIN_HEIGHT		200
#define MIN_HEIGHT		240

typedef struct {
	modestate_t	type;
	int			width;
	int			height;
	int			modenum;
	int			dib;
	int			fullscreen;
	int			bpp;
	int			halfscreen;
	char		modedesc[33];
} vmode_t;

typedef struct {
	int	red,
		green,
		blue,
		alpha,
		depth,
		stencil;
} attributes_t;

static attributes_t	vid_attribs;
static int	num_fmodes;
static int	num_wmodes;
static int	*nummodes;
qboolean	fullscreen = false;	// windowed mode is default
static int	bpp = 16;
static const SDL_VideoInfo	*vid_info;
static SDL_Surface	*screen;

viddef_t	vid;		// global video state
modestate_t	modestate = MS_UNINIT;
static int	WRHeight, WRWidth;
static int	vid_default = -1;	// modenum of 640x480 as a safe default
static int	vid_modenum = -1;	// current video mode, set after mode setting succeeds
static int	vid_maxwidth = 640, vid_maxheight = 480;
// cvar vid_mode must be set before calling VID_SetMode, VID_ChangeVideoMode or VID_Restart_f
cvar_t		vid_mode = {"vid_mode","0", false};
cvar_t		vid_config_glx = {"vid_config_glx","640", true};
cvar_t		vid_config_gly = {"vid_config_gly","480", true};

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
	{1024, 768},	// 5, RES_640X480 + 2
	{1280, 1024},	// 6
	{1600, 1200}	// 7
};

#define MAX_MODE_LIST	40
#define MAX_STDMODES	(sizeof(std_modes) / sizeof(std_modes[0]))
#define NUM_LOWRESMODES	(RES_640X480)
static vmode_t	fmodelist[MAX_MODE_LIST+1];	// list of enumerated fullscreen modes
static vmode_t	wmodelist[MAX_STDMODES +1];	// list of standart 4:3 windowed modes
static vmode_t	*modelist;	// modelist in use, points to one of the above lists
static qboolean	vid_initialized = false;
extern qboolean	draw_reinit;

byte		globalcolormap[VID_GRADES*256];

cvar_t		_enable_mouse = {"_enable_mouse","1", true};
static int	enable_mouse;
qboolean	in_mode_set = false;

const char	*gl_vendor;
const char	*gl_renderer;
const char	*gl_version;
const char	*gl_extensions;
#ifdef GL_DLSYM
static const char	*gl_library  = NULL;
static qboolean	GL_OpenLibrary(const char *name);
#endif
int		gl_max_size = 256;
qboolean	is_3dfx = false;
float		gldepthmin, gldepthmax;
extern int	numgltextures;

static int	multisample = 0; // never set this to non-zero if SDL isn't multisampling-capable
static qboolean	sdl_has_multisample = false;

typedef void	(*FX_SET_PALETTE_EXT)(int, int, int, int, int, const void*);
FX_SET_PALETTE_EXT	MyglColorTableEXT;
qboolean	is8bit = false;
static void VID_Init8bitPalette (void);

float		RTint[256],GTint[256],BTint[256];
unsigned short	d_8to16table[256];
unsigned	d_8to24table[256];
//unsigned	d_8to24table3dfx[256];
unsigned	d_8to24TranslucentTable[256];
#ifdef	OLD_8_BIT_PALETTE_CODE
unsigned char	inverse_pal[(1<<INVERSE_PAL_TOTAL_BITS)+1]; // +1: COM_LoadStackFile puts a 0 at the end of the data
#else
unsigned char	d_15to8table[65536];
#endif

cvar_t		gl_ztrick = {"gl_ztrick","0",true};
cvar_t		gl_purge_maptex = {"gl_purge_maptex", "1", true};
		/* whether or not map-specific OGL textures
		   are flushed from map. default == yes  */

extern int	lightmap_textures;
extern int	lightmap_bytes;	// in gl_rsurf.c

// multitexturing
qboolean	gl_mtexable = false;
static int	num_tmus = 1;

qboolean	scr_skipupdate;
static		qboolean fullsbardraw = false;

void VID_MenuDraw (void);
void VID_MenuKey (int key);

static void ClearAllStates (void);
static void GL_Init (void);
#ifdef GL_DLSYM
static void GL_Init_Functions(void);
#endif
static void GL_ResetFunctions(void);

qboolean	have_stencil = false;

// Gamma stuff
#define USE_GAMMA_RAMPS	0	// change to 1 if want to use ramps for gamma
#define	EVIL_3DFX_LIB	"lib3dfxgamma.so"
#if USE_GAMMA_RAMPS
static unsigned short	orig_ramps[3][256];	// for hw- or 3dfx-gamma
extern unsigned short	ramps[3][256];	// for hw- or 3dfx-gamma
static int	(*glGetDeviceGammaRamp3DFX_fp)(void *) = NULL;
static int	(*glSetDeviceGammaRamp3DFX_fp)(void *) = NULL;
static void	VID_SetGammaRamp (void);
#else
static void	VID_SetGamma (void);
#endif
static void	*fx_gammalib = NULL;
static int	(*fxGammaCtl)(float) = NULL;
static qboolean	fx_gamma   = false;	// 3dfx-specific gamma control
static qboolean	gammaworks = false;	// whether hw-gamma works
qboolean	gl_dogamma = false;	// none of the above two, use gl tricks
static void	VID_InitGamma (void);
static void	VID_ShutdownGamma (void);
static qboolean	VID_Check3dfxGamma(void);

extern void	D_ClearOpenGLTextures(int);
extern void	R_InitParticleTexture(void);
extern void	Mod_ReloadTextures (void);

// window manager stuff
#if defined(H2W)
#	define WM_TITLEBAR_TEXT	"HexenWorld"
#	define WM_ICON_TEXT	"HexenWorld"
#elif defined(H2MP)
#	define WM_TITLEBAR_TEXT	"Portal of Praevus"
#	define WM_ICON_TEXT	"PRAEVUS"
#else
#	define WM_TITLEBAR_TEXT	"Hexen II"
#	define WM_ICON_TEXT	"HEXEN2"
#endif

//====================================

// for compatability with software renderer

void VID_LockBuffer (void)
{
}

void VID_UnlockBuffer (void)
{
}

void D_BeginDirectRect (int x, int y, byte *pbitmap, int width, int height)
{
}

void D_EndDirectRect (int x, int y, int width, int height)
{
}

void VID_HandlePause (qboolean paused)
{
}


//====================================

static void VID_SetIcon (void)
{
	SDL_Surface *icon;
	SDL_Color color;
	Uint8 *ptr;
	int i, mask;
#if defined(H2W)
// hexenworld
#	include "../icons/h2w_ico.xbm"
#elif defined(H2MP)
// hexen2 with mission pack
#	include "icons/h2mp_ico.xbm"
#else
// plain hexen2
#	include "icons/h2_ico.xbm"
#endif

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
		for (mask = 1; mask != 0x100; mask <<= 1)
		{
			*ptr = (HOT_ICON_bits[i] & mask) ? 1 : 0;
			ptr++;
		}		
	}

	SDL_WM_SetIcon(icon, NULL);
	SDL_FreeSurface(icon);
}

static int sort_modes (const void *arg1, const void *arg2)
{
	const vmode_t *a1, *a2;
	a1 = (vmode_t *) arg1;
	a2 = (vmode_t *) arg2;

	if (a1->width == a2->width)
		return a1->height - a2->height;	// lowres-to-highres
	//	return a2->height - a1->height;	// highres-to-lowres
	else
		return a1->width - a2->width;	// lowres-to-highres
	//	return a2->width - a1->width;	// highres-to-lowres
}

static void VID_PrepareModes (SDL_Rect **sdl_modes)
{
	int	i, j;
	qboolean	not_multiple;

	num_fmodes = 0;
	num_wmodes = 0;

	// Add the standart 4:3 modes to the windowed modes list
	// In an unlikely case that we receive no fullscreen modes,
	// this will be our modes list (kind of...)
	for (i = 0; i < MAX_STDMODES; i++)
	{
		wmodelist[num_wmodes].width = std_modes[i].width;
		wmodelist[num_wmodes].height = std_modes[i].height;
		wmodelist[num_wmodes].halfscreen = 0;
		wmodelist[num_wmodes].fullscreen = 0;
		wmodelist[num_wmodes].bpp = 16;
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
		num_wmodes = RES_640X480 + 1;
		modelist = (vmode_t *)wmodelist;
		nummodes = &num_wmodes;
		vid_default = RES_640X480;
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
		vid_default = RES_640X480;
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

		// avoid resolutions < 320x240
		if (not_multiple && sdl_modes[i]->w >= MIN_WIDTH && sdl_modes[i]->h >= MIN_HEIGHT)
		{
			fmodelist[num_fmodes].width = sdl_modes[i]->w;
			fmodelist[num_fmodes].height = sdl_modes[i]->h;
			// FIXME: look at gl_vidnt.c and learn how to
			// really functionalize the halfscreen field.
			fmodelist[num_fmodes].halfscreen = 0;
			fmodelist[num_fmodes].fullscreen = 1;
			fmodelist[num_fmodes].bpp = 16;
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

	// SDL versions older than 1.2.8 have sorting problems
	qsort(fmodelist, num_fmodes, sizeof fmodelist[0], sort_modes);

	vid_maxwidth = fmodelist[num_fmodes-1].width;
	vid_maxheight = fmodelist[num_fmodes-1].height;

	// find the 640x480 default resolution. this shouldn't fail
	// at all (for any adapter suporting the VGA/XGA legacy).
	for (i = 0; i < num_fmodes; i++)
	{
		if (fmodelist[i].width == 640 && fmodelist[i].height == 480)
		{
			vid_default = i;
			break;
		}
	}

	if (vid_default < 0)
	{
		// No 640x480? Unexpected, at least today..
		// Easiest thing is to set the default mode
		// as the highest reported one.
		Con_Printf("WARNING: 640x480 not found in fullscreen modes\n"
			   "Using the largest reported dimension as default\n");
		vid_default = num_fmodes;
	}

	// limit the windowed (standart) modes list to desktop dimensions
	for (i = 0; i < num_wmodes; i++)
	{
		if (wmodelist[i].width > vid_maxwidth || wmodelist[i].height > vid_maxheight)
			break;
	}
	if (i < num_wmodes)
		num_wmodes = i;
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

#ifdef GL_DLSYM
static qboolean GL_OpenLibrary(const char *name)
{
	int	ret;
	char	gl_liblocal[MAX_OSPATH];

	ret = SDL_GL_LoadLibrary(name);

	if (ret < 0)
	{
		// In case of user-specified gl library, look for it under the
		// installation directory, too: the user may forget providing
		// a valid path information. In that case, make sure it doesnt
		// contain any path information and exists in our own basedir,
		// then try loading it
		if ( name && !strchr(name, '/') )
		{
			snprintf (gl_liblocal, MAX_OSPATH, "%s/%s", com_basedir, name);
			if (access(gl_liblocal, R_OK) == -1)
				return false;

			Con_Printf ("Failed loading gl library %s\n"
				    "Trying to load %s\n", name, gl_liblocal);

			ret = SDL_GL_LoadLibrary(gl_liblocal);
			if (ret < 0)
				return false;

			Con_Printf("Using GL library: %s\n", gl_liblocal);
			return true;
		}

		return false;
	}

	if (name)
		Con_Printf("Using GL library: %s\n", name);
	else
		Con_Printf("Using system GL library\n");

	return true;
}
#endif	// GL_DLSYM


static int VID_SetMode (int modenum)
{
	Uint32	flags;
	int	i;

	in_mode_set = true;

	//flags = (SDL_OPENGL|SDL_NOFRAME);
	flags = (SDL_OPENGL);
	if (fullscreen)
		flags |= SDL_FULLSCREEN;

	// setup the attributes
	if (bpp >= 32)
	{
		SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 );
		SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 );
		SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 );
		SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 8 );
		SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 24 );
		SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 8 );
	}
	else
	{
		SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 5 );
		SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 5 );
		SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 5 );
		SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16 );
	}
	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

	i = COM_CheckParm ("-fsaa");
	if (i && i < com_argc-1)
		multisample = atoi(com_argv[i+1]);
	if (multisample && !sdl_has_multisample)
	{
		multisample = 0;
		Con_Printf ("SDL ver < %d, multisampling disabled\n", SDL_VER_WITH_MULTISAMPLING);
	}
	if (multisample)
	{
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, multisample);
	}

	Con_Printf ("Requested mode %d: %dx%dx%d\n", modenum, modelist[modenum].width, modelist[modenum].height, bpp);
	screen = SDL_SetVideoMode (modelist[modenum].width, modelist[modenum].height, bpp, flags);
	if (!screen)
	{
		if (!multisample)
		{
			Sys_Error ("Couldn't set video mode: %s", SDL_GetError());
		}
		else
		{
			Con_Printf ("multisample window failed\n");
			multisample = 0;
			SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
			SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, multisample);
			screen = SDL_SetVideoMode (modelist[modenum].width, modelist[modenum].height, bpp, flags);
			if (!screen)
				Sys_Error ("Couldn't set video mode: %s", SDL_GetError());
		}
	}

	// success. set vid_modenum properly and adjust other vars.
	vid_modenum = modenum;
	modestate = (fullscreen) ? MS_FULLDIB : MS_WINDOWED;
	WRWidth = vid.width = vid.conwidth = modelist[modenum].width;
	WRHeight = vid.height = vid.conheight = modelist[modenum].height;

	// This will display a bigger hud and readable fonts at high
	// resolutions. The fonts will be somewhat distorted, though
	i = COM_CheckParm("-conwidth");
	if (i != 0 && i < com_argc-1)
	{
		vid.conwidth = atoi(com_argv[i+1]);
		vid.conwidth &= 0xfff8; // make it a multiple of eight
		if (vid.conwidth < MIN_WIDTH)
			vid.conwidth = MIN_WIDTH;
		// pick a conheight that matches with correct aspect
		vid.conheight = vid.conwidth*3 / 4;
		i = COM_CheckParm("-conheight");
		if (i != 0 && i < com_argc-1)
			vid.conheight = atoi(com_argv[i+1]);
		//if (vid.conheight < MIN_HEIGHT)
		if (vid.conheight < 200)
			vid.conheight = 200;
		if (vid.conwidth > modelist[modenum].width)
			vid.conwidth = modelist[modenum].width;
		if (vid.conheight > modelist[modenum].height)
			vid.conheight = modelist[modenum].height;

		vid.width = vid.conwidth;
		vid.height = vid.conheight;
	}
	// end of conwidth hack

	SDL_GL_GetAttribute(SDL_GL_BUFFER_SIZE, &i);
	Con_Printf ("Video Mode Set : %dx%dx%d\n", vid.width, vid.height, i);
	if (multisample)
	{
		SDL_GL_GetAttribute(SDL_GL_MULTISAMPLESAMPLES, &multisample);
		Con_Printf ("multisample buffer with %i samples\n", multisample);
	}

	// collect the actual attributes
	memset (&vid_attribs, 0, sizeof(attributes_t));
	SDL_GL_GetAttribute(SDL_GL_RED_SIZE, &vid_attribs.red);
	SDL_GL_GetAttribute(SDL_GL_GREEN_SIZE, &vid_attribs.green);
	SDL_GL_GetAttribute(SDL_GL_BLUE_SIZE, &vid_attribs.blue);
	SDL_GL_GetAttribute(SDL_GL_ALPHA_SIZE, &vid_attribs.alpha);
	SDL_GL_GetAttribute(SDL_GL_DEPTH_SIZE, &vid_attribs.depth);
	SDL_GL_GetAttribute(SDL_GL_STENCIL_SIZE, &vid_attribs.stencil);
	Con_Printf ("vid_info: red: %d, green: %d, blue: %d, alpha: %d, depth: %d\n",
			vid_attribs.red, vid_attribs.green, vid_attribs.blue, vid_attribs.alpha, vid_attribs.depth);

	// setup the window manager stuff
	VID_SetIcon();
	SDL_WM_SetCaption(WM_TITLEBAR_TEXT, WM_ICON_TEXT);

	IN_HideMouse ();

	in_mode_set = false;

	return true;
}


//====================================

int		texture_extension_number = 1;

static void CheckMultiTextureExtensions(void)
{
	gl_mtexable = false;

	if (COM_CheckParm("-nomtex"))
	{
		Con_Printf("Multitexture extensions disabled\n");
	}
	else if (strstr(gl_extensions, "GL_ARB_multitexture"))
	{
		Con_Printf("ARB Multitexture extensions found\n");

		glGetIntegerv_fp(GL_MAX_TEXTURE_UNITS_ARB, &num_tmus);
		if (num_tmus < 2)
		{
			Con_Printf("not enough TMUs, ignoring multitexture\n");
			return;
		}

		glMultiTexCoord2fARB_fp = (void *) SDL_GL_GetProcAddress("glMultiTexCoord2fARB");
		glActiveTextureARB_fp = (void *) SDL_GL_GetProcAddress("glActiveTextureARB");
		if ((glMultiTexCoord2fARB_fp == NULL) ||
		    (glActiveTextureARB_fp == NULL))
		{
			Con_Printf ("Couldn't link to multitexture functions\n");
			return;
		}

		Con_Printf("Found %i TMUs support\n", num_tmus);
		gl_mtexable = true;

		// start up with the correct texture selected!
		glDisable_fp(GL_TEXTURE_2D);
		glActiveTextureARB_fp(GL_TEXTURE0_ARB);
	}
	else
	{
		Con_Printf("GL_ARB_multitexture not found\n");
	}
}

static void CheckStencilBuffer(void)
{
	have_stencil = false;

#ifdef GL_DLSYM
	glStencilFunc_fp = (glStencilFunc_f) SDL_GL_GetProcAddress("glStencilFunc");
	glStencilOp_fp = (glStencilOp_f) SDL_GL_GetProcAddress("glStencilOp");
	glClearStencil_fp = (glClearStencil_f) SDL_GL_GetProcAddress("glClearStencil");
	if ((glStencilFunc_fp == NULL) ||
	    (glStencilOp_fp == NULL)   ||
	    (glClearStencil_fp == NULL))
	{
		Con_Printf ("glStencil functions not available\n");
		return;
	}
#endif

	if (vid_attribs.stencil)
	{
		Con_Printf("Stencil buffer created with %d bits\n", vid_attribs.stencil);
		have_stencil = true;
	}
}

static void GL_InitLightmapBits (void)
{
	gl_lightmap_format = GL_LUMINANCE;
	if (COM_CheckParm ("-lm_1"))
		gl_lightmap_format = GL_LUMINANCE;
	else if (COM_CheckParm ("-lm_a"))
		gl_lightmap_format = GL_ALPHA;
	else if (COM_CheckParm ("-lm_i"))
		gl_lightmap_format = GL_INTENSITY;
//	else if (COM_CheckParm ("-lm_2"))
//		gl_lightmap_format = GL_RGBA4;
	else if (COM_CheckParm ("-lm_4"))
		gl_lightmap_format = GL_RGBA;

	switch (gl_lightmap_format)
	{
	case GL_RGBA:
		lightmap_bytes = 4;
		break;
//	case GL_RGBA4:
//		lightmap_bytes = 2;
//		break;
	case GL_LUMINANCE:
	case GL_INTENSITY:
	case GL_ALPHA:
		lightmap_bytes = 1;
		break;
	}
}

/*
===============
GL_Init
===============
*/
static void GL_Init (void)
{
#ifdef GL_DLSYM
	// initialize gl function pointers
	GL_Init_Functions();
#endif
	gl_vendor = (const char *)glGetString_fp (GL_VENDOR);
	Con_Printf ("GL_VENDOR: %s\n", gl_vendor);
	gl_renderer = (const char *)glGetString_fp (GL_RENDERER);
	Con_Printf ("GL_RENDERER: %s\n", gl_renderer);

	gl_version = (const char *)glGetString_fp (GL_VERSION);
	Con_Printf ("GL_VERSION: %s\n", gl_version);
	gl_extensions = (const char *)glGetString_fp (GL_EXTENSIONS);
	Con_DPrintf ("GL_EXTENSIONS: %s\n", gl_extensions);

	glGetIntegerv_fp(GL_MAX_TEXTURE_SIZE, &gl_max_size);
	if (gl_max_size < 256)	// Refuse to work when less than 256
		Sys_Error ("hardware capable of min. 256k opengl texture size needed");
	if (gl_max_size > 1024)	// We're cool with 1024, write a cmdline override if necessary
		gl_max_size = 1024;
	Con_Printf("OpenGL max.texture size: %i\n", gl_max_size);

	is_3dfx = false;
	if (!Q_strncasecmp ((char *)gl_renderer, "3dfx",  4)  ||
	    !Q_strncasecmp ((char *)gl_renderer, "Glide", 5)  ||
	    !Q_strncasecmp ((char *)gl_renderer, "Mesa Glide", 10))
	{
	// This should hopefully detect Voodoo1 and Voodoo2
	// hardware and possibly Voodoo Rush.
	// Voodoo Banshee, Voodoo3 and later are hw-accelerated
	// by DRI in XFree86-4.x and should be: is_3dfx = false.
		Con_Printf("3dfx Voodoo found\n");
		is_3dfx = true;
	}

	if (Q_strncasecmp(gl_renderer,"PowerVR",7)==0)
		fullsbardraw = true;

	CheckMultiTextureExtensions();
	CheckStencilBuffer();
	GL_InitLightmapBits();

	glClearColor_fp (1,0,0,0);
	glCullFace_fp(GL_FRONT);
	glEnable_fp(GL_TEXTURE_2D);

	glEnable_fp(GL_ALPHA_TEST);
	glAlphaFunc_fp(GL_GREATER, 0.632); // 1 - e^-1 : replaced 0.666 to avoid clipping of smaller fonts/graphics

	glPolygonMode_fp (GL_FRONT_AND_BACK, GL_FILL);
	glShadeModel_fp (GL_FLAT);

	glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// previously GL_CLAMP was GL_REPEAT S.A
	glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	glBlendFunc_fp (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

//	glTexEnvf_fp(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexEnvf_fp(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	if (multisample)
	{
		glEnable_fp (GL_MULTISAMPLE_ARB);
		Con_Printf ("enabled %i sample fsaa\n", multisample);
	}
}

#ifdef GL_DLSYM
static void GL_Init_Functions(void)
{
  glBegin_fp = (glBegin_f) SDL_GL_GetProcAddress("glBegin");
  if (glBegin_fp == 0) {Sys_Error("glBegin not found in GL library");}
  glEnd_fp = (glEnd_f) SDL_GL_GetProcAddress("glEnd");
  if (glEnd_fp == 0) {Sys_Error("glEnd not found in GL library");}
  glEnable_fp = (glEnable_f) SDL_GL_GetProcAddress("glEnable");
  if (glEnable_fp == 0) {Sys_Error("glEnable not found in GL library");}
  glDisable_fp = (glDisable_f) SDL_GL_GetProcAddress("glDisable");
  if (glDisable_fp == 0) {Sys_Error("glDisable not found in GL library");}
#ifdef H2W
  glIsEnabled_fp = (glIsEnabled_f) SDL_GL_GetProcAddress("glIsEnabled");
  if (glIsEnabled_fp == 0) {Sys_Error("glIsEnabled not found in GL library");}
#endif
  glFinish_fp = (glFinish_f) SDL_GL_GetProcAddress("glFinish");
  if (glFinish_fp == 0) {Sys_Error("glFinish not found in GL library");}
  glFlush_fp = (glFlush_f) SDL_GL_GetProcAddress("glFlush");
  if (glFlush_fp == 0) {Sys_Error("glFlush not found in GL library");}
  glClear_fp = (glClear_f) SDL_GL_GetProcAddress("glClear");
  if (glClear_fp == 0) {Sys_Error("glClear not found in GL library");}

  glOrtho_fp = (glOrtho_f) SDL_GL_GetProcAddress("glOrtho");
  if (glOrtho_fp == 0) {Sys_Error("glOrtho not found in GL library");}
  glFrustum_fp = (glFrustum_f) SDL_GL_GetProcAddress("glFrustum");
  if (glFrustum_fp == 0) {Sys_Error("glFrustum not found in GL library");}
  glViewport_fp = (glViewport_f) SDL_GL_GetProcAddress("glViewport");
  if (glViewport_fp == 0) {Sys_Error("glViewport not found in GL library");}
  glPushMatrix_fp = (glPushMatrix_f) SDL_GL_GetProcAddress("glPushMatrix");
  if (glPushMatrix_fp == 0) {Sys_Error("glPushMatrix not found in GL library");}
  glPopMatrix_fp = (glPopMatrix_f) SDL_GL_GetProcAddress("glPopMatrix");
  if (glPopMatrix_fp == 0) {Sys_Error("glPopMatrix not found in GL library");}
  glLoadIdentity_fp = (glLoadIdentity_f) SDL_GL_GetProcAddress("glLoadIdentity");
  if (glLoadIdentity_fp == 0) {Sys_Error("glLoadIdentity not found in GL library");}
  glMatrixMode_fp = (glMatrixMode_f) SDL_GL_GetProcAddress("glMatrixMode");
  if (glMatrixMode_fp == 0) {Sys_Error("glMatrixMode not found in GL library");}
  glLoadMatrixf_fp = (glLoadMatrixf_f) SDL_GL_GetProcAddress("glLoadMatrixf");
  if (glLoadMatrixf_fp == 0) {Sys_Error("glLoadMatrixf not found in GL library");}

  glVertex2f_fp = (glVertex2f_f) SDL_GL_GetProcAddress("glVertex2f");
  if (glVertex2f_fp == 0) {Sys_Error("glVertex2f not found in GL library");}
  glVertex3f_fp = (glVertex3f_f) SDL_GL_GetProcAddress("glVertex3f");
  if (glVertex3f_fp == 0) {Sys_Error("glVertex3f not found in GL library");}
  glVertex3fv_fp = (glVertex3fv_f) SDL_GL_GetProcAddress("glVertex3fv");
  if (glVertex3fv_fp == 0) {Sys_Error("glVertex3fv not found in GL library");}
  glTexCoord2f_fp = (glTexCoord2f_f) SDL_GL_GetProcAddress("glTexCoord2f");
  if (glTexCoord2f_fp == 0) {Sys_Error("glTexCoord2f not found in GL library");}
  glTexCoord3f_fp = (glTexCoord3f_f) SDL_GL_GetProcAddress("glTexCoord3f");
  if (glTexCoord3f_fp == 0) {Sys_Error("glTexCoord3f not found in GL library");}
  glColor4f_fp = (glColor4f_f) SDL_GL_GetProcAddress("glColor4f");
  if (glColor4f_fp == 0) {Sys_Error("glColor4f not found in GL library");}
  glColor4fv_fp = (glColor4fv_f) SDL_GL_GetProcAddress("glColor4fv");
  if (glColor4fv_fp == 0) {Sys_Error("glColor4fv not found in GL library");}
#ifdef H2W
  glColor4ub_fp = (glColor4ub_f) SDL_GL_GetProcAddress("glColor4ub");
  if (glColor4ub_fp == 0) {Sys_Error("glColor4ub not found in GL library");}
#endif
  glColor4ubv_fp = (glColor4ubv_f) SDL_GL_GetProcAddress("glColor4ubv");
  if (glColor4ubv_fp == 0) {Sys_Error("glColor4ubv not found in GL library");}
  glColor3f_fp = (glColor3f_f) SDL_GL_GetProcAddress("glColor3f");
  if (glColor3f_fp == 0) {Sys_Error("glColor3f not found in GL library");}
  glColor3ubv_fp = (glColor3ubv_f) SDL_GL_GetProcAddress("glColor3ubv");
  if (glColor3ubv_fp == 0) {Sys_Error("glColor3ubv not found in GL library");}
  glClearColor_fp = (glClearColor_f) SDL_GL_GetProcAddress("glClearColor");
  if (glClearColor_fp == 0) {Sys_Error("glClearColor not found in GL library");}

  glRotatef_fp = (glRotatef_f) SDL_GL_GetProcAddress("glRotatef");
  if (glRotatef_fp == 0) {Sys_Error("glRotatef not found in GL library");}
  glTranslatef_fp = (glTranslatef_f) SDL_GL_GetProcAddress("glTranslatef");
  if (glTranslatef_fp == 0) {Sys_Error("glTranslatef not found in GL library");}

  glBindTexture_fp = (glBindTexture_f) SDL_GL_GetProcAddress("glBindTexture");
  if (glBindTexture_fp == 0) {Sys_Error("glBindTexture not found in GL library");}
  glDeleteTextures_fp = (glDeleteTextures_f) SDL_GL_GetProcAddress("glDeleteTextures");
  if (glDeleteTextures_fp == 0) {Sys_Error("glDeleteTextures not found in GL library");}
  glTexParameterf_fp = (glTexParameterf_f) SDL_GL_GetProcAddress("glTexParameterf");
  if (glTexParameterf_fp == 0) {Sys_Error("glTexParameterf not found in GL library");}
  glTexEnvf_fp = (glTexEnvf_f) SDL_GL_GetProcAddress("glTexEnvf");
  if (glTexEnvf_fp == 0) {Sys_Error("glTexEnvf not found in GL library");}
  glScalef_fp = (glScalef_f) SDL_GL_GetProcAddress("glScalef");
  if (glScalef_fp == 0) {Sys_Error("glScalef not found in GL library");}
  glTexImage2D_fp = (glTexImage2D_f) SDL_GL_GetProcAddress("glTexImage2D");
  if (glTexImage2D_fp == 0) {Sys_Error("glTexImage2D not found in GL library");}
#ifdef H2W
  glTexSubImage2D_fp = (glTexSubImage2D_f) SDL_GL_GetProcAddress("glTexSubImage2D");
  if (glTexSubImage2D_fp == 0) {Sys_Error("glTexSubImage2D not found in GL library");}
#endif

  glAlphaFunc_fp = (glAlphaFunc_f) SDL_GL_GetProcAddress("glAlphaFunc");
  if (glAlphaFunc_fp == 0) {Sys_Error("glAlphaFunc not found in GL library");}
  glBlendFunc_fp = (glBlendFunc_f) SDL_GL_GetProcAddress("glBlendFunc");
  if (glBlendFunc_fp == 0) {Sys_Error("glBlendFunc not found in GL library");}
  glShadeModel_fp = (glShadeModel_f) SDL_GL_GetProcAddress("glShadeModel");
  if (glShadeModel_fp == 0) {Sys_Error("glShadeModel not found in GL library");}
  glPolygonMode_fp = (glPolygonMode_f) SDL_GL_GetProcAddress("glPolygonMode");
  if (glPolygonMode_fp == 0) {Sys_Error("glPolygonMode not found in GL library");}
  glDepthMask_fp = (glDepthMask_f) SDL_GL_GetProcAddress("glDepthMask");
  if (glDepthMask_fp == 0) {Sys_Error("glDepthMask not found in GL library");}
  glDepthRange_fp = (glDepthRange_f) SDL_GL_GetProcAddress("glDepthRange");
  if (glDepthRange_fp == 0) {Sys_Error("glDepthRange not found in GL library");}
  glDepthFunc_fp = (glDepthFunc_f) SDL_GL_GetProcAddress("glDepthFunc");
  if (glDepthFunc_fp == 0) {Sys_Error("glDepthFunc not found in GL library");}

  glDrawBuffer_fp = (glDrawBuffer_f) SDL_GL_GetProcAddress("glDrawBuffer");
  if (glDrawBuffer_fp == 0) {Sys_Error("glDrawBuffer not found in GL library");}
  glReadBuffer_fp = (glDrawBuffer_f) SDL_GL_GetProcAddress("glReadBuffer");
  if (glReadBuffer_fp == 0) {Sys_Error("glReadBuffer not found in GL library");}
  glReadPixels_fp = (glReadPixels_f) SDL_GL_GetProcAddress("glReadPixels");
  if (glReadPixels_fp == 0) {Sys_Error("glReadPixels not found in GL library");}
  glHint_fp = (glHint_f) SDL_GL_GetProcAddress("glHint");
  if (glHint_fp == 0) {Sys_Error("glHint not found in GL library");}
  glCullFace_fp = (glCullFace_f) SDL_GL_GetProcAddress("glCullFace");
  if (glCullFace_fp == 0) {Sys_Error("glCullFace not found in GL library");}

  glGetIntegerv_fp = (glGetIntegerv_f) SDL_GL_GetProcAddress("glGetIntegerv");
  if (glGetIntegerv_fp == 0) {Sys_Error("glGetIntegerv not found in GL library");}

  glGetString_fp = (glGetString_f) SDL_GL_GetProcAddress("glGetString");
  if (glGetString_fp == 0) {Sys_Error("glGetString not found in GL library");}
  glGetFloatv_fp = (glGetFloatv_f) SDL_GL_GetProcAddress("glGetFloatv");
  if (glGetFloatv_fp == 0) {Sys_Error("glGetFloatv not found in GL library");}
}

static void GL_ResetFunctions(void)
{
  glBegin_fp = NULL;
  glEnd_fp = NULL;
  glEnable_fp = NULL;
  glDisable_fp = NULL;
  glIsEnabled_fp = NULL;
  glFinish_fp = NULL;
  glFlush_fp = NULL;
  glClear_fp = NULL;

  glOrtho_fp = NULL;
  glFrustum_fp = NULL;
  glViewport_fp = NULL;
  glPushMatrix_fp = NULL;
  glPopMatrix_fp = NULL;
  glLoadIdentity_fp = NULL;
  glMatrixMode_fp = NULL;
  glLoadMatrixf_fp = NULL;

  glVertex2f_fp = NULL;
  glVertex3f_fp = NULL;
  glVertex3fv_fp = NULL;
  glTexCoord2f_fp = NULL;
  glTexCoord3f_fp = NULL;
  glColor4f_fp = NULL;
  glColor4fv_fp = NULL;
  glColor4ub_fp = NULL;
  glColor4ubv_fp = NULL;
  glColor3f_fp = NULL;
  glColor3ubv_fp = NULL;
  glClearColor_fp = NULL;

  glRotatef_fp = NULL;
  glTranslatef_fp = NULL;

  glBindTexture_fp = NULL;
  glDeleteTextures_fp = NULL;
  glTexParameterf_fp = NULL;
  glTexEnvf_fp = NULL;
  glScalef_fp = NULL;
  glTexImage2D_fp = NULL;
  glTexSubImage2D_fp = NULL;

  glAlphaFunc_fp = NULL;
  glBlendFunc_fp = NULL;
  glShadeModel_fp = NULL;
  glPolygonMode_fp = NULL;
  glDepthMask_fp = NULL;
  glDepthRange_fp = NULL;
  glDepthFunc_fp = NULL;

  glDrawBuffer_fp = NULL;
  glReadBuffer_fp = NULL;
  glReadPixels_fp = NULL;
  glHint_fp = NULL;
  glCullFace_fp = NULL;

  glGetIntegerv_fp = NULL;

  glGetString_fp = NULL;
  glGetFloatv_fp = NULL;

  have_stencil = false;
  glStencilFunc_fp = NULL;
  glStencilOp_fp = NULL;
  glClearStencil_fp = NULL;

  gl_mtexable = false;
  glActiveTextureARB_fp = NULL;
  glMultiTexCoord2fARB_fp = NULL;

  is8bit = false;
  MyglColorTableEXT = NULL;
}
#else	// GL_DLSYM
static void GL_ResetFunctions(void)
{
  gl_mtexable = false;
  glActiveTextureARB_fp = NULL;
  glMultiTexCoord2fARB_fp = NULL;

  is8bit = false;
  MyglColorTableEXT = NULL;
}
#endif

static qboolean VID_Check3dfxGamma(void)
{
	if ( ! COM_CheckParm("-3dfxgamma") )
		return false;

	// look for the 3dfx gamma library at the root of the installation
	fx_gammalib = dlopen(va("%s/%s", com_basedir, EVIL_3DFX_LIB), RTLD_GLOBAL | RTLD_NOW);

	if (!fx_gammalib) // look for a global installation then
		fx_gammalib = dlopen(EVIL_3DFX_LIB, RTLD_GLOBAL | RTLD_NOW);

	if (fx_gammalib)
	{
#if USE_GAMMA_RAMPS
// not recommended for Voodoo1, currently crashes
		glGetDeviceGammaRamp3DFX_fp = dlsym(fx_gammalib, "glGetDeviceGammaRamp3DFX");
		glSetDeviceGammaRamp3DFX_fp = dlsym(fx_gammalib, "glSetDeviceGammaRamp3DFX");
		if (glSetDeviceGammaRamp3DFX_fp != NULL && glSetDeviceGammaRamp3DFX_fp != NULL)
		{
			if (glGetDeviceGammaRamp3DFX_fp(orig_ramps) != 0)
				return true;
		}

		glGetDeviceGammaRamp3DFX_fp = NULL;
		glSetDeviceGammaRamp3DFX_fp = NULL;
#else	// not using gamma ramps
		fxGammaCtl = dlsym(fx_gammalib, "do3dfxGammaCtrl");
		if (fxGammaCtl)
		{
			int gtmp;

			gtmp = fxGammaCtl(-1);
			switch (gtmp)
			{
			case 2:	// Glide2x
			case 3:	// Glide3x
				return true;
			}
		}

		fxGammaCtl = NULL;
#endif
		dlclose (fx_gammalib);
		fx_gammalib = NULL;
	}

	return false;
}

static void VID_InitGamma(void)
{
	if (is_3dfx)
	{
	// we don't have WGL_3DFX_gamma_control or an equivalent
	// in unix. If we have it one day, I'll put stuff checking
	// for and linking to it here.
	// Otherwise, assuming is_3dfx means Voodoo1 or Voodoo2,
	// this means we dont have hw-gamma, just use gl_dogamma

	// Here is an evil hack to abuse the Glide symbols exposed on us
		fx_gamma = VID_Check3dfxGamma();
		if (!fx_gamma)
			gl_dogamma = true;
	}

	if (!fx_gamma && !gl_dogamma)
	{
	// we may also use SDL_GetGammaRamp/SDL_SetGammaRamp
	// but let's just stick to what AoT guys did for now.
#if USE_GAMMA_RAMPS
		// if the thing below works, it'll get the
		// original gamma to be restored upon exit
		if (SDL_GetGammaRamp(orig_ramps[0], orig_ramps[1], orig_ramps[2]) == 0)
#else
		if (SDL_SetGamma(v_gamma.value,v_gamma.value,v_gamma.value) == 0)
#endif
			gammaworks = true;
		else
			gl_dogamma = true;
	}

	if (fx_gamma)
		Con_Printf("using 3dfx glide gamma controls\n");
	if (gl_dogamma)
		Con_Printf("gamma not available, using gl tricks\n");
}

/*
============================
Gamma functions for UNIX/SDL
============================
*/
#if USE_GAMMA_RAMPS
/*
============================
VID_SetGammaRamp

used when USE_GAMMA_RAMPS is
defined to 1.
============================
*/
static void VID_SetGammaRamp(void)
{
	if (glSetDeviceGammaRamp3DFX_fp != NULL && fx_gamma)
		glSetDeviceGammaRamp3DFX_fp(ramps);
	else if (!gl_dogamma && gammaworks)
		SDL_SetGammaRamp(ramps[0], ramps[1], ramps[2]);
}
#else
/*
============================
VID_SetGamma

used when USE_GAMMA_RAMPS is
defined to 0.
============================
*/
static void VID_SetGamma(void)
{
	float value;

	if ((v_gamma.value != 0)&&(v_gamma.value > (1/GAMMA_MAX)))
		value = 1.0/v_gamma.value;
	else
		value = GAMMA_MAX;

	if (fxGammaCtl != NULL && fx_gamma)
		fxGammaCtl(value);
	else if (!gl_dogamma && gammaworks)
		SDL_SetGamma(value,value,value);
}
#endif

void VID_ShiftPalette (unsigned char *palette)
{
#if USE_GAMMA_RAMPS
	VID_SetGammaRamp();
#else
	VID_SetGamma();
#endif
}

static void VID_ShutdownGamma (void)
{
#if USE_GAMMA_RAMPS
	// restore hardware gamma
	if (glSetDeviceGammaRamp3DFX_fp != NULL && fx_gamma)
		glSetDeviceGammaRamp3DFX_fp(orig_ramps);
	else if (!fx_gamma && !gl_dogamma && gammaworks)
		SDL_SetGammaRamp(orig_ramps[0], orig_ramps[1], orig_ramps[2]);
	glGetDeviceGammaRamp3DFX_fp = NULL;
	glSetDeviceGammaRamp3DFX_fp = NULL;
#endif
	fxGammaCtl = NULL;
	if (fx_gammalib != NULL)
		dlclose (fx_gammalib);
	fx_gammalib = NULL;
}

/*
=================
GL_BeginRendering
=================
*/
void GL_BeginRendering (int *x, int *y, int *width, int *height)
{
	*x = *y = 0;
	*width = WRWidth;
	*height = WRHeight;

//	glViewport_fp (*x, *y, *width, *height);
}


void GL_EndRendering (void)
{
	if (!scr_skipupdate)
		SDL_GL_SwapBuffers();

// handle the mouse state when windowed if that's changed
#if 0	// change to 1 if dont want to disable mouse in fullscreen
	if (!fullscreen)
#endif
		if ((int)_enable_mouse.value != enable_mouse)
		{
			if (_enable_mouse.value)
				IN_ActivateMouse ();
			else
				IN_DeactivateMouse ();

			enable_mouse = (int)_enable_mouse.value;
		}

	if (fullsbardraw)
		Sbar_Changed();
}


int ColorIndex[16] =
{
	0, 31, 47, 63, 79, 95, 111, 127, 143, 159, 175, 191, 199, 207, 223, 231
};

unsigned ColorPercent[16] =
{
	25, 51, 76, 102, 114, 127, 140, 153, 165, 178, 191, 204, 216, 229, 237, 247
};

#ifdef DO_BUILD
// these two procedures should have been used by Raven to
// generate the gfx/invpal.lmp file which resides in pak0
static int ConvertTrueColorToPal (unsigned char *true_color, unsigned char *palette)
{
	int	i;
	long	min_dist;
	int	min_index;
	long	r, g, b;

	min_dist = 256 * 256 + 256 * 256 + 256 * 256;
	min_index = -1;
	r = ( long )true_color[0];
	g = ( long )true_color[1];
	b = ( long )true_color[2];

	for (i = 0; i < 256; i++)
	{
		long palr, palg, palb, dist;
		long dr, dg, db;

		palr = palette[3*i];
		palg = palette[3*i+1];
		palb = palette[3*i+2];
		dr = palr - r;
		dg = palg - g;
		db = palb - b;
		dist = dr * dr + dg * dg + db * db;
		if (dist < min_dist)
		{
			min_dist = dist;
			min_index = i;
		}
	}
	return min_index;
}

static void VID_CreateInversePalette (unsigned char *palette)
{
	FILE	*FH;
	long	r, g, b;
	long	index = 0;
	unsigned char	true_color[3];
	char	path[MAX_OSPATH];

	for (r = 0; r < ( 1 << INVERSE_PAL_R_BITS ); r++)
	{
		for (g = 0; g < ( 1 << INVERSE_PAL_G_BITS ); g++)
		{
			for (b = 0; b < ( 1 << INVERSE_PAL_B_BITS ); b++)
			{
				true_color[0] = ( unsigned char )( r << ( 8 - INVERSE_PAL_R_BITS ) );
				true_color[1] = ( unsigned char )( g << ( 8 - INVERSE_PAL_G_BITS ) );
				true_color[2] = ( unsigned char )( b << ( 8 - INVERSE_PAL_B_BITS ) );
				inverse_pal[index] = ConvertTrueColorToPal( true_color, palette );
				index++;
			}
		}
	}

	snprintf (path, MAX_OSPATH, "%s/data1/gfx", com_basedir);
	Sys_mkdir (path);
	snprintf (path, MAX_OSPATH, "%s/data1/gfx/invpal.lmp", com_basedir);
	FH = fopen(path, "wb");
	if (!FH)
		Sys_Error ("Couldn't create %s", path);
	//fwrite (inverse_pal, 1, sizeof(inverse_pal), FH);
	fwrite (inverse_pal, 1, (sizeof(inverse_pal))-1, FH);
	fclose (FH);
	Con_Printf ("Created %s\n", path);
}
#endif


void VID_SetPalette (unsigned char *palette)
{
	byte	*pal;
	unsigned short	r,g,b;
	int		v;
	unsigned short	i, p, c;
	unsigned	*table;
//	unsigned	*table3dfx;
#ifndef OLD_8_BIT_PALETTE_CODE
	int		r1,g1,b1;
	int		j,k,l,m;
	FILE	*f;
	char	s[MAX_OSPATH];
#endif
	static qboolean	been_here = false;

//
// 8 8 8 encoding
//
	pal = palette;
	table = d_8to24table;
//	table3dfx = d_8to24table3dfx;
	for (i=0 ; i<256 ; i++)
	{
		r = pal[0];
		g = pal[1];
		b = pal[2];
		pal += 3;

		v = (255<<24) + (r<<0) + (g<<8) + (b<<16);
		*table++ = v;
//		v = (255<<24) + (r<<16) + (g<<8) + (b<<0);
//		*table3dfx++ = v;
	}

	d_8to24table[255] &= 0xffffff;	// 255 is transparent

	pal = palette;
	table = d_8to24TranslucentTable;

	for (i=0; i<16;i++)
	{
		c = ColorIndex[i]*3;

		r = pal[c];
		g = pal[c+1];
		b = pal[c+2];

		for(p=0;p<16;p++)
		{
			v = (ColorPercent[15-p]<<24) + (r<<0) + (g<<8) + (b<<16);
			//v = (255<<24) + (r<<0) + (g<<8) + (b<<16);
			*table++ = v;

			RTint[i*16+p] = ((float)r) / ((float)ColorPercent[15-p]);
			GTint[i*16+p] = ((float)g) / ((float)ColorPercent[15-p]);
			BTint[i*16+p] = ((float)b) / ((float)ColorPercent[15-p]);
		}
	}

	// Initialize the palettized textures data
	if (been_here)
		return;

#ifdef OLD_8_BIT_PALETTE_CODE
	// This is original hexen2 code for palettized textures
	// Hexenworld replaced it with quake's newer code below
#   ifdef DO_BUILD
	VID_CreateInversePalette (palette);
#   else
	COM_LoadStackFile ("gfx/invpal.lmp", inverse_pal, sizeof(inverse_pal));
#   endif

#else // end of OLD_8_BIT_PALETTE_CODE
	COM_FOpenFile("glhexen/15to8.pal", &f, true);
	if (f)
	{
		fread(d_15to8table, 1<<15, 1, f);
		fclose(f);
	}
	else
	{	// JACK: 3D distance calcs:
		// k is last closest, l is the distance
		Con_Printf ("Creating 15to8.pal ..");
		for (i=0,m=0; i < (1<<15); i++,m++)
		{
			/* Maps
			000000000000000
			000000000011111 = Red  = 0x1F
			000001111100000 = Blue = 0x03E0
			111110000000000 = Grn  = 0x7C00
			*/
			r = ((i & 0x1F) << 3)+4;
			g = ((i & 0x03E0) >> 2)+4;
			b = ((i & 0x7C00) >> 7)+4;
#   if 0
			r = (i << 11);
			g = (i << 6);
			b = (i << 1);
			r >>= 11;
			g >>= 11;
			b >>= 11;
#   endif
			pal = (unsigned char *)d_8to24table;
			for (v=0,k=0,l=10000; v<256; v++,pal+=4)
			{
				r1 = r-pal[0];
				g1 = g-pal[1];
				b1 = b-pal[2];
				j = sqrt(((r1*r1)+(g1*g1)+(b1*b1)));
				if (j<l)
				{
					k=v;
					l=j;
				}
			}
			d_15to8table[i]=k;
			if (m >= 1000)
				m=0;
		}
		sprintf(s, "%s/glhexen", com_userdir);
		Sys_mkdir (s);
		sprintf(s, "%s/glhexen/15to8.pal", com_userdir);
		if ((f = fopen(s, "wb")))
		{
			fwrite(d_15to8table, 1<<15, 1, f);
			fclose(f);
		}
		Con_Printf(". done\n");
	}
#endif	// end of new 8_BIT_PALETTE_CODE
	been_here = true;
}


/*
=================
VID_ChangeVideoMode 
intended only as a callback for VID_Restart_f 
=================
*/
static void VID_ChangeVideoMode(int newmode)
{
	int j, temp;

	if (!screen)
		return;

	temp = scr_disabled_for_loading;
	scr_disabled_for_loading = true;

	// restore gamma (just in case), reset gamma function pointers
	VID_ShutdownGamma();
	CDAudio_Pause ();
	MIDI_Pause(2);
	S_ClearBuffer ();

	// Unload all textures and reset texture counts
	D_ClearOpenGLTextures(0);
	texture_extension_number = 1;
	lightmap_textures = 0;
	for (j = 0; j < MAX_LIGHTMAPS; j++)
		lightmap_modified[j] = true;

	// reset all opengl function pointers (just in case)
	GL_ResetFunctions();

	// Avoid the SDL_QUIT event
	// Avoid re-registering commands and re-allocating memory
	draw_reinit = true;

	// temporarily disable input devices
	IN_DeactivateMouse ();
	IN_ShowMouse ();

	// Kill device and rendering contexts
	SDL_FreeSurface(screen);
	SDL_QuitSubSystem(SDL_INIT_VIDEO);	// also unloads the opengl driver

	// re-init sdl_video, set the mode and re-init opengl
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
		Sys_Error ("Couldn't init video: %s", SDL_GetError());
#ifdef GL_DLSYM
	if (!GL_OpenLibrary(gl_library))
		Sys_Error ("Unable to load GL library %s", gl_library);
#endif
	VID_SetMode (newmode);
	// re-get the video info since we re-inited sdl_video
	vid_info = SDL_GetVideoInfo();
	GL_Init();
	VID_InitGamma();
	if (COM_CheckParm("-paltex"))
		VID_Init8bitPalette();

	// re-init input devices
	IN_Init ();
	ClearAllStates ();
	CDAudio_Resume ();
	MIDI_Pause(1);

	// Reload graphics wad file (Draw_PicFromWad writes glpic_t data (sizes,
	// texnums) right on top of the original pic data, so the pic data will
	// be dirty after gl textures are loaded the first time; we need to load
	// a clean version)
	W_LoadWadFile ("gfx.wad");

	// Reload pre-map pics, fonts, console, etc
	Draw_Init();
	SCR_Init();
	Sbar_Init();
	// Reload the particle texture
	R_InitParticleTexture();

	vid.recalc_refdef = 1;

	// Reload model textures and player skins
	Mod_ReloadTextures();
	// rebuild the lightmaps
	GL_BuildLightmaps();

	// finished reloading all images
	draw_reinit = false;
	scr_disabled_for_loading = temp;

	// apply our gamma
	VID_ShiftPalette(NULL);
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

void	VID_Shutdown (void)
{
	VID_ShutdownGamma();
	SDL_Quit();
}


/*
===================================================================

MAIN WINDOW

===================================================================
*/

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

static void VID_Init8bitPalette (void)
{
	// Check for 8bit Extensions and initialize them.
	int i;
	char thePalette[256*3];
	char *oldPalette, *newPalette;

	MyglColorTableEXT = NULL;
	if (strstr(gl_extensions, "GL_EXT_shared_texture_palette"))
	{
		MyglColorTableEXT = (void *)SDL_GL_GetProcAddress("glColorTableEXT");
		if (MyglColorTableEXT == NULL)
		{
			Con_Printf ("Unable to init palettized textures\n");
			return;
		}
		Con_Printf("8-bit palettized textures enabled\n");
		glEnable_fp( GL_SHARED_TEXTURE_PALETTE_EXT );
		oldPalette = (char *) d_8to24table; //d_8to24table3dfx;
		newPalette = thePalette;

		for (i=0;i<256;i++)
		{
			*newPalette++ = *oldPalette++;
			*newPalette++ = *oldPalette++;
			*newPalette++ = *oldPalette++;
			oldPalette++;
		}

		MyglColorTableEXT(GL_SHARED_TEXTURE_PALETTE_EXT, GL_RGB, 256,
			GL_RGB, GL_UNSIGNED_BYTE, (void *) thePalette);
		is8bit = true;
	}
}

/*
===================
VID_Init
===================
*/
void	VID_Init (unsigned char *palette)
{
	int	i, width, height;
	char	gldir[MAX_OSPATH];
	SDL_Rect	**enumlist;
	const SDL_version	*sdl_version;

	Cvar_RegisterVariable (&vid_mode);
	Cvar_RegisterVariable (&_enable_mouse);
	Cvar_RegisterVariable (&gl_ztrick);
	Cvar_RegisterVariable (&gl_purge_maptex);

	Cmd_AddCommand ("vid_showinfo", &VID_ShowInfo_f);
	Cmd_AddCommand ("vid_listmodes", &VID_ListModes_f);
	Cmd_AddCommand ("vid_nummodes", &VID_NumModes_f);
	Cmd_AddCommand ("vid_restart", &VID_Restart_f);

	vid.numpages = 2;

	// prepare directories for caching mesh files
	sprintf (gldir, "%s/glhexen", com_userdir);
	Sys_mkdir (gldir);
	sprintf (gldir, "%s/glhexen/boss", com_userdir);
	Sys_mkdir (gldir);
	sprintf (gldir, "%s/glhexen/puzzle", com_userdir);
	Sys_mkdir (gldir);

	// see if the SDL version we linked to is multisampling-capable
	sdl_version = SDL_Linked_Version();
	if (SDL_VERSIONNUM(sdl_version->major,sdl_version->minor,sdl_version->patch) >= SDL_VER_WITH_MULTISAMPLING)
		sdl_has_multisample = true;

	// enable vsync for nvidia geforce or newer - S.A
	if (COM_CheckParm("-sync") || COM_CheckParm("-vsync"))
	{
		setenv("__GL_SYNC_TO_VBLANK", "1", 1);
		Con_Printf ("Nvidia GL vsync enabled\n");
	}

	// set fxMesa mode to fullscreen, don't let it it cheat multitexturing
	setenv ("MESA_GLX_FX","f",1);
	setenv ("FX_DONT_FAKE_MULTITEX","1",1);

	// init sdl
	// the first check is actually unnecessary
	if ( (SDL_WasInit(SDL_INIT_AUDIO)) == 0 )
		if (SDL_Init(SDL_INIT_VIDEO) < 0)
			Sys_Error ("Couldn't init video: %s", SDL_GetError());

#ifdef GL_DLSYM
	if ((i = COM_CheckParm("--gllibrary")))
		gl_library = com_argv[i+1];
	else if ((i = COM_CheckParm("-g")))
		gl_library = com_argv[i+1];
	else
		gl_library = NULL;	// trust SDL's wisdom here

	// load the opengl library
	if (!GL_OpenLibrary(gl_library))
		Sys_Error ("Unable to load GL library %s", gl_library);
#endif

	// this will contain the "best bpp" for the current display
	// make sure to re-retrieve it if you ever re-init sdl_video
	vid_info = SDL_GetVideoInfo();

	// retrieve the list of fullscreen modes
	enumlist = SDL_ListModes(NULL, SDL_OPENGL|SDL_FULLSCREEN);

	i = COM_CheckParm("-bpp");
	if (i && i < com_argc-1)
	{
		bpp = atoi(com_argv[i+1]);
	}

	// prepare the modelists, find the actual modenum for vid_default
	VID_PrepareModes(enumlist);

	// set vid_mode to our safe default first
	Cvar_SetValue ("vid_mode", vid_default);

	// windowed mode is default
	// see if the user wants fullscreen
	if (COM_CheckParm("-f") || COM_CheckParm("-fullscreen") || COM_CheckParm("--fullscreen"))
	{
		fullscreen = true;
		if (!num_fmodes) // FIXME: see below, as well
			Sys_Error ("No fullscreen modes available at this color depth");
	}

	// user is always right ...
	i = COM_CheckParm("-width");
	if (i && i < com_argc-1)
	{	// FIXME: this part doesn't know about a disaster case
		// like we aren't reported any fullscreen modes.
		width = atoi(com_argv[i+1]);

		i = COM_CheckParm("-height");
		if (i && i < com_argc-1)
			height = atoi(com_argv[i+1]);
		else	// proceed with 4/3 ratio
			height = 3 * width / 4;

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
			modelist[*nummodes].bpp = 16;
			sprintf (modelist[*nummodes].modedesc,"%d x %d (user mode)",width,height);
			Cvar_SetValue ("vid_mode", *nummodes);
			(*nummodes)++;	// ugly, I know. but works
		}
		else
		{
			Con_Printf ("ignoring invalid -width and/or -height arguments\n");
		}
	}

	vid.maxwarpwidth = WARP_WIDTH;
	vid.maxwarpheight = WARP_HEIGHT;
	vid.colormap = host_colormap;
	vid.fullbright = 256 - LittleLong (*((int *)vid.colormap + 2048));

	// so Con_Printfs don't mess us up by forcing vid and snd updates
	i = scr_disabled_for_loading;
	scr_disabled_for_loading = true;
	//set the mode
	VID_SetMode ((int)vid_mode.value);
	ClearAllStates ();
	GL_Init ();
	VID_InitGamma();

	// avoid the 3dfx splash screen on resolution changes
	setenv ("FX_GLIDE_NO_SPLASH","0",1);

	// set our palette
	VID_SetPalette (palette);

	// enable paletted textures if -paltex cmdline arg is used
	MyglColorTableEXT = NULL;
	if (COM_CheckParm("-paltex"))
		VID_Init8bitPalette();

	vid_initialized = true;
	scr_disabled_for_loading = i;
	vid.recalc_refdef = 1;

	Con_SafePrintf ("Video initialized.\n");

	vid_menudrawfn = VID_MenuDraw;
	vid_menukeyfn = VID_MenuKey;

	if (COM_CheckParm("-fullsbar"))
		fullsbardraw = true;
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
	if (!num_fmodes)
		return;

	S_ClearBuffer ();

	// This doesn't seem to cause any trouble even
	// with is_3dfx == true and FX_GLX_MESA == f
	if (SDL_WM_ToggleFullScreen(screen)==1)
	{
		fullscreen = !fullscreen;
		modestate = (fullscreen) ? MS_FULLDIB : MS_WINDOWED;
		if (fullscreen)
		{
#if 0	// change to 1 if dont want to disable mouse in fullscreen
			if (!_enable_mouse.value)
				Cvar_SetValue ("_enable_mouse", 1);
#endif
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
	}
	else
	{
		Con_Printf ("SDL_WM_ToggleFullScreen failed\n");
	}
}


#ifndef H2W
//unused in hexenworld
void D_ShowLoadingSize(void)
{
	if (!vid_initialized)
		return;

	glDrawBuffer_fp  (GL_FRONT);

	SCR_DrawLoading();

	glDrawBuffer_fp  (GL_BACK);

	glFlush_fp();
}
#endif


//========================================================
// Video menu stuff
//========================================================

static int	vid_menunum;
static int	vid_cursor;
static qboolean	vid_menu_firsttime = true;

enum {
	VID_FULLSCREEN,	// make sure the fullscreen entry (0)
	VID_RESOLUTION,	// is lower than resolution entry (1)
	VID_ITEMS
};

/*
================
VID_MenuDraw
================
*/
void VID_MenuDraw (void)
{
	ScrollTitle("gfx/menu/title7.lmp");

	if (vid_menu_firsttime)
	{	// settings for entering the menu first time
		vid_menunum = vid_modenum;
		vid_cursor = (num_fmodes) ? 0 : VID_RESOLUTION;
		vid_menu_firsttime = false;
	}

	if (num_fmodes)
	{
		M_Print (76, 92 + 8*VID_FULLSCREEN, "Fullscreen: ");
		M_DrawCheckbox (76+12*8, 92 + 8*VID_FULLSCREEN, modestate);
	}

	M_Print (76, 92 + 8*VID_RESOLUTION, "Resolution: ");
	if (vid_menunum == vid_modenum)
		M_PrintWhite (76+12*8, 92 + 8*VID_RESOLUTION, modelist[vid_menunum].modedesc);
	else
		M_Print (76+12*8, 92 + 8*VID_RESOLUTION, modelist[vid_menunum].modedesc);

	M_DrawCharacter (64, 92 + vid_cursor*8, 12+((int)(realtime*4)&1));
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
	case K_UPARROW:
		S_LocalSound ("raven/menu1.wav");
		vid_cursor--;
		if (!num_fmodes && vid_cursor == VID_FULLSCREEN)
			vid_cursor--;
		if (vid_cursor < 0)
			vid_cursor = VID_ITEMS-1;
		break;

	case K_DOWNARROW:
		S_LocalSound ("raven/menu1.wav");
		vid_cursor++;
		if (vid_cursor >= VID_ITEMS)
			vid_cursor = 0;
		if (!num_fmodes && vid_cursor == VID_FULLSCREEN)
			vid_cursor++;
		break;

	case K_ENTER:
		switch (vid_cursor)
		{
		case VID_FULLSCREEN:
			VID_ToggleFullscreen();
			break;
		case VID_RESOLUTION:
			if (vid_menunum != vid_modenum)
			{
				Cvar_SetValue("vid_mode", vid_menunum);
				VID_Restart_f();
			}
			break;
		}
		return;

	case K_ESCAPE:
		M_Menu_Options_f ();
		break;

	case K_LEFTARROW:
		switch (vid_cursor)
		{
		case VID_FULLSCREEN:
			VID_ToggleFullscreen();
			break;
		case VID_RESOLUTION:
			S_LocalSound ("raven/menu1.wav");
			vid_menunum--;
			if (vid_menunum < 0)
				vid_menunum = 0;
			break;
		}
		return;

	case K_RIGHTARROW:
		switch (vid_cursor)
		{
		case VID_FULLSCREEN:
			VID_ToggleFullscreen();
			break;
		case VID_RESOLUTION:
			S_LocalSound ("raven/menu1.wav");
			vid_menunum++;
			if (vid_menunum >= *nummodes)
				vid_menunum = *nummodes - 1;
			break;
		}
		return;

	default:
		break;
	}
}

