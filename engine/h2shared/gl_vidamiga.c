/*
 * gl_vidamiga.c -- GL vid component for AmigaOS & variants.
 * $Id$
 *
 * Copyright (C) 1996-1997  Id Software, Inc.
 * Copyright (C) 1997-1998  Raven Software Corp.
 * Copyright (C) 2004-2005  Steven Atkinson <stevenaaus@yahoo.com>
 * Copyright (C) 2005-2016  O.Sezer <sezero@users.sourceforge.net>
 * Copyright (C) 2012-2016  Szilárd Biró <col.lawrence@gmail.com>
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

#ifdef GL_DLSYM
#error GL_DLSYM not supported.
#endif
#define	__GL_FUNC_EXTERN

#include <cybergraphx/cybergraphics.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/cybergraphics.h>

#ifdef __AROS__ /* TODO: glA support. */
#include <GL/arosmesa.h>
#elif defined __MORPHOS__
#include <intuition/intuitionbase.h>
#elif defined __amigaos4__
#error AmigaOS4 support not yet available
#elif defined(__AMIGA__) && defined(REFGL_MINIGL)
#include <mgl/gl.h>
#elif defined(__AMIGA__) && defined(REFGL_AMESA)
#include <GL/Amigamesa.h>
#else
#error Unknown / Unsupported AmigaOS variant.
#endif

#ifdef __AROS__
typedef AROSMesaProc AMIGAGL_Proc;
#else
typedef void (*AMIGAGL_Proc)();
#endif

#include "quakedef.h"
#include "cfgfile.h"
#include "bgmusic.h"
#include "cdaudio.h"
#include "filenames.h"

#if !(defined(__AROS__) || defined(__MORPHOS__))
#if !defined(IPTR) /* SDI headers may define it */
typedef ULONG IPTR;
#define IPTR IPTR
#endif
#endif

#define WARP_WIDTH		320
#define WARP_HEIGHT		200
#define MAXWIDTH		10000
#define MAXHEIGHT		10000
#define MIN_WIDTH		320
//#define MIN_HEIGHT		200
#define MIN_HEIGHT		240
#define MAX_DESC		33

typedef struct {
	modestate_t	type;
	int			width;
	int			height;
	int			modenum;
	int			fullscreen;
	int			bpp;
/*	int			halfscreen;*/
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
	{640, 480},	// 3 == RES_640X480, this is our default, below
			//		this is the lowresmodes region.
			//		either do not change its order,
			//		or change the above define, too
	{800,  600},	// 4, RES_640X480 + 1
	{1024, 768},	// 5, RES_640X480 + 2
	{1280, 1024},	// 6
	{1600, 1200}	// 7
};

#define MAX_MODE_LIST	128
#define MAX_STDMODES	(sizeof(std_modes) / sizeof(std_modes[0]))
#define NUM_LOWRESMODES	(RES_640X480)
static vmode_t	fmodelist[MAX_MODE_LIST+1];	// list of enumerated fullscreen modes
static vmode_t	wmodelist[MAX_STDMODES +1];	// list of standart 4:3 windowed modes
static vmode_t	*modelist;	// modelist in use, points to one of the above lists

static int	num_fmodes;
static int	num_wmodes;
static int	*nummodes;
static int	bpp = 16;

#if defined(H2W)
#	define WM_TITLEBAR_TEXT	"HexenWorld"
/*#	define WM_ICON_TEXT	"HexenWorld"*/
#else
#	define WM_TITLEBAR_TEXT	"Hexen II"
/*#	define WM_ICON_TEXT	"HEXEN2"*/
#endif

typedef struct {
	int	red,
		green,
		blue,
		alpha,
		depth,
		stencil;
} attributes_t;
static attributes_t	vid_attribs;

static void VID_KillContext (void);
struct Window *window = NULL; /* used by in_amiga.c */
static struct Screen *screen = NULL;
#ifdef __AROS__
static AROSMesaContext context = NULL;
#elif defined __MORPHOS__
GLContext *__tglContext = NULL;
static qboolean contextinit = false;
#elif defined(__AMIGA__) && defined(REFGL_MINIGL)
static int lockmode = MGL_LOCK_MANUAL;
#elif defined(__AMIGA__) && defined(REFGL_AMESA)
static AmigaMesaContext context = NULL;
#endif
static qboolean	vid_menu_fs;
static qboolean	fs_toggle_works = true;

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
#if defined(__AMIGA__) && !defined(__MORPHOS__) /* KLUDGE -- for amigaos3 */
static cvar_t	vid_config_fscr= {"vid_config_fscr", "1", CVAR_ARCHIVE};
#else
static cvar_t	vid_config_fscr= {"vid_config_fscr", "0", CVAR_ARCHIVE};
#endif
// cvars for compatibility with the software version
static cvar_t	vid_config_swx = {"vid_config_swx", "320", CVAR_ARCHIVE};
static cvar_t	vid_config_swy = {"vid_config_swy", "240", CVAR_ARCHIVE};

byte		globalcolormap[VID_GRADES*256];
float		RTint[256], GTint[256], BTint[256];
unsigned short	d_8to16table[256];
unsigned int	d_8to24table[256];
unsigned int	d_8to24TranslucentTable[256];
unsigned char	*inverse_pal;

// gl stuff
static void GL_Init (void);

