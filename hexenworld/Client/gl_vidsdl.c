/*
   gl_vidsdl.c -- SDL GL vid component
   Select window size and mode and init SDL in GL mode.

   $Id:


	Changed 7/11/04 by S.A.
	- Fixed fullscreen opengl mode, window sizes
	- Options are now: -fullscreen, -height, -width, -bpp

	Changed 27/12/04
	- Fullscreen modes are normally 3-5, but we only
	  use the traditional modes 0 and 3.
	- Mode 3 has been changed to "1" to allow it to be
	  represented as a boolean (and a menu selection)

	Changed 08/02/05 by O.S
	- Removed cvar _vid_default_mode_win
	- Removed all mode descriptions
	- Removed all nummodes and VID_NumModes stuff
	- Removed all VID_GetXXX and VID_DescXXX stuff

	- cvar_t vid_mode is either:
		MODE_WINDOWED		0
		MODE_FULLSCREEN_DEFAULT	1

	- The "-mode" option has been removed
	- There was no on-the-fly video mode switching
	  before, and there won't be any in the future
*/

#include "quakedef.h"
#include "quakeinc.h"

#include "SDL.h"
#include <dlfcn.h>

#define MAX_MODE_LIST	30
#define VID_ROW_SIZE	3
#define WARP_WIDTH		320
#define WARP_HEIGHT		200
#define MAXWIDTH		10000
#define MAXHEIGHT		10000
#define BASEWIDTH		320
#define BASEHEIGHT		200

#define MODE_WINDOWED		0
#define NO_MODE			(MODE_WINDOWED - 1)
#define MODE_FULLSCREEN_DEFAULT	(MODE_WINDOWED + 1)

typedef struct {
	modestate_t	type;
	int			width;
	int			height;
	int			modenum;
	int			dib;
	int			fullscreen;
	int			bpp;
	int			halfscreen;
	char		modedesc[17];
} vmode_t;

SDL_Surface	*screen;

viddef_t	vid;		// global video state
modestate_t	modestate = MS_UNINIT;
int		WRHeight, WRWidth;
int		vid_default = MODE_WINDOWED;	// windowed mode is default
cvar_t		vid_mode = {"vid_mode","0", false};
cvar_t		_vid_default_mode = {"_vid_default_mode","0", true};
static vmode_t	modelist[MAX_MODE_LIST];
static qboolean	vid_initialized = false;

byte		globalcolormap[VID_GRADES*256];

cvar_t		_windowed_mouse = {"_windowed_mouse","1", true};
static int	windowed_mouse;
qboolean	in_mode_set = false;	// do we need this?..
extern qboolean	grab;

glfunc_t	glfunc;
const char	*gl_vendor;
const char	*gl_renderer;
const char	*gl_version;
const char	*gl_extensions;
char		*gl_library;
extern qboolean	is_3dfx;
extern cvar_t	gl_multitex;
qboolean	gl_mtexable = false;
float		gldepthmin, gldepthmax;

#ifndef GL_SHARED_TEXTURE_PALETTE_EXT
#define GL_SHARED_TEXTURE_PALETTE_EXT 0x81FB
#endif
typedef void	(*lp3DFXFUNC) (int, int, int, int, int, const void*);
lp3DFXFUNC	MyglColorTableEXT;
qboolean	is8bit = false;

float		RTint[256],GTint[256],BTint[256];
unsigned char	d_15to8table[65536];
unsigned short	d_8to16table[256];
unsigned	d_8to24table[256];
unsigned	d_8to24table3dfx[256];
unsigned	d_8to24TranslucentTable[256];

cvar_t		gl_ztrick = {"gl_ztrick","0",true};

qboolean	scr_skipupdate;
static		qboolean fullsbardraw = false;

void VID_MenuDraw (void);
void VID_MenuKey (int key);

void ClearAllStates (void);
void GL_Init (void);
void GL_Init_Functions(void);
void VID_SetGamma(float value);
void VID_SetGamma_f(void);


//====================================

// direct draw software compatability stuff
void VID_HandlePause (qboolean pause)
{
}

void VID_ForceLockState (int lk)
{
}

void VID_LockBuffer (void)
{
}

void VID_UnlockBuffer (void)
{
}

int VID_ForceUnlockedAndReturnState (void)
{
	return 0;
}

void D_BeginDirectRect (int x, int y, byte *pbitmap, int width, int height)
{
}

