/*
   gl_vidsdl.c -- SDL GL vid component

   Select window size and mode and init SDL in GL mode.

   Changed 7/11/04 by S.A.
   Fixed fullscreen opengl mode, window sizes
   Options are now:
     -fullscreen | -window, -height , -width , -bpp

   This file is a bit of a mess.
   Fullscreen modes are normally 3-5, but we only use modes 0 and 3
	MODE_WINDOWED		0
	MODE_FULLSCREEN_DEFAULT	3

   The "-mode" option has been removed

*/

#include "quakedef.h"
#include "winquake.h"
#include "resource.h"

#include "SDL.h"

#define MAX_MODE_LIST	30
#define VID_ROW_SIZE	3
#define WARP_WIDTH		320
#define WARP_HEIGHT		200
#define MAXWIDTH		10000
#define MAXHEIGHT		10000
#define BASEWIDTH		320
#define BASEHEIGHT		200

#define MODE_WINDOWED			0
#define NO_MODE					(MODE_WINDOWED - 1)
#define MODE_FULLSCREEN_DEFAULT	(MODE_WINDOWED + 3)

SDL_Surface *screen;

byte globalcolormap[VID_GRADES*256];

extern qboolean grab;
extern qboolean is_3dfx;
extern qboolean is_PowerVR;

qboolean in_mode_set = false;

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

typedef struct {
	int			width;
	int			height;
} lmode_t;

lmode_t	lowresmodes[] = {
	{320, 200},
	{320, 240},
	{400, 300},
	{512, 384},
};

const char *gl_vendor;
const char *gl_renderer;
const char *gl_version;
const char *gl_extensions;

qboolean		DDActive;
qboolean		scr_skipupdate;

static vmode_t	modelist[MAX_MODE_LIST];
static int		nummodes;
static vmode_t	*pcurrentmode;
static vmode_t	badmode;

//static DEVMODE	gdevmode;
static qboolean	vid_initialized = false;
static qboolean	windowed, leavecurrentmode;
static qboolean vid_canalttab = false;
static qboolean vid_wassuspended = false;
static int		windowed_mouse;
//static HICON	hIcon;

int			DIBWidth, DIBHeight;
int		WRHeight, WRWidth;
//RECT		WindowRect;
//DWORD		WindowStyle, ExWindowStyle;

//HWND	mainwindow, dibwindow;

int			vid_modenum = NO_MODE;
int			vid_realmode;
int			vid_default = MODE_WINDOWED;
static int	windowed_default;
unsigned char	vid_curpal[256*3];
static qboolean fullsbardraw = false;
float RTint[256],GTint[256],BTint[256];

//HGLRC	baseRC;
//HDC		maindc;

glvert_t glv;

cvar_t	gl_ztrick = {"gl_ztrick","1"};

//HWND WINAPI InitializeWindow (HINSTANCE hInstance, int nCmdShow);

viddef_t	vid;				// global video state

unsigned short	d_8to16table[256];
unsigned	d_8to24table[256];
unsigned char d_15to8table[65536];
unsigned	d_8to24TranslucentTable[256];

float		gldepthmin, gldepthmax;

modestate_t	modestate = MS_UNINIT;

void VID_MenuDraw (void);
void VID_MenuKey (int key);

//LONG WINAPI MainWndProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
//void AppActivate(BOOL fActive, BOOL minimize);
char *VID_GetModeDescription (int mode);
void ClearAllStates (void);
void VID_UpdateWindowStatus (void);
void GL_Init (void);
void VID_SetGamma(float value);
void VID_SetGamma_f(void);

//PROC glArrayElementEXT;
//PROC glColorPointerEXT;
//PROC glTexCoordPointerEXT;
//PROC glVertexPointerEXT;

typedef void (*lp3DFXFUNC) (int, int, int, int, int, const void*);
lp3DFXFUNC MyglColorTableEXT;
qboolean is8bit = false;
qboolean isPermedia = false;
qboolean gl_mtexable = false;

//====================================

cvar_t		vid_mode = {"vid_mode","0", false};
// Note that 0 is MODE_WINDOWED
cvar_t		_vid_default_mode = {"_vid_default_mode","0", true};
cvar_t		_vid_default_mode_win = {"_vid_default_mode_win","3", true};
cvar_t		vid_wait = {"vid_wait","0"};
cvar_t		vid_nopageflip = {"vid_nopageflip","0", true};
cvar_t		_vid_wait_override = {"_vid_wait_override", "0", true};
cvar_t		vid_config_x = {"vid_config_x","800", true};
cvar_t		vid_config_y = {"vid_config_y","600", true};
cvar_t		vid_stretch_by_2 = {"vid_stretch_by_2","1", true};
cvar_t		_windowed_mouse = {"_windowed_mouse","0", true};