static const char	*gl_vendor;
static const char	*gl_renderer;
static const char	*gl_version;
static const char	*gl_extensions;
qboolean	is_3dfx = false;

GLint		gl_max_size = 256;
static qboolean	have_NPOT = false;
qboolean	gl_tex_NPOT = false;
static cvar_t	gl_texture_NPOT = {"gl_texture_NPOT", "0", CVAR_ARCHIVE};
GLfloat		gl_max_anisotropy;
float		gldepthmin, gldepthmax;

// palettized textures
static qboolean	have8bit = false;
qboolean	is8bit = false;
static cvar_t	vid_config_gl8bit = {"vid_config_gl8bit", "0", CVAR_ARCHIVE};

static qboolean	gammaworks = false;	// whether hw-gamma works

// multitexturing
qboolean	gl_mtexable = false;
static GLint	num_tmus = 1;
static qboolean	have_mtex = false;
static cvar_t	gl_multitexture = {"gl_multitexture", "0", CVAR_ARCHIVE};

// stencil buffer
qboolean	have_stencil = false;

// this is useless: things aren't like those in quake
//static qboolean	fullsbardraw = false;

// menu drawing
static void VID_MenuDraw (void);
static void VID_MenuKey (int key);

// input stuff
static void ClearAllStates (void);
static int	enable_mouse;
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
	if (_enable_mouse.integer/* && (modestate == MS_WINDOWED)*/)
	{
		// for consistency, don't show pointer - S.A
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
	ULONG flags;

	in_mode_set = true;

	VID_KillContext();

	flags = WFLG_ACTIVATE | WFLG_RMBTRAP;

	Con_SafePrintf ("Requested mode %d: %dx%dx%d\n", modenum, modelist[modenum].width, modelist[modenum].height, bpp);

#if defined(REFGL_MINIGL)
	/* Hyperion's MiniGL 1.2 handles window creation within itself. */
	if (vid_config_fscr.integer)
		mglChooseWindowMode(GL_FALSE);
	else	mglChooseWindowMode(GL_TRUE );

#else /* ! REFGL_MINIGL */
	if (vid_config_fscr.integer)
	{
	    ULONG ModeID;

	    ModeID = BestCModeIDTags(
			CYBRBIDTG_Depth, bpp,
			CYBRBIDTG_NominalWidth, modelist[modenum].width,
			CYBRBIDTG_NominalHeight, modelist[modenum].height,
			TAG_DONE);

		#ifndef __MORPHOS__
		screen = OpenScreenTags(0,
			ModeID != INVALID_ID ? SA_DisplayID : TAG_IGNORE, ModeID,
			SA_Width, modelist[modenum].width,
			SA_Height, modelist[modenum].height,
			SA_Depth, bpp,
			SA_Quiet, TRUE,
			TAG_DONE);
		#else
		screen = OpenScreenTags(0,
			ModeID != INVALID_ID ? SA_DisplayID : TAG_IGNORE, ModeID,
			SA_Width, modelist[modenum].width,
			SA_Height, modelist[modenum].height,
			SA_Depth, bpp,
			SA_Quiet, TRUE,
			SA_GammaControl, TRUE,
			SA_3DSupport, TRUE,
			TAG_DONE);
		#endif
	}

	if (screen)
	{
		flags |= WFLG_BORDERLESS;
	}
	else
	{
		Cvar_SetValueQuick (&vid_config_fscr, 0);
		flags |= WFLG_DRAGBAR | WFLG_DEPTHGADGET | WFLG_CLOSEGADGET;
	}

	window = OpenWindowTags(0,
		WA_InnerWidth, modelist[modenum].width,
		WA_InnerHeight, modelist[modenum].height,
		WA_Title, (IPTR)WM_TITLEBAR_TEXT,
		WA_Flags, flags,
		screen ? WA_CustomScreen : TAG_IGNORE, (IPTR)screen,
		WA_IDCMP, IDCMP_CLOSEWINDOW | IDCMP_ACTIVEWINDOW | IDCMP_INACTIVEWINDOW,
		TAG_DONE);

	if (!window) goto fail;
#endif /* ! REFGL_MINIGL */

	WRWidth = vid.width = vid.conwidth = modelist[modenum].width;
	WRHeight = vid.height = vid.conheight = modelist[modenum].height;

#ifdef __AROS__
	context = AROSMesaCreateContextTags(
			AMA_Window, window,
			AMA_Left, screen ? 0 : window->BorderLeft,
			AMA_Top, screen ? 0 : window->BorderTop,
			AMA_Width, vid.width,
			AMA_Height, vid.height,
			screen ? AMA_Screen : TAG_IGNORE, screen,
			AMA_DoubleBuf, GL_TRUE,
			AMA_RGBMode, GL_TRUE,
			AMA_NoAccum, GL_TRUE,
			TAG_DONE);

	if (!context) goto fail;
	AROSMesaMakeCurrent(context);

#elif defined __MORPHOS__
	__tglContext = GLInit();
	if (!__tglContext) goto fail;
	if (screen && !(TinyGLBase->lib_Version == 0 && TinyGLBase->lib_Revision < 4)) {
		if (!(contextinit = glAInitializeContextScreen(screen)))
			goto fail;
	} else {
		if (!(contextinit = glAInitializeContextWindowed(window)))
			goto fail;
	}

#elif defined(__AMIGA__) && defined(REFGL_MINIGL)
	if (!mglCreateContext(0,0,vid.width,vid.height))
		goto fail;
	mglLockMode(lockmode);
	window = MGLGetWindowHandle(mini_CurrentContext);
	ModifyIDCMP(window, IDCMP_CLOSEWINDOW | IDCMP_ACTIVEWINDOW | IDCMP_INACTIVEWINDOW);

#elif defined(__AMIGA__) && defined(REFGL_AMESA)
	context = AmigaMesaCreateContextTags(
			AMA_Window, window,
			AMA_Left, screen ? 0 : window->BorderLeft,
			AMA_Bottom, screen ? 0 : window->BorderBottom,
			AMA_Width, vid.width,
			AMA_Height, vid.height,
			screen ? AMA_Screen : TAG_IGNORE, screen,
			AMA_DoubleBuf, GL_TRUE,
			AMA_RGBMode, GL_TRUE,
			AMA_NoAccum, GL_TRUE,
			TAG_DONE);

	if (!context) goto fail;
	AmigaMesaMakeCurrent(context, context->buffer);
#endif

	vid.height = vid.conheight = modelist[modenum].height;
	vid.rowbytes = vid.conrowbytes = vid.width = vid.conwidth = modelist[modenum].width;

	// set vid_modenum properly and adjust other vars
	vid_modenum = modenum;
	modestate = (screen) ? MS_FULLDIB : MS_WINDOWED;
	Cvar_SetValueQuick (&vid_config_glx, modelist[vid_modenum].width);
	Cvar_SetValueQuick (&vid_config_gly, modelist[vid_modenum].height);

	// setup the effective console width
	VID_ConWidth(modenum);

	Con_SafePrintf ("Video Mode Set : %dx%dx%d\n", modelist[modenum].width, modelist[modenum].height, bpp);

	//IN_HideMouse ();

	in_mode_set = false;

	return true;

fail:
	VID_KillContext();

	in_mode_set = false;

	return false;
}