void D_EndDirectRect (int x, int y, int width, int height)
{
}


/* Init SDL */

int VID_SetMode (int modenum)
{
	Uint32	flags;
	int	temp;
	int	sdl_tmp;	// for SDL GL attributes actually set

	// so Con_Printfs don't mess us up by forcing vid and snd updates
	temp = scr_disabled_for_loading;
	scr_disabled_for_loading = true;

	if ((modelist[modenum].type != MS_WINDOWED) && (modelist[modenum].type != MS_FULLSCREEN))
		Sys_Error ("VID_SetMode: Bad mode type in modelist");

	// handle both fullscreen and windowed modes -S.A

/*	SDL doco recons you need this. S.A.
	SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 5 );
	SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 5 );
	SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 5 );
	SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16 );
	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 0 );
*/

	if (modenum == MODE_FULLSCREEN_DEFAULT )
		flags = (SDL_OPENGL|SDL_FULLSCREEN);
	else 
		flags = (SDL_OPENGL);
		// flags = (SDL_OPENGL|SDL_NOFRAME);

	if (modenum==0)
		modestate = MS_WINDOWED;
	else	
		modestate = MS_FULLSCREEN;

	WRHeight = vid.height = vid.conheight = modelist[modenum].height; // BASEHEIGHT;
	WRWidth = vid.width = vid.conwidth =   modelist[modenum].width; //  BASEWIDTH ;

	vid.numpages = 2;
	
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
		Sys_Error ("Couldn't init video: %s", SDL_GetError());

	if (SDL_GL_LoadLibrary(gl_library) < 0)
		Sys_Error("VID: Couldn't load GL library: %s", SDL_GetError());

	//SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, modelist[modenum].bpp);
	Con_SafePrintf ("Requesting Mode: %dx%dx%d\n", vid.width, vid.height, modelist[modenum].bpp);
	if (!(screen = SDL_SetVideoMode (vid.width,vid.height,modelist[modenum].bpp, flags)))
		Sys_Error ("Couldn't set video mode: %s", SDL_GetError());

	SDL_GL_GetAttribute(SDL_GL_BUFFER_SIZE, &sdl_tmp);
	Con_SafePrintf ("Video Mode Set : %dx%dx%d\n", vid.width, vid.height, sdl_tmp);

#if defined(H2W)
	SDL_WM_SetCaption("HexenWorld", "HexenWorld");
#elif defined(H2MP)
	SDL_WM_SetCaption("Portal of Praevus", "PRAEVUS");
#else
	SDL_WM_SetCaption("Hexen II", "HEXEN2");
#endif


	if (modelist[modenum].type == MS_WINDOWED)
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
	}
	else
	{
		IN_ActivateMouse ();
		IN_HideMouse ();
	}

	scr_disabled_for_loading = temp;

	Cvar_SetValue ("vid_mode", modenum);

	return true;
}


//====================================

int		texture_extension_number = 1;

void CheckMultiTextureExtensions(void) 
{
	GLint texture_units = 1;

	Cvar_RegisterVariable (&gl_multitex);

	if (strstr(gl_extensions, "GL_ARB_multitexture ") && !COM_CheckParm("-nomtex"))
	{
		Con_Printf ("Multitexture extensions found.\n");

		glfunc.glActiveTextureARB_fp = (glActiveTextureARB_f) SDL_GL_GetProcAddress("glActiveTextureARB");
		glfunc.glMultiTexCoord2fARB_fp = (glMultiTexCoord2fARB_f) SDL_GL_GetProcAddress ("glMultiTexCoord2fARB");
		glfunc.glGetIntegerv_fp(GL_MAX_TEXTURE_UNITS_ARB,&texture_units);

		if ((glfunc.glActiveTextureARB_fp == 0)   ||
		    (glfunc.glMultiTexCoord2fARB_fp == 0) ||
		    (texture_units < 2)) {
			Con_Printf ("Multitexture extensions disabled.\n");
			glfunc.glActiveTextureARB_fp = NULL;
			glfunc.glMultiTexCoord2fARB_fp = NULL;
			gl_mtexable = false;
			return;
		}

		Con_Printf ("Multitexture extensions enabled.\n");
		gl_mtexable = true;
	}
}

