/*
 * gl_viddos.c -- MSDOS OpenGL, based on gl_vidsdl.c and gl_vidnt.c.
 *
 * Copyright (C) 1996-1997  Id Software, Inc.
 * Copyright (C) 1997-1998  Raven Software Corp.
 * Copyright (C) 2015 O.Sezer <sezero@users.sourceforge.net>
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

#define	__GL_FUNC_EXTERN

#include "quakedef.h"
#include "cfgfile.h"
#include "bgmusic.h"
#include "cdaudio.h"
#include <unistd.h>
#include "filenames.h"

#include "gl_dos.h"
#include "sys_dxe.h"

/* DOSGL interface */
int  (*DOSGL_InitCtx ) (int *width, int *height, int *bpp);
void (*DOSGL_Shutdown) (void);
void (*DOSGL_EndFrame) (void);
void * (*DOSGL_GetProcAddress) (const char *);
const char * (*DOSGL_APIName) (void);

#define WARP_WIDTH		320
#define WARP_HEIGHT		200
#define MAXWIDTH		10000
#define MAXHEIGHT		10000
#define MIN_WIDTH		320
//#define MIN_HEIGHT		200
#define MIN_HEIGHT		240
#define MAX_DESC		33

typedef struct {
	int			width;
	int			height;
	int			modenum;
	int			bpp;
	int			halfscreen;
	char		modedesc[MAX_DESC];
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
	{640, 480},	// 3 == RES_640X480, this is our default
	{800, 600},
	{856, 480},	// GR_RESOLUTION_856x480
	{960, 720},	// GR_RESOLUTION_960x720
	{1024,768},
	{1280,1024},
	{1600,1200}
};

#define MAX_MODE_LIST	20
#define MAX_STDMODES	(sizeof(std_modes) / sizeof(std_modes[0]))
static vmode_t	modelist[MAX_MODE_LIST+1];

static int	nummodes;
static int	bpp = 16;

typedef struct {
	int	red,
		green,
		blue,
		alpha,
		depth,
		stencil;
} attributes_t;

// vars for vid state
viddef_t	vid;			// global video state
modestate_t	modestate = MS_UNINIT;
static int	vid_default = -1;	// modenum of 640x480 as a safe default
static int	vid_modenum = -1;	// current video mode, set after mode setting succeeds
static int	vid_maxwidth = 640, vid_maxheight = 480;
static qboolean	vid_conscale = false;
static int	WRHeight, WRWidth;

static qboolean	vid_initialized = false;
qboolean	in_mode_set = false;

// cvar vid_mode must be set before calling
// VID_SetMode, VID_ChangeVideoMode or VID_Restart_f
static cvar_t	vid_mode = {"vid_mode", "0", CVAR_NONE};
static cvar_t	vid_config_consize = {"vid_config_consize", "640", CVAR_ARCHIVE};
static cvar_t	vid_config_glx = {"vid_config_glx", "640", CVAR_ARCHIVE};
static cvar_t	vid_config_gly = {"vid_config_gly", "480", CVAR_ARCHIVE};
// cvars for compatibility with the software version
static cvar_t	vid_nopageflip = {"vid_nopageflip", "0", CVAR_ARCHIVE};
static cvar_t	_vid_wait_override = {"_vid_wait_override", "0", CVAR_ARCHIVE};
static cvar_t	_vid_default_mode = {"_vid_default_mode", "0", CVAR_ARCHIVE};

byte		globalcolormap[VID_GRADES*256];
float		RTint[256], GTint[256], BTint[256];
unsigned short	d_8to16table[256];
unsigned int	d_8to24table[256];
unsigned int	d_8to24TranslucentTable[256];
unsigned char	*inverse_pal;

// gl stuff
static void GL_Init (void);

#ifdef GL_DLSYM
static const char	*gl_library;
#endif
static void		*gl_handle;

static const char	*gl_vendor;
static const char	*gl_renderer;
static const char	*gl_version;
static const char	*gl_extensions;
qboolean	is_3dfx = false;

GLint		gl_max_size = 256;
static qboolean	gl_has_NPOT = false;
qboolean	gl_tex_NPOT = false;
cvar_t		gl_texture_NPOT = {"gl_texture_NPOT", "0", CVAR_ARCHIVE};
GLfloat		gl_max_anisotropy;
float		gldepthmin, gldepthmax;

// palettized textures
static qboolean	have8bit = false;
qboolean	is8bit = false;
static cvar_t	vid_config_gl8bit = {"vid_config_gl8bit", "0", CVAR_ARCHIVE};

// Gamma stuff
#define	USE_GAMMA_RAMPS			0

/* 3dfx gamma hacks: stuff are in fx_gamma.c
 * Note: gamma ramps crashes voodoo graphics */
#define	USE_3DFX_RAMPS			0
#if defined(USE_3DFXGAMMA)
#include "fx_gamma.h"
#endif

#if (USE_GAMMA_RAMPS) || (defined(USE_3DFXGAMMA) && (USE_3DFX_RAMPS))
static unsigned short	orig_ramps[3][256];
#endif

static qboolean	fx_gamma   = false;	// 3dfx-specific gamma control
static qboolean	gammaworks = false;	// whether hw-gamma works

// multitexturing
qboolean	gl_mtexable = false;
static GLint	num_tmus = 1;

// stencil buffer
qboolean	have_stencil = false;

// this is useless: things aren't like those in quake
//static qboolean	fullsbardraw = false;