int			window_center_x, window_center_y, window_x, window_y, window_width, window_height;
RECT		window_rect;

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

/*
void CenterWindow(HWND hWndCenter, int width, int height, BOOL lefttopjustify)
{
    RECT    rect;
    int     CenterX, CenterY;

	CenterX = (GetSystemMetrics(SM_CXSCREEN) - width) / 2;
	CenterY = (GetSystemMetrics(SM_CYSCREEN) - height) / 2;
	if (CenterX > CenterY*2)
		CenterX >>= 1;	// dual screens
	CenterX = (CenterX < 0) ? 0: CenterX;
	CenterY = (CenterY < 0) ? 0: CenterY;
	SetWindowPos (hWndCenter, NULL, CenterX, CenterY, 0, 0,
			SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW | SWP_DRAWFRAME);
}
*/

/* Init SDL */

qboolean VID_SetWindowedMode (int modenum)
{
	// handle both fullscreen and windowed modes -S.A

	Uint32 flags;

/* SDL doco recons you need this. S.A.
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

	// debug printf ("VID_SetWindowedMode, modenum %i",modenum);

	// Linux version only uses modes 0, 3
	if (modenum==0)
		modestate = MS_WINDOWED;
	else	
		modestate = MS_FULLDIB;
	
	WRHeight = vid.height = vid.conheight = modelist[modenum].height; // BASEHEIGHT;
	WRWidth = vid.width = vid.conwidth =   modelist[modenum].width; //  BASEWIDTH ;

	vid.numpages = 2;

	if (SDL_Init(SDL_INIT_VIDEO) < 0)
		return false;
	if (!(screen = SDL_SetVideoMode (vid.width,vid.height,modelist[modenum].bpp, flags)))
		return false;

	SDL_WM_SetCaption ("GLHexenWorld", "GLHexenWorld");

	return true;


	// "#if 0" removed
}

// procedure VID_SetFullDIBMode is history
qboolean VID_SetFullDIBMode (int modenum) {
	return VID_SetWindowedMode (modenum);
}

int VID_SetMode (int modenum, unsigned char *palette)
{
	int				original_mode, temp;
	qboolean		stat;

	// so Con_Printfs don't mess us up by forcing vid and snd updates
	temp = scr_disabled_for_loading;
	scr_disabled_for_loading = true;

#warning Is this correct?!?
#if 0
	Snd_ReleaseBuffer ();
#endif
	CDAudio_Pause ();

	if (vid_modenum == NO_MODE)
		original_mode = windowed_default;
	else
		original_mode = vid_modenum;

	if (modelist[modenum].type == MS_WINDOWED)
	{
		if (_windowed_mouse.value)
		{
			//debug puts ("trace1\n");
			stat = VID_SetWindowedMode(modenum);
			IN_ActivateMouse ();
			IN_HideMouse ();
		}
		else
		{
			//debug puts ("trace2\n");
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
		Sys_Error ("VID_SetMode: Bad mode type in modelist");

	window_width = DIBWidth;
	window_height = DIBHeight;
	VID_UpdateWindowStatus ();

	CDAudio_Resume ();
#warning Is this correct?!?
#if 0
	Snd_AcquireBuffer ();
#endif
	scr_disabled_for_loading = temp;

	if (!stat)
	{
		Sys_Error ("Couldn't set video mode: %s", SDL_GetError());
	}

// now we try to make sure we get the focus on the mode switch, because
// sometimes in some systems we don't.  We grab the foreground, then
// finish setting up, pump all our messages, and sleep for a little while
// to let messages finish bouncing around the system, then we put
// ourselves at the top of the z order, then grab the foreground again,
// Who knows if it helps, but it probably doesn't hurt
#if 0
	SetForegroundWindow (mainwindow);
#endif
	VID_SetPalette (palette);
	vid_modenum = modenum;
	Cvar_SetValue ("vid_mode", (float)vid_modenum);

#if 0
	while (PeekMessage (&msg, NULL, 0, 0, PM_REMOVE))
	{
      	TranslateMessage (&msg);
      	DispatchMessage (&msg);
	}

	Sleep (100);

	SetWindowPos (mainwindow, HWND_TOP, 0, 0, 0, 0,
				  SWP_DRAWFRAME | SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW |
				  SWP_NOCOPYBITS);

	SetForegroundWindow (mainwindow);
#endif
// fix the leftover Alt from any Alt-Tab or the like that switched us away
	ClearAllStates ();

	if (!msg_suppress_1)
		Con_SafePrintf ("Video mode %s initialized.\n", VID_GetModeDescription (vid_modenum));

	VID_SetPalette (palette);

	vid.recalc_refdef = 1;

	return true;
}


void VID_UpdateWindowStatus (void)
{
	window_rect.left = window_x;
	window_rect.top = window_y;
	window_rect.right = window_x + window_width;
	window_rect.bottom = window_y + window_height;
	window_center_x = (window_rect.left + window_rect.right) / 2;
	window_center_y = (window_rect.top + window_rect.bottom) / 2;

	IN_UpdateClipCursor ();
}


//====================================

BINDTEXFUNCPTR bindTexFunc;

#define TEXTURE_EXT_STRING "GL_EXT_texture_object"

void CheckTextureExtensions (void)
{
	char		*tmp;
	qboolean	texture_ext;
//	HINSTANCE	hInstGL;

	texture_ext = false;
	/* check for texture extension */
	tmp = (unsigned char *)glGetString(GL_EXTENSIONS);
	while (*tmp)
	{
		if (strncmp((const char*)tmp, TEXTURE_EXT_STRING, strlen(TEXTURE_EXT_STRING)) == 0)
			texture_ext = true;
		tmp++;
	}

	if (!texture_ext || COM_CheckParm ("-gl11") )
	{
#if 0
		hInstGL = LoadLibrary("opengl32.dll");
		
		if (hInstGL == NULL)
			Sys_Error ("Couldn't load opengl32.dll\n");
#endif

#if 0
		bindTexFunc = (void *)GetProcAddress(hInstGL,"glBindTexture");
#else
		bindTexFunc = (void *)SDL_GL_GetProcAddress("glBindTexture");
#endif
		if (!bindTexFunc)
			Sys_Error ("No texture objects!");
		return;
	}

/* load library and get procedure adresses for texture extension API */
#if 0
	if ((bindTexFunc = (BINDTEXFUNCPTR)
		wglGetProcAddress((LPCSTR) "glBindTextureEXT")) == NULL)
#else
	if ((bindTexFunc =
				SDL_GL_GetProcAddress("glBindTextureEXT")) == NULL)
#endif

	{
		Sys_Error ("GetProcAddress for BindTextureEXT failed");
		return;
	}
}