/*
===============
GL_Init
===============
*/
void GL_Init (void)
{
	GL_Init_Functions();
	gl_vendor = (const char *)glfunc.glGetString_fp (GL_VENDOR);
	Con_Printf ("GL_VENDOR: %s\n", gl_vendor);
	gl_renderer = (const char *)glfunc.glGetString_fp (GL_RENDERER);
	Con_Printf ("GL_RENDERER: %s\n", gl_renderer);

	gl_version = (const char *)glfunc.glGetString_fp (GL_VERSION);
	Con_Printf ("GL_VERSION: %s\n", gl_version);
	gl_extensions = (const char *)glfunc.glGetString_fp (GL_EXTENSIONS);
//	Con_Printf ("GL_EXTENSIONS: %s\n", gl_extensions);

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

	glfunc.glClearColor_fp (1,0,0,0);
	glfunc.glCullFace_fp(GL_FRONT);
	glfunc.glEnable_fp(GL_TEXTURE_2D);

	glfunc.glEnable_fp(GL_ALPHA_TEST);
	glfunc.glAlphaFunc_fp(GL_GREATER, 0.666);

	glfunc.glPolygonMode_fp (GL_FRONT_AND_BACK, GL_FILL);
	glfunc.glShadeModel_fp (GL_FLAT);

	glfunc.glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glfunc.glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	// previously GL_CLAMP was GL_REPEAT S.A
	glfunc.glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glfunc.glTexParameterf_fp(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	glfunc.glBlendFunc_fp (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

//	glfunc.glTexEnvf_fp(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glfunc.glTexEnvf_fp(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
}


void GL_Init_Functions(void)
{
  glfunc.glBegin_fp = (glBegin_f) SDL_GL_GetProcAddress("glBegin");
  if (glfunc.glBegin_fp == 0) {Sys_Error("glBegin not found in GL library");}
  glfunc.glEnd_fp = (glEnd_f) SDL_GL_GetProcAddress("glEnd");
  if (glfunc.glEnd_fp == 0) {Sys_Error("glEnd not found in GL library");}
  glfunc.glEnable_fp = (glEnable_f) SDL_GL_GetProcAddress("glEnable");
  if (glfunc.glEnable_fp == 0) {Sys_Error("glEnable not found in GL library");}
  glfunc.glDisable_fp = (glDisable_f) SDL_GL_GetProcAddress("glDisable");
  if (glfunc.glDisable_fp == 0) {Sys_Error("glDisable not found in GL library");}
  glfunc.glIsEnabled_fp = (glIsEnabled_f) SDL_GL_GetProcAddress("glIsEnabled");
  if (glfunc.glIsEnabled_fp == 0) {Sys_Error("glIsEnabled not found in GL library");}
  glfunc.glFinish_fp = (glFinish_f) SDL_GL_GetProcAddress("glFinish");
  if (glfunc.glFinish_fp == 0) {Sys_Error("glFinish not found in GL library");}
  glfunc.glClear_fp = (glClear_f) SDL_GL_GetProcAddress("glClear");
  if (glfunc.glClear_fp == 0) {Sys_Error("glClear not found in GL library");}

  glfunc.glOrtho_fp = (glOrtho_f) SDL_GL_GetProcAddress("glOrtho");
  if (glfunc.glOrtho_fp == 0) {Sys_Error("glOrtho not found in GL library");}
  glfunc.glFrustum_fp = (glFrustum_f) SDL_GL_GetProcAddress("glFrustum");
  if (glfunc.glFrustum_fp == 0) {Sys_Error("glFrustum not found in GL library");}
  glfunc.glViewport_fp = (glViewport_f) SDL_GL_GetProcAddress("glViewport");
  if (glfunc.glViewport_fp == 0) {Sys_Error("glViewport not found in GL library");}
  glfunc.glPushMatrix_fp = (glPushMatrix_f) SDL_GL_GetProcAddress("glPushMatrix");
  if (glfunc.glPushMatrix_fp == 0) {Sys_Error("glPushMatrix not found in GL library");}
  glfunc.glPopMatrix_fp = (glPopMatrix_f) SDL_GL_GetProcAddress("glPopMatrix");
  if (glfunc.glPopMatrix_fp == 0) {Sys_Error("glPopMatrix not found in GL library");}
  glfunc.glLoadIdentity_fp = (glLoadIdentity_f) SDL_GL_GetProcAddress("glLoadIdentity");
  if (glfunc.glLoadIdentity_fp == 0) {Sys_Error("glLoadIdentity not found in GL library");}
  glfunc.glMatrixMode_fp = (glMatrixMode_f) SDL_GL_GetProcAddress("glMatrixMode");
  if (glfunc.glMatrixMode_fp == 0) {Sys_Error("glMatrixMode not found in GL library");}
  glfunc.glLoadMatrixf_fp = (glLoadMatrixf_f) SDL_GL_GetProcAddress("glLoadMatrixf");
  if (glfunc.glLoadMatrixf_fp == 0) {Sys_Error("glLoadMatrixf not found in GL library");}

  glfunc.glVertex2f_fp = (glVertex2f_f) SDL_GL_GetProcAddress("glVertex2f");
  if (glfunc.glVertex2f_fp == 0) {Sys_Error("glVertex2f not found in GL library");}
  glfunc.glVertex3f_fp = (glVertex3f_f) SDL_GL_GetProcAddress("glVertex3f");
  if (glfunc.glVertex3f_fp == 0) {Sys_Error("glVertex3f not found in GL library");}
  glfunc.glVertex3fv_fp = (glVertex3fv_f) SDL_GL_GetProcAddress("glVertex3fv");
  if (glfunc.glVertex3fv_fp == 0) {Sys_Error("glVertex3fv not found in GL library");}
  glfunc.glTexCoord2f_fp = (glTexCoord2f_f) SDL_GL_GetProcAddress("glTexCoord2f");
  if (glfunc.glTexCoord2f_fp == 0) {Sys_Error("glTexCoord2f not found in GL library");}
  glfunc.glTexCoord3f_fp = (glTexCoord3f_f) SDL_GL_GetProcAddress("glTexCoord3f");
  if (glfunc.glTexCoord3f_fp == 0) {Sys_Error("glTexCoord3f not found in GL library");}
  glfunc.glColor4f_fp = (glColor4f_f) SDL_GL_GetProcAddress("glColor4f");
  if (glfunc.glColor4f_fp == 0) {Sys_Error("glColor4f not found in GL library");}
  glfunc.glColor4fv_fp = (glColor4fv_f) SDL_GL_GetProcAddress("glColor4fv");
  if (glfunc.glColor4fv_fp == 0) {Sys_Error("glColor4fv not found in GL library");}
  glfunc.glColor4ub_fp = (glColor4ub_f) SDL_GL_GetProcAddress("glColor4ub");
  if (glfunc.glColor4ub_fp == 0) {Sys_Error("glColor4ub not found in GL library");}
  glfunc.glColor4ubv_fp = (glColor4ubv_f) SDL_GL_GetProcAddress("glColor4ubv");
  if (glfunc.glColor4ubv_fp == 0) {Sys_Error("glColor4ubv not found in GL library");}
  glfunc.glColor3f_fp = (glColor3f_f) SDL_GL_GetProcAddress("glColor3f");
  if (glfunc.glColor3f_fp == 0) {Sys_Error("glColor3f not found in GL library");}
  glfunc.glColor3ubv_fp = (glColor3ubv_f) SDL_GL_GetProcAddress("glColor3ubv");
  if (glfunc.glColor3ubv_fp == 0) {Sys_Error("glColor3ubv not found in GL library");}
  glfunc.glClearColor_fp = (glClearColor_f) SDL_GL_GetProcAddress("glClearColor");
  if (glfunc.glClearColor_fp == 0) {Sys_Error("glClearColor not found in GL library");}

  glfunc.glRotatef_fp = (glRotatef_f) SDL_GL_GetProcAddress("glRotatef");
  if (glfunc.glRotatef_fp == 0) {Sys_Error("glRotatef not found in GL library");}
  glfunc.glTranslatef_fp = (glTranslatef_f) SDL_GL_GetProcAddress("glTranslatef");
  if (glfunc.glTranslatef_fp == 0) {Sys_Error("glTranslatef not found in GL library");}

  glfunc.glBindTexture_fp = (glBindTexture_f) SDL_GL_GetProcAddress("glBindTexture");
  if (glfunc.glBindTexture_fp == 0) {Sys_Error("glBindTexture not found in GL library");}
  glfunc.glDeleteTextures_fp = (glDeleteTextures_f) SDL_GL_GetProcAddress("glDeleteTextures");
  if (glfunc.glDeleteTextures_fp == 0) {Sys_Error("glDeleteTextures not found in GL library");}
  glfunc.glTexParameterf_fp = (glTexParameterf_f) SDL_GL_GetProcAddress("glTexParameterf");
  if (glfunc.glTexParameterf_fp == 0) {Sys_Error("glTexParameterf not found in GL library");}
  glfunc.glTexEnvf_fp = (glTexEnvf_f) SDL_GL_GetProcAddress("glTexEnvf");
  if (glfunc.glTexEnvf_fp == 0) {Sys_Error("glTexEnvf not found in GL library");}
  glfunc.glScalef_fp = (glScalef_f) SDL_GL_GetProcAddress("glScalef");
  if (glfunc.glScalef_fp == 0) {Sys_Error("glScalef not found in GL library");}
  glfunc.glTexImage2D_fp = (glTexImage2D_f) SDL_GL_GetProcAddress("glTexImage2D");
  if (glfunc.glTexImage2D_fp == 0) {Sys_Error("glTexImage2D not found in GL library");}
  glfunc.glTexSubImage2D_fp = (glTexSubImage2D_f) SDL_GL_GetProcAddress("glTexSubImage2D");
  if (glfunc.glTexSubImage2D_fp == 0) {Sys_Error("glTexSubImage2D not found in GL library");}

  glfunc.glAlphaFunc_fp = (glAlphaFunc_f) SDL_GL_GetProcAddress("glAlphaFunc");
  if (glfunc.glAlphaFunc_fp == 0) {Sys_Error("glAlphaFunc not found in GL library");}
  glfunc.glBlendFunc_fp = (glBlendFunc_f) SDL_GL_GetProcAddress("glBlendFunc");
  if (glfunc.glBlendFunc_fp == 0) {Sys_Error("glBlendFunc not found in GL library");}
  glfunc.glShadeModel_fp = (glShadeModel_f) SDL_GL_GetProcAddress("glShadeModel");
  if (glfunc.glShadeModel_fp == 0) {Sys_Error("glShadeModel not found in GL library");}
  glfunc.glPolygonMode_fp = (glPolygonMode_f) SDL_GL_GetProcAddress("glPolygonMode");
  if (glfunc.glPolygonMode_fp == 0) {Sys_Error("glPolygonMode not found in GL library");}
  glfunc.glDepthMask_fp = (glDepthMask_f) SDL_GL_GetProcAddress("glDepthMask");
  if (glfunc.glDepthMask_fp == 0) {Sys_Error("glDepthMask not found in GL library");}
  glfunc.glDepthRange_fp = (glDepthRange_f) SDL_GL_GetProcAddress("glDepthRange");
  if (glfunc.glDepthRange_fp == 0) {Sys_Error("glDepthRange not found in GL library");}
  glfunc.glDepthFunc_fp = (glDepthFunc_f) SDL_GL_GetProcAddress("glDepthFunc");
  if (glfunc.glDepthFunc_fp == 0) {Sys_Error("glDepthFunc not found in GL library");}

  glfunc.glDrawBuffer_fp = (glDrawBuffer_f) SDL_GL_GetProcAddress("glDrawBuffer");
  if (glfunc.glDrawBuffer_fp == 0) {Sys_Error("glDrawBuffer not found in GL library");}
  glfunc.glReadBuffer_fp = (glDrawBuffer_f) SDL_GL_GetProcAddress("glReadBuffer");
  if (glfunc.glReadBuffer_fp == 0) {Sys_Error("glReadBuffer not found in GL library");}
  glfunc.glReadPixels_fp = (glReadPixels_f) SDL_GL_GetProcAddress("glReadPixels");
  if (glfunc.glReadPixels_fp == 0) {Sys_Error("glReadPixels not found in GL library");}
  glfunc.glHint_fp = (glHint_f) SDL_GL_GetProcAddress("glHint");
  if (glfunc.glHint_fp == 0) {Sys_Error("glHint not found in GL library");}
  glfunc.glCullFace_fp = (glCullFace_f) SDL_GL_GetProcAddress("glCullFace");
  if (glfunc.glCullFace_fp == 0) {Sys_Error("glCullFace not found in GL library");}

  glfunc.glGetIntegerv_fp = (glGetIntegerv_f) SDL_GL_GetProcAddress("glGetIntegerv");
  if (glfunc.glGetIntegerv_fp == 0) {Sys_Error("glGetIntegerv not found in GL library");}

  glfunc.glGetString_fp = (glGetString_f) SDL_GL_GetProcAddress("glGetString");
  if (glfunc.glGetString_fp == 0) {Sys_Error("glGetString not found in GL library");}
  glfunc.glGetFloatv_fp = (glGetFloatv_f) SDL_GL_GetProcAddress("glGetFloatv");
  if (glfunc.glGetFloatv_fp == 0) {Sys_Error("glGetFloatv not found in GL library");}
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

	/* if value==0, just apply current settings.
	   this is usefull at startup */

	VID_ApplyGamma();
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

//	glfunc.glViewport_fp (*x, *y, *width, *height);
}


void GL_EndRendering (void)
{
	if (!scr_skipupdate)
		SDL_GL_SwapBuffers();

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


void VID_SetPalette (unsigned char *palette)
{
	byte	*pal;
	unsigned short r,g,b;
	int     v;
	int     r1,g1,b1;
	int		j,k,l,m;
	unsigned short i, p, c;
	unsigned	*table, *table3dfx;
	FILE	*f;
	char	s[MAX_OSPATH];

//
// 8 8 8 encoding
//
	pal = palette;
	table = d_8to24table;
	table3dfx = d_8to24table3dfx;
	for (i=0 ; i<256 ; i++)
	{
		r = pal[0];
		g = pal[1];
		b = pal[2];
		pal += 3;
		
//		v = (255<<24) + (r<<16) + (g<<8) + (b<<0);
//		v = (255<<0) + (r<<8) + (g<<16) + (b<<24);
		v = (255<<24) + (r<<0) + (g<<8) + (b<<16);
		*table++ = v;
		v = (255<<24) + (r<<16) + (g<<8) + (b<<0);
		*table3dfx++ = v;
	}

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

			RTint[i*16+p] = ((float)r) / ((float)ColorPercent[15-p]) ;
			GTint[i*16+p] = ((float)g) / ((float)ColorPercent[15-p]);
			BTint[i*16+p] = ((float)b) / ((float)ColorPercent[15-p]);
		}
	}

	// JACK: 3D distance calcs - k is last closest, l is the distance.
	// FIXME: Precalculate this and cache to disk.

	COM_FOpenFile("glhexen/15to8.pal", &f, true);
	if (f)
	{
		fread(d_15to8table, 1<<15, 1, f);
		fclose(f);
	} 
	else 
	{
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
#if 0
			r = (i << 11);
			g = (i << 6);
			b = (i << 1);
			r >>= 11;
			g >>= 11;
			b >>= 11;
#endif
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
			{
#ifdef _DEBUG
				sprintf(s, "Done - %d\n", i);
				OutputDebugString(s);
#endif
				m=0;
			}
		}
		sprintf(s, "%s/glhexen", com_userdir);
 		Sys_mkdir (s);
		sprintf(s, "%s/glhexen/15to8.pal", com_userdir);
		if ((f = fopen(s, "wb")))
		{
			fwrite(d_15to8table, 1<<15, 1, f);
			fclose(f);
		}
	}

	d_8to24table[255] &= 0xffffff;	// 255 is transparent
}