// menu drawing
static void VID_MenuDraw (void);
static void VID_MenuKey (int key);

// input stuff
static void ClearAllStates (void);
cvar_t		_enable_mouse = {"_enable_mouse", "1", CVAR_ARCHIVE};


//====================================

static qboolean GL_ParseExtensionList (const char *list, const char *name)
{
	const char	*start;
	const char	*where, *terminator;

	if (!list || !name || !*name)
		return false;
	if (strchr(name, ' ') != NULL)
		return false;	// extension names must not have spaces

	start = list;
	while (1) {
		where = strstr (start, name);
		if (!where)
			break;
		terminator = where + strlen (name);
		if (where == start || where[-1] == ' ')
			if (*terminator == ' ' || *terminator == '\0')
				return true;
		start = terminator;
	}
	return false;
}

//====================================

void VID_LockBuffer (void)
{
// nothing to do
}

void VID_UnlockBuffer (void)
{
// nothing to do
}

void VID_HandlePause (qboolean paused)
{
	if (paused)	IN_DeactivateMouse ();
	else		IN_ActivateMouse ();
}

//====================================

static void VID_ConWidth (int modenum)
{
	int	w, h;

	if (!vid_conscale)
	{
		Cvar_SetValueQuick (&vid_config_consize, modelist[modenum].width);
		return;
	}

	w = vid_config_consize.integer;
	w &= ~7; /* make it a multiple of eight */
	if (w < MIN_WIDTH)
		w = MIN_WIDTH;
	else if (w > modelist[modenum].width)
		w = modelist[modenum].width;

	h = w * modelist[modenum].height / modelist[modenum].width;
	if (h < 200 /* MIN_HEIGHT */ ||
	    h > modelist[modenum].height || w > modelist[modenum].width)
	{
		vid_conscale = false;
		Cvar_SetValueQuick (&vid_config_consize, modelist[modenum].width);
		return;
	}
	vid.width = vid.conwidth = w;
	vid.height = vid.conheight = h;
	if (w != modelist[modenum].width)
		vid_conscale = true;
	else	vid_conscale = false;
}

void VID_ChangeConsize (int dir)
{
	int	w, h;

	switch (dir)
	{
	case -1: /* smaller text */
		w = ((float)vid.conwidth/(float)vid.width + 0.05f) * vid.width; /* use 0.10f increment ?? */
		w &= ~7; /* make it a multiple of eight */
		if (w > modelist[vid_modenum].width)
			w = modelist[vid_modenum].width;
		break;

	case 1: /* bigger text */
		w = ((float)vid.conwidth/(float)vid.width - 0.05f) * vid.width;
		w &= ~7; /* make it a multiple of eight */
		if (w < MIN_WIDTH)
			w = MIN_WIDTH;
		break;

	default:	/* bad key */
		return;
	}

	h = w * modelist[vid_modenum].height / modelist[vid_modenum].width;
	if (h < 200)
		return;
	vid.width = vid.conwidth = w;
	vid.height = vid.conheight = h;
	Cvar_SetValueQuick (&vid_config_consize, vid.conwidth);
	vid.recalc_refdef = 1;
	if (vid.conwidth != modelist[vid_modenum].width)
		vid_conscale = true;
	else	vid_conscale = false;
}

float VID_ReportConsize(void)
{
	return (float)modelist[vid_modenum].width/vid.conwidth;
}


static qboolean VID_SetMode (int modenum)
{
	int	width, height;

	in_mode_set = true;

	width = modelist[modenum].width;
	height = modelist[modenum].height;
	Con_SafePrintf ("Requested mode %d: %dx%dx%d\n", modenum, width, height, bpp);

	if (DOSGL_InitCtx(&width, &height, &bpp) < 0)
		Sys_Error("DOSGL: Failed creating context.");

	// set vid_modenum properly and adjust other vars
	vid_modenum = modenum;
	modestate = MS_FULLDIB;
	Cvar_SetValueQuick (&vid_config_glx, modelist[vid_modenum].width);
	Cvar_SetValueQuick (&vid_config_gly, modelist[vid_modenum].height);
	WRWidth = vid.width = vid.conwidth = modelist[modenum].width;
	WRHeight = vid.height = vid.conheight = modelist[modenum].height;

	// setup the effective console width
	VID_ConWidth(modenum);

	Con_SafePrintf ("Video Mode Set : %dx%dx%d\n", width, height, bpp);

	in_mode_set = false;

	return true;
}


//====================================

static void VID_Init8bitPalette (void)
{
	// Check for 8bit Extensions and initialize them.
	int			i;
	char	thePalette[256*3];
	char	*oldPalette, *newPalette;

	have8bit = false;
	is8bit = false;

	if (GL_ParseExtensionList(gl_extensions, "GL_EXT_shared_texture_palette"))
	{
		glColorTableEXT_fp = (glColorTableEXT_f) DOSGL_GetProcAddress("glColorTableEXT");
		if (glColorTableEXT_fp == NULL)
			return;

		have8bit = true;
		if (!vid_config_gl8bit.integer)
			return;

		oldPalette = (char *) d_8to24table;
		newPalette = thePalette;
		for (i = 0; i < 256; i++)
		{
			*newPalette++ = *oldPalette++;
			*newPalette++ = *oldPalette++;
			*newPalette++ = *oldPalette++;
			oldPalette++;
		}

		glEnable_fp (GL_SHARED_TEXTURE_PALETTE_EXT);
		glColorTableEXT_fp (GL_SHARED_TEXTURE_PALETTE_EXT, GL_RGB, 256,
					GL_RGB, GL_UNSIGNED_BYTE, (void *) thePalette);
		is8bit = true;
		Con_SafePrintf("8-bit palettized textures enabled\n");
	}
}