void CheckArrayExtensions (void)
{
	char		*tmp;

	/* check for texture extension */
	tmp = (unsigned char *)glGetString(GL_EXTENSIONS);
	while (*tmp)
	{
		if (strncmp((const char*)tmp, "GL_EXT_vertex_array", strlen("GL_EXT_vertex_array")) == 0)
		{
			if (
((SDL_GL_GetProcAddress("glArrayElementEXT")) == NULL) ||
((SDL_GL_GetProcAddress("glColorPointerEXT")) == NULL) ||
((SDL_GL_GetProcAddress("glTexCoordPointerEXT")) == NULL) ||
((SDL_GL_GetProcAddress("glVertexPointerEXT")) == NULL) )
			{
				Sys_Error ("GetProcAddress for vertex extension failed");
				return;
			}
			return;
		}
		tmp++;
	}

	Sys_Error ("Vertex array extension not present");
}

extern HW_glActiveTextureARB hwglActiveTextureARB;
extern HW_glMultiTexCoord2fARB hwglMultiTexCoord2fARB;

//int		texture_mode = GL_NEAREST;
//int		texture_mode = GL_NEAREST_MIPMAP_NEAREST;
//int		texture_mode = GL_NEAREST_MIPMAP_LINEAR;
int		texture_mode = GL_LINEAR;
//int		texture_mode = GL_LINEAR_MIPMAP_NEAREST;
//int		texture_mode = GL_LINEAR_MIPMAP_LINEAR;

int		texture_extension_number = 1;

extern cvar_t gl_multitex;