//====================================

#if defined(__AROS__)
static AMIGAGL_Proc AMIGAGL_GetProcAddress (const char *s)
{
	return AROSMesaGetProcAddress((const GLubyte *) s);
}

#elif defined(__MORPHOS__)
static void MY_glMultiTexCoord2fARB (GLenum unit, GLfloat s, GLfloat t)
{
	GLMultiTexCoord2fARB(__tglContext, unit, s, t);
}

static void MY_glActiveTextureARB (GLenum unit)
{
	GLActiveTextureARB(__tglContext, unit);
}

static AMIGAGL_Proc AMIGAGL_GetProcAddress (const char *s)
{
	if (strcmp(s, "glMultiTexCoord2fARB") == 0)
		return (AMIGAGL_Proc)MY_glMultiTexCoord2fARB;
	if (strcmp(s, "glActiveTextureARB") == 0)
		return (AMIGAGL_Proc)MY_glActiveTextureARB;

	return NULL;
}

#elif defined(__AMIGA__) && defined(REFGL_MINIGL)
static void MY_glMultiTexCoord2fARB (GLenum unit, GLfloat s, GLfloat t)
{
	GLMultiTexCoord2fARB(mini_CurrentContext, unit, s, t);
}

static void MY_glActiveTextureARB (GLenum unit)
{
	GLActiveTextureARB(mini_CurrentContext, unit);
}

static AMIGAGL_Proc AMIGAGL_GetProcAddress (const char *s)
{
	if (strcmp(s, "glMultiTexCoord2fARB") == 0)
		return (AMIGAGL_Proc)MY_glMultiTexCoord2fARB;
	if (strcmp(s, "glActiveTextureARB") == 0)
		return (AMIGAGL_Proc)MY_glActiveTextureARB;

	return NULL;
}

#elif defined(__AMIGA__) && defined(REFGL_AMESA)
/* NOTE: StormMesa 3.0 has EXT_multitexture, not ARB_multitexture.. */
/*                     250 == ( GL_TEXTURE0_ARB - GL_TEXTURE0_EXT ) */
static void MY_glMultiTexCoord2fARB (GLenum unit, GLfloat s, GLfloat t)
{
	glMultiTexCoord2fEXT (unit - 250, s, t);
}

static void MY_glActiveTextureARB (GLenum unit)
{
	glSelectTextureEXT (unit - 250);
}

static void MY_glColorTableEXT (GLenum target, GLenum internalfmt, GLsizei width, GLenum format, GLenum type, const GLvoid *table)
{
	glColorTableEXT (target, internalfmt, width, format, type, table);
}

static void MY_glGetTexParameterfv (GLenum target, GLenum pname, GLfloat *params)
{
	glGetTexParameterfv (target, pname, params);
}

static AMIGAGL_Proc AMIGAGL_GetProcAddress (const char *s)
{
	if (strcmp(s, "glMultiTexCoord2fARB") == 0)
		return (AMIGAGL_Proc)MY_glMultiTexCoord2fARB;
	if (strcmp(s, "glActiveTextureARB") == 0)
		return (AMIGAGL_Proc)MY_glActiveTextureARB;
	if (strcmp(s, "glColorTableEXT") == 0)
		return (AMIGAGL_Proc)MY_glColorTableEXT;
	if (strcmp(s, "glGetTexParameterfv") == 0)
		return (AMIGAGL_Proc)MY_glGetTexParameterfv;

	return NULL;
}
#endif