#if !defined(USE_3DFXGAMMA)
static inline int Init_3dfxGammaCtrl (void)		{ return 0; }
static inline void Shutdown_3dfxGamma (void)		{ }
static inline int do3dfxGammaCtrl (float value)			{ return 0; }
static inline int glGetDeviceGammaRamp3DFX (void *arrays)	{ return 0; }
static inline int glSetDeviceGammaRamp3DFX (void *arrays)	{ return 0; }
static inline qboolean VID_Check3dfxGamma (void)	{ return false; }
#else
static qboolean VID_Check3dfxGamma (void)
{
	int		ret;

	if (COM_CheckParm("-no3dfxgamma"))
		return false;

#if USE_3DFX_RAMPS /* not recommended for Voodoo1, currently crashes */
	ret = glGetDeviceGammaRamp3DFX(orig_ramps);
	if (ret != 0)
	{
		Con_SafePrintf ("Using 3dfx glide3 specific gamma ramps\n");
		return true;
	}
#else
	ret = Init_3dfxGammaCtrl();
	if (ret > 0)
	{
		Con_SafePrintf ("Using 3dfx glide%d gamma controls\n", ret);
		return true;
	}
#endif
	return false;
}
#endif	/* USE_3DFXGAMMA */

static void VID_InitGamma (void)
{
	gammaworks = fx_gamma = false;
	/* we don't have WGL_3DFX_gamma_control or an equivalent in dos. */
	/* Here is an evil hack abusing the exposed Glide symbols: */
	if (is_3dfx)
		fx_gamma = VID_Check3dfxGamma();

	if (!gammaworks && !fx_gamma)
		Con_SafePrintf("gamma adjustment not available\n");
}

static void VID_ShutdownGamma (void)
{
#if USE_3DFX_RAMPS
	if (fx_gamma) glSetDeviceGammaRamp3DFX(orig_ramps);
#else
/*	if (fx_gamma) do3dfxGammaCtrl(1);*/
#endif
	Shutdown_3dfxGamma();
}

static void VID_SetGamma (void)
{
#if (!USE_GAMMA_RAMPS) || (!USE_3DFX_RAMPS)
	float	value = (v_gamma.value > (1.0 / GAMMA_MAX)) ?
			(1.0 / v_gamma.value) : GAMMA_MAX;
#endif
#if USE_3DFX_RAMPS
	if (fx_gamma) glSetDeviceGammaRamp3DFX(ramps);
#else
	if (fx_gamma) do3dfxGammaCtrl(value);
#endif
}

void VID_ShiftPalette (unsigned char *palette)
{
	VID_SetGamma();
}


static void CheckMultiTextureExtensions (void)
{
	gl_mtexable = false;

	if (COM_CheckParm("-nomtex"))
	{
		Con_SafePrintf("Multitexture extensions disabled\n");
	}
	else if (GL_ParseExtensionList(gl_extensions, "GL_ARB_multitexture"))
	{
		Con_SafePrintf("ARB Multitexture extensions found\n");

		glGetIntegerv_fp(GL_MAX_TEXTURE_UNITS_ARB, &num_tmus);
		if (num_tmus < 2)
		{
			Con_SafePrintf("not enough TMUs, ignoring multitexture\n");
			return;
		}

		glMultiTexCoord2fARB_fp = (glMultiTexCoord2fARB_f) DOSGL_GetProcAddress("glMultiTexCoord2fARB");
		glActiveTextureARB_fp = (glActiveTextureARB_f) DOSGL_GetProcAddress("glActiveTextureARB");
		if ((glMultiTexCoord2fARB_fp == NULL) ||
		    (glActiveTextureARB_fp == NULL))
		{
			Con_SafePrintf ("Couldn't link to multitexture functions\n");
			return;
		}

		Con_SafePrintf("Found %i TMUs support\n", (int) num_tmus);
		gl_mtexable = true;
		glDisable_fp(GL_TEXTURE_2D);
		glActiveTextureARB_fp(GL_TEXTURE0_ARB);
	}
	else
	{
		Con_SafePrintf("GL_ARB_multitexture not found\n");
	}
}

static void CheckAnisotropyExtensions (void)
{
	gl_max_anisotropy = 1;

	Con_SafePrintf("Anisotropic filtering ");
	if (GL_ParseExtensionList(gl_extensions, "GL_EXT_texture_filter_anisotropic"))
	{
		GLfloat test1, test2;
		GLuint tex;

		glGetTexParameterfv_fp = (glGetTexParameterfv_f) DOSGL_GetProcAddress("glGetTexParameterfv");
		if (glGetTexParameterfv_fp == NULL)
		{
			Con_SafePrintf("... can't check driver-lock status\n... ");
			goto _skiptest;
		}
		// test to make sure we really have control over it
		// 1.0 and 2.0 should always be legal values.
		glGenTextures_fp(1, &tex);
		glBindTexture_fp(GL_TEXTURE_2D, tex);
		glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0f);
		glGetTexParameterfv_fp(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, &test1);
		glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 2.0f);
		glGetTexParameterfv_fp(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, &test2);
		glDeleteTextures_fp(1, &tex);
		if (test1 != 1 || test2 != 2)
		{
			Con_SafePrintf("driver-locked @ %.1f\n", test1);
		}
		else
		{
		_skiptest:
			glGetFloatv_fp(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &gl_max_anisotropy);
			if (gl_max_anisotropy < 2)
				Con_SafePrintf("broken\n");
			else	Con_SafePrintf("found, max %.1f\n", gl_max_anisotropy);
		}
	}
	else
	{
		Con_SafePrintf("not found\n");
	}
}