void CheckMultiTextureExtensions(void) 
{
	GLint texture_units = 1;

	Cvar_RegisterVariable (&gl_multitex);

	if (strstr(gl_extensions, "GL_ARB_multitexture ") && !COM_CheckParm("-nomtex"))
	{
		Con_Printf ("Multitexture extensions found.\n");
		hwglActiveTextureARB = (HW_glActiveTextureARB) SDL_GL_GetProcAddress("glActiveTextureARB");
		hwglMultiTexCoord2fARB = (HW_glMultiTexCoord2fARB) SDL_GL_GetProcAddress ("glMultiTexCoord2fARB");
		
		glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB,&texture_units);

		if (texture_units < 2)
			return;

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
	gl_vendor = glGetString (GL_VENDOR);
	Con_Printf ("GL_VENDOR: %s\n", gl_vendor);
	gl_renderer = glGetString (GL_RENDERER);
	Con_Printf ("GL_RENDERER: %s\n", gl_renderer);

	gl_version = glGetString (GL_VERSION);
	Con_Printf ("GL_VERSION: %s\n", gl_version);
	gl_extensions = glGetString (GL_EXTENSIONS);
	Con_Printf ("GL_EXTENSIONS: %s\n", gl_extensions);

	if (!Q_strncasecmp ((char *)gl_renderer, "3dfx",4))
	{
		is_3dfx = true;
	}

	if (!Q_strncasecmp ((char *)gl_renderer, "PowerVR PCX1",12) ||
		!Q_strncasecmp ((char *)gl_renderer, "PowerVR PCX2",12))
	{
		is_PowerVR = true;
	}

//	Con_Printf ("%s %s\n", gl_renderer, gl_version);

    if (strnicmp(gl_renderer,"PowerVR",7)==0)
         fullsbardraw = true;

    if (strnicmp(gl_renderer,"Permedia",8)==0)
         isPermedia = true;

	CheckTextureExtensions ();
	CheckMultiTextureExtensions ();

	glClearColor (1,0,0,0);
	glCullFace(GL_FRONT);
	glEnable(GL_TEXTURE_2D);

	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.666);

	glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
	glShadeModel (GL_FLAT);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

//	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

#if 0
	CheckArrayExtensions ();

	glEnable (GL_VERTEX_ARRAY_EXT);
	glEnable (GL_TEXTURE_COORD_ARRAY_EXT);
	glVertexPointerEXT (3, GL_FLOAT, 0, 0, &glv.x);
	glTexCoordPointerEXT (2, GL_FLOAT, 0, 0, &glv.s);
	glColorPointerEXT (3, GL_FLOAT, 0, 0, &glv.r);
#endif
}

/*
=================
VID_SetGamma

=================
*/

void VID_SetGamma(float value)
{
//#ifdef PLATFORM_UNIX
  SDL_SetGamma(value,value,value);
//#endif
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


/*
=================
GL_BeginRendering

=================
*/
void GL_BeginRendering (int *x, int *y, int *width, int *height)
{
	extern cvar_t gl_clear;

	*x = *y = 0;
#if 0
	*width = WindowRect.right - WindowRect.left;
	*height = WindowRect.bottom - WindowRect.top;
#else
	*width = WRWidth;
	*height = WRHeight;
#endif

//    if (!wglMakeCurrent( maindc, baseRC ))
//		Sys_Error ("wglMakeCurrent failed");

//	glViewport (*x, *y, *width, *height);
}


void GL_EndRendering (void)
{
#if 0
	if (!scr_skipupdate || block_drawing)
		SwapBuffers(maindc);
#else
	if (!scr_skipupdate)
		SDL_GL_SwapBuffers();
#endif

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

unsigned	d_8to24table3dfx[256];

void	VID_SetPalette (unsigned char *palette)
{
	byte	*pal;
	unsigned short r,g,b;
	int     v;
	int     r1,g1,b1;
	int		j,k,l,m;
	unsigned short i, p, c;
	unsigned	*table, *table3dfx;
	FILE *f;
	char s[255];
//	HWND hDlg, hProgress;

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
#if 0
		hDlg = CreateDialog(global_hInstance, MAKEINTRESOURCE(IDD_PROGRESS), 
			NULL, NULL);
		hProgress = GetDlgItem(hDlg, IDC_PROGRESS);
		SendMessage(hProgress, PBM_SETSTEP, 1, 0);
		SendMessage(hProgress, PBM_SETRANGE, 0, MAKELPARAM(0, 33));
#endif
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
#if 0
				SendMessage(hProgress, PBM_STEPIT, 0, 0);
#endif
				m=0;
			}
		}
		sprintf(s, "%s/glhexen", com_userdir);
 		Sys_mkdir (s);
		sprintf(s, "%s/glhexen/15to8.pal", com_userdir);
		if (f = fopen(s, "wb"))
		{
			fwrite(d_15to8table, 1<<15, 1, f);
			fclose(f);
		}
#if 0
		DestroyWindow(hDlg);
#endif
	}

	d_8to24table[255] &= 0xffffff;	// 255 is transparent
}

