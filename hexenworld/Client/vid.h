// vid.h -- video driver defs

#define VID_CBITS	6
#define VID_GRADES	(1 << VID_CBITS)

#define GAMMA_MAX 3

// a pixel can be one, two, or four bytes
typedef byte pixel_t;

typedef struct vrect_s
{
	int				x,y,width,height;
	struct vrect_s	*pnext;
} vrect_t;

typedef enum {MS_WINDOWED, MS_FULLSCREEN, MS_FULLDIB, MS_UNINIT} modestate_t;
#define MODE_WINDOWED		0
#define NO_MODE			(MODE_WINDOWED - 1)
#if !defined(GLQUAKE) && defined(_WIN32)
#define MODE_FULLSCREEN_DEFAULT	(MODE_WINDOWED + 3)
#else
#define MODE_FULLSCREEN_DEFAULT	(MODE_WINDOWED + 1)
#endif
#define MODE_SETTABLE_WINDOW	2

typedef struct
{
	pixel_t			*buffer;		// invisible buffer
	pixel_t			*colormap;		// 256 * VID_GRADES size
	unsigned short	*colormap16;	// 256 * VID_GRADES size
	int				fullbright;		// index of first fullbright color
	unsigned		rowbytes;	// may be > width if displayed in a window
	unsigned		width;		
	unsigned		height;
	float			aspect;		// width / height -- < 0 is taller than wide
	int				numpages;
	int				recalc_refdef;	// if true, recalc vid-based stuff
	pixel_t			*conbuffer;
	int				conrowbytes;
	unsigned		conwidth;
	unsigned		conheight;
	int				maxwarpwidth;
	int				maxwarpheight;
	pixel_t			*direct;		// direct drawing to framebuffer, if not
									//  NULL
} viddef_t;

extern byte globalcolormap[VID_GRADES*256], lastglobalcolor, *lastsourcecolormap;

extern	viddef_t	vid;				// global video state
extern	unsigned short	d_8to16table[256];
extern	unsigned	d_8to24table[256];
extern	unsigned	d_8to24TranslucentTable[256];
extern void (*vid_menudrawfn)(void);
extern void (*vid_menukeyfn)(int key);

void	VID_SetPalette (unsigned char *palette);
// called at startup and after any gamma correction

void	VID_ShiftPalette (unsigned char *palette);
// called for bonus and pain flashes, and for underwater color changes
// in gl mode, used to update hardware gamma.

void	VID_Init (unsigned char *palette);
// Called at startup to set up translation tables, takes 256 8 bit RGB values
// the palette data will go away after the call, so it must be copied off if
// the video driver will need it again

void	VID_Shutdown (void);
// Called at shutdown

void	VID_Update (vrect_t *rects);
// flushes the given rectangles from the view buffer to the screen

#if defined(PLATFORM_UNIX) && defined(GLQUAKE)
// doesn't use the palette arg anymore in GL mode. on unix/sdl. O.S
int VID_SetMode (int modenum);
#else
int VID_SetMode (int modenum, unsigned char *palette);
#endif
// sets the mode; only used by the Quake engine for resetting to mode 0 (the
// base mode) on memory allocation failures

#ifndef H2W
void VID_HandlePause (qboolean pause);
// called only on Win32, when pause happens, so the mouse can be released
#endif

void VID_ApplyGamma (void);
// really applies gamma correction after the v_gamma variable is changed


void VID_ForceLockState (int lk);
int VID_ForceUnlockedAndReturnState (void);
void VID_SetDefaultMode (void);
void ToggleFullScreenSA (void);	// from Steven
#ifndef H2W
void D_ShowLoadingSize(void);
#endif