#if 0 /* No.. */
static void CheckSetGlobalPalette (void)
{
	gl3DfxSetPaletteEXT_f gl3DfxSetPaletteEXT_fp;

	if (GL_ParseExtensionList(gl_extensions, "3DFX_set_global_palette"))
	{
		gl3DfxSetPaletteEXT_fp = (gl3DfxSetPaletteEXT_f) AMIGAGL_GetProcAddress("gl3DfxSetPaletteEXT");
		if (!gl3DfxSetPaletteEXT_fp)
			gl3DfxSetPaletteEXT_fp = (gl3DfxSetPaletteEXT_f) AMIGAGL_GetProcAddress("3DFX_set_global_palette");
		if (!gl3DfxSetPaletteEXT_fp)
			return;
		Con_SafePrintf("Found 3DFX_set_global_palette\n");
	}
	else if (GL_ParseExtensionList(gl_extensions, "POWERVR_set_global_palette"))
	{
		gl3DfxSetPaletteEXT_fp = (gl3DfxSetPaletteEXT_f) AMIGAGL_GetProcAddress("glSetGlobalPalettePOWERVR");
		if (!gl3DfxSetPaletteEXT_fp)
			gl3DfxSetPaletteEXT_fp = (gl3DfxSetPaletteEXT_f) AMIGAGL_GetProcAddress("POWERVR_set_global_palette");
		if (!gl3DfxSetPaletteEXT_fp)
			return;
		Con_SafePrintf("Found POWERVR_set_global_palette\n");
	}
	else {
		return;
	}

	have8bit = true;
	if (!vid_config_gl8bit.integer)
		return;
	else
	{
		int i;
		GLubyte table[256][4];
		char *oldpal;

		is8bit = true;
		oldpal = (char *) d_8to24table;
		for (i = 0; i < 256; i++) {
			table[i][2] = *oldpal++;
			table[i][1] = *oldpal++;
			table[i][0] = *oldpal++;
			table[i][3] = 255;
			oldpal++;
		}
		glEnable_fp (GL_SHARED_TEXTURE_PALETTE_EXT);
		gl3DfxSetPaletteEXT_fp ((GLuint *)table);
	}
}
#endif /* #if 0 */

static void CheckSharedTexturePalette (void)
{
	glColorTableEXT_f glColorTableEXT_fp;

	if (!GL_ParseExtensionList(gl_extensions, "GL_EXT_shared_texture_palette"))
		return;

	glColorTableEXT_fp = (glColorTableEXT_f) AMIGAGL_GetProcAddress("glColorTableEXT");
	if (glColorTableEXT_fp == NULL)
		return;

	have8bit = true;
	Con_SafePrintf("Found GL_EXT_shared_texture_palette\n");
	if (!vid_config_gl8bit.integer)
		return;
	else
	{
		int i;
		char thePalette[256*3];
		char *oldPalette, *newPalette;

		is8bit = true;
		oldPalette = (char *) d_8to24table;
		newPalette = thePalette;
		for (i = 0; i < 256; i++) {
			*newPalette++ = *oldPalette++;
			*newPalette++ = *oldPalette++;
			*newPalette++ = *oldPalette++;
			oldPalette++;
		}

		glEnable_fp (GL_SHARED_TEXTURE_PALETTE_EXT);
		glColorTableEXT_fp (GL_SHARED_TEXTURE_PALETTE_EXT, GL_RGB, 256,
					GL_RGB, GL_UNSIGNED_BYTE, (void *) thePalette);
	}
}

static void VID_Init8bitPalette (void)
{
	have8bit = false;
	is8bit = false;

	/* Check for 8bit Extensions and initialize them */
	CheckSharedTexturePalette();
#if 0	/* No.. */
	if (!have8bit)
		CheckSetGlobalPalette();
#endif

	if (is8bit)
		Con_SafePrintf("8-bit palettized textures enabled\n");
}


static void VID_InitGamma (void)
{
	gammaworks = false;

#ifdef __MORPHOS__
	if (screen && (IntuitionBase->LibNode.lib_Version > 50 ||
			(IntuitionBase->LibNode.lib_Version == 50 && IntuitionBase->LibNode.lib_Revision >= 74)))
		gammaworks = true;
#endif

	if (!gammaworks)
		Con_SafePrintf("gamma adjustment not available\n");
}

void VID_ShiftPalette (unsigned char *palette)
{
#ifdef __MORPHOS__
	if (screen)
	{
		SetAttrs(screen,
				SA_GammaRed, gammatable,
				SA_GammaGreen, gammatable,
				SA_GammaBlue, gammatable,
				TAG_DONE);
	}
#endif
}


