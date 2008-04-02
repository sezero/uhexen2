/*
	vid.h
	video driver defs

	$Id: vid.h,v 1.36 2008-04-02 19:10:34 sezero Exp $
*/

#ifndef __VID_DEFS_H
#define __VID_DEFS_H

#define VID_CBITS	6
#define VID_GRADES	(1 << VID_CBITS)

#define GAMMA_MAX	3.0

// a pixel can be one, two, or four bytes
typedef byte pixel_t;

typedef struct vrect_s
{
	int		x, y, width, height;
	struct vrect_s	*pnext;
} vrect_t;

typedef enum
{
	MS_WINDOWED,
	MS_FULLSCREEN,
	MS_FULLDIB,
	MS_FULLDIRECT,
	MS_UNINIT
} modestate_t;

#define MODE_WINDOWED		0
#define NO_MODE			(MODE_WINDOWED - 1)
#define MODE_FULLSCREEN_DEFAULT	(MODE_WINDOWED + 3)
#define MODE_SETTABLE_WINDOW	2

typedef struct
{
	pixel_t		*buffer;	// invisible buffer
	pixel_t		*colormap;	// 256 * VID_GRADES size
	unsigned short	*colormap16;	// 256 * VID_GRADES size
	int		fullbright;	// index of first fullbright color
	unsigned int	rowbytes;	// may be > width if displayed in a window
	unsigned int	width;
	unsigned int	height;
	float		aspect;		// width / height -- < 0 is taller than wide
	int		numpages;
	int		recalc_refdef;	// if true, recalc vid-based stuff
	pixel_t		*conbuffer;
	int		conrowbytes;
	unsigned int	conwidth;
	unsigned int	conheight;
	int		maxwarpwidth;
	int		maxwarpheight;
	pixel_t		*direct;	// direct drawing to framebuffer,
					// if not NULL
} viddef_t;

extern	byte		globalcolormap[VID_GRADES*256];
extern	byte		lastglobalcolor, *lastsourcecolormap;

extern	viddef_t	vid;			// global video state
extern	modestate_t	modestate;
extern	qboolean	in_mode_set;

extern	unsigned short	d_8to16table[256];
extern	unsigned int	d_8to24table[256];
extern	unsigned int	d_8to24TranslucentTable[256];

extern	cvar_t		_enable_mouse;

void VID_SetPalette (unsigned char *palette);
// called at startup and after any gamma correction

void VID_ShiftPalette (unsigned char *palette);
// called for bonus and pain flashes, and for underwater color changes
// in gl mode, used to update hardware gamma.

void VID_Init (unsigned char *palette);
// Called at startup to set up translation tables, takes 256 8 bit RGB values
// the palette data will go away after the call, so it must be copied off if
// the video driver will need it again

void VID_Shutdown (void);
// Called at shutdown

void VID_Update (vrect_t *rects);
// flushes the given rectangles from the view buffer to the screen

#if defined(PLATFORM_DOS) || defined(SVGAQUAKE)
#define VID_LockBuffer()	do {} while (0)
#define VID_UnlockBuffer()	do {} while (0)
#elif defined(GLQUAKE)
#define VID_LockBuffer()	do {} while (0)
#define VID_UnlockBuffer()	do {} while (0)
#else
void VID_LockBuffer (void);
void VID_UnlockBuffer (void);
// vid buffer locking, not used in opengl version or DOS/SVGA software version
#endif

#if defined(PLATFORM_DOS) || defined(SVGAQUAKE)
#define VID_HandlePause(x)	do {} while (0)
#else
void VID_HandlePause (qboolean paused);
// called on windowed environments when pause happens, so the mouse can be released
#endif

void VID_ToggleFullscreen (void);	// from Steven
// toggles between windowed/fullscreen modes. for unix/sdl

#if defined(GLQUAKE)
void VID_ChangeConsize (int key);
// changes effective console size. callback for the opengl features menu

char *VID_ReportConsize(void);
// reports effective console size as a string to the opengl features menu
#endif	/* ! GLQUAKE */

void D_ShowLoadingSize (void);
// displays progress bars while loading a map. (not used in hexenworld.)

extern void (*vid_menudrawfn)(void);
extern void (*vid_menukeyfn)(int key);
// video menu function pointers

#endif	/* __VID_DEFS_H */