static void CheckNonPowerOfTwoTextures (void)
{
/* On Mac OS X, old Radeons lie about NPOT textures capability, they
 * fallback to software with mipmap NPOT textures.  see, e.g.:
 * http://lists.apple.com/archives/mac-opengl/2006/Dec/msg00000.html
 * http://lists.apple.com/archives/mac-opengl/2009/Oct/msg00040.html
 * http://www.idevgames.com/forums/printthread.php?tid=3814&page=2
 * MH says NVIDIA once did the same with their GeForce FX on Windows:
 * http://forums.inside3d.com/viewtopic.php?f=10&t=4832
 * Therefore, advertisement of this extension is an unreliable way of
 * detecting the actual capability: we are binding NPOT support to a
 * cvar defaulting to disabled.
 */
	if (GL_ParseExtensionList(gl_extensions, "GL_ARB_texture_non_power_of_two"))
	{
		gl_has_NPOT = true;
		Con_SafePrintf("Found ARB_texture_non_power_of_two\n");
	}
	else
	{
		gl_has_NPOT = false;
		Cvar_SetROM("gl_texture_NPOT", "0");
	}
	gl_tex_NPOT = !!gl_texture_NPOT.integer;
}

static void CheckStencilBuffer (void)
{
	have_stencil = false;
	/* FIXME ??? */
}


static void DOSGL_Init (void)
{
	int rc = -1;

	if (rc < 0) rc = DMESA_LoadAPI (gl_handle);
	if (rc < 0) rc = SAGE_LoadAPI (gl_handle);
	if (rc < 0) rc = FXMESA_LoadAPI (gl_handle);
	if (rc < 0) {
		Sys_Error("Unable to find a supported API for DOSGL.");
	}
	Con_SafePrintf("DOSGL: driver using %s API.\n", DOSGL_APIName());
}

#ifdef GL_DLSYM
static qboolean GL_OpenLibrary (const char *name)
{
	Con_SafePrintf("Loading OpenGL library %s\n", name);

	// open the library
	if (!(gl_handle = Sys_dlopen(name, false)))
	{
		Con_SafePrintf("Unable to dlopen %s\n", name);
		return false;
	}

	return true;
}

static void GL_CloseLibrary (void)
{
	// clear the DOSGL function pointers
	DOSGL_InitCtx  = NULL;
	DOSGL_Shutdown = NULL;
	DOSGL_EndFrame = NULL;
	DOSGL_GetProcAddress = NULL;
	DOSGL_APIName = NULL;

	// free the library
	if (gl_handle != NULL)
	{
		Sys_dlclose(gl_handle);
		gl_handle = NULL;
	}
}
#endif	/* GL_DLSYM */