void VID_SetDefaultMode (void)
{
	IN_DeactivateMouse ();
}


void	VID_Shutdown (void)
{
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

void VID_Init8bitPalette() 
{
	// Check for 8bit Extensions and initialize them.
	int i;
	char thePalette[256*3];
	char *oldPalette, *newPalette;

	MyglColorTableEXT = (void *)SDL_GL_GetProcAddress("glColorTableEXT");

	// enable paletted textures only when -paltex cmdline arg is used
	if (MyglColorTableEXT && COM_CheckParm("-paltex") &&
	    strstr(gl_extensions, "GL_EXT_shared_texture_palette")) {

		Con_SafePrintf("8-bit GL extensions enabled.\n");
		glfunc.glEnable_fp( GL_SHARED_TEXTURE_PALETTE_EXT );
		oldPalette = (char *) d_8to24table; //d_8to24table3dfx;
		newPalette = thePalette;

		for (i=0;i<256;i++) {
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
	int	width, height, bpp;
	char	gldir[MAX_OSPATH];

	Cvar_RegisterVariable (&vid_mode);
	Cvar_RegisterVariable (&_vid_default_mode);
	Cvar_RegisterVariable (&_windowed_mouse);
	Cvar_RegisterVariable (&gl_ztrick);

	Cmd_AddCommand ("vid_setgamma", VID_SetGamma_f);

	gl_library=NULL;
	if (COM_CheckParm("--gllibrary")) {
		gl_library = com_argv[COM_CheckParm("--gllibrary")+1];
		Con_Printf("Using GL library: %s\n",gl_library);
	}
	else if (COM_CheckParm("-g")) {
		gl_library = com_argv[COM_CheckParm("-g")+1];
		Con_Printf("Using GL library: %s\n",gl_library);
	}

	modelist[0].type = MS_WINDOWED;
	modelist[0].width = 640;
	modelist[0].height = 480;
	strcpy (modelist[0].modedesc, "640x480");
	modelist[0].modenum = MODE_WINDOWED;
	modelist[0].dib = 1;
	modelist[0].fullscreen = 0;
	modelist[0].halfscreen = 0;
	modelist[0].bpp = 16;

	// mode[1] has been hacked to be like the (missing) mode[3] S.A.

	modelist[1].type = MS_FULLSCREEN;
	modelist[1].width = 640;
	modelist[1].height = 480;
	strcpy (modelist[1].modedesc, "640x480");
	modelist[1].modenum = MODE_FULLSCREEN_DEFAULT;
	modelist[1].dib = 1;
	modelist[1].fullscreen = 1;
	modelist[1].halfscreen = 0;
	modelist[1].bpp = 16;

	// modelist[2-5] have been removed

	grab = 1;

	/*********************************
	 * command line processing (S.A) *
	 *********************************/

	// windowed mode is default
	// see if the user wants fullscreen
	if (COM_CheckParm("-fullscreen") || COM_CheckParm("-f") ||
		COM_CheckParm("-fs") || COM_CheckParm("--fullscreen"))
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
		if (vid_default == MODE_WINDOWED)
			width=640;
		else
			width=800;

		// we currently do "normal" modes with 4/3 ratio
		height = 3 * width / 4;
	}

	if (COM_CheckParm("-bpp"))
	{
		bpp = atoi(com_argv[COM_CheckParm("-bpp")+1]);
	}
	else
	{
		// this should be native bpp ??? S.A.
		bpp = 16;
	}
	modelist[vid_default].width = width;
	modelist[vid_default].height = height;
	modelist[vid_default].bpp = bpp;
	// we even don't need this, but heck, here we go...
	sprintf (modelist[vid_default].modedesc,"%dx%dx%d",width,height,bpp);

	vid.maxwarpwidth = WARP_WIDTH;
	vid.maxwarpheight = WARP_HEIGHT;
	vid.colormap = host_colormap;
	vid.fullbright = 256 - LittleLong (*((int *)vid.colormap + 2048));

	VID_SetMode (vid_default);

	ClearAllStates ();

	GL_Init ();

	// use com_userdir instead of com_gamedir to stock cached mesh files
	sprintf (gldir, "%s/glhexen", com_userdir);
	Sys_mkdir (gldir);
	sprintf (gldir, "%s/glhexen/boss", com_userdir);
	Sys_mkdir (gldir);
	sprintf (gldir, "%s/glhexen/puzzle", com_userdir);
	Sys_mkdir (gldir);

	VID_SetPalette (palette);

	// Check for 3DFX Extensions and initialize them.
	VID_Init8bitPalette();

	vid_initialized = true;

	vid.recalc_refdef = 1;

	Con_SafePrintf ("Video initialized.\n");

	vid_menudrawfn = VID_MenuDraw;
	vid_menukeyfn = VID_MenuKey;

	if (COM_CheckParm("-fullsbar"))
		fullsbardraw = true;
}


#define MAX_COLUMN_SIZE		9
#define MODE_AREA_HEIGHT	(MAX_COLUMN_SIZE + 2)

/*
================
VID_MenuDraw
================
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
	M_Print (13*8, 4 + MODE_AREA_HEIGHT * 8 + 8*8,
			 "-bpp    <depth>");
}

void ToggleFullScreenSA ()
{
	// This doesn't seem to cause any trouble even
	// with is_3dfx == true and FX_GLX_MESA == f
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
	case K_ESCAPE:
	case K_ENTER:
		S_LocalSound ("raven/menu1.wav");
		M_Menu_Options_f ();
		break;

	default:
		break;
	}
}