static void CheckMultiTextureExtensions (void)
{
	gl_mtexable = have_mtex = false;

	if (COM_CheckParm("-nomtex"))
	{
		Con_SafePrintf("Multitexture extensions disabled\n");
	}
	#if defined (REFGL_AMESA) /* StormMesa 3.0 has EXT_multitexture .. */
	else if (GL_ParseExtensionList(gl_extensions, "GL_EXT_multitexture"))
	#else
	else if (GL_ParseExtensionList(gl_extensions, "GL_ARB_multitexture"))
	#endif
	{
		Con_SafePrintf("ARB Multitexture extensions found\n");

		#if defined (REFGL_AMESA)
		glGetIntegerv_fp(GL_MAX_TEXTURES_EXT, &num_tmus);
		#else
		glGetIntegerv_fp(GL_MAX_TEXTURE_UNITS_ARB, &num_tmus);
		#endif
		if (num_tmus < 2)
		{
			Con_SafePrintf("ignoring multitexture (%i TMUs)\n", (int) num_tmus);
			return;
		}

		glMultiTexCoord2fARB_fp = (glMultiTexCoord2fARB_f) AMIGAGL_GetProcAddress("glMultiTexCoord2fARB");
		glActiveTextureARB_fp = (glActiveTextureARB_f) AMIGAGL_GetProcAddress("glActiveTextureARB");
		if (glMultiTexCoord2fARB_fp == NULL || glActiveTextureARB_fp == NULL)
		{
			Con_SafePrintf ("Couldn't link to multitexture functions\n");
			return;
		}

		have_mtex = true;
		if (!gl_multitexture.integer)
		{
			Con_SafePrintf("ignoring multitexture (cvar disabled)\n");
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
		GLfloat test1 = 0, test2 = 0;
		GLuint tex;
		glGetTexParameterfv_f glGetTexParameterfv_fp;

		glGetTexParameterfv_fp = (glGetTexParameterfv_f) AMIGAGL_GetProcAddress("glGetTexParameterfv");
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
 * detecting the actual capability.
 */
	gl_tex_NPOT = have_NPOT = false;
	if (GL_ParseExtensionList(gl_extensions, "GL_ARB_texture_non_power_of_two"))
	{
		have_NPOT = true;
		Con_SafePrintf("Found ARB_texture_non_power_of_two\n");
		if (!gl_texture_NPOT.integer) {
			Con_SafePrintf("ignoring texture_NPOT (cvar disabled)\n");
		}
		else {
			gl_tex_NPOT = true;
		}
	}
	else
	{
		Con_SafePrintf("GL_ARB_texture_non_power_of_two not found\n");
	}
}

static void CheckStencilBuffer (void)
{
	have_stencil = !!vid_attribs.stencil;
}

static void GL_ResetFunctions (void)
{
#define GL_FUNCTION_OPT(ret, func, params) \
	func##_fp = NULL;
#include "gl_func.h"

	have_stencil = false;
	gl_mtexable = false;
	have_mtex = false;
	have8bit = false;
	is8bit = false;
	have_NPOT = false;
	gl_tex_NPOT = false;
}

/*
===============
GL_Init
===============
*/
static void GL_Init (void)
{
	// collect the visual attributes
	memset (&vid_attribs, 0, sizeof(attributes_t));
	glGetIntegerv_fp(GL_RED_BITS, &vid_attribs.red);
	glGetIntegerv_fp(GL_GREEN_BITS, &vid_attribs.green);
	glGetIntegerv_fp(GL_BLUE_BITS, &vid_attribs.blue);
	glGetIntegerv_fp(GL_ALPHA_BITS, &vid_attribs.alpha);
	glGetIntegerv_fp(GL_DEPTH_BITS, &vid_attribs.depth);
	glGetIntegerv_fp(GL_STENCIL_BITS, &vid_attribs.stencil);
	Con_SafePrintf ("R:%d G:%d B:%d A:%d, Z:%d, S:%d\n",
			vid_attribs.red, vid_attribs.green, vid_attribs.blue, vid_attribs.alpha,
			vid_attribs.depth, vid_attribs.stencil);

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
	#if defined(__AMIGA__) && defined(REFGL_MINIGL)
	#error port to use mglSetZOffset() instead of glPolygonOffset()
	#else
	glPolygonOffset_fp(0.05f, 25.0f);
	#endif
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

#if defined(__AMIGA__) && defined(REFGL_MINIGL)
	glHint_fp (GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
#endif
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
#if defined(__AMIGA__) && defined(REFGL_MINIGL)
	mglLockDisplay();
#endif
}


void GL_EndRendering (void)
{
	if (!scr_skipupdate)
#ifdef __AROS__
		AROSMesaSwapBuffers(context);
#elif defined __MORPHOS__
		glASwapBuffers();
		//GLASwapBuffers(__tglContext);
#elif defined(__AMIGA__) && defined(REFGL_MINIGL)
		mglSwitchDisplay(); //performs unlock
#elif defined(__AMIGA__) && defined(REFGL_AMESA)
		AmigaMesaSwapBuffers(context);
#endif

// handle the mouse state when windowed if that's changed
	if (_enable_mouse.integer != enable_mouse /*&& modestate == MS_WINDOWED*/)
	{
		if (_enable_mouse.integer)
			IN_ActivateMouse ();
		else	IN_DeactivateMouse ();

		enable_mouse = _enable_mouse.integer;
	}

//	if (fullsbardraw)
//		Sbar_Changed();
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
	/*VID_ShutdownGamma();*/
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
	//IN_ShowMouse ();

	if (!VID_SetMode (newmode))
		Sys_Error ("Couldn't set video mode");

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

	IN_ReInit ();
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
	if (vid_mode.integer < 0 || vid_mode.integer >= *nummodes)
	{
		Con_Printf ("Bad video mode %d\n", vid_mode.integer);
		Cvar_SetValueQuick (&vid_mode, vid_modenum);
		return;
	}

	Con_Printf ("Re-initializing video:\n");
	VID_ChangeVideoMode (vid_mode.integer);
}

static int sort_modes (const void *arg1, const void *arg2)
{
	const vmode_t *a1, *a2;
	a1 = (const vmode_t *) arg1;
	a2 = (const vmode_t *) arg2;

	if (a1->width == a2->width)
		return a1->height - a2->height;	// lowres-to-highres
	//	return a2->height - a1->height;	// highres-to-lowres
	else
		return a1->width - a2->width;	// lowres-to-highres
	//	return a2->width - a1->width;	// highres-to-lowres
}

static void VID_PrepareModes (void)
{
	int	i;
	ULONG id;
	APTR handle;
	struct DimensionInfo diminfo;

	num_fmodes = 0;
	num_wmodes = 0;

	// Add the standart 4:3 modes to the windowed modes list
	// In an unlikely case that we receive no fullscreen modes,
	// this will be our modes list (kind of...)
	for (i = 0; i < (int)MAX_STDMODES; i++)
	{
		wmodelist[num_wmodes].width = std_modes[i].width;
		wmodelist[num_wmodes].height = std_modes[i].height;
		/*wmodelist[num_wmodes].halfscreen = 0;*/
		wmodelist[num_wmodes].fullscreen = 0;
		wmodelist[num_wmodes].bpp = 16;
		q_snprintf (wmodelist[num_wmodes].modedesc, MAX_DESC,
				"%d x %d", std_modes[i].width, std_modes[i].height);
		num_wmodes++;
	}

	// fullscreen modes
	id = INVALID_ID;

	while((id = NextDisplayInfo(id)) != INVALID_ID)
	{
		handle = FindDisplayInfo(id);

		if (handle)
		{
			if (!GetDisplayInfoData(handle, &diminfo, sizeof(diminfo), DTAG_DIMS, 0))
				continue;

#ifdef __AROS__
			if (diminfo.MaxDepth != 24)
#else
			if (diminfo.MaxDepth != 16)
#endif
				continue;

			fmodelist[num_fmodes].width = diminfo.Nominal.MaxX + 1;
			fmodelist[num_fmodes].height = diminfo.Nominal.MaxY + 1;
			fmodelist[num_fmodes].fullscreen = 1;
			fmodelist[num_fmodes].bpp = 16; // diminfo.MaxDepth
			q_snprintf (fmodelist[num_fmodes].modedesc, MAX_DESC, "%d x %d", (fmodelist[num_fmodes].width), (fmodelist[num_fmodes].height));
			//Con_SafePrintf ("fmodelist[%d].modedesc = %s maxdepth %d\n", num_fmodes, fmodelist[num_fmodes].modedesc, diminfo.MaxDepth);

			if (++num_fmodes == MAX_MODE_LIST)
				break;
		}
	}

	if (num_fmodes > 1)
		qsort(fmodelist, num_fmodes, sizeof(vmode_t), sort_modes);

	// disaster scenario #1: no fullscreen modes. bind to the
	// windowed modes list. limit it to 640x480 max. because
	// we don't know the desktop dimensions
	if (num_fmodes == 0)
	{
		Con_SafePrintf ("No fullscreen video modes available\n");
		num_wmodes = RES_640X480 + 1;
		modelist = wmodelist;
		nummodes = &num_wmodes;
		vid_default = RES_640X480;
		Cvar_SetValueQuick (&vid_config_glx, modelist[vid_default].width);
		Cvar_SetValueQuick (&vid_config_gly, modelist[vid_default].height);
		return;
	}

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
	modelist = fmodelist;

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
		Con_SafePrintf ("WARNING: 640x480 not found in fullscreen modes\n"
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

	Cvar_SetValueQuick (&vid_config_glx, modelist[vid_default].width);
	Cvar_SetValueQuick (&vid_config_gly, modelist[vid_default].height);
}

static void VID_ListModes_f (void)
{
	int	i;

	Con_Printf ("Maximum allowed mode: %d x %d\n", vid_maxwidth, vid_maxheight);
	Con_Printf ("Windowed modes enabled:\n");
	for (i = 0; i < num_wmodes; i++)
		Con_Printf ("%2d:  %d x %d\n", i, wmodelist[i].width, wmodelist[i].height);
	Con_Printf ("Fullscreen modes enumerated:");
	if (num_fmodes)
	{
		Con_Printf ("\n");
		for (i = 0; i < num_fmodes; i++)
			Con_Printf ("%2d:  %d x %d\n", i, fmodelist[i].width, fmodelist[i].height);
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

/*
===================
VID_Init
===================
*/
void	VID_Init (unsigned char *palette)
{
	int	i, temp, width, height;
	const char	*read_vars[] = {
				"vid_config_fscr",
				"vid_config_gl8bit",
				"vid_config_glx",
				"vid_config_gly",
				"vid_config_consize",
				"gl_texture_NPOT",
				"gl_multitexture",
				"gl_lightmapfmt" };
#define num_readvars	( sizeof(read_vars)/sizeof(read_vars[0]) )

	Cvar_RegisterVariable (&vid_config_gl8bit);
	Cvar_RegisterVariable (&vid_config_fscr);
	Cvar_RegisterVariable (&vid_config_swy);
	Cvar_RegisterVariable (&vid_config_swx);
	Cvar_RegisterVariable (&vid_config_gly);
	Cvar_RegisterVariable (&vid_config_glx);
	Cvar_RegisterVariable (&vid_config_consize);
	Cvar_RegisterVariable (&vid_mode);
	Cvar_RegisterVariable (&_enable_mouse);
	Cvar_RegisterVariable (&gl_texture_NPOT);
	Cvar_RegisterVariable (&gl_lightmapfmt);
	Cvar_RegisterVariable (&gl_multitexture);

	Cmd_AddCommand ("vid_listmodes", VID_ListModes_f);
	Cmd_AddCommand ("vid_nummodes", VID_NumModes_f);
	Cmd_AddCommand ("vid_restart", VID_Restart_f);

	VID_InitPalette (palette);

	vid.numpages = 2;

#if defined(REFGL_MINIGL)
	mini_CurrentContext = NULL;/* should I do this? */
	if (MGLInit() == GL_FALSE) {
		Sys_Error ("Couldn't initialize MiniGL");
	}
	mglChoosePixelDepth (16);/* set pixel depth to 16 */
#endif

	// prepare the modelists, find the actual modenum for vid_default
	VID_PrepareModes();

	// set vid_mode to our safe default first
	Cvar_SetValueQuick (&vid_mode, vid_default);

	// perform an early read of config.cfg
	CFG_ReadCvars (read_vars, num_readvars);

	// windowed mode is default
	// see if the user wants fullscreen
	if (COM_CheckParm("-fullscreen") || COM_CheckParm("-f"))
	{
		Cvar_SetQuick (&vid_config_fscr, "1");
	}
	else if (COM_CheckParm("-window") || COM_CheckParm("-w"))
	{
		Cvar_SetQuick (&vid_config_fscr, "0");
	}

	if (vid_config_fscr.integer && !num_fmodes) // FIXME: see below, as well
		Sys_Error ("No fullscreen modes available at this color depth");

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
	i = COM_CheckParm("-bpp");
	if (i && i < com_argc-1)
	{
		bpp = atoi(com_argv[i+1]);
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
		Cvar_SetValueQuick (&vid_mode, i);
	}
	else if ( (width <= vid_maxwidth && width >= MIN_WIDTH &&
		   height <= vid_maxheight && height >= MIN_HEIGHT) ||
		  COM_CheckParm("-force") )
	{
		modelist[*nummodes].width = width;
		modelist[*nummodes].height = height;
		/*modelist[*nummodes].halfscreen = 0;*/
		modelist[*nummodes].fullscreen = 1;
		modelist[*nummodes].bpp = 16;
		q_snprintf (modelist[*nummodes].modedesc, MAX_DESC, "%d x %d (user mode)", width, height);
		Cvar_SetValueQuick (&vid_mode, *nummodes);
		(*nummodes)++;
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
	if (!VID_SetMode (vid_mode.integer))
		Sys_Error ("Couldn't set video mode");
	ClearAllStates ();

	GL_SetupLightmapFmt();
	GL_Init ();
	VID_InitGamma();
	VID_Init8bitPalette();

	// lock the early-read cvars until Host_Init is finished
	for (i = 0; i < (int)num_readvars; i++)
		Cvar_LockVar (read_vars[i]);

	vid_initialized = true;
	scr_disabled_for_loading = temp;
	vid.recalc_refdef = 1;

	Con_SafePrintf ("Video initialized.\n");

	vid_menudrawfn = VID_MenuDraw;
	vid_menukeyfn = VID_MenuKey;

//	if (COM_CheckParm("-fullsbar"))
//		fullsbardraw = true;
}


void	VID_Shutdown (void)
{
	VID_KillContext();
#ifdef REFGL_MINIGL
	MGLTerm();
#endif
}


static void VID_KillContext (void)
{
#ifdef __AROS__
	if (context)
	{
		AROSMesaDestroyContext(context);
		context = NULL;
	}
#elif defined __MORPHOS__
	if (contextinit)
	{
		if (screen && !(TinyGLBase->lib_Version == 0 && TinyGLBase->lib_Revision < 4))
			glADestroyContextScreen();
		else
			glADestroyContextWindowed();
		contextinit = false;
	}
	if (__tglContext)
	{
		GLClose(__tglContext);
		__tglContext = NULL;
	}
#elif defined(__AMIGA__) && defined(REFGL_MINIGL)
	if (mini_CurrentContext)
	{
		mglDeleteContext();
		window = NULL;
		screen = NULL;
		mini_CurrentContext = NULL;
	}
#elif defined(__AMIGA__) && defined(REFGL_AMESA)
	if (context)
	{
		AmigaMesaDestroyContext(context);
		context = NULL;
	}
#endif

#ifndef REFGL_MINIGL
	if (window)
	{
		CloseWindow(window);
		window = NULL;
	}

	if (screen)
	{
		CloseScreen(screen);
		screen = NULL;
	}
#endif
}


/*
================
VID_ToggleFullscreen
Handles switching between fullscreen/windowed modes
and brings the mouse to a proper state afterwards
================
*/
extern qboolean menu_disabled_mouse;
void VID_ToggleFullscreen (void)
{
	// implement this
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
static qboolean	want_fstoggle, need_apply;
static qboolean	vid_menu_firsttime = true;

enum {
	VID_FULLSCREEN,	// make sure the fullscreen entry (0)
	VID_RESOLUTION,	// is lower than resolution entry (1)
	VID_MULTISAMPLE,
	VID_MULTITEXTURE,
	VID_NPOT,
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
		vid_menu_fs = (modestate != MS_WINDOWED);
		vid_cursor = (num_fmodes) ? 0 : VID_RESOLUTION;
		vid_menu_firsttime = false;
	}

	want_fstoggle = ( ((modestate == MS_WINDOWED) && vid_menu_fs) || ((modestate != MS_WINDOWED) && !vid_menu_fs) );

	need_apply = (vid_menunum != vid_modenum) || want_fstoggle ||
			(have_mtex && (gl_mtexable != !!gl_multitexture.integer)) ||
			(have_NPOT && (gl_tex_NPOT != !!gl_texture_NPOT.integer)) ||
			(have8bit && (is8bit != !!vid_config_gl8bit.integer));

	M_Print (76, 92 + 8*VID_FULLSCREEN, "Fullscreen: ");
	M_DrawYesNo (76+12*8, 92 + 8*VID_FULLSCREEN, vid_menu_fs, !want_fstoggle);

	M_Print (76, 92 + 8*VID_RESOLUTION, "Resolution: ");
	if (vid_menunum == vid_modenum)
		M_PrintWhite (76+12*8, 92 + 8*VID_RESOLUTION, modelist[vid_menunum].modedesc);
	else
		M_Print (76+12*8, 92 + 8*VID_RESOLUTION, modelist[vid_menunum].modedesc);

	M_Print (76, 92 + 8*VID_MULTISAMPLE, "Antialiasing  :");
	M_PrintWhite (76+16*8, 92 + 8*VID_MULTISAMPLE, "Not found");

	M_Print (76, 92 + 8*VID_MULTITEXTURE, "Multitexturing:");
	if (have_mtex)
		M_DrawYesNo (76+16*8, 92 + 8*VID_MULTITEXTURE, gl_multitexture.integer, (gl_mtexable == !!gl_multitexture.integer));
	else
		M_PrintWhite (76+16*8, 92 + 8*VID_MULTITEXTURE, "Not found");

	M_Print (76, 92 + 8*VID_NPOT, "NPOT textures :");
	if (have_NPOT)
		M_DrawYesNo (76+16*8, 92 + 8*VID_NPOT, gl_texture_NPOT.integer, (gl_tex_NPOT == !!gl_texture_NPOT.integer));
	else
		M_PrintWhite (76+16*8, 92 + 8*VID_NPOT, "Not found");

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
		vid_cursor = (num_fmodes) ? 0 : VID_RESOLUTION;
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
			vid_cursor = (num_fmodes) ? 0 : VID_RESOLUTION;
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
				vid_cursor = (num_fmodes) ? 0 : VID_RESOLUTION;
			}
		}
		break;

	case K_ENTER:
		switch (vid_cursor)
		{
		case VID_RESET:
			vid_menu_fs = (modestate != MS_WINDOWED);
			vid_menunum = vid_modenum;
			Cvar_SetValueQuick (&vid_config_gl8bit, is8bit);
			vid_cursor = (num_fmodes) ? 0 : VID_RESOLUTION;
			break;
		case VID_APPLY:
			if (need_apply)
			{
				Cvar_SetValueQuick(&vid_mode, vid_menunum);
				Cvar_SetValueQuick(&vid_config_fscr, vid_menu_fs);
				VID_Restart_f();
			}
			vid_cursor = (num_fmodes) ? 0 : VID_RESOLUTION;
			break;
		}
		return;

	case K_LEFTARROW:
		switch (vid_cursor)
		{
		case VID_FULLSCREEN:
			vid_menu_fs = !vid_menu_fs;
			if (fs_toggle_works)
				VID_ToggleFullscreen();
			break;
		case VID_RESOLUTION:
			S_LocalSound ("raven/menu1.wav");
			vid_menunum--;
			if (vid_menunum < 0)
				vid_menunum = 0;
			break;
		case VID_MULTISAMPLE:
			break;
		case VID_MULTITEXTURE:
			if (have_mtex)
				Cvar_SetQuick (&gl_multitexture, gl_multitexture.integer ? "0" : "1");
			break;
		case VID_NPOT:
			if (have_NPOT)
				Cvar_SetQuick (&gl_texture_NPOT, gl_texture_NPOT.integer ? "0" : "1");
			break;
		case VID_PALTEX:
			if (have8bit)
				Cvar_SetQuick (&vid_config_gl8bit, vid_config_gl8bit.integer ? "0" : "1");
			break;
		}
		return;

	case K_RIGHTARROW:
		switch (vid_cursor)
		{
		case VID_FULLSCREEN:
			vid_menu_fs = !vid_menu_fs;
			if (fs_toggle_works)
				VID_ToggleFullscreen();
			break;
		case VID_RESOLUTION:
			S_LocalSound ("raven/menu1.wav");
			vid_menunum++;
			if (vid_menunum >= *nummodes)
				vid_menunum = *nummodes - 1;
			break;
		case VID_MULTISAMPLE:
			break;
		case VID_MULTITEXTURE:
			if (have_mtex)
				Cvar_SetQuick (&gl_multitexture, gl_multitexture.integer ? "0" : "1");
			break;
		case VID_NPOT:
			if (have_NPOT)
				Cvar_SetQuick (&gl_texture_NPOT, gl_texture_NPOT.integer ? "0" : "1");
			break;
		case VID_PALTEX:
			if (have8bit)
				Cvar_SetQuick (&vid_config_gl8bit, vid_config_gl8bit.integer ? "0" : "1");
			break;
		}
		return;

	default:
		break;
	}
}