BOOL	gammaworks;

void	VID_ShiftPalette (unsigned char *palette)
{
	extern	byte ramps[3][256];
	
//	VID_SetPalette (palette);

//	gammaworks = SetDeviceGammaRamp (maindc, ramps);
}


void VID_SetDefaultMode (void)
{
	IN_DeactivateMouse ();
}


void	VID_Shutdown (void)
{
	SDL_Quit();
#if 0
   	HGLRC hRC;
   	HDC	  hDC;

	if (vid_initialized)
	{
		vid_canalttab = false;
		hRC = wglGetCurrentContext();
    	hDC = wglGetCurrentDC();

    	wglMakeCurrent(NULL, NULL);

    	if (hRC)
    	    wglDeleteContext(hRC);

		if (hDC && dibwindow)
			ReleaseDC(dibwindow, hDC);

		if (modestate == MS_FULLDIB)
			ChangeDisplaySettings (NULL, 0);

		if (maindc && dibwindow)
			ReleaseDC (dibwindow, maindc);

		AppActivate(false, false);
	}
#endif
}


//==========================================================================

#if 0
BOOL bSetupPixelFormat(HDC hDC)
{
    static PIXELFORMATDESCRIPTOR pfd = {
	sizeof(PIXELFORMATDESCRIPTOR),	// size of this pfd
	1,				// version number
	PFD_DRAW_TO_WINDOW 		// support window
	|  PFD_SUPPORT_OPENGL 	// support OpenGL
	|  PFD_DOUBLEBUFFER ,	// double buffered
	PFD_TYPE_RGBA,			// RGBA type
	24,				// 24-bit color depth
	0, 0, 0, 0, 0, 0,		// color bits ignored
	0,				// no alpha buffer
	0,				// shift bit ignored
	0,				// no accumulation buffer
	0, 0, 0, 0, 			// accum bits ignored
	32,				// 32-bit z-buffer	
	0,				// no stencil buffer
	0,				// no auxiliary buffer
	PFD_MAIN_PLANE,			// main layer
	0,				// reserved
	0, 0, 0				// layer masks ignored
    };
    int pixelformat;

    if ( (pixelformat = ChoosePixelFormat(hDC, &pfd)) == 0 )
    {
        MessageBox(NULL, "ChoosePixelFormat failed", "Error", MB_OK);
        return FALSE;
    }

    if (SetPixelFormat(hDC, pixelformat, &pfd) == FALSE)
    {
        MessageBox(NULL, "SetPixelFormat failed", "Error", MB_OK);
        return FALSE;
    }

    return TRUE;
}
#endif


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
	K_F6, K_F7, K_F8, K_F9, K_F10, K_PAUSE  ,    0  , K_HOME, 
	K_UPARROW,K_PGUP,'-',K_LEFTARROW,'5',K_RIGHTARROW,'+',K_END, //4 
	K_DOWNARROW,K_PGDN,K_INS,K_DEL,0,0,             0,              K_F11, 
	K_F12,0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0,        // 5 
	0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0, 
	0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0,        // 6 
	0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0, 
	0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0         // 7 
					}; 