#ifdef GL_DLSYM
static void GL_Init_Functions (void)
{
#define GL_FUNCTION(ret, func, params)				\
    do {							\
	func##_fp = (func##_f) Sys_dlsym(gl_handle, "_"#func);	\
	if (func##_fp == NULL)					\
		Sys_Error("%s not found in GL library", "_"#func);	\
    } while (0);
#define GL_FUNCTION_OPT(ret, func, params)
#include "gl_func.h"
#undef	GL_FUNCTION_OPT
#undef	GL_FUNCTION
}
#endif	/* GL_DLSYM */

static void GL_ResetFunctions (void)
{
#ifdef	GL_DLSYM
#define GL_FUNCTION(ret, func, params)	\
	func##_fp = NULL;
#define GL_FUNCTION_OPT(ret, func, params)
#include "gl_func.h"
#undef	GL_FUNCTION_OPT
#undef	GL_FUNCTION
#endif	/* GL_DLSYM */

	have_stencil = false;

	gl_mtexable = false;
	glActiveTextureARB_fp = NULL;
	glMultiTexCoord2fARB_fp = NULL;

	have8bit = false;
	is8bit = false;

	gl_has_NPOT = false;
	gl_tex_NPOT = false;
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
	Con_SafePrintf ("GL_VENDOR: %s\n", gl_vendor);
	gl_renderer = (const char *)glGetString_fp (GL_RENDERER);
	Con_SafePrintf ("GL_RENDERER: %s\n", gl_renderer);

	gl_version = (const char *)glGetString_fp (GL_VERSION);
	Con_SafePrintf ("GL_VERSION: %s\n", gl_version);
	gl_extensions = (const char *)glGetString_fp (GL_EXTENSIONS);
	Con_SafeDPrintf ("GL_EXTENSIONS: %s\n", gl_extensions);

	glGetIntegerv_fp(GL_MAX_TEXTURE_SIZE, &gl_max_size);
	if (gl_max_size < 256)	// Refuse to work when less than 256
		Sys_Error ("hardware capable of min. 256k opengl texture size needed");
	Con_SafePrintf("OpenGL max.texture size: %i\n", (int) gl_max_size);

	is_3dfx = false;
	if (!q_strncasecmp(gl_renderer, "3dfx", 4)	  ||
	    !q_strncasecmp(gl_renderer, "SAGE Glide", 10) ||
	    !q_strncasecmp(gl_renderer, "Glide ", 6)	  || /* possible with Mesa 3.x/4.x/5.0.x */
	    !q_strncasecmp(gl_renderer, "Mesa Glide", 10))
	{
	// This should hopefully detect Voodoo1 and Voodoo2
	// hardware and possibly Voodoo Rush.
	// Voodoo Banshee, Voodoo3 and later are hw-accelerated
	// by DRI in XFree86-4.x and should be: is_3dfx = false.
		Con_SafePrintf("3dfx Voodoo found\n");
		is_3dfx = true;
	}

	CheckMultiTextureExtensions();
	CheckAnisotropyExtensions();
	CheckNonPowerOfTwoTextures();
	CheckStencilBuffer();

	glClearColor_fp (1,0,0,0);
	glCullFace_fp(GL_FRONT);
	glEnable_fp(GL_TEXTURE_2D);

	glEnable_fp(GL_ALPHA_TEST);
	glAlphaFunc_fp(GL_GREATER, 0.632); // 1 - e^-1 : replaced 0.666 to avoid clipping of smaller fonts/graphics
#if 0 /* causes side effects at least in 16 bpp.  */
	/* Get rid of Z-fighting for textures by offsetting the
	 * drawing of entity models compared to normal polygons.
	 * (See: R_DrawBrushModel.)
	 * (Only works if gl_ztrick is turned off) */
	glPolygonOffset_fp(0.05f, 25.0f);
#endif /* #if 0 */
	glPolygonMode_fp (GL_FRONT_AND_BACK, GL_FILL);
	glShadeModel_fp (GL_FLAT);

	glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glBlendFunc_fp (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

//	glTexEnvf_fp(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexEnvf_fp(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
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
}


void GL_EndRendering (void)
{
	if (!scr_skipupdate)
	{
		DOSGL_EndFrame();
	}
}


int ColorIndex[16] =
{
	0, 31, 47, 63, 79, 95, 111, 127, 143, 159, 175, 191, 199, 207, 223, 231
};

unsigned int ColorPercent[16] =
{
	25, 51, 76, 102, 114, 127, 140, 153, 165, 178, 191, 204, 216, 229, 237, 247
};

#define	INVERSE_PALNAME	"gfx/invpal.lmp"
static int ConvertTrueColorToPal (unsigned char *true_color, unsigned char *palette)
{
	int	i;
	long	min_dist;
	int	min_index;
	long	r, g, b;

	min_dist = 256 * 256 + 256 * 256 + 256 * 256;
	min_index = -1;
	r = (long) true_color[0];
	g = (long) true_color[1];
	b = (long) true_color[2];

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
	long	r, g, b;
	long	idx = 0;
	unsigned char	true_color[3];

	Con_SafePrintf ("Creating inverse palette\n");

	for (r = 0; r < (1 << INVERSE_PAL_R_BITS); r++)
	{
		for (g = 0; g < (1 << INVERSE_PAL_G_BITS); g++)
		{
			for (b = 0; b < (1 << INVERSE_PAL_B_BITS); b++)
			{
				true_color[0] = (unsigned char)(r << (8 - INVERSE_PAL_R_BITS));
				true_color[1] = (unsigned char)(g << (8 - INVERSE_PAL_G_BITS));
				true_color[2] = (unsigned char)(b << (8 - INVERSE_PAL_B_BITS));
				inverse_pal[idx] = ConvertTrueColorToPal(true_color, palette);
				idx++;
			}
		}
	}

	FS_CreatePath(FS_MakePath(FS_USERDIR, NULL, INVERSE_PALNAME));
	FS_WriteFile (INVERSE_PALNAME, inverse_pal, INVERSE_PAL_SIZE);
}

static void VID_InitPalette (unsigned char *palette)
{
	byte	*pal;
	unsigned short	r, g, b;
	unsigned short	i, p, c;
	unsigned int	v, *table;
	int		mark;

#if ENDIAN_RUNTIME_DETECT
	switch (host_byteorder)
	{
	case BIG_ENDIAN:	/* R G B A */
		MASK_r	=	0xff000000;
		MASK_g	=	0x00ff0000;
		MASK_b	=	0x0000ff00;
		MASK_a	=	0x000000ff;
		SHIFT_r	=	24;
		SHIFT_g	=	16;
		SHIFT_b	=	8;
		SHIFT_a	=	0;
		break;
	case LITTLE_ENDIAN:	/* A B G R */
		MASK_r	=	0x000000ff;
		MASK_g	=	0x0000ff00;
		MASK_b	=	0x00ff0000;
		MASK_a	=	0xff000000;
		SHIFT_r	=	0;
		SHIFT_g	=	8;
		SHIFT_b	=	16;
		SHIFT_a	=	24;
		break;
	default:
		break;
	}
	MASK_rgb	=	(MASK_r|MASK_g|MASK_b);
#endif	/* ENDIAN_RUNTIME_DETECT */

//
// 8 8 8 encoding
//
	pal = palette;
	table = d_8to24table;
	for (i = 0; i < 256; i++)
	{
		r = pal[0];
		g = pal[1];
		b = pal[2];
		pal += 3;

		v = (255 << SHIFT_a) + (r << SHIFT_r) + (g << SHIFT_g) + (b << SHIFT_b);
		*table++ = v;
	}

	d_8to24table[255] &= MASK_rgb;	// 255 is transparent

	pal = palette;
	table = d_8to24TranslucentTable;

	for (i = 0; i < 16; i++)
	{
		c = ColorIndex[i] * 3;

		r = pal[c];
		g = pal[c + 1];
		b = pal[c + 2];

		for (p = 0; p < 16; p++)
		{
			v = (ColorPercent[15 - p] << SHIFT_a) + (r << SHIFT_r) + (g << SHIFT_g) + (b << SHIFT_b);
			*table++ = v;

			RTint[i*16 + p] = ((float)r) / ((float)ColorPercent[15-p]);
			GTint[i*16 + p] = ((float)g) / ((float)ColorPercent[15-p]);
			BTint[i*16 + p] = ((float)b) / ((float)ColorPercent[15-p]);
		}
	}

	// Initialize the palettized textures data
	mark = Hunk_LowMark ();
	inverse_pal = (unsigned char *) FS_LoadHunkFile (INVERSE_PALNAME, NULL);
	if (inverse_pal != NULL && fs_filesize != INVERSE_PAL_SIZE)
	{
		Hunk_FreeToLowMark (mark);
		inverse_pal = NULL;
	}
	if (inverse_pal == NULL)
	{
		inverse_pal = (unsigned char *) Hunk_AllocName (INVERSE_PAL_SIZE + 1, INVERSE_PALNAME);
		VID_CreateInversePalette (palette);
	}
}

void VID_SetPalette (unsigned char *palette)
{
// nothing to do
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
	Key_ClearStates ();
	IN_ClearStates ();
}

/*
=================
VID_ChangeVideoMode
intended only as a callback for VID_Restart_f
=================
*/
static void VID_ChangeVideoMode (int newmode)
{
	int	temp;

	temp = scr_disabled_for_loading;
	scr_disabled_for_loading = true;

	// restore gamma, reset gamma function pointers
	VID_ShutdownGamma();
	CDAudio_Pause ();
	BGM_Pause ();
	S_ClearBuffer ();

	// Unload all textures and reset texture counts
	D_ClearOpenGLTextures(0);
	memset (lightmap_textures, 0, sizeof(lightmap_textures));

	// reset all opengl function pointers
	GL_ResetFunctions();

	// Avoid re-registering commands and re-allocating memory
	draw_reinit = true;

	// temporarily disable input devices
	IN_DeactivateMouse();

	// Kill device and rendering contexts
	DOSGL_Shutdown();

	VID_SetMode (newmode);

	// Reload graphics wad file (Draw_PicFromWad writes glpic_t data (sizes,
	// texnums) right on top of the original pic data, so the pic data will
	// be dirty after gl textures are loaded the first time; we need to load
	// a clean version)
	W_LoadWadFile ("gfx.wad");

	// Initialize extensions and default OpenGL parameters
	GL_Init();
	VID_InitGamma();
	VID_Init8bitPalette();

	// Reload pre-map pics, fonts, console, etc
	Draw_Init();
	SCR_Init();
	// R_Init() stuff:
	R_InitParticleTexture();
	R_InitExtraTextures ();
#if defined(H2W)
	R_InitNetgraphTexture();
#endif	/* H2W */
	Sbar_Init();
	vid.recalc_refdef = 1;

	ClearAllStates ();
	CDAudio_Resume ();
	BGM_Resume ();

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
	if (vid_mode.integer < 0 || vid_mode.integer >= nummodes)
	{
		Con_Printf ("Bad video mode %d\n", vid_mode.integer);
		Cvar_SetValueQuick (&vid_mode, vid_modenum);
		return;
	}

	Con_Printf ("Re-initializing video:\n");
	VID_ChangeVideoMode (vid_mode.integer);
}

static void VID_PrepareModes (void)
{
	int	i;

	nummodes = 0;

	// Add the standart 4:3 modes to the windowed modes list
	// In an unlikely case that we receive no fullscreen modes,
	// this will be our modes list (kind of...)
	for (i = 0; i < (int)MAX_STDMODES; i++)
	{
		modelist[nummodes].width = std_modes[i].width;
		modelist[nummodes].height = std_modes[i].height;
		modelist[nummodes].halfscreen = 0;
		modelist[nummodes].bpp = 16;
		q_snprintf (modelist[nummodes].modedesc, MAX_DESC,
				"%d x %d", std_modes[i].width, std_modes[i].height);
		nummodes++;
	}

	vid_maxwidth = modelist[nummodes-1].width;
	vid_maxheight = modelist[nummodes-1].height;
	vid_default = RES_640X480;

	Cvar_SetValueQuick (&vid_config_glx, modelist[vid_default].width);
	Cvar_SetValueQuick (&vid_config_gly, modelist[vid_default].height);
}

static void VID_ListModes_f (void)
{
	int	i;

	Con_Printf ("Maximum allowed mode: %d x %d\n", vid_maxwidth, vid_maxheight);
	Con_Printf ("Supported modes:\n");
	for (i = 0; i < nummodes; i++)
		Con_Printf ("%2d:  %d x %d\n", i, modelist[i].width, modelist[i].height);
}

static void VID_NumModes_f (void)
{
	Con_Printf ("%d video modes in current list\n", nummodes);
}

/*
===================
VID_Init
===================
*/
void	VID_Init (unsigned char *palette)
{
	static char fxmesa_env_multitex[32] = "FX_DONT_FAKE_MULTITEX=1";
	static char fxglide_env_nosplash[32] = "FX_GLIDE_NO_SPLASH=0";
	int	i, temp, width, height;
	const char	*read_vars[] = {
				"vid_config_gl8bit",
				"vid_config_glx",
				"vid_config_gly",
				"vid_config_consize",
				"gl_texture_NPOT",
				"gl_lightmapfmt" };
#define num_readvars	( sizeof(read_vars)/sizeof(read_vars[0]) )

	Cvar_RegisterVariable (&vid_config_gl8bit);
	Cvar_RegisterVariable (&vid_config_gly);
	Cvar_RegisterVariable (&vid_config_glx);
	Cvar_RegisterVariable (&vid_config_consize);
	Cvar_RegisterVariable (&vid_mode);
	Cvar_RegisterVariable (&_enable_mouse);
	Cvar_RegisterVariable (&gl_texture_NPOT);
	Cvar_RegisterVariable (&gl_lightmapfmt);
	Cvar_RegisterVariable (&vid_nopageflip);
	Cvar_RegisterVariable (&_vid_wait_override);
	Cvar_RegisterVariable (&_vid_default_mode);

	Cmd_AddCommand ("vid_listmodes", VID_ListModes_f);
	Cmd_AddCommand ("vid_nummodes", VID_NumModes_f);
	Cmd_AddCommand ("vid_restart", VID_Restart_f);

	VID_InitPalette (palette);

	vid.numpages = 2;

	// don't let fxMesa cheat multitexturing
	putenv (fxmesa_env_multitex);

#ifdef GL_DLSYM
	i = COM_CheckParm("-gllibrary");
	if (i == 0)
		i = COM_CheckParm ("-g");
	if (i && i < com_argc - 1)
		gl_library = com_argv[i+1];
	else
		gl_library = "gl.dxe";

	// load the opengl library
	if (!GL_OpenLibrary(gl_library))
		Sys_Error ("Unable to load GL library %s", gl_library);
#endif
	DOSGL_Init();

	i = COM_CheckParm("-bpp");
	if (i && i < com_argc-1)
	{
		bpp = atoi(com_argv[i+1]);
	}

	// prepare the modelists, find the actual modenum for vid_default
	VID_PrepareModes();

	// set vid_mode to our safe default first
	Cvar_SetValueQuick (&vid_mode, vid_default);

	// perform an early read of config.cfg
	CFG_ReadCvars (read_vars, num_readvars);

	width = vid_config_glx.integer;
	height = vid_config_gly.integer;

	if (vid_config_consize.integer != width)
		vid_conscale = true;

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
	}

	// user requested a mode either from the config or from the
	// command line
	// scan existing modes to see if this is already available
	// if not, add this as the last "valid" video mode and set
	// vid_mode to it only if it doesn't go beyond vid_maxwidth
	i = 0;
	while (i < nummodes)
	{
		if (modelist[i].width == width && modelist[i].height == height)
			break;
		i++;
	}
	if (i < nummodes)
	{
		Cvar_SetValueQuick (&vid_mode, i);
	}
	else if ( (width <= vid_maxwidth && width >= MIN_WIDTH &&
		   height <= vid_maxheight && height >= MIN_HEIGHT) ||
		  COM_CheckParm("-force") )
	{
		modelist[nummodes].width = width;
		modelist[nummodes].height = height;
		modelist[nummodes].halfscreen = 0;
		modelist[nummodes].bpp = 16;
		q_snprintf (modelist[nummodes].modedesc, MAX_DESC, "%d x %d (user mode)", width, height);
		Cvar_SetValueQuick (&vid_mode, nummodes);
		nummodes++;
	}
	else
	{
		Con_SafePrintf ("ignoring invalid -width and/or -height arguments\n");
	}

	if (!vid_conscale)
		Cvar_SetValueQuick (&vid_config_consize, width);

	// This will display a bigger hud and readable fonts at high
	// resolutions. The fonts will be somewhat distorted, though
	i = COM_CheckParm("-conwidth");
	if (i != 0 && i < com_argc-1)
		i = atoi(com_argv[i + 1]);
	else	i = vid_config_consize.integer;
	if (i < MIN_WIDTH)	i = MIN_WIDTH;
	else if (i > width)	i = width;
	Cvar_SetValueQuick(&vid_config_consize, i);
	if (vid_config_consize.integer != width)
		vid_conscale = true;

	if (COM_CheckParm("-paltex"))
		Cvar_SetQuick (&vid_config_gl8bit, "1");

	vid.maxwarpwidth = WARP_WIDTH;
	vid.maxwarpheight = WARP_HEIGHT;
	vid.colormap = host_colormap;
	vid.fullbright = 256 - LittleLong (*((int *)vid.colormap + 2048));

	temp = scr_disabled_for_loading;
	scr_disabled_for_loading = true;
	//set the mode
	VID_SetMode (vid_mode.integer);
	ClearAllStates ();

	GL_SetupLightmapFmt();
	GL_Init ();
	VID_InitGamma();
	VID_Init8bitPalette();

	// avoid the 3dfx splash screen on resolution changes
	putenv (fxglide_env_nosplash);

	// lock the early-read cvars until Host_Init is finished
	for (i = 0; i < (int)num_readvars; i++)
		Cvar_LockVar (read_vars[i]);

	vid_initialized = true;
	scr_disabled_for_loading = temp;
	vid.recalc_refdef = 1;

	Con_SafePrintf ("Video initialized.\n");

	vid_menudrawfn = VID_MenuDraw;
	vid_menukeyfn = VID_MenuKey;
}


void	VID_Shutdown (void)
{
	VID_ShutdownGamma();
	if (DOSGL_Shutdown)
		DOSGL_Shutdown ();
#ifdef GL_DLSYM
	GL_CloseLibrary();
#endif
}


/*
================
VID_ToggleFullscreen
Handles switching between fullscreen/windowed modes
and brings the mouse to a proper state afterwards
================
*/
void VID_ToggleFullscreen (void)
{
	// always fullscreen in DOS
}


#ifndef H2W /* unused in hexenworld */
void D_ShowLoadingSize (void)
{
#if defined(DRAW_PROGRESSBARS)
	int cur_perc;
	static int prev_perc;

	if (!vid_initialized)
		return;

	cur_perc = loading_stage * 100;
	if (total_loading_size)
		cur_perc += current_loading_size * 100 / total_loading_size;
	if (cur_perc == prev_perc)
		return;
	prev_perc = cur_perc;

	glDrawBuffer_fp (GL_FRONT);

	SCR_DrawLoading();

	glFlush_fp();

	glDrawBuffer_fp (GL_BACK);
#endif	/* DRAW_PROGRESSBARS */
}
#endif


//========================================================
// Video menu stuff
//========================================================

static int	vid_menunum;
static int	vid_cursor;
static qboolean	need_apply;
static qboolean	vid_menu_firsttime = true;

enum {
	VID_RESOLUTION,
	VID_PALTEX,
	VID_BLANKLINE,	// spacer line
	VID_RESET,
	VID_APPLY,
	VID_ITEMS
};

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
static void VID_MenuDraw (void)
{
	ScrollTitle("gfx/menu/title7.lmp");

	if (vid_menu_firsttime)
	{	// settings for entering the menu first time
		vid_menunum = vid_modenum;
		vid_cursor = 0;
		vid_menu_firsttime = false;
	}

	need_apply = (vid_menunum != vid_modenum) ||
			(have8bit && (is8bit != !!vid_config_gl8bit.integer));

	M_Print (76, 92 + 8*VID_RESOLUTION, "Resolution: ");
	if (vid_menunum == vid_modenum)
		M_PrintWhite (76+12*8, 92 + 8*VID_RESOLUTION, modelist[vid_menunum].modedesc);
	else
		M_Print (76+12*8, 92 + 8*VID_RESOLUTION, modelist[vid_menunum].modedesc);

	M_Print (76, 92 + 8*VID_PALTEX, "8 bit textures:");
	if (have8bit)
		M_DrawYesNo (76+16*8, 92 + 8*VID_PALTEX, vid_config_gl8bit.integer, (is8bit == !!vid_config_gl8bit.integer));
	else
		M_PrintWhite (76+16*8, 92 + 8*VID_PALTEX, "Not found");

	if (need_apply)
	{
		M_Print (76, 92 + 8*VID_RESET, "RESET CHANGES");
		M_Print (76, 92 + 8*VID_APPLY, "APPLY CHANGES");
	}

	M_DrawCharacter (64, 92 + vid_cursor*8, 12+((int)(realtime*4)&1));
}

/*
================
VID_MenuKey
================
*/
static void VID_MenuKey (int key)
{
	switch (key)
	{
	case K_ESCAPE:
		vid_cursor = 0;
		M_Menu_Options_f ();
		break;

	case K_UPARROW:
		S_LocalSound ("raven/menu1.wav");
		vid_cursor--;
		if (vid_cursor < 0)
		{
			vid_cursor = (need_apply) ? VID_ITEMS-1 : VID_BLANKLINE-1;
		}
		else if (vid_cursor == VID_BLANKLINE)
		{
			vid_cursor--;
		}
		break;

	case K_DOWNARROW:
		S_LocalSound ("raven/menu1.wav");
		vid_cursor++;
		if (vid_cursor >= VID_ITEMS)
		{
			vid_cursor = 0;
			break;
		}
		if (vid_cursor >= VID_BLANKLINE)
		{
			if (need_apply)
			{
				if (vid_cursor == VID_BLANKLINE)
					vid_cursor++;
			}
			else
			{
				vid_cursor = 0;
			}
		}
		break;

	case K_ENTER:
		switch (vid_cursor)
		{
		case VID_RESET:
			vid_menunum = vid_modenum;
			Cvar_SetValueQuick (&vid_config_gl8bit, is8bit);
			vid_cursor = 0;
			break;
		case VID_APPLY:
			if (need_apply)
			{
				Cvar_SetValueQuick(&vid_mode, vid_menunum);
				VID_Restart_f();
			}
			vid_cursor = 0;
			break;
		}
		return;

	case K_LEFTARROW:
		switch (vid_cursor)
		{
		case VID_RESOLUTION:
			S_LocalSound ("raven/menu1.wav");
			vid_menunum--;
			if (vid_menunum < 0)
				vid_menunum = 0;
			break;
		case VID_PALTEX:
			if (have8bit)
				Cvar_SetValueQuick (&vid_config_gl8bit, !vid_config_gl8bit.integer);
			break;
		}
		return;

	case K_RIGHTARROW:
		switch (vid_cursor)
		{
		case VID_RESOLUTION:
			S_LocalSound ("raven/menu1.wav");
			vid_menunum++;
			if (vid_menunum >= nummodes)
				vid_menunum = nummodes - 1;
			break;
		case VID_PALTEX:
			if (have8bit)
				Cvar_SetValueQuick (&vid_config_gl8bit, !vid_config_gl8bit.integer);
			break;
		}
		return;

	default:
		break;
	}
}