byte        shiftscantokey[128] = 
					{ 
//  0           1       2       3       4       5       6       7 
//  8           9       A       B       C       D       E       F 
	0  ,    27,     '!',    '@',    '#',    '$',    '%',    '^', 
	'&',    '*',    '(',    ')',    '_',    '+',    K_BACKSPACE, 9, // 0 
	'Q',    'W',    'E',    'R',    'T',    'Y',    'U',    'I', 
	'O',    'P',    '{',    '}',    13 ,    K_CTRL,'A',  'S',      // 1 
	'D',    'F',    'G',    'H',    'J',    'K',    'L',    ':', 
	'"' ,    '~',    K_SHIFT,'|',  'Z',    'X',    'C',    'V',      // 2 
	'B',    'N',    'M',    '<',    '>',    '?',    K_SHIFT,'*', 
	K_ALT,' ',   0  ,    K_F1, K_F2, K_F3, K_F4, K_F5,   // 3 
	K_F6, K_F7, K_F8, K_F9, K_F10, K_PAUSE  ,    0  , K_HOME, 
	K_UPARROW,K_PGUP,'_',K_LEFTARROW,'%',K_RIGHTARROW,'+',K_END, //4 
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
	if (scantokey[key] == 0)
		Con_DPrintf("key 0x%02x has no translation\n", key);
	return scantokey[key];
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
VID_GetModeDescription
=================
*/
char *VID_GetModeDescription (int mode)
{
	char		*pinfo;
	vmode_t		*pv;
	static char	temp[100];

	if ((mode < 0) || (mode >= nummodes))
		return NULL;

	if (!leavecurrentmode)
	{
		pv = VID_GetModePtr (mode);
		pinfo = pv->modedesc;
	}
	else
	{
		sprintf (temp, "Desktop resolution (%dx%d)",
				 modelist[MODE_FULLSCREEN_DEFAULT].width,
				 modelist[MODE_FULLSCREEN_DEFAULT].height);
		pinfo = temp;
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

	pv = VID_GetModePtr (mode);
	if (modelist[mode].type == MS_FULLDIB)
	{
		if (!leavecurrentmode)
		{
			sprintf(pinfo,"%s fullscreen", pv->modedesc);
		}
		else
		{
			sprintf (pinfo, "Desktop resolution (%dx%d)",
					 modelist[MODE_FULLSCREEN_DEFAULT].width,
					 modelist[MODE_FULLSCREEN_DEFAULT].height);
		}
	}
	else
	{
		if (modestate == MS_WINDOWED)
			sprintf(pinfo, "%s windowed", pv->modedesc);
		else
			sprintf(pinfo, "windowed");
	}

	return pinfo;
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
	int		t, modenum;
	
	modenum = atoi (Cmd_Argv(1));

	t = leavecurrentmode;
	leavecurrentmode = 0;

	Con_Printf ("%s\n", VID_GetExtModeDescription (modenum));

	leavecurrentmode = t;
}


/*
=================
VID_DescribeModes_f
=================
*/
void VID_DescribeModes_f (void)
{
	int			i, lnummodes, t;
	char		*pinfo;
	vmode_t		*pv;

	lnummodes = VID_NumModes ();

	t = leavecurrentmode;
	leavecurrentmode = 0;

	for (i=1 ; i<lnummodes ; i++)
	{
		pv = VID_GetModePtr (i);
		pinfo = VID_GetExtModeDescription (i);
		Con_Printf ("%2d: %s\n", i, pinfo);
	}

	leavecurrentmode = t;
}

// "if 0" removed

qboolean VID_Is8bit() {
	return is8bit;
}

#define GL_SHARED_TEXTURE_PALETTE_EXT 0x81FB

void VID_Init8bitPalette() 
{
	// Check for 8bit Extensions and initialize them.
	int i;
	char thePalette[256*3];
	char *oldPalette, *newPalette;

#if 0
	glColorTableEXT = (void *)wglGetProcAddress("glColorTableEXT");
#else
	MyglColorTableEXT = (void *)SDL_GL_GetProcAddress("glColorTableEXT");
#endif
    if (MyglColorTableEXT && strstr(gl_extensions, "GL_EXT_shared_texture_palette")) {
		Con_SafePrintf("8-bit GL extensions enabled.\n");
	    glEnable( GL_SHARED_TEXTURE_PALETTE_EXT );
		oldPalette = (char *) d_8to24table; //d_8to24table3dfx;
		newPalette = thePalette;
		for (i=0;i<256;i++) {
			*newPalette++ = *oldPalette++;
			*newPalette++ = *oldPalette++;
			*newPalette++ = *oldPalette++;
			oldPalette++;
		}
		MyglColorTableEXT(GL_SHARED_TEXTURE_PALETTE_EXT, GL_RGB, 256, GL_RGB, GL_UNSIGNED_BYTE,
			(void *) thePalette);
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
	int		i, existingmode;
	int		basenummodes, width, height, bpp, findbpp, done;
	byte	*ptmp;
	char	gldir[MAX_OSPATH];
#if 0
	HDC		hdc;
	DEVMODE	devmode;
#endif

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
	Cvar_RegisterVariable (&gl_ztrick);

	Cmd_AddCommand ("vid_nummodes", VID_NumModes_f);
	Cmd_AddCommand ("vid_describecurrentmode", VID_DescribeCurrentMode_f);
	Cmd_AddCommand ("vid_describemode", VID_DescribeMode_f);
	Cmd_AddCommand ("vid_describemodes", VID_DescribeModes_f);
	Cmd_AddCommand ("vid_setgamma", VID_SetGamma_f);

        modelist[0].type = MS_WINDOWED;
        modelist[0].width = 640;
        modelist[0].height = 480;
        strcpy (modelist[0].modedesc, "640x480");
        modelist[0].modenum = MODE_WINDOWED;
        modelist[0].dib = 1;
        modelist[0].fullscreen = 0;
        modelist[0].halfscreen = 0;
        modelist[0].bpp = 16;

        modelist[1].type = MS_WINDOWED;
        modelist[1].width = 800;
        modelist[1].height = 600;
        strcpy (modelist[1].modedesc, "800x600");
        modelist[1].modenum = MODE_WINDOWED + 1;
        modelist[1].dib = 1;
        modelist[1].fullscreen = 0;
        modelist[1].halfscreen = 0;
        modelist[1].bpp = 16;

        modelist[2].type = MS_WINDOWED;
        modelist[2].width = 1024;
        modelist[2].height = 768;
        strcpy (modelist[2].modedesc, "1024x768");
        modelist[2].modenum = MODE_WINDOWED + 2;
        modelist[2].dib = 1;
        modelist[2].fullscreen = 0;
        modelist[2].halfscreen = 0;
        modelist[2].bpp = 16;

        modelist[3].type = MS_FULLDIB;
        modelist[3].width = 640;
        modelist[3].height = 480;
        strcpy (modelist[3].modedesc, "640x480");
        modelist[3].modenum = MODE_WINDOWED + 3;
        modelist[3].dib = 1;
        modelist[3].fullscreen = 1;
        modelist[3].halfscreen = 0;
        modelist[3].bpp = 16;

        modelist[4].type = MS_FULLDIB;
        modelist[4].width = 800;
        modelist[4].height = 600;
        strcpy (modelist[4].modedesc, "800x600");
        modelist[4].modenum = MODE_WINDOWED + 4;
        modelist[4].dib = 1;
        modelist[4].fullscreen = 1;
        modelist[4].halfscreen = 0;
        modelist[4].bpp = 16;

        modelist[5].type = MS_FULLDIB;
        modelist[5].width = 1024;
        modelist[5].height = 768;
        strcpy (modelist[5].modedesc, "1024x768");
        modelist[5].modenum = MODE_WINDOWED + 5;
        modelist[5].dib = 1;
        modelist[5].fullscreen = 1;
        modelist[5].halfscreen = 0;

	VID_SetPalette (palette);

	basenummodes = nummodes = 6;

	/*********************************
	 * command line processing (S.A) *
	 *********************************/

	// window mode is default 

	grab = 1;

	if (COM_CheckParm("-fullscreen") || COM_CheckParm("-f") ||
		COM_CheckParm("-fs") || COM_CheckParm("--fullscreen"))
	{
		windowed = false;
		vid_default = MODE_FULLSCREEN_DEFAULT;
	}

	if (COM_CheckParm("-window") || COM_CheckParm("-w") ||
			COM_CheckParm("--windowed"))
	{
		windowed = true;
		vid_default = MODE_WINDOWED;
	}

	if (COM_CheckParm("-width"))
	{
		width = atoi(com_argv[COM_CheckParm("-width")+1]);
	}
	else
	{
		if (vid_default == MODE_WINDOWED) 
			width=640;
		else
			width=800;

	}

	if (COM_CheckParm("-height"))
	{
		height = atoi(com_argv[COM_CheckParm("-height")+1]);
	}
	else
	{
		switch (width) {
			case 1600: height=1200; break;
			case 1280: height=1024; break;
			case 1024: height=768;  break;
			case 800:  height=600;  break;
			case 640:  height=480;  break;
			case 512:  height=384;  break;
			case 400:  height=300;  break;
			case 320:  height=240;  break;

			default:   height=480;
		}
	}
	if (COM_CheckParm("-bpp"))
	{
		bpp = atoi(com_argv[COM_CheckParm("-bpp")+1]);
		findbpp = 0;
	}
	else
	{
		// this should be native bpp ??? S.A.
		bpp = 16;
	}
	modelist[vid_default].width = width;
	modelist[vid_default].height = height;
	sprintf (modelist[vid_default].modedesc,"%dx%dx%d",width,height,bpp);

	vid_initialized = true;

/*	if ((i = COM_CheckParm("-conwidth")) != 0)
		vid.conwidth = atoi(com_argv[i+1]);
	else
		vid.conwidth = 640;

	vid.conwidth &= 0xfff8; // make it a multiple of eight

	if (vid.conwidth < 320)
		vid.conwidth = 320;

	// pick a conheight that matches with correct aspect
	vid.conheight = vid.conwidth*3 / 4;

	if ((i = COM_CheckParm("-conheight")) != 0)
		vid.conheight = atoi(com_argv[i+1]);
	if (vid.conheight < 200)
		vid.conheight = 200;
*/
	vid.maxwarpwidth = WARP_WIDTH;
	vid.maxwarpheight = WARP_HEIGHT;
	vid.colormap = host_colormap;
	vid.fullbright = 256 - LittleLong (*((int *)vid.colormap + 2048));

	
	VID_SetMode (vid_default, palette);


	GL_Init ();

	/*sprintf (gldir, "%s/glhexen", com_gamedir);
	Sys_mkdir (gldir);
	sprintf (gldir, "%s/glhexen/boss", com_gamedir);
	Sys_mkdir (gldir);
	sprintf (gldir, "%s/glhexen/puzzle", com_gamedir);
	Sys_mkdir (gldir);*/

	sprintf (gldir, "%s/glhexen", com_userdir);
	Sys_mkdir (gldir);
	sprintf (gldir, "%s/glhexen/boss", com_userdir);
	Sys_mkdir (gldir);
	sprintf (gldir, "%s/glhexen/puzzle", com_userdir);
	Sys_mkdir (gldir);

	vid_realmode = vid_modenum;

	VID_SetPalette (palette);
	
	// Check for 3DFX Extensions and initialize them.
	VID_Init8bitPalette();

	// vid_menudrawfn = NULL;
	vid_menudrawfn = VID_MenuDraw;
	vid_menukeyfn = VID_MenuKey;

	strcpy (badmode.modedesc, "Bad mode");
	vid_canalttab = true;

	if (COM_CheckParm("-fullsbar"))
		fullsbardraw = true;
}


//========================================================
// Video menu stuff
//========================================================

extern void M_Menu_Options_f (void);
extern void M_Print (int cx, int cy, char *str);
extern void M_PrintWhite (int cx, int cy, char *str);
extern void M_DrawCharacter (int cx, int line, int num);
extern void M_DrawTransPic (int x, int y, qpic_t *pic);
extern void M_DrawPic (int x, int y, qpic_t *pic);

static int	vid_line, vid_wmodes;

typedef struct
{
	int		modenum;
	char	*desc;
	int		iscur;
} modedesc_t;

#define MAX_COLUMN_SIZE		9
#define MODE_AREA_HEIGHT	(MAX_COLUMN_SIZE + 2)
#define MAX_MODEDESCS		(MAX_COLUMN_SIZE*3)

static modedesc_t	modedescs[MAX_MODEDESCS];

/*
================
VID_MenuDraw
================
*/
void VID_MenuDraw (void)
{
	qpic_t		*p;
	char		*ptr;
	int			lnummodes, i, j, k, column, row, dup, dupmode;
	char		temp[100];
	vmode_t		*pv;

	ScrollTitle("gfx/menu/title7.lmp");

	vid_wmodes = 0;
	lnummodes = VID_NumModes ();
	
	for (i=1 ; (i<lnummodes) && (vid_wmodes < MAX_MODEDESCS) ; i++)
	{
		ptr = VID_GetModeDescription (i);
		pv = VID_GetModePtr (i);

		k = vid_wmodes;

		modedescs[k].modenum = i;
		modedescs[k].desc = ptr;
		modedescs[k].iscur = 0;

		if (i == vid_modenum)
			modedescs[k].iscur = 1;

		vid_wmodes++;

	}

	// if (vid_wmodes > 0) S.A
	if (0)
	{
		M_Print (2*8, 60+0*8, "Fullscreen Modes (WIDTHxHEIGHTxBPP)");

		column = 8;
		row = 60+2*8;

		for (i=0 ; i<vid_wmodes ; i++)
		{
			if (modedescs[i].iscur)
				M_PrintWhite (column, row, modedescs[i].desc);
			else
				M_Print (column, row, modedescs[i].desc);

			column += 13*8;

			if ((i % VID_ROW_SIZE) == (VID_ROW_SIZE - 1))
			{
				column = 8;
				row += 8;
			}
		}
	}

	M_Print (0*8, 4 + MODE_AREA_HEIGHT * 8 + 8*0,
			 "Select video modes from the command line");
	M_Print (13*8, 4 + MODE_AREA_HEIGHT * 8 + 8*2,
			 "-window");
	M_Print (13*8, 4 + MODE_AREA_HEIGHT * 8 + 8*3,
			 "-fullscreen");
	M_Print (13*8, 4 + MODE_AREA_HEIGHT * 8 + 8*5,
			 "-width  <width>");
	M_Print (13*8, 4 + MODE_AREA_HEIGHT * 8 + 8*6,
			 "-height <height>");
	M_Print (13*8, 4 + MODE_AREA_HEIGHT * 8 + 8*7,
			 "-bpp    <depth>");
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
