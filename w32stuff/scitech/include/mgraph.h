/****************************************************************************
*
*						MegaGraph Graphics Library
*
*  ========================================================================
*
*    The contents of this file are subject to the SciTech MGL Public
*    License Version 1.0 (the "License"); you may not use this file
*    except in compliance with the License. You may obtain a copy of
*    the License at http://www.scitechsoft.com/mgl-license.txt
*
*    Software distributed under the License is distributed on an
*    "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
*    implied. See the License for the specific language governing
*    rights and limitations under the License.
*
*    The Original Code is Copyright (C) 1991-1998 SciTech Software, Inc.
*
*    The Initial Developer of the Original Code is SciTech Software, Inc.
*    All Rights Reserved.
*
*  ========================================================================
*
* Language:		ANSI C
* Environment:	IBM PC (MS DOS)
*
* Description:	Header file for the MegaGraph Graphics Library. You can
*				defined one of the following to specify which MGL API you
*				wish to use.
*
*					MGL_LITE	- Compile for the MGL/Lite API
*					MGL_FIX3D	- Compile for the MGL/3D API (fixed point)
*
*				If you do not define any of these, MGL_FIX3D will be defined
*				automatically for compatibility with older versions of the
*				MGL.
*
****************************************************************************/

#ifndef	__MGRAPH_H
#define	__MGRAPH_H

#ifndef	__SCITECH_H
#include "scitech.h"
#endif

#include <stdio.h>
#include "mgl/mglver.h"

#if	!defined(MGL_LITE) && !defined(MGL_PRO) && !defined(MGL_FIX3D) \
	&& !defined(MGL_FLT3D)
#define	MGL_FIX3D
#endif

/*---------------------- Macros and type definitions ----------------------*/

#pragma pack(4)

/* Define the version number for the MGL release */

#define MGL_VERSION_STR	\
	MGL_RELEASE_MAJOR_STR "." MGL_RELEASE_MINOR_STR

/* Define the calling conventions for all public MGL functions. For simplicity
 * we define all public functions as __cdecl calling conventions, so that
 * they are the same across all compilers and runtime DLL's.
 */

#define	MGLAPI	_EXPORT __cdecl
#define	ASMAPI	_EXPORT __cdecl

/****************************************************************************
REMARKS:
Defines the graphics subsystems available. This is the value returned in
the driver parameter for MGL_detectGraph and MGL_init. Note that if a driver
returns a value of grOPENGL_MGL_MINI, it is an MGL OpenGL mini-driver and
does not implement the entire OpenGL API. It is up to the application
developer to find out from the hardware vendor who supplied the driver
exactly what features are supported and to make use of those features in
their application.

HEADER:
mgraph.h

MEMBERS:
grDETECT			- Auto detect the graphics subsystem
grNONE				- No graphics hardware detected
grVGA				- Standard VGA
grFULLDIB			- Fullscreen DIBSection's on Win95/NT
grVESA				- VESA VBE compliant SuperVGA
grSVGA				- Unaccelerated SuperVGA
grACCEL				- Accelerated SuperVGA
grDDRAW				- Unaccelerated DirectDraw
grDDRAWACCEL		- Accelerated DirectDraw
grDDRAW3D			- 3D Accelerated DirectDraw
grOPENGL			- Hardware Accelerated OpenGL (ICD/MCD)
grOPENGL_MGL_MINI	- MGL specific hardware OpenGL mini-driver
grOPENGL_MGL		- MGL specific hardware OpenGL driver
grMAXDRIVER			- Maximum driver number
****************************************************************************/
typedef enum {
	grDETECT        = -1,
	grNONE          = 0,
	grVGA,
	grFULLDIB,
	grVESA,
	grSVGA,
	grACCEL,
	grDDRAW,
	grDDRAWACCEL,
	grDDRAW3D,
	grOPENGL,
	grOPENGL_MGL_MINI,
	grOPENGL_MGL,
	grMAXDRIVER,
	} MGL_driverType;


/****************************************************************************
REMARKS:
Defines the supported graphics modes. This is the value passed in the mode
parameter to MGL_init and MGL_changeDisplayMode. Note that although we
define symbolic constants for all the available graphics modes supported
by this version of the library, for maximum compatibility with future
versions of the library (which may define new modes as they become
available), the best way to search for a supported mode is to search the
list of modes returned by MGL_availableModes, and look for one that has
the desired color depth and resolution by calling MGL_modeResolution. Check
out how this is done in the SciTech Game Framework source code for more
information.

Note: The only video modes supported by this graphics library are those
that support at least 16 colors per pixel.
****************************************************************************/
typedef enum {
	/* 16 color VGA video modes */
    grVGA_320x200x16,
	grVGA_640x200x16,
	grVGA_640x350x16,
	grVGA_640x400x16,
	grVGA_640x480x16,
	grSVGA_800x600x16,

    /* 256 color VGA video modes */
    grVGA_320x200x256,

	/* 256 color VGA ModeX video modes */
    grVGAX_320x200x256,
	grVGAX_320x240x256,
	grVGAX_320x400x256,
	grVGAX_320x480x256,

	/* 256 color VGA/SuperVGA video modes */
    grSVGA_320x200x256,
	grSVGA_320x240x256,
	grSVGA_320x400x256,
	grSVGA_320x480x256,
	grSVGA_400x300x256,
	grSVGA_512x384x256,
	grSVGA_640x350x256,
	grSVGA_640x400x256,
	grSVGA_640x480x256,
	grSVGA_800x600x256,
	grSVGA_1024x768x256,
	grSVGA_1152x864x256,
	grSVGA_1280x960x256,
	grSVGA_1280x1024x256,
	grSVGA_1600x1200x256,

	/* 32,768 color Super VGA video modes */
    grSVGA_320x200x32k,
	grSVGA_320x240x32k,
	grSVGA_320x400x32k,
	grSVGA_320x480x32k,
	grSVGA_400x300x32k,
	grSVGA_512x384x32k,
	grSVGA_640x350x32k,
    grSVGA_640x400x32k,
    grSVGA_640x480x32k,
    grSVGA_800x600x32k,
    grSVGA_1024x768x32k,
	grSVGA_1152x864x32k,
	grSVGA_1280x960x32k,
	grSVGA_1280x1024x32k,
    grSVGA_1600x1200x32k,

	/* 65,536 color Super VGA video modes */
    grSVGA_320x200x64k,
	grSVGA_320x240x64k,
	grSVGA_320x400x64k,
	grSVGA_320x480x64k,
	grSVGA_400x300x64k,
	grSVGA_512x384x64k,
	grSVGA_640x350x64k,
	grSVGA_640x400x64k,
	grSVGA_640x480x64k,
	grSVGA_800x600x64k,
	grSVGA_1024x768x64k,
	grSVGA_1152x864x64k,
	grSVGA_1280x960x64k,
	grSVGA_1280x1024x64k,
	grSVGA_1600x1200x64k,

	/* 16 million color, 24 bits per pixel Super VGA video modes */
    grSVGA_320x200x16m,
	grSVGA_320x240x16m,
	grSVGA_320x400x16m,
	grSVGA_320x480x16m,
	grSVGA_400x300x16m,
	grSVGA_512x384x16m,
	grSVGA_640x350x16m,
	grSVGA_640x400x16m,
	grSVGA_640x480x16m,
	grSVGA_800x600x16m,
	grSVGA_1024x768x16m,
	grSVGA_1152x864x16m,
	grSVGA_1280x960x16m,
	grSVGA_1280x1024x16m,
	grSVGA_1600x1200x16m,

	/* 16 million color, 32 bits per pixel Super VGA video modes */
    grSVGA_320x200x4G,
	grSVGA_320x240x4G,
	grSVGA_320x400x4G,
	grSVGA_320x480x4G,
	grSVGA_400x300x4G,
	grSVGA_512x384x4G,
	grSVGA_640x350x4G,
	grSVGA_640x400x4G,
	grSVGA_640x480x4G,
	grSVGA_800x600x4G,
	grSVGA_1024x768x4G,
	grSVGA_1152x864x4G,
	grSVGA_1280x960x4G,
	grSVGA_1280x1024x4G,
	grSVGA_1600x1200x4G,

	/* Render into Windowing System DC (Windows, OS/2 PM, X11) */
    grWINDOWED,

    /* Maximum mode number                  */
    grMAXMODE,                  
	} MGL_modeType;


/****************************************************************************
REMARKS:
Defines the error codes returned by MGL_result. If a function fails for
any reason, you can check the error code return by MGL_result to determine
the cause of the failure, or display an error message to the user with
MGL_errorMsg.

HEADER:
mgraph.h

MEMBERS:
grOK            - No error
grNoInit        - Graphics driver has not been installed
grNotDetected   - Graphics hardware was not detected
grDriverNotFound- Graphics driver file not found
grBadDriver     - File loaded was not a graphics driver
grLoadMem       - Not enough memory to load graphics driver
grInvalidMode   - Invalid graphics mode for selected driver
grError         - General graphics error
grInvalidName   - Invalid driver name
grNoMem         - Not enough memory to perform operation
grNoModeSupport - Select video mode not supported by hard.
grInvalidFont   - Invalid font data
grBadFontFile   - File loaded was not a font file
grFontNotFound  - Font file was not found
grOldDriver     - Driver file is an old version
grInvalidDevice - Invalid device for selected operation
grInvalidDC     - Invalid device context
grInvalidCursor - Invalid cursor file
grCursorNotFound- Cursor file was not found
grInvalidIcon   - Invalid icon file
grIconNotFound  - Icon file was not found
grInvalidBitmap - Invalid bitmap file
grBitmapNotFound- Bitmap file was not found
grZbufferTooBig - Zbuffer allocation is too large
grNewFontFile   - Only Windows 2.x font files supported
grNoDoubleBuff  - Double buffering is not available
grNoHardwareBlt - No hardware bitBlt for OffscreenDC
grNoOffscreenMem- No available offscreen memory
grInvalidPF     - Invalid pixel format for memory DC
grLastError     - Last error number in list
****************************************************************************/
typedef enum {
    grOK            = 0,
    grNoInit        = -1,
    grNotDetected   = -2,
    grDriverNotFound= -3,
    grBadDriver     = -4,
    grLoadMem       = -5,
    grInvalidMode   = -6,
    grError         = -8,
    grInvalidName   = -9,
    grNoMem         = -10,
    grNoModeSupport = -11,
    grInvalidFont   = -12,
    grBadFontFile   = -13,
    grFontNotFound  = -14,
    grOldDriver     = -15,
    grInvalidDevice = -16,
    grInvalidDC     = -17,
    grInvalidCursor = -18,
    grCursorNotFound= -19,
    grInvalidIcon   = -20,
    grIconNotFound  = -21,
    grInvalidBitmap = -22,
    grBitmapNotFound= -23,
    grZbufferTooBig = -24,
    grNewFontFile   = -25,
    grNoDoubleBuff  = -26,
    grNoHardwareBlt = -28,
    grNoOffscreenMem= -29,
    grInvalidPF     = -30,
    grLastError     = -31,
	} MGL_errorType;

/****************************************************************************
REMARKS:
Defines flags to enable and disable clipping via MGL_setClipMode.

HEADER:
mgraph.h
****************************************************************************/
typedef enum {
    MGL_CLIPOFF     = 0,
    MGL_CLIPON      = 1,
    } MGL_clipType;

/****************************************************************************
REMARKS:
Defines the color mapped modes. By default the MGL starts up in the default
mode, however you can change the color map mode to use RGB halftone
dithering with the MGL_setColorMapMode function. Note that if you do this,
you must also re-program the color palette to use the RGB halftone palette
returned by MGL_getHalfTonePalette.

HEADER:
mgraph.h

MEMBERS:
MGL_CMAP_MODE       - Normal Color mapped mode
MGL_DITHER_RGB_MODE - 24 bit RGB halftone dithered
****************************************************************************/
typedef enum {
    MGL_CMAP_MODE,
    MGL_DITHER_RGB_MODE,
	} MGL_colorModes;


/****************************************************************************
REMARKS:
Defines the MGL standard colors. This is the standard set of colors for the
IBM PC in DOS graphics modes. The default palette will have been programmed
to contain these values by the MGL when a graphics mode is started. If the
palette has been changed, the colors on the screen will not correspond to
the names defined here. Under a Windowing system (like Windows, OS/2 PM or
X Windows) these colors will not correspond to the default colors. For
Windows see the MGL_WIN_COLORS enumeration which defines the default colors
in a windowed mode.

HEADER:
mgraph.h
****************************************************************************/
typedef enum {
	MGL_BLACK,						/* dark colors	*/
	MGL_BLUE,
	MGL_GREEN,
	MGL_CYAN,
	MGL_RED,
	MGL_MAGENTA,
	MGL_BROWN,
	MGL_LIGHTGRAY,
	MGL_DARKGRAY,					/* light colors	*/
	MGL_LIGHTBLUE,
	MGL_LIGHTGREEN,
	MGL_LIGHTCYAN,
	MGL_LIGHTRED,
	MGL_LIGHTMAGENTA,
	MGL_YELLOW,
	MGL_WHITE,
    } MGL_COLORS;

/****************************************************************************
REMARKS:
Defines the Windows standard colors for 256 color graohics modes when in
a window. 8,9,246,247 are reserved and you should not count on these colors
always being the same. For 16 color bitmaps, colors 248-255 are mapped to
colors 8-15.

HEADER:
mgraph.h
****************************************************************************/
typedef enum {
	MGL_WIN_BLACK 			= 0,
	MGL_WIN_DARKRED			= 1,
	MGL_WIN_DARKGREEN		= 2,
	MGL_WIN_DARKYELLOW		= 3,
	MGL_WIN_DARKBLUE		= 4,
	MGL_WIN_DARKMAGENTA		= 5,
	MGL_WIN_DARKCYAN		= 6,
	MGL_WIN_LIGHTGRAY		= 7,
	MGL_WIN_TURQUOISE		= 8,		/* Reserved; dont count on this	*/
	MGL_WIN_SKYBLUE			= 9,		/* Reserved; dont count on this	*/
	MGL_WIN_CREAM			= 246,		/* Reserved; dont count on this	*/
	MGL_WIN_MEDIUMGRAY		= 247,		/* Reserved; dont count on this	*/
	MGL_WIN_DARKGRAY		= 248,
	MGL_WIN_LIGHTRED		= 249,
	MGL_WIN_LIGHTGREEN		= 250,
	MGL_WIN_LIGHTYELLOW		= 251,
	MGL_WIN_LIGHTBLUE		= 252,
	MGL_WIN_LIGHTMAGENTA	= 253,
    MGL_WIN_LIGHTCYAN		= 254,
	MGL_WIN_WHITE 			= 255,
	} MGL_WIN_COLORS;    

/****************************************************************************
REMARKS:
Defines the marker types passed to MGL_setMarkerStyle

HEADER:
mgraph.h

MEMBERS:
MGL_MARKER_SQUARE   - Draw a solid square at the marker location
MGL_MARKER_CIRCLE   - Draw a solid circle at the marker location
MGL_MARKER_X        - Draw a cross (X) at the marker location
****************************************************************************/
typedef enum {
	MGL_MARKER_SQUARE,
	MGL_MARKER_CIRCLE,
	MGL_MARKER_X,
	} MGL_markerStyleType;

/****************************************************************************
REMARKS:
Defines the Windows standard colors for 256 color graohics modes when in
a window. 8,9,246,247 are reserved and you should not count on these colors
always being the same. For 16 color bitmaps, colors 248-255 are mapped to
colors 8-15.

HEADER:
mgraph.h

MEMBERS:
MGL_REPLACE_MODE	- Replace mode
MGL_AND_MODE		- AND mode
MGL_OR_MODE			- OR mode
MGL_XOR_MODE		- XOR mode
****************************************************************************/
typedef enum {
	MGL_REPLACE_MODE,
	MGL_AND_MODE,
	MGL_OR_MODE,
	MGL_XOR_MODE,
	} MGL_writeModeType;

/****************************************************************************
REMARKS:
Defines the pen styles passed to MGL_setPenStyle. These styles define the
different fill styles that can be used when the filling the interior of
filled primitives and also the outline of non-filled primitives.

HEADER:
mgraph.h

MEMBERS
MGL_BITMAP_SOLID        - Fill with a solid color
MGL_BITMAP_OPAQUE       - Fill with an opaque bitmap pattern. Where bits in
                          the pattern are a 1, the foreground color is used.
                          Where bits in the pattern are a 0, the background
                          color is used. The pattern itself is defined as an
                          8x8 monochrome bitmap.
MGL_BITMAP_TRANSPARENT  - Fill with a transparent bitmap pattern. Where bits
                          in the pattern are a 1, the foreground color is
                          used. Where bits in the pattern are a 0, the pixel
                          is left unmodified on the screen. The pattern itself
                          is defined as an 8x8 monochrome bitmap.
MGL_PIXMAP              - Fill with a color pixmap pattern. The pixmap pattern
                          is defined as an 8x8 array of color_t values,
                          where each entry corresponds to the color values
                          packed for the appropriate color mode (ie: a color
                          index in color map modes and a packed RGB value in
                          HiColor and TrueColor modes).
****************************************************************************/
typedef enum {
	MGL_BITMAP_SOLID,
	MGL_BITMAP_OPAQUE,
	MGL_BITMAP_TRANSPARENT,
	MGL_PIXMAP,
    } MGL_penStyleType;

/****************************************************************************
REMARKS:
Defines the line styles passed to MGL_setLineStyle.

HEADER:
mgraph.h

MEMBERS:
MGL_LINE_PENSTYLE   - Line drawn in current pen style
MGL_LINE_STIPPLE    - Line drawn with current line stipple pattern. The
                      line stipple pattern is a 16x1 pattern that defines
                      which pixels should be drawn in the line. Where a bit
                      is a 1 in the pattern, a pixel will be drawn and where
                      a bit is a 0 in the pattern, the pixel will be left
                      untouched on the screen.
****************************************************************************/
typedef enum {
    MGL_LINE_PENSTYLE,
    MGL_LINE_STIPPLE,
	} MGL_lineStyleType;

/****************************************************************************
REMARKS:
Defines the polygon types passed to MGL_setPolygonType.

HEADER:
mgraph.h

MEMBERS:
MGL_CONVEX_POLYGON  - Monotone vertical polygon (includes convex polygons).
                      A monotone vertical polygon is one whereby there will
                      never be a horizontal line that can intersect the
                      polygon at more than two edges at a time. Note that
                      if you set the polygon type to this value and you
                      pass it a non-monotone vertical polygon, the output
                      results are undefined.
MGL_COMPLEX_POLYGON - Non-Simple polygons. When set to this mode the MGL
                      will correctly rasterise all polygon types that you
                      pass to it, however the drawing will be slower.
MGL_AUTO_POLYGON    - Auto detect the polygon type. In this mode the MGL
                      will examine the polygon vertices passed in and will
                      automatically draw it with the faster routines if it
                      is monotone vertical. Note that this does incur an
                      overhead for the checking code, so if you know all
                      your polygons are monotone vertical or convex, then
                      you should set the type to MGL_CONVEX_POLYGON.
****************************************************************************/
typedef enum {
    MGL_CONVEX_POLYGON,
    MGL_COMPLEX_POLYGON,
    MGL_AUTO_POLYGON,
	} MGL_polygonType;

/****************************************************************************
REMARKS:
Defines the text justification styles passed to MGL_setTextJustify.

HEADER:
mgraph.h

MEMBERS:
MGL_LEFT_TEXT       - Justify from left
MGL_TOP_TEXT        - Justify from top
MGL_CENTER_TEXT     - Center the text
MGL_RIGHT_TEXT      - Justify from right
MGL_BOTTOM_TEXT     - Justify from bottom
MGL_BASELINE_TEXT   - Justify from the baseline
****************************************************************************/
typedef enum {
    MGL_LEFT_TEXT       =   0,
    MGL_TOP_TEXT        =   0,
    MGL_CENTER_TEXT     =   1,
    MGL_RIGHT_TEXT      =   2,
    MGL_BOTTOM_TEXT     =   2,
    MGL_BASELINE_TEXT   =   3,
	} MGL_textJustType;

/****************************************************************************
REMARKS:
Defines the text direction styles passed to MGL_setTextDirection

HEADER:
mgraph.h

MEMBERS:
MGL_LEFT_DIR        - Text goes to left
MGL_UP_DIR          - Text goes up
MGL_RIGHT_DIR       - Text goes right
MGL_DOWN_DIR        - Text goes down
****************************************************************************/
typedef enum {
    MGL_LEFT_DIR        =   0,
    MGL_UP_DIR          =   1,
    MGL_RIGHT_DIR       =   2,
    MGL_DOWN_DIR        =   3,
    } MGL_textDirType;

/****************************************************************************
REMARKS:
Defines the different font types

HEADER:
mgraph.h

MEMBERS:
MGL_VECTORFONT  - Vector font
MGL_FIXEDFONT   - Fixed width bitmap font
MGL_PROPFONT    - Proportional width bitmap font
****************************************************************************/
typedef enum {
    MGL_VECTORFONT = 1,
    MGL_FIXEDFONT,
    MGL_PROPFONT,
    } MGL_fontType;

/****************************************************************************
REMARKS:
Defines the different palette rotation directions

HEADER:
mgraph.h

MEMBERS:
MGL_ROTATE_UP   - Rotate the palette values up
MGL_ROTATE_DOWN - Rotate the palette values down
****************************************************************************/
typedef enum {
    MGL_ROTATE_UP,
    MGL_ROTATE_DOWN,
	} MGL_palRotateType;

/****************************************************************************
REMARKS:
Defines the border drawing styles passed to MGL_drawBorderCoord.

HEADER:
mgraph.h

MEMBERS:
MGL_BDR_INSET   - Interior is inset into screen
MGL_BDR_OUTSET  - Interior is outset from screen
MGL_BDR_OUTLINE - Border is 3d outline
****************************************************************************/
typedef enum {
    MGL_BDR_INSET,
    MGL_BDR_OUTSET,
    MGL_BDR_OUTLINE,
	} MGL_bdrStyleType;

/****************************************************************************
REMARKS:
Defines for waitVRT flag for MGL_setVisualPage, MGL_swapBuffers and
MGL_glSwapBuffers.

HEADER:
mgraph.h

MEMBERS:
MGL_tripleBuffer    - Triple buffer. This mode enables hardware or software
					  triple buffering if available on the target system.
					  In this case when triple buffering is available the
					  MGL will ensure that there is no flicker when flipping
					  pages, however your frame rate will run at the maximum
					  rate until you get to the physical refresh rate of
					  the screen (ie: 60fps or higher). Note that if there
					  is no hardware or software triple buffering available,
					  this function will work like regular double buffering.
					  Note also that you /must/ have at least 3 pages
					  available for triple buffering to work.
MGL_waitVRT         - Wait for vertical retrace. This mode always waits for
					  the vertical retrace when swapping display pages, and
					  is required if only have two pages available to avoid
					  flicker during animation.
MGL_dontWait        - Don't wait for retrace. This mode simply programs
					  the display start address change and returns. This may
					  cause flicker on the screen during animation, and is
					  mostly useful for debugging and testing purposes to
					  see what the raw framerate of an animation is. Also
					  if you dont have hardware or software triple available,
					  and you allocate at least 3 pages you can achieve the
					  effect of triple buffering if you know that the frame
					  rate of your animation will not exceed the refresh
					  rate of the screen.
****************************************************************************/
typedef enum {
	MGL_tripleBuffer    = 0,
	MGL_waitVRT         = 1,
	MGL_dontWait        = 2,
	} MGL_waitVRTFlagType;

/****************************************************************************
REMARKS:
Defines the flags passed to MGL_setRefreshRate. You can pass the value
of MGL_DEFAULT_REFRESH to set the refresh rate to the adapter default. 

HEADER:
mgraph.h

MEMBERS:
MGL_DEFAULT_REFRESH	- Use the default refresh rate for the graphics mode
MGL_INTERLACED_MODE	- Set the mode to be interlaced (not always supported)
****************************************************************************/
typedef enum {
	MGL_DEFAULT_REFRESH	= -1,
	MGL_INTERLACED_MODE	= 0x4000,
	} MGL_refreshRateType;

/* Standard fullscreen display driver names 	*/

#define	MGL_VGA4NAME		"VGA4.DRV"		/* Standard VGA drivers			*/
#define	MGL_VGA8NAME		"VGA8.DRV"
#define	MGL_VGAXNAME		"VGAX.DRV"

#define	MGL_SVGA4NAME		"SVGA4.DRV"		/* Generic SuperVGA drivers		*/
#define	MGL_SVGA8NAME		"SVGA8.DRV"
#define	MGL_SVGA16NAME		"SVGA16.DRV"
#define	MGL_SVGA24NAME		"SVGA24.DRV"
#define	MGL_SVGA32NAME		"SVGA32.DRV"

#define	MGL_LINEAR8NAME		"LINEAR8.DRV"	/* Linear framebuffer drivers	*/
#define	MGL_LINEAR16NAME	"LINEAR16.DRV"
#define	MGL_LINEAR24NAME	"LINEAR24.DRV"
#define	MGL_LINEAR32NAME	"LINEAR32.DRV"

#define	MGL_ACCEL8NAME		"ACCEL8.DRV"	/* VBE/AF Accelerated drivers	*/
#define	MGL_ACCEL16NAME		"ACCEL16.DRV"
#define	MGL_ACCEL24NAME		"ACCEL24.DRV"
#define	MGL_ACCEL32NAME		"ACCEL32.DRV"

#define	MGL_FULLDIB8NAME	"FDIB8.DRV"		/* Fullscreen DIB drivers		*/
#define	MGL_FULLDIB16NAME	"FDIB16.DRV"
#define	MGL_FULLDIB24NAME	"FDIB24.DRV"
#define	MGL_FULLDIB32NAME	"FDIB32.DRV"

#define	MGL_DDRAW8NAME		"DDRAW8.DRV"	/* DirectDraw drivers			*/
#define	MGL_DDRAW16NAME		"DDRAW16.DRV"
#define	MGL_DDRAW24NAME		"DDRAW24.DRV"
#define	MGL_DDRAW32NAME		"DDRAW32.DRV"

#define	MGL_OPENGLNAME		"OPENGL.DRV"	/* Hardware OpenGL drivers		*/

#define	MGL_FSOGL8NAME		"FSOGL8.DRV"	/* Fullsreen OpenGL drivers		*/
#define	MGL_FSOGL16NAME		"FSOGL16.DRV"
#define	MGL_FSOGL24NAME		"FSOGL24.DRV"
#define	MGL_FSOGL32NAME		"FSOGL32.DRV"

/* Standard windowed display driver names 	*/

#define	MGL_WDDRAW8NAME		"WDDRAW8.DRV"	/* DirectDraw windowed drivers	*/
#define	MGL_WDDRAW16NAME	"WDDRAW16.DRV"
#define	MGL_WDDRAW24NAME	"WDDRAW24.DRV"
#define	MGL_WDDRAW32NAME	"WDDRAW32.DRV"

/* Standard memory driver names 	*/

#define	MGL_PACKED1NAME		"PACK1.DRV"
#define	MGL_PACKED4NAME		"PACK4.DRV"
#define	MGL_PACKED8NAME		"PACK8.DRV"
#define	MGL_PACKED16NAME	"PACK16.DRV"
#define	MGL_PACKED24NAME	"PACK24.DRV"
#define	MGL_PACKED32NAME	"PACK32.DRV"

/* Standard bitmap names	*/

#define	MGL_EMPTY_FILL	_MGL_getEmptyPat()
#define	MGL_GRAY_FILL	_MGL_getGrayPat()
#define	MGL_SOLID_FILL	_MGL_getSolidPat()

/* Event message masks for keyDown events */

#define	EVT_ASCIIMASK	0x00FF	/* Ascii code of key pressed			*/
#define	EVT_SCANMASK	0xFF00	/* Scan code of key pressed				*/
#define	EVT_COUNTMASK	0x7FFF0000L		/* Count for KEYREPEAT's		*/

/* Macros to extract values from the message fields */

#define	EVT_asciiCode(m)	( (uchar) (m & EVT_ASCIIMASK) )
#define	EVT_scanCode(m)		( (uchar) ( (m & EVT_SCANMASK) >> 8 ) )
#define	EVT_repeatCount(m)	( (short) ( (m & EVT_COUNTMASK) >> 16 ) )

/****************************************************************************
REMARKS:
Defines the event message masks for mouse events

HEADER:
mgraph.h

MEMBERS:
EVT_LEFTBMASK   - Left button is held down
EVT_RIGHTBMASK  - Right button is held down
EVT_BOTHBMASK   - Both left and right held down together
EVT_ALLBMASK    - All buttons pressed
****************************************************************************/
typedef enum {
    EVT_LEFTBMASK   = 0x0001,
    EVT_RIGHTBMASK  = 0x0004,
    EVT_BOTHBMASK   = 0x0005,
    EVT_ALLBMASK    = 0x0005,
    } MGL_eventMsgMaskType;

/****************************************************************************
REMARKS:
Defines the event modifier masks. These are the masks used to extract
the modifier information from the modifiers field of the event_t structure.
Note that the values in the modifiers field represent the values of these
modifier keys at the time the event occurred, not the time you decided
to process the event.

HEADER:
mgraph.h

MEMBERS:
EVT_LEFTBUT     - Set if left mouse button was down
EVT_RIGHTBUT    - Set if right mouse button was down
EVT_RIGHTSHIFT  - Set if right shift was down
EVT_LEFTSHIFT   - Set if left shift was down
EVT_CTRLSTATE   - Set if ctrl key was down
EVT_ALTSTATE    - Set if alt key was down
EVT_LEFTCTRL    - Set if left ctrl key was down
EVT_LEFTALT     - Set if left alt key was down
EVT_SHIFTKEY    - Mask for any shift key down
****************************************************************************/
typedef enum {
    EVT_LEFTBUT     = 0x0001,
    EVT_RIGHTBUT    = 0x0002,
    EVT_RIGHTSHIFT  = 0x0008,
    EVT_LEFTSHIFT   = 0x0010,
    EVT_CTRLSTATE   = 0x0020,
    EVT_ALTSTATE    = 0x0040,
    EVT_LEFTCTRL    = 0x0080,
    EVT_LEFTALT     = 0x0100,
    EVT_SHIFTKEY    = 0x0018,
    } MGL_eventModMaskType;

/****************************************************************************
REMARKS:
Defines the event codes returned in the event_t structures what field. Note
that these are defined as a set of mutually exlusive bit fields, so you
can test for multiple event types using the combined event masks defined
in the MGL_eventMaskType enumeration.

HEADER:
mgraph.h

MEMBERS:
EVT_NULLEVT     - A null event
EVT_KEYDOWN     - Key down event
EVT_KEYREPEAT   - Key repeat event
EVT_KEYUP       - Key up event
EVT_MOUSEDOWN   - Mouse down event
EVT_MOUSEUP     - Mouse up event
EVT_MOUSEMOVE   - Mouse movement event
EVT_TIMERTICK   - Timer tick event
EVT_USEREVT     - First user event
****************************************************************************/
typedef enum {
    EVT_NULLEVT     = 0x0000,
    EVT_KEYDOWN     = 0x0001,
    EVT_KEYREPEAT   = 0x0002,
    EVT_KEYUP       = 0x0004,
    EVT_MOUSEDOWN   = 0x0008,
    EVT_MOUSEUP     = 0x0010,
    EVT_MOUSEMOVE   = 0x0020,
    EVT_TIMERTICK   = 0x0040,
    EVT_USEREVT     = 0x0080,
    } MGL_eventType;

/****************************************************************************
REMARKS:
Defines the event code masks you can use to test for multiple types of
events, since the event codes are mutually exlusive bit fields.

HEADER:
mgraph.h

MEMBERS:
EVT_KEYEVT      - Mask for any key event
EVT_MOUSEEVT    - Mask for any mouse event
EVT_MOUSECLICK  - Mask for any mouse click event
EVT_EVERYEVT    - Mask for any event
****************************************************************************/
typedef enum {
    EVT_KEYEVT      = (EVT_KEYDOWN | EVT_KEYREPEAT | EVT_KEYUP),
    EVT_MOUSEEVT    = (EVT_MOUSEDOWN | EVT_MOUSEUP | EVT_MOUSEMOVE),
    EVT_MOUSECLICK  = (EVT_MOUSEDOWN | EVT_MOUSEUP),
    EVT_EVERYEVT    = 0xFFFF,
    } MGL_eventMaskType;

/****************************************************************************
REMARKS:
Defines the suspend application callback flags, passed to the suspend
application callback registered with the MGL. This callback is called
when the user presses one of the system key sequences indicating that
they wish to change the active application. The MGL will catch these
events and if you have registered a callback, will call the callback to
save the state of the application so that it can be properly restored
when the user switches back to your application. The MGL takes care of
all the details about saving and restoring the state of the hardware,
and all your application needs to do is save its own state so that you can
re-draw the application screen upon re-activation.

Note: Your application suspend callback may get called twice with the
      MGL_DEACTIVATE flag in order to test whether the switch should
      occur (under both DirectX and WinDirect fullscreen modes).

Note: When your callback is called with the MGL_DEACTIVATE flag, you
      cannot assume that you have access to the display memory surfaces
      as they may have been lost by the time your callback has been called.

HEADER:
mgraph.h

MEMBERS:
MGL_DEACTIVATE  - Application losing active focus
MGL_REACTIVATE  - Application regaining active focus
****************************************************************************/
typedef enum {
    MGL_DEACTIVATE  = 0x0001,
    MGL_REACTIVATE  = 0x0002,
    } MGL_suspendAppFlagsType;

/****************************************************************************
REMARKS:
Defines the return codes that the application can return from the suspend
application callback registered with the MGL. The default value to be
returned is MGL_SUSPEND_APP and this will cause the application execution to
be suspended until the application is re-activated again by the user. During
this time the application will exist on the task bar under Windows 95 and
Windows NT in minimised form.

MGL_NO_DEACTIVATE signals to the MGL that the application does not want
to allow switching to occur, and the switch request will be ignored and
the app will remain in fullscreen mode. This is valid in both WinDirect
and DirectX modes under Windows 3.1 and Windows 95, and allows you to give
power users the option of disabling the ability to switch away from the
application back to the desktop. Note however that under Windows NT this
is not possible, so even if you return this under NT the switch will
still occur.

MGL_NO_SUSPEND_APP can be used to tell the MGL to switch back to the
Windows desktop, but not to suspend the applications execution. This must
be used with care as the suspend application callback is then responsible
for setting a flag in the application that will /stop/ the application from
doing any rendering directly to the framebuffer while the application is
minimised on the task bar (since the application no lonter owns the screen!).
This return value is most useful for networked games that need to maintain
network connectivity while the user has temporarily switched back to the
Windows desktop. Hence you can ensure that you main loop continues to run,
including networking and AI code, but no drawing occurs to the screen.

Note: The MGL ensures that your application will /never/ be switched away
      from outside of a message processing loop. Hence as long as you do
      not process messages inside your drawing loops, you will never lose
      the active focus (and your surfaces) while drawing, but only during
      event processing.

HEADER:
mgraph.h

MEMBERS:
MGL_NO_DEACTIVATE   - Don't allow app to be deactivated
MGL_SUSPEND_APP     - Suspend application execution until restored
MGL_NO_SUSPEND_APP  - Don't suspend execution, but allow switch
****************************************************************************/
typedef enum {
	MGL_NO_DEACTIVATE   = 0,
	MGL_SUSPEND_APP     = 1,
	MGL_NO_SUSPEND_APP  = 2,
	} MGL_suspendAppCodesType;

/****************************************************************************
REMARKS:
Structure describing an integer point passed to the MGL.

HEADER:
mgraph.h

MEMBERS:
x   - X coordinate for the point
y   - Y coordinate for the point
****************************************************************************/
typedef struct {
	int	x,y;
	} point_t;

/****************************************************************************
REMARKS:
Structure describing an integer rectangle. Note that MGL defines and uses
rectangles such that the bottom and right coordinates are not actually
included in the pixels that define a raster coordinate rectangle. This
allows for correct handling of overlapping rectangles without drawing any
pixels twice.

HEADER:
mgraph.h

MEMBERS:
left    - Left coordinate of the rectangle
top     - Top coordinate of the rectangle
right	- Right coordinate of the rectangle
bottom  - Bottom coordinate of the rectangle
****************************************************************************/
typedef struct {
	int	left;
	int	top;
	int	right;
	int	bottom;
	} rect_t;

/****************************************************************************
REMARKS:
Type definition for all color values used in MGL. All color values are 32
bits wide, and can be either a 4 or 8 bit color index, or a packed RGB tuple
depending on the pixel format for the display mode. For packed RGB display
modes, the colors may contain 15, 16, 24 or 32 bits of color information, and
the format of the RGB colors is stored in the pixel_format_t structure. You
should use the MGL_packColor family of functions to encode color values in
RGB modes, and use the MGL_unpackColor family of functions to extract color
values in RGB modes.

HEADER:
mgraph.h
****************************************************************************/
typedef ulong       color_t;

#ifndef __FX_FIXED_H
/****************************************************************************
REMARKS:
Type definition for all standard 32-bit fixed point values used in MGL.
Standard fixed point values are 32-bits wide, and represented in 16.16 fixed
point format (16 bits of integer, 16 bits of fraction). These numbers can
represent signed numbers from +32767.9 to -32768.9.

Note: If you are doing fixed point arithmetic for screen coordinate
	  calculations, be very careful of overflow conditions when doing
	  multiplication operations.

HEADER:
mgraph.h
****************************************************************************/
typedef long            fix32_t;
#else
#define fix32_t         FXFixed
#endif

/****************************************************************************
REMARKS:
Structure describing a 16.16 fixed point coordinate.

HEADER:
mgraph.h

MEMBERS:
x   - Fixed point x coordinate
y   - Fixed point y coordinate
****************************************************************************/
typedef struct {
	fix32_t x,y;
	} fxpoint_t;

/* Macros to convert between integer and 32 bit fixed point format */

#define	MGL_FIX_1			0x10000L
#define	MGL_FIX_2			0x20000L
#define	MGL_FIX_HALF		0x08000L
#define	MGL_TOFIX(i)		((long)(i) << 16)
#define MGL_FIXTOINT(f)		((int)((f) >> 16))
#define	MGL_FIXROUND(f) 	((int)(((f) + MGL_FIX_HALF) >> 16))

/* Region structure */

#ifdef	BUILD_MGL
/* {secret} */
struct _span_t;
/* {secret} */
typedef struct _span_t  span_t;
#else
/* {secret} */
typedef void        span_t;
#endif

/****************************************************************************
REMARKS:
Structure representing a complex region. Complex regions are used to
represent non-rectangular areas as unions of smaller rectangles (the
smallest being a single pixel). You can use complex regions to build
complex clipping regions for user interface library development (such as
the SciTech MegaVision Library which makes extensive use of the MGL's
region management functions).

HEADER:
mgraph.h

MEMBERS:
rect    - Bounding rectangle for the region. If the spans field below is NULL,
          then the region is a simple region and is composed of only a single
          rectangle. Note however that you can have a simple region that
          consists of only single rectangle in the span structure (usually
          after complex region arithmetic). You can use the
          MGL_isSimpleRegion function to determine if the region contains
          only a single rectangle.
spans   - Pointer to the internal region span structure.
****************************************************************************/
typedef struct {
    rect_t      rect;
    span_t      *spans;
	} region_t;

/****************************************************************************
REMARKS:
Structure representing a single color palette entry. Color palette entries
are used to build the color lookup tables for all the device contexts used
in the MGL, which are used to define the final color values for colors in
color index modes (8-bits per pixel and below). Color palette information is
always stored in 8-bits per primary format (ie: 8-bits of red, green and blue
information), and will be converted by MGL to the appropriate color format
used by the underlying hardware when the hardware palette is programmed.
Hence in standard VGA modes (which only use 6-bits per primary) the bottom
two bits of color information will be lost when the palette is programmed.

HEADER:
mgraph.h

MEMBERS:
blue    - Blue channel color (0 - 255)
green   - Green channel color (0 - 255)
red     - Red channel color (0 - 255)
alpha   - Alignment value (not used and should always be 0)
****************************************************************************/
#pragma pack(1)
typedef struct {
    uchar   blue;
    uchar   green;
    uchar   red;
    uchar   alpha;
	} palette_t;
#pragma pack(4)

/* Maximum value for each palette entry component */

#define	PALMAX		255				/* Max value for palette components	*/

/****************************************************************************
REMARKS:
Structure representing the format of an RGB pixel. This structure is used
to describe the current RGB pixel format used by all MGL device contexts
with pixel depths greater than or equal to 15-bits per pixel. The pixel
formats for 15 and 16-bit modes are constant and never change, however there
are 2 possible pixel formats for 24 bit RGB modes and 4 possible formats for
32 bit RGB modes that are supported by the MGL. The possible modes for
24-bits per pixel are:

24-bit		- Description
RGB			- Values are packed with Red in byte 2, Green in byte 1 and Blue
			  in byte 0. This is the standard format used by all 24 bit
			  Windows BMP files, and the native display format for most
			  graphics hardware on the PC.
BGR			- Values are packed with Blue in byte 2, Green in byte 1 and Red
			  in byte 0. This format is the native display format for some
			  graphics hardware on the PC.

The possible modes for 32-bits per pixel are:

32-bit		- Description
ARGB		- Values are packed with Red in byte 2, Green in byte 1 and Blue
			  in byte 0 and alpha in byte 3.
ABGR		- Values are packed with Blue in byte 2, Green in byte 1 and Red
			  in byte 0 and alpha in byte 3.
RGBA		- Values are packed with Red in byte 3, Green in byte 2 and Blue
			  in byte 1 and alpha in byte 0.
BGRA		- Values are packed with Blue in byte 3, Green in byte 2 and Red
              in byte 1 and alpha in byte 0.

If you intend to write your own direct rendering code for 15-bits per pixel
and higher graphics modes, you will need to write your code so that it will
adapt to the underlying pixel format used by the hardware to display the
correct colors on the screen. The MGL will perform pixel format translation
on the fly for MGL_bitBlt operations, but this can be time consuming. The
formula for packing the pixel data into the proper positions given three
8-bit RGB values is as follows:

	color = ((color_t)((R >> redAdjust) & redMask)
			<< redPos)
		  | ((color_t)((G >> greenAdjust) & greenMask)
			<< greenPos)
		  | ((color_t)((B >> blueAdjust) & blueMask)
		    << bluePos);

Alternatively you can unpack the color values with the following code:

	R = (((color) >> redPos) & redMask)
		<< redAdjust;
	G = (((color) >> greenPos) & greenMask)
		<< greenAdjust;
	B = (((color) >> bluePos) & blueMask)
	    << blueAdjust;

Note: For 32-bit modes, the alpha channel information is unused, but should
      always be set to zero. Some hardware devices interpret the alpha
      channel information so unless you use a value of zero, you will get
      some strange looking results on the screen.

HEADER:
mgraph.h

MEMBERS:
redMask     - Unshifted 8 bit mask for the red color channel, and will be
              5-bits wide for a 5-bit color channel or 8-bits wide for an
              8-bit color channel.
greenMask	- Unshifted 8 bit mask for the green color channel.
blueMask	- Unshifted 8 bit mask for the blue color channel.
rsvdMask	- Unshifted 8 bit mask for the reserved or alpha channel.
redPos      - Bit position for bit 0 of the red color channel information.
redAdjust	- Number of bits to shift the 8 bit red value right
greenPos	- Bit position for bit 0 of the green color channel information.
greenAdjust	- Number of bits to shift the 8 bit green value right
bluePos     - Bit position for bit 0 of the blue color channel information.
blueAdjust	- Number of bits to shift the 8 bit blue value right
rsvdPost	- Bit position for bit 0 of the reserved channel information
rsvdAdjust  - Number of bits to shift the 32 bit reserved value right
****************************************************************************/
typedef struct {
    uint    redMask,greenMask;
	uint	blueMask,rsvdMask;
    int     redPos,redAdjust;
    int     greenPos,greenAdjust;
    int     bluePos,blueAdjust;
    int     rsvdPos,rsvdAdjust;
	} pixel_format_t;

/****************************************************************************
REMARKS:
Structure used to return elliptical arc starting and ending coordinates.
This structure is used to obtain the exact center, starting and ending
coordinates after an elliptical arc has been rasterized, so that you can
properly turn the arc into a pie slice for example.

HEADER:
mgraph.h

MEMBERS:
x       - x coordinate of the center of the elliptical arc
y       - y coordinate of the center of the elliptical arc
startX	- x coordinate of the starting pixel on the elliptical arc
startY	- y coordinate of the starting pixel on the elliptical arc
endX	- x coordinate of the ending pixel on the elliptical arc
endY    - y coordinate of the ending pixel on the elliptical arc
****************************************************************************/
typedef struct {
    int     x,y;
    int     startX,startY;
    int     endX,endY;
	} arc_coords_t;

/****************************************************************************
REMARKS:
Structure representing a loaded mouse cursor. This is the structure of the
mouse cursor data after it has been loaded from disk by the MGL, and is used
to set the mouse cursor shape. You can build your own mouse cursors manually
by filling in this structure.

The mouse cursor is drawn by the MGL by first using the cursor AND mask to
punch a hole in the background of the display surface, and then the cursor
XOR mask is XOR'ed into the display surface using the currently active mouse
cursor color. This method is compatible with the way that Microsoft Windows
displays mouse cursors on the screen.

Note however that the AND mask for the cursor is expected to have a 0 where
the background will be left alone and a 1 where the background should be set
to 0. This is the opposite of the mask definition stored in the Windows .CUR
cursor files, and MGL internally inverts this data when it loads the mouse
cursors. The main reason for this inversion is for performance reasons, since
the MGL needs to work with the mask stored this way internally for faster
internal drawing when drawing the cursor in software.

HEADER:
mgraph.h

MEMBERS:
xorMask     - 32x32 bit XOR pixel mask
andMask     - 32x32 bit AND pixel mask (see note above)
xHotSpot    - x coordinate of the mouse hotspot location. The mouse hotspot
              location is used to properly align the mouse cursor image to
              the actual mouse cursor position on the screen
yHotSpot    - y coordinate of the mouse hotspot location
****************************************************************************/
typedef struct {
	ulong		xorMask[32];
	ulong		andMask[32];
	int			xHotSpot;
	int			yHotSpot;
	} cursor_t;

/****************************************************************************
REMARKS:
Structure representing a loaded lightweight bitmap image. This is the
structure of Windows .BMP files after they have been loaded from disk with
the MGL_loadBitmap function. Lightweight bitmaps have very little memory
overhead once loaded from disk, since the entire bitmap information is storeds
in a single contiguous block of memory (although this is not necessary;
see below). However the only thing you can do with a lightweight bitmap is
display it to any MGL device context, using either stretching or transparency
(MGL_putBitmap, MGL_stretchBitmap, MGL_putBitmapTransparent). If you need to
be able to draw on the bitmap surface, then you should load the bitmap into
an MGL memory device context where you can call any of the standard MGL
drawing functions and BitBlt operations on the bitmap. The only disadvantage
of doing this is that a memory device context has a lot more memory overhead
involved in maintaining the device context information.

You can build you own lightweight bitmap loading routines by creating the
proper header information and loading the bitmap information into this
structure. Note that although the MGL loads the bitmap files from disk with
the bitmap surface, pixel format information and palette information all
loaded into a single memory block, this is not necessary. If you wish you
can create your own lightweight bitmaps with the bitmap surface allocated
in a separate memory block and then use this bitmap header to blast
information from this memory block to a device context as fast as possible.

HEADER:
mgraph.h

MEMBERS:
width           - Width of the bitmap in pixels
height          - Height of the bitmap in pixels
bitsPerPixel	- Pixel depth of the bitmap
bytesPerLine    - Scanline width for the bitmap. The scanline width must
                  always be aligned to a DWORD boundary, so the minimum
                  scanline width is 4 bytes.
Surface         - Pointer to the bitmap surface.
pal             - Pointer to the bitmap palette. If this field is NULL, the
                  bitmap does not have an associated palette.
pf              - Pointer to the bitmap pixel format info. This field will be
                  NULL for all bitmaps with 8 or less bits per pixel, but
                  will always be properly filled in for bitmaps with 15 or
                  more bits per pixel.
****************************************************************************/
typedef struct {
    int         width;
    int         height;
    int         bitsPerPixel;
    int         bytesPerLine;
    uchar       *surface;
    palette_t   *pal;
    pixel_format_t *pf;

	/* ... palette, pixel format and bitmap data are store contiguously */
	} bitmap_t;

/****************************************************************************
REMARKS:
Structure representing a loaded icon. Icons are used by the MGL to display
small, transparent bitmap images that can be of any dimension. The standard
Windows .ICO files can store icons in 32x32 and 64x64 formats, although the
MGL can load icons of any dimensions if you can find an editor that will
allow you to create large icons.

Icons are always drawn by the MGL by first using the icon AND mask to punch
a hole in the background of the display surface, and then the icon bitmap
XOR mask is XOR'ed into the display surface. This method is compatible
with the way that Microsoft Windows displays icons on the screen.

HEADER:
mgraph.h

MEMBERS:
byteWidth   - Width of the monochrome AND mask in bytes. Must be consistent
              with the bitmap width in the xorMask structure.
andMask     - Pointer to the AND mask information, which is stored
              contiguously in memory after the header block. The dimensions
              of the AND mask is defined by the dimensions of the xorMask
              bitmap image.
xorMask     - Bitmap image header block, containing information about the
              mask used to draw the icon image. The actual bitmap surface
              and palette data is stored contiguously in memory after the
              header block.
****************************************************************************/
typedef struct {
    int         byteWidth;
    uchar       *andMask;
    bitmap_t    xorMask;

	/* ... AND mask and bitmap structure are stored contiguously */
	} icon_t;

/* Default cursor name */

#define	MGL_DEF_CURSOR		_MGL_getDefCursor()

/* Size of font name in font structure */

#define	_MGL_FNAMESIZE		58

/****************************************************************************
REMARKS:
Structure representing a loaded MGL font file. MGL font files come in two
flavors, either vector fonts or bitmap fonts. Vector fonts represent the
characters in the font as a set of lines that are drawn, and vector fonts
can be scaled and rotated to any desired angle. Vector fonts however do not
look very good when rasterized in high resolution. Bitmap fonts represent
the characters in the font as small monochrome bitmaps, and can be either
fixed width fonts or proportional fonts.

The MGL can load both MGL 1.x style font files (vector and bitmap fonts) or
Windows 2.x style bitmap font files. For creating your own font files, you
should use any standard Windows font file editor and save the fonts in
Windows 2.x format (which is the standard format used by Windows 3.x,
Windows 95 and Windows NT for bitmap fonts).

HEADER:
mgraph.h

MEMBERS:
name        - Name of the font file (i.e. Helvetica Bold)
fontType    - Integer representing the type of font file. Will be one of
              the values defined by the MGL_fontType enumeration.
maxWidth	- Maximum character width in pixels
maxKern     - Maximum character kern value in pixels. The kern value for
              the character is the number of pixels it extends back past
              the character origin (such as the tail of the lowercase j
              character for some fonts).
fontWidth   - Font character width. This is the standard character width
              for the font, which may actually be smaller than the maximum
              width.
fontHeight  - Standard height of the font (not including the leading value).
ascent      - Font ascent value. The ascent value is the number of pixels
              that the font extends up from the font's baseline.
descent     - Font descent value. The descent value is the number of pixels
              that the font extends down from the font's baseline.
leading     - Font leading value. The leading value is the number of
              vertical pixels of space that are usually required between
              two lines of text drawn with this font.
****************************************************************************/
#pragma pack(1)
typedef struct {
    char            name[_MGL_FNAMESIZE];
    short           fontType;
    short           maxWidth;
    short           maxKern;
    short           fontWidth;
    short           fontHeight;
    short           ascent;
    short           descent;
    short           leading;
    } font_t;
#pragma pack(4)

/****************************************************************************
REMARKS:
Structure representing text metrics for a font or a single character, in the
current text attributes. For bitmap fonts you can get all the metric
information from the font_t structure, however for vector fonts, this routine
will provide the proper metrics for the font after being scaled by the
current font character scaling size. This structure is also used to obtain
specified 'tightest fit' metrics information about any character in the
font.

HEADER:
mgraph.h

MEMBERS:
width       - Actual width of the character in pixels
fontWidth   - Font character width, including any extra padding between this
              character and the next character. This value is used to advance
              the current position to the start of the next character, and
              can be larger than the actual character width (in order to put
              space between the characters).
fontHeight  - Standard height of the font (not including the leading value).
ascent      - Font or character ascent value. The ascent value is the number
              of pixels that the font extends up from the font's baseline.
descent     - Font or character descent value. The descent value is the
              number of pixels that the font extends down from the font's
              baseline.
leading     - Font leading value. The leading value is the number of
              vertical pixels of space that are usually required between
              two lines of text drawn with this font.
kern        - Character kern value. The kern value for the character is the
              number of pixels it extends back past the character origin
              (such as the tail of the lowercase j character for some fonts).
****************************************************************************/
typedef struct {
    int             width;
    int             fontWidth;
    int             fontHeight;
    int             ascent;
    int             descent;
    int             leading;
    int             kern;
	} metrics_t;

/****************************************************************************
REMARKS:
Structure representing the current text rasterizing settings. This structure
is used to group all these settings together in the MGL, and allows you to
save and restore the text rendering settings as a single unit.

HEADER:
mgraph.h

MEMBERS:
horizJust   - Horizontal justification value. Will be one of the values
              defined by the MGL_textJustType enumeration.
vertJust    - Vertical justification value. Will be one of the values
              defined by the MGL_textJustType enumeration.
dir         - Current text direction value. Will be one of the values
              defined by the MGL_textDirType enumeration.
szNumerx	- Current text x size numerator value
szNumery	- Current text y size numerator value
szDenomx	- Current text x size denominator value
szDenomy	- Current text y size denominator value
spaceExtra  - Current text space extra value. The space extra value is the
              number of extra pixels to be added to every space character
              when rendering the line of text.
font        - Pointer to current active font loaded in memory.
****************************************************************************/
typedef struct {
    int             horizJust;
    int             vertJust;
    int             dir;
    int             szNumerx;
    int             szNumery;
    int             szDenomx;
    int             szDenomy;
    int             spaceExtra;
    font_t          *font;
	} text_settings_t;

/* Macros to access the (left,top) and (right,bottom) points of a
 * rectangle.
 */

#define	MGL_leftTop(r)		(((point_t *) &(r))[0])
#define	MGL_rightBottom(r)	(((point_t *) &(r))[1])

/****************************************************************************
REMARKS:
Type definition for an 8x8 monochrome bitmap pattern. This is used to specify
the monochrome bitmap pattern used for filling solid objects when the pen
style is MGL_BITMAP_OPAQUE or MGL_BITMAP_TRANSPARENT.

When the pen style is MGL_BITMAP_OPAQUE, where bits in the pattern are a 1,
the foreground color is used. Where bits in the pattern are a 0, the
background color is used.

When the pen style is MGL_BITMAP_TRANSPARENT, where bits in the pattern are
a 1, the foreground color is used. Where bits in the pattern are a 0, the
pixel is left unmodified on the screen.

HEADER:
mgraph.h
****************************************************************************/
typedef uchar   pattern_t[8];

/****************************************************************************
REMARKS:
Type definition for an 8x8 color pixmap pattern. This is used to specify the
color pixmap pattern used for filling solid objects when the pen style is
in MGL_PIXMAP mode. The pixmap pattern is defined as an 8x8 array of
color_t values, where each entry corresponds to the color values packed for
the appropriate color mode (ie: a color index in color map modes and a
packed RGB value in HiColor and TrueColor modes).

HEADER:
mgraph.h
****************************************************************************/
typedef color_t pixpattern_t[8][8];

/****************************************************************************
REMARKS:
Structure representing the current MGL rendering attributes. This structure
groups all of the MGL rendering state variables, and can be used to save
and restore the entire MGL rendering state for any device context as a single
unit.

Note: You should only save and restore the state to the same device context!

HEADER:
mgraph.h

MEMBERS:
color           - Current foreground color
backColor       - Current background color
colorMode       - Current color map mode. Will be either MGL_CMAP_MODE or
                  MGL_DITHER_RGB_MODE. This value only affects 8 bit
                  rasterizing modes, and determines whether rasterizing
                  should be performed using straight color indexes or by
                  dithering 24 bit RGB values on the fly to 8 bit colors.
markerSize      - Current marker size
markerStyle     - Current marker style. Will be one of values defined by the
                  MGL_markerStyleType enumeration.
markerColor     - Current marker color
bdrBright       - Current border bright color
bdrDark         - Current border dark color
CP              - Current Position coordinate
writeMode       - Current write mode. Will be one of the values defined by the
                  MGL_writeModeType enumeration.
penStyle        - Current pen fill style. Will be one of values defined by the
                  MGL_penStyleType enumeration.
penHeight       - Current pen height
penWidth        - Current pen width
penPat          - Current pen 8x8 monochrome bitmap pattern
penPixPat       - Current pen 8x8 color pixmap pattern
lineStyle       - Current line style. Will be one of the values defined by the
                  MGL_lineStyleType enumeration.
lineStipple     - Current 16-bit line stipple mask.
stippleCount    - Current line stipple counter.
viewPort        - Current viewport. All coordinates are offset relative to the
                  current viewport when rendered, and output is always
                  clipped to the viewport boundaries.
viewPortOrg     - Current viewport logical origin. You can change the
                  logical viewport origin independently after the viewport
                  has been set. This effectively changes the logical
                  coordinate of the top left corner of the viewport from the
                  default of (0,0) to a new value (i.e. -10,-10).
clipRect        - Current clip rectangle, stored in local viewport
                  coordinates.
clip            - True if clipping is currently on, false if not.
polyType        - Current polygon rasterizing type. Will be one of the values
                  defined by the MGL_polygonType enumeration.
ts              - Current text drawing attributes
****************************************************************************/
typedef struct {
    color_t         color;
    color_t         backColor;
    int             colorMode;
    int             markerSize;
    int             markerStyle;
    color_t         markerColor;
    color_t         bdrBright;
    color_t         bdrDark;
    point_t         CP;
    int             writeMode;
    int             penStyle;
    int             penHeight;
    int             penWidth;
    pattern_t       penPat;
    pixpattern_t    penPixPat;
    int             lineStyle;
    uint            lineStipple;
    uint            stippleCount;
    rect_t          viewPort;
    point_t         viewPortOrg;
    rect_t          clipRect;
    int             clip;
    int             polyType;
    text_settings_t ts;
	} attributes_t;

/****************************************************************************
REMARKS:
Structure representing the attributes for a specific video mode. This
structure is also used to store the rendering dimentions for all device
context surfaces in the MGLDC structure.

HEADER:
mgraph.h

MEMBERS:
xRes                - Device x resolution - 1
yRes                - Device y resolution - 1
bitsPerPixel        - Pixel depth
numberOfPlanes      - Number of planes (always 1 for memory devices)
maxColor            - Maximum color for device - 1
maxPage             - Maximum number of hardware display pages - 1
bytesPerLine        - Number of bytes in a single device scanline
aspectRatio         - Device pixel aspect ratio ((horiz/vert) * 1000)
pageSize            - Number of bytes in a hardware display page
scratch1            - Internal scratch value
scratch2            - Internal scratch value
redMaskSize         - Size of RGB red mask
redFieldPosition    - Number of bits in RGB red field
greenMaskSize       - Size of RGB green mask
greenFieldPosition	- Number of bits in RGB green field
blueMaskSize        - Size of RGB blue mask
blueFieldPosition	- Number of bits in RGB blue field
rsvdMaskSize        - Size of RGB reserved mask
rsvdFieldPosition   - Number of bits in RGB reserved field
****************************************************************************/
typedef struct {
    int     xRes;
    int     yRes;
    int     bitsPerPixel;
    int     numberOfPlanes;
    color_t maxColor;
    int     maxPage;
    int     bytesPerLine;
    int     aspectRatio;
    long    pageSize;
    int     scratch1;
    int     scratch2;
    int     redMaskSize;
    int     redFieldPosition;
    int     greenMaskSize;
    int     greenFieldPosition;
    int     blueMaskSize;
    int     blueFieldPosition;
    int     rsvdMaskSize;
	int     rsvdFieldPosition;
    ulong	modeFlags;
	} gmode_t;

/****************************************************************************
REMARKS:
Structure representing the information passed to the MGL's OpenGL binding
functions to both choose an appropriate visual that is supported by the
OpenGL implementation and to pass in the information for the visual when
a rendering context is created. Application code will fill in the
structure and call MGL_glChooseVisual to find out a valid set of visual
information that is close to what they requested, then call MGL_glSetVisual
to make that the current visual for a specific MGL device context. The
the next call to MGL_glCreateContext will use the visual information
previously requested with the call to MGL_glSetVisual.

HEADER:
mgraph.h

MEMBERS:
rgb_flag        - True for an RGB mode, false for color index modes
alpha_flag      - True for alpha buffers (8-bits deep)
db_flag         - True for double buffered, false for single buffered
depth_size      - Size of depth buffer in bits
stencil_size    - Size of stencil buffer in bits
accum_size      - Size of accumulation buffer in bits
****************************************************************************/
typedef struct {
    ibool       rgb_flag;
    ibool       alpha_flag;
    ibool       db_flag;
    int         depth_size;
    int         stencil_size;
    int         accum_size;
    } MGLVisual;

/****************************************************************************
REMARKS:
Structure representing the public structure of all MGL device contexts.
You can use the information in this structure to directly access the
device surface for developing your own custom rendering code with the MGL.

HEADER:
mgraph.h

MEMBERS:
a           - Current device attributes
surface     - Pointer to device surface. In banked modes this pointer will
              point to the start of the banked framebuffer memory (generally
              the 0xA0000 physical memory address), and if you plan to draw
              directly to video memory you will have to use the SVGA_bank
              function to change banks during drawing. In hardware (or
              virtual) linear framebuffer modes, this pointer will be a
              pointer to the start of the linear framebuffer memory, and you
              can render directly to it as a single block of memory. Note
              that you should also if the device surface is virtualised
              for display device contexts by calling (MGL_surfaceAccessType).
              If the device surface is virtualised in software, you will
              need to ensure that you only access the surface on BYTE, WORD
              or DWORD boundaries.
zbuffer     - Pointer to device Z-buffer for 3D rendering if allocated,
              or NULL if the Z-buffer has not been allocated, or if the
              Z-buffer cannot be directly accessed.
zbits       - Number of bits per Z-buffer coordinate. Z-buffers in the MGL
              can currently be either 16 or 32-bits deep.
zwidth      - Width of the Z-buffer in Z-buffer coordinates. This is
              usually equal to the device resolution, but may be more if
              the Z-buffer width is rounded by the device driver.
mi          - Mode attribute information for the device
pf          - Current pixel format for the device context.
colorTab    - Color lookup table cache for the device context. In color
              map modes this is an array of 256 palette_t structures that
              represent the color palette for the device context. In
              15-bits per pixel and higher modes, this is an array of 256
              color_t values which contains a lookup table with pre-packed
              color values for the current display mode. This is used by
              the MGL when translating color index bitmaps and drawing
              them in 15-bit and higher modes.
shadeTab    - Shade table for the device context, or NULL if no shade table
              has been allocated. The MGL's 3D rendering functions allow
              you to render smooth shaded, color index primitives even in
              15-bits per pixel and higher modes. In these modes the color
              indexes that you pass to the MGL are actually indexes into
              this shade table, and this table is used as a final
              translation to get the real color for a pixel.
bankOffset  - Current offset of starting bank number for the current
              display page. This value is useful if you are implementing
              your own custom banked rendering routines, and you will need
              to add this value to get to the first bank in video memory
              for the currently active display page.
****************************************************************************/
typedef struct {
    attributes_t a;
    void    *surface;
    void    *zbuffer;
    int     zbits;
    int     zwidth;
    gmode_t mi;
    pixel_format_t pf;
    color_t *colorTab;
    color_t *shadeTab;
    int     bankOffset;

	/* Remainder of Device Context structure is private and internal	*/
#if 0
    } MGLDC;
#else
	} publicDevCtx_t;
#endif

#ifndef	BUILD_MGL
/* {secret} */
typedef publicDevCtx_t			MGLDC;
#else
/* {secret} */
struct internalDevCtx_t;
/* {secret} */
typedef struct internalDevCtx_t MGLDC;
#endif

/****************************************************************************
REMARKS:
Structure describing the information contained in an event extracted from
the event queue.

HEADER:
mgraph.h

MEMBERS:
which       - Window identifier for message for use by high level window manager
              code (i.e. MegaVision GUI or Windows API).
what        - Type of event that occurred. Will be one of the values defined by
              the MGL_eventType enumeration.
when        - Time that the event occurred in milliseconds since startup
where_x     - X coordinate of the mouse cursor location at the time of the event
              (in screen coordinates)
where_y     - Y coordinate of the mouse cursor location at the time of the event
              (in screen coordinates)
message     - Event specific message for the event. For use events this can be
              any user specific information. For keyboard events this contains
              the ASCII code in bits 0-7, the keyboard scan code in bits 8-15 and
              the character repeat count in bits 16-30. You can use the
              EVT_asciiCode, EVT_scanCode and EVT_repeatCount macros to extract
              this information from the message field. For mouse events this
              contains information about which button was pressed, and will be a
              combination of the flags defined by the MGL_eventMsgMaskType
              enumeration.
modifiers   - Contains additional information about the state of the keyboard
              shift modifiers (Ctrl, Alt and Shift keys) when the event
              occurred. For mouse events it will also contain the state of
              the mouse buttons. Will be a combination of the values defined
              by the MGL_eventModMaskType enumeration.
next        - Internal use; do not use.
prev        - Internal use; do not use.
****************************************************************************/
typedef struct {
    ulong       which;
    uint        what;
    ulong       when;
    int         where_x;
	int			where_y;
    ulong       message;
    ulong       modifiers;
    int         next;
    int         prev;
	} event_t;

/****************************************************************************
REMARKS:
Structure representing the set of file I/O functions that can be overridden
in the MGL. When you override the file I/O functions in the MGL, you must
provide a compatible function for each of the entries in this structure
that behave identically to the standard C library I/O functions of
similar names.

Note: Once you have overridden the file I/O functions, you can access the
      overriden functions from other libraries and DLL's by calling the
	  MGL_fopen family of functions, which are simply stubs to call the
      currently overriden function via the function pointers.

HEADER:
mgraph.h

MEMBERS:
fopen   - Standard C fopen function replacement
fclose  - Standard C fclose function replacement
fseek   - Standard C fseek function replacement
ftell   - Standard C ftell function replacement
fread   - Standard C fread function replacement
fwrite  - Standard C fwrite function replacement
****************************************************************************/
typedef struct {
	FILE *	(*fopen)(const char *filename,const char *mode);
	int 	(*fclose)(FILE *f);
	int 	(*fseek)(FILE *f,long offset,int whence);
	long 	(*ftell)(FILE *f);
	size_t	(*fread)(void *ptr,size_t size,size_t n,FILE *f);
	size_t	(*fwrite)(const void *ptr,size_t size,size_t n,FILE *f);
	} fileio_t;

/****************************************************************************
REMARKS:
Defines the flags for the types of direct surface access provided.

HEADER:
mgraph.h

MEMBERS:
MGL_NO_ACCESS       - Surface cannot be accessed
MGL_VIRTUAL_ACCESS  - Surface is virtualised
MGL_LINEAR_ACCESS   - Surface can be linearly accessed
****************************************************************************/
typedef enum {
    MGL_NO_ACCESS       = 0x0,
    MGL_VIRTUAL_ACCESS  = 0x1,
    MGL_LINEAR_ACCESS   = 0x2,
    MGL_SURFACE_FLAGS   = 0x3,
    } MGL_surfaceAccessFlagsType;

#define MGL_NO_ZACCESS      0x0     /* Zbuffer cannot be accessed       */
#define MGL_VIRTUAL_ZACCESS 0x4     /* Zbuffer is virtualised in        */
#define MGL_LINEAR_ZACCESS  0x8     /* Zbuffer can be linearly accessed */
#define MGL_ZBUFFER_FLAGS   0xC

/****************************************************************************
REMARKS:
Defines the flags returned by the MGL_modeFlags functions. This function
allows you to enumerate and detect support for different types of hardware
features for a specific graphics mode after calling MGL_detectGraph, but
before you actually initialise the desired mode. This will allow your
application to search for fullscreen graphics modes that have the features
that you desire (such as 2D or 3D acceleration).

HEADER:
mgraph.h

MEMBERS:
MGL_HAVE_LINEAR			- Graphics mode supports a hardware linear
						  framebuffer. 
MGL_HAVE_REFRESH_CTRL	- Graphics mode supports refresh rate control,
						  allowing you to increase the refresh rate to
						  a desired value (such as high refresh rates for
						  stereo LC shutter glasses support).
MGL_HAVE_INTERLACED		- Graphics mode supports interlaced operation, and
						  you can request and interlaced mode via the
                          refresh rate control mechanism in the MGL.
MGL_HAVE_DOUBLE_SCAN	- Graphics mode supports double scan operation.
MGL_HAVE_TRIPLEBUFFER	- Graphics mode supports hardware triple buffering,
						  allowing your application to use true triple
						  buffering without any visible flickering. 
MGL_HAVE_STEREO			- Graphics mode supports hardware stereo page
						  flipping, providing hardware support for stereo LC
						  shutter glasses. 
MGL_HAVE_STEREO_DUAL	- Graphics mode supports hardware stereo page
						  flipping, with dual display start addresses.
MGL_HAVE_STEREO_HWSYNC	- Graphics mode provides hardware stereo sync support
						  via an external connector for stereo LC shutter
						  glasses.
MGL_HAVE_STEREO_EVCSYNC	- Graphics mode provides support for the EVC stereo
						  connector. If this bit is set, the above bit will
						  also be set.
MGL_HAVE_HWCURSOR		- Graphics mode supports a hardware cursor.
MGL_HAVE_ACCEL_2D		- Graphics mode supports 2D hardware acceleration.
						  2D acceleration may be provided either by WinDirect
                          and a VESA VBE/AF driver, or via DirectDraw.
MGL_HAVE_ACCEL_3D		- Graphics mode supports 3D hardware acceleration.
						  Hardware 3D acceleration is always provided in the
                          form of an OpenGL hardware driver of some form.
MGL_HAVE_ACCEL_VIDEO	- Graphics mode supports hardware video acceleration,
						  either via WinDirect and a VESA VBE/AF driver, or
						  via DirectDraw.
MGL_HAVE_VIDEO_XINTERP	- Graphics mode supports hardware video with
						  interpolation along the X axis.
MGL_HAVE_VIDEO_YINTERP	- Graphics mode supports hardware video with
						  interpolation along the Y axis.
****************************************************************************/
typedef enum {
	MGL_HAVE_LINEAR			= 0x00000001,
	MGL_HAVE_REFRESH_CTRL	= 0x00000002,
	MGL_HAVE_INTERLACED		= 0x00000004,
	MGL_HAVE_DOUBLE_SCAN	= 0x00000008,
	MGL_HAVE_TRIPLEBUFFER	= 0x00000010,
	MGL_HAVE_STEREO			= 0x00000020,
	MGL_HAVE_STEREO_DUAL	= 0x00000040,
	MGL_HAVE_STEREO_HWSYNC	= 0x00000080,
	MGL_HAVE_STEREO_EVCSYNC	= 0x00000100,
	MGL_HAVE_HWCURSOR		= 0x00000200,
	MGL_HAVE_ACCEL_2D		= 0x00000400,
	MGL_HAVE_ACCEL_3D		= 0x00000800,
	MGL_HAVE_ACCEL_VIDEO	= 0x00001000,
	MGL_HAVE_VIDEO_XINTERP	= 0x00002000,
	MGL_HAVE_VIDEO_YINTERP	= 0x00004000,
	} MGL_modeFlagsType;

/****************************************************************************
REMARKS:
Defines the flags for the types of hardware acceleration supported by
the device context. This will allow the application to tailor the use of
MGL functions depending upon whether specific hardware support is
available. Hence applications can use specialised software rendering
support if the desired hardware support is not available on the end
user system.

NOTE: If the hardware flags are not MGL_HW_NONE, you /must/ call
      the MGL_beginDirectAccess and MGL_endDirectAccess functions
      before and after any custom code that does direct framebuffer
      rendering!! This is not necessary for non-accelerated device
      contexts, so you might want to optimise these calls out if
      there is no hardware acceleration support.

HEADER:
mgraph.h

MEMBERS:
MGL_HW_NONE         - No hardware acceleration
MGL_HW_LINE         - Hardware line drawing
MGL_HW_STIPPLE_LINE - Hardware stippled line drawing
MGL_HW_POLY         - Hardware polygon filling
MGL_HW_RECT         - Hardware rectangle fill
MGL_HW_PATT_RECT    - Hardware pattern rectangle fill
MGL_HW_CLRPATT_RECT - Hardware color pattern fill
MGL_HW_SCR_BLT      - Hardware screen/screen bitBlt
MGL_HW_SRCTRANS_BLT - Hardware source transparent blt
MGL_HW_DSTTRANS_BLT - Hardware dest. transparent blt
MGL_HW_MONO_BLT     - Hardware monochrome blt
MGL_HW_CLIP         - Hardware clipping
****************************************************************************/
typedef enum {
    MGL_HW_NONE         = 0x0000,
    MGL_HW_LINE         = 0x0010,
    MGL_HW_STIPPLE_LINE = 0x0020,
    MGL_HW_POLY         = 0x0040,
    MGL_HW_RECT         = 0x0080,
    MGL_HW_PATT_RECT    = 0x0100,
    MGL_HW_CLRPATT_RECT = 0x0200,
    MGL_HW_SCR_BLT      = 0x0400,
    MGL_HW_SRCTRANS_BLT = 0x0800,
    MGL_HW_DSTTRANS_BLT = 0x1000,
	MGL_HW_MONO_BLT     = 0x2000,
	MGL_HW_CLIP         = 0x4000,
	MGL_HW_FLAGS        = 0xFFF0,
	} MGL_hardwareFlagsType;

/****************************************************************************
REMARKS:
MGL_glSetOpenGL flags to select the OpenGL implementation.
In the AUTO mode we automatically determine which version of OpenGL to
use depending on the target runtime system. For Win32 unless there
is hardware acceleration available we choose Silicon Graphic's
OpenGL, but if hardware acceleration is present we use the regular
Microsoft OpenGL implementation so we can utilize the hardware. For DOS
we currently use the Mesa implementation, but you can also force Mesa to
be used for the Windows evironment if you wish.

HEADER:
mgraph.h

MEMBERS:
MGL_GL_AUTO			- Automatically choose OpenGL implementation
MGL_GL_MICROSOFT	- Force Microsoft OpenGL implementation
MGL_GL_SGI			- Force SGI OpenGL implementation
MGL_GL_MESA			- Force Mesa OpenGL implementation
MGL_GL_HWMGL		- Force MGL specific hardware OpenGL implementation
****************************************************************************/
typedef enum {
	MGL_GL_AUTO,
	MGL_GL_MICROSOFT,
	MGL_GL_SGI,
	MGL_GL_MESA,
	MGL_GL_HWMGL,
	} MGL_glOpenGLType;

/****************************************************************************
REMARKS:
MGL_glCreateContext flags to initialize the pixel format used by the OpenGL
rendering context. If you pass in MGL_GL_VISUAL, the visual used will be
the one currently selected by the previous call to MGL_glSetVisual, and
provides the application programmer with complete control over the pixel
formats used.

You can pass in a combination of any of the other flags (ie:
MGL_GL_RGB | MGL_GL_DOUBLE | MGL_GL_DEPTH) to let the MGL know what you
want and to have it automatically select an appropriate visual for you.
This provides a quick and simple way to get application code up and
running. 

HEADER:
mgraph.h

MEMBERS:
MGL_GL_VISUAL	- Use curently assigned visual from call to MGL_glSetVisual
MGL_GL_FORCEMEM	- Force system memory back buffer for all rendering 
MGL_GL_RGB		- Select RGB rendering mode (/default/)
MGL_GL_INDEX	- Select color index display mode
MGL_GL_SINGLE	- Select single buffered display mode (/default/)
MGL_GL_DOUBLE	- Select double buffered display mode
MGL_GL_ACCUM	- Enable accumulation buffer (16 bits)
MGL_GL_ALPHA	- Enable alpha buffer (8 bit)
MGL_GL_DEPTH	- Enable depth buffer (16 bits)
MGL_GL_STENCIL	- Enable stencil buffer (8 bits)
****************************************************************************/
typedef enum {
	MGL_GL_VISUAL				= 0x8000,
	MGL_GL_FORCEMEM				= 0x4000,
	MGL_GL_RGB					= 0x0000,
	MGL_GL_INDEX				= 0x0001,
	MGL_GL_SINGLE				= 0x0000,
	MGL_GL_DOUBLE				= 0x0002,
	MGL_GL_ACCUM				= 0x0004,
	MGL_GL_ALPHA				= 0x0008,
	MGL_GL_DEPTH				= 0x0010,
	MGL_GL_STENCIL				= 0x0020,
	} MGL_glContextFlagsType;

/****************************************************************************
REMARKS:
Defines the flags passed to MGL_setActivePage to let the MGL know which
buffer you wish to draw to when running in stereo mode (ie: after a
display device context created with MGL_createStereoDisplayDC). This
value is logical 'or'ed with the page parameter to MGL_setActivePage.

HEADER:
mgraph.h

MEMBERS:
MGL_LEFT_BUFFER		- Draw to the left buffer in stereo modes
MGL_RIGHT_BUFFER	- Draw to the right buffer in stereo modes
****************************************************************************/
typedef enum {
	MGL_LEFT_BUFFER				= 0x0000,
	MGL_RIGHT_BUFFER			= 0x8000,
	} MGL_stereoBufType;

/****************************************************************************
REMARKS:
Defines the flags passed to MGL_setStereoSyncType to let the MGL know what
type of stereo synchronisation method should be used when running on a
system without hardware stereo sync for LC shutter glasses. By default the
MGL will assume the hardware stereo sync works properly if the BIOS/Drivers
report that this feature is available, however in cases where the BIOS
mis-reports this or where the user has glasses that dont support this,
you can disable automatic use of hardware stereo sync by 'or'ing in
MGL_STEREO_IGNORE_HARDWARE when you call MGL_setStereoSyncType.    

HEADER:
mgraph.h

MEMBERS:
MGL_STEREO_BLUE_CODE		- Use the blue code synchronisation methods as supported
							  by StereoGraphics LC shutter glases.
MGL_STEREO_PARALLEL_PORT	- Use the parallel port sychronisation method as supported
							  by NuVision LC shutter glasses.
MGL_STEREO_SERIAL_PORT		- Use the sereial port sychronisation method as supported
							  by WooBoo CyberBoy LC shutter glasses.
MGL_STEREO_IGNORE_HARDWARE	- Tell the MGL to ignore hardware stereo sync and use the
							  specified software stereo sync mechanism.
****************************************************************************/
typedef enum {
	MGL_STEREO_BLUE_CODE			= 0,
	MGL_STEREO_PARALLEL_PORT		= 1,
	MGL_STEREO_SERIAL_PORT			= 2,
	MGL_STEREO_IGNORE_HW_STEREO		= 0x8000,
	} MGL_stereoSyncType;

#ifdef	DECLARE_HANDLE
/* {secret} */
typedef HWND		MGL_HWND;
/* {secret} */
typedef HBITMAP		MGL_HBITMAP;
#else
/* {secret} */
typedef void		*MGL_HWND;
/* {secret} */
typedef void		*MGL_HBITMAP;
#endif

#ifdef	__cplusplus
extern "C" {			/* Use "C" linkage when in C++ mode	*/
#endif

/*------------------------- Function Prototypes ---------------------------*/

/*---------------------------------------------------------------------------
 * Routines bound to a specific device context. These routines all take
 * an MGLDC as a parmeter for the context to work with and hence dont work
 * with the current context. If however the context passed is the currently
 * active context, all changes to that context are reflected in the
 * currently active context as well.
 *-------------------------------------------------------------------------*/

/* Environment detection and initialisation */

int 	MGLAPI MGL_registerDriver(const char *name,void *driver);
void	MGLAPI MGL_unregisterAllDrivers(void);
void	MGLAPI MGL_registerAllDispDrivers(ibool useLinear,ibool useDirectDraw,ibool useWinDirect);
void 	MGLAPI MGL_registerAllDispDriversExt(ibool useWinDirect,ibool useDirectDraw,ibool useVGA,ibool useVGAX,ibool useVBE,ibool useLinear,ibool useVBEAF,ibool useFullscreenDIB);
void	MGLAPI MGL_registerAllOpenGLDrivers(void);
void	MGLAPI MGL_registerAllMemDrivers(void);
void 	MGLAPI MGL_detectGraph(int *driver,int *mode);
uchar *	MGLAPI MGL_availableModes(void);
int		MGLAPI MGL_availablePages(int mode);
int		MGLAPI MGL_modeResolution(int mode,int *xRes,int *yRes,int *bitsPerPixel);
ulong	MGLAPI MGL_modeFlags(int mode);
ibool	MGLAPI MGL_init(int *driver,int *mode,const char *mglpath);
ibool	MGLAPI MGL_initWindowed(const char *mglpath);
void 	MGLAPI MGL_exit(void);
void 	MGLAPI MGL_setBufSize(unsigned size);
void	MGLAPI MGL_fatalError(const char *msg);
int 	MGLAPI MGL_result(void);
void	MGLAPI MGL_setResult(int result);
const char * MGLAPI MGL_errorMsg(int err);
const char * MGLAPI MGL_modeName(int mode);
const char * MGLAPI MGL_modeDriverName(int mode);
const char * MGLAPI MGL_driverName(int driver);
int		MGLAPI MGL_getDriver(MGLDC *dc);
int		MGLAPI MGL_getMode(MGLDC *dc);

/* Device context management */

ibool	MGLAPI MGL_changeDisplayMode(int mode);
MGLDC	* MGLAPI MGL_createDisplayDC(int numBuffers);
MGLDC	* MGLAPI MGL_createStereoDisplayDC(int numBuffers,int refreshRate);
MGLDC	* MGLAPI MGL_createScrollingDC(int virtualX,int virtualY,int numBuffers);
MGLDC	* MGLAPI MGL_createOffscreenDC(void);
MGLDC	* MGLAPI MGL_createLinearOffscreenDC(void);
MGLDC 	* MGLAPI MGL_createMemoryDC(int xSize,int ySize,int bitsPerPixel,pixel_format_t *pf);
MGLDC 	* MGLAPI MGL_createCustomDC(int xSize,int ySize,int bitsPerPixel,pixel_format_t *pf,int bytesPerLine,void *surface,MGL_HBITMAP hbm);
ibool	MGLAPI MGL_destroyDC(MGLDC *dc);
ibool	MGLAPI MGL_isDisplayDC(MGLDC *dc);
ibool	MGLAPI MGL_isWindowedDC(MGLDC *dc);
ibool	MGLAPI MGL_isMemoryDC(MGLDC *dc);
int		MGLAPI MGL_surfaceAccessType(MGLDC *dc);
long	MGLAPI MGL_getHardwareFlags(MGLDC *dc);
void 	MGLAPI MGL_makeSubDC(MGLDC *dc,int left,int top,int right,int bottom);

/* Stereo support */

void	MGLAPI MGL_startStereo(MGLDC *dc);
void	MGLAPI MGL_stopStereo(MGLDC *dc);
void	MGLAPI MGL_setStereoSyncType(int type);
void	MGLAPI MGL_setBlueCodeIndex(int index);

/* MGL OpenGL binding functions */

ibool	MGLAPI MGL_glHaveHWOpenGL(void);
void	MGLAPI MGL_glSetOpenGLType(int type);
const char ** MGLAPI MGL_glEnumerateDrivers(void);
ibool	MGLAPI MGL_glSetDriver(const char *name);
void	MGLAPI MGL_glChooseVisual(MGLDC *dc,MGLVisual *visual);
ibool	MGLAPI MGL_glSetVisual(MGLDC *dc,MGLVisual *visual);
void	MGLAPI MGL_glGetVisual(MGLDC *dc,MGLVisual *visual);
ibool 	MGLAPI MGL_glCreateContext(MGLDC *dc,int flags);
void 	MGLAPI MGL_glMakeCurrent(MGLDC *dc);
void 	MGLAPI MGL_glDeleteContext(MGLDC *dc);
void	MGLAPI MGL_glSwapBuffers(MGLDC *dc,int waitVRT);
void	MGLAPI MGL_glResizeBuffers(MGLDC *dc);
void * 	MGLAPI MGL_glGetProcAddress(const char *procName);

/* Enable and disable MGL 2D functions with OpenGL surfaces. These functions
 * save and restore the state of the OpenGL rendering context across 2D MGL
 * functions. If you do a lot of 2D MGL drawing, you can bracket all calls with
 * these two functions which will speed up the drawing significantly since the
 * state will only be saved and restored once. 
 */

void	MGLAPI MGL_glEnableMGLFuncs(MGLDC *dc);
void	MGLAPI MGL_glDisableMGLFuncs(MGLDC *dc);

/* OpenGL palette manipulation support. The reason we provide palette
 * manipulation routines specific for OpenGL support is so that when rendering
 * in double buffered modes with a system memory backbuffer, the palette for
 * the backbuffer is kept consistent with the hardware front buffer.
 */

void 	MGLAPI MGL_glSetPalette(MGLDC *dc,palette_t *pal,int numColors,int startIndex);
void 	MGLAPI MGL_glRealizePalette(MGLDC *dc,int numColors,int startIndex,int waitVRT);

/* Viewport and clip rectangle manipulation bound to a specific DC */

void 	MGLAPI MGL_setViewportDC(MGLDC *dc,rect_t view);
void	MGLAPI MGL_setRelViewportDC(MGLDC *dc,rect_t view);
void 	MGLAPI MGL_getViewportDC(MGLDC *dc,rect_t *view);
void 	MGLAPI MGL_setViewportOrgDC(MGLDC *dc,point_t org);
void	MGLAPI MGL_getViewportOrgDC(MGLDC *dc,point_t *org);
void	MGLAPI MGL_globalToLocalDC(MGLDC *dc,point_t *p);
void	MGLAPI MGL_localToGlobalDC(MGLDC *dc,point_t *p);
int 	MGLAPI MGL_maxxDC(MGLDC *dc);
int 	MGLAPI MGL_maxyDC(MGLDC *dc);
void 	MGLAPI MGL_setClipRectDC(MGLDC *dc,rect_t clip);
void 	MGLAPI MGL_getClipRectDC(MGLDC *dc,rect_t *clip);
void 	MGLAPI MGL_setClipModeDC(MGLDC *dc,ibool mode);
ibool 	MGLAPI MGL_getClipModeDC(MGLDC *dc);

/* Color and palette manipulation */

color_t	MGLAPI MGL_realColor(MGLDC *dc,int color);
color_t	MGLAPI MGL_rgbColor(MGLDC *dc,uchar R,uchar G,uchar B);
void 	MGLAPI MGL_setPaletteEntry(MGLDC *dc,int entry,uchar red,uchar green,uchar blue);
void 	MGLAPI MGL_getPaletteEntry(MGLDC *dc,int entry,uchar *red,uchar *green,uchar *blue);
void 	MGLAPI MGL_setPalette(MGLDC *dc,palette_t *pal,int numColors,int startIndex);
void 	MGLAPI MGL_getPalette(MGLDC *dc,palette_t *pal,int numColors,int startIndex);
void	ASMAPI MGL_rotatePalette(MGLDC *dc,int numColors,int startIndex,int direction);
ibool	ASMAPI MGL_fadePalette(MGLDC *dc,palette_t *fullIntensity,int numColors,int startIndex,uchar intensity);
void	MGLAPI MGL_realizePalette(MGLDC *dc,int numColors,int startIndex,int waitVRT);
int		MGLAPI MGL_getPaletteSize(MGLDC *dc);
void	MGLAPI MGL_getDefaultPalette(MGLDC *dc,palette_t *pal);
void 	MGLAPI MGL_setDefaultPalette(MGLDC *dc);
void	MGLAPI MGL_setPaletteSnowLevel(MGLDC *dc,int level);
int		MGLAPI MGL_getPaletteSnowLevel(MGLDC *dc);
#ifndef	MGL_LITE
ibool	MGLAPI MGL_checkIdentityPalette(ibool enable);
void	MGLAPI MGL_mapToPalette(MGLDC *dc,palette_t *pal);
#endif

/* Generic device context information and manipulation */

ibool 	MGLAPI MGL_haveWidePalette(MGLDC *dc);
int		MGLAPI MGL_getBitsPerPixel(MGLDC *dc);
color_t	MGLAPI MGL_maxColor(MGLDC *dc);
int		MGLAPI MGL_maxPage(MGLDC *dc);
int 	MGLAPI MGL_sizex(MGLDC *dc);
int 	MGLAPI MGL_sizey(MGLDC *dc);
void 	MGLAPI MGL_getPixelFormat(MGLDC *dc,pixel_format_t *pf);
void	* MGLAPI MGL_computePixelAddr(MGLDC *dc,int x,int y);

/* Double buffering support */

void	MGLAPI MGL_setActivePage(MGLDC *dc,int page);
int		MGLAPI MGL_getActivePage(MGLDC *dc);
void	MGLAPI MGL_setVisualPage(MGLDC *dc,int page,int waitVRT);
int		MGLAPI MGL_getVisualPage(MGLDC *dc);
void	MGLAPI MGL_setDisplayStart(MGLDC *dc,int x,int y,int waitFlag);
void	MGLAPI MGL_getDisplayStart(MGLDC *dc,int *x,int *y);
void	MGLAPI MGL_vSync(MGLDC *dc);
ibool	MGLAPI MGL_doubleBuffer(MGLDC *dc);
void	MGLAPI MGL_singleBuffer(MGLDC *dc);
void	MGLAPI MGL_swapBuffers(MGLDC *dc,int waitVRT);

/* Event handling support */

ibool 	MGLAPI EVT_getNext(event_t *evt,uint mask);
ibool	MGLAPI EVT_peekNext(event_t *evt,uint mask);
ibool	MGLAPI EVT_post(ulong which,uint what,ulong message,ulong modifiers);
void	MGLAPI EVT_flush(uint mask);
void 	MGLAPI EVT_halt(event_t *evt,uint mask);
int		MGLAPI EVT_setTimerTick(int ticks);
ibool 	MGLAPI EVT_isKeyDown(uchar scanCode);

/* Generic helper functions */

ulong	MGLAPI MGL_getTicks(void);
ulong	MGLAPI MGL_getTickResolution(void);
void	MGLAPI MGL_delay(int millseconds);
void	MGLAPI MGL_beep(int freq,int milliseconds);

/*---------------------------------------------------------------------------
 * Routines bound to the currently active context. All these routines work
 * with the currently active context and do not reflect any changes made
 * to the global context to the original user supplied context (because it
 * may be cached). The cached DC is automatically flushed back to the
 * original DC when a new context is enabled with MGL_makeCurrentDC().
 *
 * Before destroying a DC that is current, make sure you call
 * MGL_makeCurrentDC(NULL) first!
 *-------------------------------------------------------------------------*/

/* Routines to change the active global device context */

MGLDC *	MGLAPI MGL_makeCurrentDC(MGLDC *dc);
ibool	MGLAPI MGL_isCurrentDC(MGLDC *dc);

/* Current device context information and manipulation */

void 	MGLAPI MGL_defaultAttributes(MGLDC *dc);
int 	MGLAPI MGL_getAspectRatio(void);
void 	MGLAPI MGL_setAspectRatio(int aspectRatio);
void 	ASMAPI MGL_setColor(color_t color);
void	MGLAPI MGL_setColorRGB(uchar R,uchar G,uchar B);
void	MGLAPI MGL_setColorCI(int index);
color_t MGLAPI MGL_getColor(void);
void 	ASMAPI MGL_setBackColor(color_t color);
color_t MGLAPI MGL_getBackColor(void);
color_t	ASMAPI MGL_packColor(pixel_format_t *pf,uchar R,uchar G,uchar B);
void	MGLAPI MGL_unpackColor(pixel_format_t *pf,color_t color,uchar *R,uchar *G,uchar *B);
color_t	ASMAPI MGL_packColorRGB(uchar R,uchar G,uchar B);
void	MGLAPI MGL_unpackColorRGB(color_t color,uchar *R,uchar *G,uchar *B);
color_t	MGLAPI MGL_defaultColor(void);
#ifndef	MGL_LITE
void	MGLAPI MGL_setMarkerSize(int size);
int		MGLAPI MGL_getMarkerSize(void);
void	MGLAPI MGL_setMarkerStyle(int style);
int		MGLAPI MGL_getMarkerStyle(void);
void	MGLAPI MGL_setMarkerColor(color_t color);
color_t	MGLAPI MGL_getMarkerColor(void);
void	MGLAPI MGL_setBorderColors(color_t bright,color_t dark);
void	MGLAPI MGL_getBorderColors(color_t *bright,color_t *dark);
void 	ASMAPI MGL_setWriteMode(int mode);
int 	MGLAPI MGL_getWriteMode(void);
void 	ASMAPI MGL_setPenStyle(int style);
int 	MGLAPI MGL_getPenStyle(void);
void 	MGLAPI MGL_setLineStyle(int style);
int 	MGLAPI MGL_getLineStyle(void);
void 	ASMAPI MGL_setLineStipple(ushort stipple);
ushort	MGLAPI MGL_getLineStipple(void);
void 	ASMAPI MGL_setLineStippleCount(uint stippleCount);
uint	MGLAPI MGL_getLineStippleCount(void);
void 	ASMAPI MGL_setPenBitmapPattern(const pattern_t *pat);
void 	MGLAPI MGL_getPenBitmapPattern(pattern_t *pat);
void 	ASMAPI MGL_setPenPixmapPattern(const pixpattern_t *pat);
void 	MGLAPI MGL_getPenPixmapPattern(pixpattern_t *pat);
void	MGLAPI MGL_setPenSize(int height,int width);
void 	MGLAPI MGL_getPenSize(int *height,int *width);
#ifndef	MGL_LITE
void 	MGLAPI MGL_setColorMapMode(int mode);
int		MGLAPI MGL_getColorMapMode(void);
#endif
void	MGLAPI MGL_setPolygonType(int type);
int		MGLAPI MGL_getPolygonType(void);
#endif
void 	MGLAPI MGL_getAttributes(attributes_t *attr);
void 	MGLAPI MGL_restoreAttributes(attributes_t *attr);

/* Device clearing */

void	ASMAPI MGL_clearDevice(void);
void 	MGLAPI MGL_clearViewport(void);

/* Viewport and clip rectangle manipulation */

void 	MGLAPI MGL_setViewport(rect_t view);
void	MGLAPI MGL_setRelViewport(rect_t view);
void 	MGLAPI MGL_getViewport(rect_t *view);
void 	MGLAPI MGL_setViewportOrg(point_t org);
void	MGLAPI MGL_getViewportOrg(point_t *org);
void	MGLAPI MGL_globalToLocal(point_t *p);
void	MGLAPI MGL_localToGlobal(point_t *p);
int 	MGLAPI MGL_maxx(void);
int 	MGLAPI MGL_maxy(void);
void 	MGLAPI MGL_setClipRect(rect_t clip);
void 	MGLAPI MGL_getClipRect(rect_t *clip);
void 	MGLAPI MGL_setClipMode(ibool mode);
ibool 	MGLAPI MGL_getClipMode(void);

/* Pixel plotting */

void 	MGLAPI MGL_pixelCoord(int x,int y);
color_t	MGLAPI MGL_getPixelCoord(int x,int y);
void	ASMAPI MGL_beginPixel(void);
void 	MGLAPI MGL_pixelCoordFast(int x,int y);
color_t	MGLAPI MGL_getPixelCoordFast(int x,int y);
void	ASMAPI MGL_endPixel(void);

/* Line drawing and clipping */

void 	MGLAPI MGL_moveToCoord(int x,int y);
void 	MGLAPI MGL_moveRelCoord(int dx,int dy);
void 	MGLAPI MGL_lineToCoord(int x,int y);
void 	MGLAPI MGL_lineRelCoord(int dx,int dy);
int 	MGLAPI MGL_getX(void);
int 	MGLAPI MGL_getY(void);
void	MGLAPI MGL_getCP(point_t* CP);
void 	MGLAPI MGL_lineCoord(int xx1,int yy1,int xx2,int yy2);
void 	MGLAPI MGL_lineCoordFX(fix32_t xx1,fix32_t yy1,fix32_t xx2,fix32_t yy2);
void    MGLAPI MGL_lineEngine(fix32_t xx1,fix32_t yy1,fix32_t xx2,fix32_t yy2,void (ASMAPI *plotPoint)(int x,int y));
ibool	MGLAPI MGL_clipLineFX(fix32_t *xx1,fix32_t *yy1,fix32_t *xx2,fix32_t *yy2,fix32_t left,fix32_t top,fix32_t right,fix32_t bottom);
#ifndef	MGL_LITE
void 	ASMAPI MGL_scanLine(int y,int xx1,int xx2);
#endif

/* Routines to perform bank switching for banked framebuffers for custom
 * rendering code. The first version is callable only from assembler and
 * requires the new bank value to be passed in the DL register. The second
 * version is callable directly from C. DO NOT CALL THESE FUNCTIONS WHEN
 * RUNNING WITH A LINEAR FRAMEBUFFER!!!
 */

void	ASMAPI SVGA_setBank(void);
void	ASMAPI SVGA_setBankC(int bank);

/* Routines to begin/end direct framebuffer access. You must bracket all
 * code that draws directly to a device context surface with these
 * functions.
 */

void 	ASMAPI MGL_beginDirectAccess(void);
void 	ASMAPI MGL_endDirectAccess(void);

/* Polygon drawing: Note that the following Cnvx polygon routines
 * only work with convex polygons. The integer coordinate versions are
 * provided for compatibility only, and convert the coordinates to fixed
 * point and call the appropriate fixed point routines below.
 */

#ifndef	MGL_LITE
void	MGLAPI MGL_fillPolygon(int count,point_t *vArray,int xOffset,int yOffset);
void	MGLAPI MGL_fillPolygonCnvx(int count,point_t *vArray,int xOffset,int yOffset);
void    ASMAPI MGL_fillPolygonFX(int count,fxpoint_t *vArray,int vinc,fix32_t xOffset,fix32_t yOffset);
void    ASMAPI MGL_fillPolygonCnvxFX(int count,fxpoint_t *vArray,int vinc,fix32_t xOffset,fix32_t yOffset);
#endif

/* Polyline drawing */

#ifndef	MGL_LITE
void 	MGLAPI MGL_marker(point_t p);
void	MGLAPI MGL_polyPoint(int count,point_t *vArray);
void 	MGLAPI MGL_polyMarker(int count,point_t *vArray);
void 	MGLAPI MGL_polyLine(int count,point_t *vArray);
#endif

/* Rectangle drawing */

#ifndef	MGL_LITE
void 	MGLAPI MGL_rectCoord(int left,int top,int right,int bottom);
void 	MGLAPI MGL_fillRectCoord(int left,int top,int right,int bottom);
#endif

/* Scanline color scanning. Thee routines are primitive, and do not perform
 * any clipping or viewport mapping, so can be used to build you own
 * high performance floodfilling routines (see the example file ffill.c
 * for pre-built high speed floodfill routines).
 */

#ifndef	MGL_LITE
int		ASMAPI MGL_scanRightForColor(int x,int y,color_t color);
int		ASMAPI MGL_scanLeftForColor(int x,int y,color_t color);
int 	ASMAPI MGL_scanRightWhileColor(int x,int y,color_t color);
int		ASMAPI MGL_scanLeftWhileColor(int x,int y,color_t color);
#endif

/* Psuedo 3D border drawing */

#ifndef	MGL_LITE
void	MGLAPI MGL_drawBorderCoord(int left,int top,int right,int bottom,int style,int thickness);
void 	MGLAPI MGL_drawHDivider(int y,int xx1,int xx2);
void 	MGLAPI MGL_drawVDivider(int x,int yy1,int yy2);
#endif

/* Ellipse drawing */

#ifndef	MGL_LITE
void	MGLAPI MGL_ellipseArc(rect_t extentRect,int startAngle,int endAngle);
void	MGLAPI MGL_ellipseArcCoord(int x,int y,int xradius,int yradius,int startAngle,int endAngle);
void	ASMAPI MGL_getArcCoords(arc_coords_t *coords);
void	MGLAPI MGL_ellipse(rect_t extentRect);
void	MGLAPI MGL_ellipseCoord(int x,int y,int xradius,int yradius);
void	MGLAPI MGL_fillEllipseArc(rect_t extentRect,int startAngle,int endAngle);
void	MGLAPI MGL_fillEllipseArcCoord(int x,int y,int xradius,int yradius,int startAngle,int endAngle);
void	MGLAPI MGL_fillEllipse(rect_t extentRect);
void	MGLAPI MGL_fillEllipseCoord(int x,int y,int xradius,int yradius);
void	MGLAPI MGL_ellipseEngine(rect_t extentRect,void (ASMAPI *setup)(int topY,int botY,int left,int right),void (ASMAPI *set4pixels)(ibool inc_x,ibool inc_y,ibool region1),void (ASMAPI *finished)(void));
void 	MGLAPI MGL_ellipseArcEngine(rect_t extentRect,int startAngle,int endAngle,arc_coords_t *ac,void (ASMAPI *plotPoint)(int x,int y));
#endif

/* Text attribute manipulation */

#ifndef	MGL_LITE
void 	MGLAPI MGL_setTextJustify(int horiz,int vert);
void 	MGLAPI MGL_getTextJustify(int *horiz,int *vert);
void 	MGLAPI MGL_setTextDirection(int direction);
int   	MGLAPI MGL_getTextDirection(void);
void	MGLAPI MGL_setTextSize(int numerx,int denomx,int numery,int denomy);
void	MGLAPI MGL_getTextSize(int *numerx,int *denomx,int *numery,int *denomy);
void	MGLAPI MGL_setSpaceExtra(int extra);
int		MGLAPI MGL_getSpaceExtra(void);
void	MGLAPI MGL_setTextSettings(text_settings_t *settings);
void	MGLAPI MGL_getTextSettings(text_settings_t *settings);
int 	MGLAPI MGL_textHeight(void);
int 	MGLAPI MGL_textWidth(const char *str);
void	MGLAPI MGL_textBounds(int x,int y,const char *str,rect_t *bounds);
int		MGLAPI MGL_charWidth(char ch);
void	MGLAPI MGL_getFontMetrics(metrics_t *metrics);
void	MGLAPI MGL_getCharMetrics(char ch,metrics_t *metrics);
int		MGLAPI MGL_maxCharWidth(void);
void	MGLAPI MGL_underScoreLocation(int *x,int *y,const char *str);
#endif

/* Text drawing */

#ifndef	MGL_LITE
void 	MGLAPI MGL_drawStr(const char *str);
void	MGLAPI MGL_drawStrXY(int x,int y,const char *str);
ibool	MGLAPI MGL_useFont(font_t *font);
font_t 	* MGLAPI MGL_getFont(void);
ibool	MGLAPI MGL_vecFontEngine(int x,int y,const char *str,void (ASMAPI *move)(int x,int y),void (ASMAPI *draw)(int x,int y));
#endif

/* BitBlt support */

void 	MGLAPI MGL_bitBltCoord(MGLDC *dst,MGLDC *src,int left,int top,int right,int bottom,int dstLeft,int dstTop,int op);
void 	MGLAPI MGL_stretchBltCoord(MGLDC *dst,MGLDC *src,int left,int top,int right,int bottom,int dstLeft,int dstTop,int dstRight,int dstBottom);
#ifndef	MGL_LITE
void 	MGLAPI MGL_getDivotCoord(MGLDC *dc,int left,int top,int right,int bottom,void *divot);
void 	MGLAPI MGL_putDivot(MGLDC *dc,void *divot);
long 	MGLAPI MGL_divotSizeCoord(MGLDC *dc,int left,int top,int right,int bottom);
void 	MGLAPI MGL_putMonoImage(MGLDC *dc,int x,int y,int byteWidth,int height,void *image);
void	MGLAPI MGL_putBitmap(MGLDC *dc,int x,int y,const bitmap_t *bitmap,int op);
void	MGLAPI MGL_putBitmapSection(MGLDC *dc,int left,int top,int right,int bottom,int dstLeft,int dstTop,const bitmap_t *bitmap,int op);
void	MGLAPI MGL_putBitmapTransparent(MGLDC *dc,int x,int y,const bitmap_t *bitmap,color_t transparent,ibool sourceTrans);
void	MGLAPI MGL_putBitmapTransparentSection(MGLDC *dc,int left,int top,int right,int bottom,int dstLeft,int dstTop,const bitmap_t *bitmap,color_t transparent,ibool sourceTrans);
void	MGLAPI MGL_putBitmapMask(MGLDC *dc,int x,int y,const bitmap_t *mask,color_t color);
void	MGLAPI MGL_stretchBitmap(MGLDC *dc,int left,int top,int right,int bottom,const bitmap_t *bitmap);
void	MGLAPI MGL_putIcon(MGLDC *dc,int x,int y,const icon_t *icon);
void 	MGLAPI MGL_transBltCoord(MGLDC *dst,MGLDC *src,int left,int top,int right,int bottom,int dstLeft,int dstTop,color_t transparent,ibool sourceTrans);
#endif

/* Linear offscreen DC BitBlt support */

#ifndef	MGL_LITE
void 	MGLAPI MGL_bitBltLinCoord(MGLDC *dst,MGLDC *src,ulong srcOfs,int dstLeft,int dstTop,int dstRight,int dstBottom,int op);
void 	MGLAPI MGL_transBltLinCoord(MGLDC *dst,MGLDC *src,ulong srcOfs,int dstLeft,int dstTop,int dstRight,int dstBottom,color_t transparent,ibool sourceTrans);
#endif

/* Monochrome bitmap manipulation */

#ifndef	MGL_LITE
void	MGLAPI MGL_drawGlyph(font_t *g,int x,int y,uchar glyph);
int		MGLAPI MGL_getGlyphWidth(font_t *font,uchar glyph);
int		MGLAPI MGL_getGlyphHeight(font_t *font);
void 	MGLAPI MGL_rotateGlyph(uchar *dst,uchar *src,int *byteWidth,int *height,int rotation);
void 	MGLAPI MGL_mirrorGlyph(uchar *dst,uchar *src,int byteWidth,int height);
#endif

/* Region management */

region_t * MGLAPI MGL_newRegion(void);
region_t * MGLAPI MGL_copyRegion(const region_t *s);
void	MGLAPI MGL_clearRegion(region_t *r);
void 	MGLAPI MGL_freeRegion(region_t *r);
void 	MGLAPI MGL_drawRegion(int x,int y,const region_t *r);

/* Region generation primitives */

#ifndef	MGL_LITE
region_t * MGLAPI MGL_rgnLineCoord(int xx1,int yy1,int xx2,int yy2,const region_t *pen);
region_t * MGLAPI MGL_rgnLineCoordFX(fix32_t xx1,fix32_t yy1,fix32_t xx2,fix32_t yy2,const region_t *pen);
/*region_t * MGLAPI MGL_rgnPolygon(int count,point_t *vArray);*/
/*region_t * MGLAPI MGL_rgnPolygonCnvx(int count,point_t *vArray);*/
region_t * MGLAPI MGL_rgnSolidRectCoord(int left,int top,int right,int bottom);
region_t * MGLAPI MGL_rgnEllipse(rect_t extentRect,const region_t *pen);
region_t * MGLAPI MGL_rgnEllipseArc(rect_t extentRect,int startAngle,int endAngle,const region_t *pen);
void	MGLAPI MGL_rgnGetArcCoords(arc_coords_t *coords);
region_t * MGLAPI MGL_rgnSolidEllipse(rect_t extentRect);
region_t * MGLAPI MGL_rgnSolidEllipseArc(rect_t extentRect,int startAngle,int endAngle);
#endif

/* Region alegbra */

#ifndef	MGL_LITE
region_t * MGLAPI MGL_sectRegion(const region_t *r1,const region_t *r2);
region_t * MGLAPI MGL_sectRegionRect(const region_t *r1,const rect_t *r2);
ibool	MGLAPI MGL_unionRegion(region_t *r1,const region_t *r2);
ibool	MGLAPI MGL_unionRegionRect(region_t *r1,const rect_t *r2);
ibool 	MGLAPI MGL_unionRegionOfs(region_t *r1,const region_t *r2,int xOffset,int yOffset);
ibool	MGLAPI MGL_diffRegion(region_t *r1,const region_t *r2);
ibool	MGLAPI MGL_diffRegionRect(region_t *r1,const rect_t *r2);
void	MGLAPI MGL_optimizeRegion(region_t *r);
void 	MGLAPI MGL_offsetRegion(region_t *r,int dx,int dy);
ibool	MGLAPI MGL_emptyRegion(const region_t *r);
ibool	MGLAPI MGL_equalRegion(const region_t *r1,const region_t *r2);
ibool	MGLAPI MGL_ptInRegionCoord(int x,int y,const region_t *r);
#endif

/* Region traversal */

/* {partOf:MGL_traverseRegion} */
typedef void (ASMAPI *rgncallback_t)(const rect_t *r);
void 	MGLAPI MGL_traverseRegion(region_t *rgn,rgncallback_t doRect);

/* RGB to 8 bit halftone dithering routines */

void    MGLAPI MGL_getHalfTonePalette(palette_t *pal);
uchar	MGLAPI MGL_halfTonePixel(int x,int y,uchar R,uchar G,uchar B);

/* Resource loading/unloading */

#ifndef	MGL_LITE
font_t * MGLAPI MGL_loadFont(const char *fontname);
font_t * MGLAPI MGL_loadFontExt(FILE *f,ulong dwOffset,ulong dwSize);
ibool	MGLAPI MGL_availableFont(const char *fontname);
void	MGLAPI MGL_unloadFont(font_t *font);
#endif
cursor_t * MGLAPI MGL_loadCursor(const char *cursorName);
cursor_t * MGLAPI MGL_loadCursorExt(FILE *f,ulong dwOffset,ulong dwSize);
ibool	MGLAPI MGL_availableCursor(const char *cursorName);
void 	MGLAPI MGL_unloadCursor(cursor_t *cursor);
#ifndef	MGL_LITE
icon_t * MGLAPI MGL_loadIcon(const char *iconName,ibool loadPalette);
icon_t * MGLAPI MGL_loadIconExt(FILE *f,ulong dwOffset,ulong dwSize,ibool loadPalette);
ibool	MGLAPI MGL_availableIcon(const char *iconName);
void	MGLAPI MGL_unloadIcon(icon_t *icon);
#endif

/* Windows BMP bitmap loading/unloading/saving */

#ifndef	MGL_LITE
bitmap_t * MGLAPI MGL_loadBitmap(const char *bitmapName,ibool loadPalette);
bitmap_t * MGLAPI MGL_loadBitmapExt(FILE *f,ulong dwOffset,ulong dwSize,ibool loadPalette);
ibool	MGLAPI MGL_availableBitmap(const char *bitmapName);
void	MGLAPI MGL_unloadBitmap(bitmap_t *bitmap);
ibool	MGLAPI MGL_getBitmapSize(const char *bitmapName,int *width,int *height,int *bitsPerPixel,pixel_format_t *pf);
ibool	MGLAPI MGL_getBitmapSizeExt(FILE *f,ulong dwOffset,ulong dwSize,int *width,int *height,int *bitsPerPixel,pixel_format_t *pf);
ibool	MGLAPI MGL_loadBitmapIntoDC(MGLDC *dc,const char *bitmapName,int dstLeft,int dstTop,ibool loadPalette);
ibool	MGLAPI MGL_loadBitmapIntoDCExt(MGLDC *dc,FILE *f,ulong dwOffset,ulong dwSize,int dstLeft,int dstTop,ibool loadPalette);
ibool	MGLAPI MGL_saveBitmapFromDC(MGLDC *dc,const char *bitmapName,int left,int top,int right,int bottom);
bitmap_t * MGLAPI MGL_getBitmapFromDC(MGLDC *dc,int left,int top,int right,int bottom,ibool savePalette);
bitmap_t * MGLAPI MGL_buildMonoMask(bitmap_t *bitmap,color_t transparent);
#endif

/* PCX bitmap loading/unloading/saving (1/4/8 bpp only) */

#ifndef	MGL_LITE
bitmap_t * MGLAPI MGL_loadPCX(const char *bitmapName,ibool loadPalette);
bitmap_t * MGLAPI MGL_loadPCXExt(FILE *f,ulong dwOffset,ulong dwSize,ibool loadPalette);
ibool	MGLAPI MGL_availablePCX(const char *bitmapName);
ibool	MGLAPI MGL_getPCXSize(const char *bitmapName,int *width,int *height,int *bitsPerPixel);
ibool	MGLAPI MGL_getPCXSizeExt(FILE *f,ulong dwOffset,ulong dwSize,int *width,int *height,int *bitsPerPixel);
ibool	MGLAPI MGL_loadPCXIntoDC(MGLDC *dc,const char *bitmapName,int dstLeft,int dstTop,ibool loadPalette);
ibool	MGLAPI MGL_loadPCXIntoDCExt(MGLDC *dc,FILE *f,ulong dwOffset,ulong dwSize,int dstLeft,int dstTop,ibool loadPalette);
ibool	MGLAPI MGL_savePCXFromDC(MGLDC *dc,const char *bitmapName,int left,int top,int right,int bottom);
#endif

/* JPEG bitmap loading/unloading/saving */

#ifndef	MGL_LITE
bitmap_t * MGLAPI MGL_loadJPEG(const char *JPEGName,int num8BitColors);
bitmap_t * MGLAPI MGL_loadJPEGExt(FILE *f,ulong dwOffset,ulong dwSize,int num8BitColors);
ibool MGLAPI MGL_availableJPEG(const char *JPEGName);
ibool MGLAPI MGL_getJPEGSize(const char *JPEGName,int *width,int *height,int *bitsPerPixel,pixel_format_t *pf);
ibool MGLAPI MGL_getJPEGSizeExt(FILE *f,ulong dwOffset,ulong dwSize,int *width,int *height,int *bitsPerPixel,pixel_format_t *pf);
ibool MGLAPI MGL_loadJPEGIntoDC(MGLDC *dc,const char *JPEGName,int dstLeft,int dstTop,int num8BitColors);
ibool MGLAPI MGL_loadJPEGIntoDCExt(MGLDC *dc,FILE *f,ulong dwOffset,ulong dwSize,int dstLeft,int dstTop,int num8BitColors);
ibool MGLAPI MGL_saveJPEGFromDC(MGLDC *dc,const char *JPEGName,int left,int top,int right,int bottom,int quality);
#endif

/* Random number generation routines for shorts and longs with full range */

void    ASMAPI MGL_srand(uint seed);
ushort	ASMAPI MGL_random(ushort max);
ulong	ASMAPI MGL_randoml(ulong max);

/* Mouse support */

ibool	MGLAPI MS_available(void);
void	MGLAPI MS_show(void);
void	MGLAPI MS_hide(void);
void	MGLAPI MS_obscure(void);
void	MGLAPI MS_setCursor(cursor_t *curs);
void	MGLAPI MS_setCursorColor(color_t color);
void 	MGLAPI MS_moveTo(int x,int y);
void	MGLAPI MS_getPos(int *x,int *y);
void	MGLAPI MS_drawCursor(void);

/* Rectangle and Point manipulation */

rect_t 	MGLAPI MGL_defRect(int left,int top,int right,int bottom);
rect_t 	MGLAPI MGL_defRectPt(point_t leftTop,point_t rightBottom);
ibool 	MGLAPI MGL_sectRect(rect_t s1,rect_t s2,rect_t *d);
ibool 	MGLAPI MGL_sectRectCoord(int left1,int top1,int right1,int bottom1,int left2,int top2,int right2,int bottom2,rect_t *d);
void 	MGLAPI MGL_unionRect(rect_t s1,rect_t s2,rect_t *d);
void 	MGLAPI MGL_unionRectCoord(int left1,int top1,int right1,int bottom1,int left2,int top2,int right2,int bottom2,rect_t *d);

/* Built-in patterns and mouse cursor */

#ifndef	MGL_LITE
/* {secret} */
pattern_t	* MGLAPI _MGL_getEmptyPat(void);
/* {secret} */
pattern_t   * MGLAPI _MGL_getGrayPat(void);
/* {secret} */
pattern_t   * MGLAPI _MGL_getSolidPat(void);
#endif
/* {secret} */
cursor_t    * MGLAPI _MGL_getDefCursor(void);

/* Fixed point multiplication/divide routines */

#if !defined(__WATCOMC__)
fix32_t	ASMAPI MGL_FixMul(fix32_t a,fix32_t b);
fix32_t	ASMAPI MGL_FixDiv(fix32_t a,fix32_t b);
fix32_t	ASMAPI MGL_FixMulDiv(fix32_t a,fix32_t b,fix32_t c);
int		ASMAPI MGL_backfacing(fix32_t dx1,fix32_t dy1,fix32_t dx2,fix32_t dy2);
void 	ASMAPI MGL_memcpy(void *dst,void *src,int n);
#else
/* For Watcom C++ we can use special inline assembler code that is much
 * faster than calling the 386 assembler functions. Currently this is the
 * the only compiler that will allow inline assembler to be expanded
 * directly as inline functions.
 */

fix32_t	MGL_FixMul(fix32_t a,fix32_t b);
#pragma aux MGL_FixMul =			\
	"imul	edx"					\
	"add	eax,8000h"				\
	"adc	edx,0"					\
	"shrd	eax,edx,16"				\
	parm [eax] [edx]				\
	value [eax]						\
	modify exact [eax edx];

fix32_t	MGL_FixDiv(fix32_t a,fix32_t b);
#pragma aux MGL_FixDiv =			\
	"xor	eax,eax"				\
	"shrd	eax,edx,16"				\
	"sar	edx,16"					\
	"idiv	ebx"              		\
	parm [edx] [ebx] 				\
	value [eax]             		\
	modify exact [eax edx];

fix32_t	MGL_FixMulDiv(fix32_t a,fix32_t b,fix32_t c);
#pragma aux MGL_FixMulDiv =			\
	"imul	ebx"					\
	"idiv	ecx"              		\
	parm [eax] [ebx] [ecx] 			\
	value [eax]             		\
	modify exact [eax edx];

int	MGL_backfacing(fix32_t dx1,fix32_t dy1,fix32_t dx2,fix32_t dy2);
#pragma aux MGL_backfacing =		\
	"imul	ebx"					\
	"mov	ebx,eax"				\
	"mov	ecx,edx"				\
	"mov	eax,esi"				\
	"imul	edi"					\
	"sub	eax,ebx"				\
	"mov	eax,1"					\
	"sbb	edx,ecx"				\
	"jns	@@Backfacing"			\
	"xor	eax,eax"				\
	"@@Backfacing:"					\
	parm [eax] [esi] [edi] [ebx]	\
	value [eax]             		\
	modify exact [eax ecx edx];

void MGL_memcpy(void *dst,void *src,int n);
#pragma aux MGL_memcpy =            \
	"mov	eax,ecx"				\
	"shr    ecx,2"                  \
	"rep    movsd"                  \
	"mov	cl,al"					\
	"and	cl,3"					\
	"rep	movsb"					\
	parm [edi] [esi] [ecx]			\
	modify exact [eax ecx esi edi];
#endif

/* The following are special memcpy routines that properly handler reading
 * and writing to virtual linear buffer memory by forcing the proper
 * alignment. Note that the copy is extended to use a DWORD copy of speed.
 */

void 	ASMAPI MGL_memcpyVIRTSRC(void *dst,void *src,int n);
void 	ASMAPI MGL_memcpyVIRTDST(void *dst,void *src,int n);

/* Function to find an MGL system file's full pathname */

/* {secret} */
ibool	MGLAPI _MGL_findFile(char *validpath,const char *dir, const char *filename, const char *mode);

/* Override the internal MGL file I/O functions */

void	MGLAPI MGL_setFileIO(fileio_t *fio);

/* Functions to call the currently overriden file I/O functions */

FILE *	MGLAPI MGL_fopen(const char *filename,const char *mode);
int 	MGLAPI MGL_fclose(FILE *f);
int 	MGLAPI MGL_fseek(FILE *f,long offset,int whence);
long 	MGLAPI MGL_ftell(FILE *f);
size_t	MGLAPI MGL_fread(void *ptr,size_t size,size_t n,FILE *f);
size_t	MGLAPI MGL_fwrite(const void *ptr,size_t size,size_t n,FILE *f);

/* The following dummy symbols are used to link in driver files to be used. A
 * driver is not active until it is linked in with the MGL_registerDriver
 * call. Because we dont export globals in DLLs, we provide functions to
 * get the address of the drivers. However for a static link library we
 * need to use globals so that if the driver data is unreferenced, it will
 * not be linked in with the code. 
 */

#ifndef	BUILD_MGL
#if	defined(MGL_DLL) && !defined(BUILD_MGLDLL)
/* {secret} */
void * MGLAPI VGA4_getDriverAddr(void);
/* {secret} */
void * MGLAPI VGAX_getDriverAddr(void);
/* {secret} */
void * MGLAPI SVGA4_getDriverAddr(void);
/* {secret} */
void * MGLAPI SVGA8_getDriverAddr(void);
/* {secret} */
void * MGLAPI SVGA16_getDriverAddr(void);
/* {secret} */
void * MGLAPI SVGA24_getDriverAddr(void);
/* {secret} */
void * MGLAPI SVGA32_getDriverAddr(void);
#if !defined(__16BIT__)
/* {secret} */
void * MGLAPI VGA8_getDriverAddr(void);
/* {secret} */
void * MGLAPI LINEAR8_getDriverAddr(void);
/* {secret} */
void * MGLAPI LINEAR16_getDriverAddr(void);
/* {secret} */
void * MGLAPI LINEAR24_getDriverAddr(void);
/* {secret} */
void * MGLAPI LINEAR32_getDriverAddr(void);
/* {secret} */
void * MGLAPI ACCEL8_getDriverAddr(void);
/* {secret} */
void * MGLAPI ACCEL16_getDriverAddr(void);
/* {secret} */
void * MGLAPI ACCEL24_getDriverAddr(void);
/* {secret} */
void * MGLAPI ACCEL32_getDriverAddr(void);
#if defined(MGLWIN) || defined(__WINDOWS__)
/* {secret} */
void * MGLAPI FULLDIB8_getDriverAddr(void);
/* {secret} */
void * MGLAPI FULLDIB16_getDriverAddr(void);
/* {secret} */
void * MGLAPI FULLDIB24_getDriverAddr(void);
/* {secret} */
void * MGLAPI FULLDIB32_getDriverAddr(void);
/* {secret} */
void * MGLAPI DDRAW8_getDriverAddr(void);
/* {secret} */
void * MGLAPI DDRAW16_getDriverAddr(void);
/* {secret} */
void * MGLAPI DDRAW24_getDriverAddr(void);
/* {secret} */
void * MGLAPI DDRAW32_getDriverAddr(void);
/* {secret} */
void * MGLAPI OPENGL_getDriverAddr(void);
/* {secret} */
void * MGLAPI FSOGL8_getDriverAddr(void);
/* {secret} */
void * MGLAPI FSOGL16_getDriverAddr(void);
/* {secret} */
void * MGLAPI FSOGL24_getDriverAddr(void);
/* {secret} */
void * MGLAPI FSOGL32_getDriverAddr(void);
#endif
#endif
/* {secret} */
void * MGLAPI PACKED1_getDriverAddr(void);
/* {secret} */
void * MGLAPI PACKED4_getDriverAddr(void);
/* {secret} */
void * MGLAPI PACKED8_getDriverAddr(void);
/* {secret} */
void * MGLAPI PACKED16_getDriverAddr(void);
/* {secret} */
void * MGLAPI PACKED24_getDriverAddr(void);
/* {secret} */
void * MGLAPI PACKED32_getDriverAddr(void);
#define VGA4_driver			VGA4_getDriverAddr()
#define VGAX_driver			VGAX_getDriverAddr()
#define SVGA4_driver		SVGA4_getDriverAddr()
#define SVGA8_driver		SVGA8_getDriverAddr()
#define SVGA16_driver		SVGA16_getDriverAddr()
#define SVGA24_driver		SVGA24_getDriverAddr()
#define SVGA32_driver		SVGA32_getDriverAddr()
#if !defined(__16BIT__)
#define VGA8_driver			VGA8_getDriverAddr()
#define LINEAR8_driver		LINEAR8_getDriverAddr()
#define LINEAR16_driver		LINEAR16_getDriverAddr()
#define LINEAR24_driver     LINEAR24_getDriverAddr()
#define LINEAR32_driver     LINEAR32_getDriverAddr()
#define ACCEL8_driver       ACCEL8_getDriverAddr()
#define ACCEL16_driver      ACCEL16_getDriverAddr()
#define ACCEL24_driver      ACCEL24_getDriverAddr()
#define ACCEL32_driver      ACCEL32_getDriverAddr()
#if defined(MGLWIN) || defined(__WINDOWS__)
#define FULLDIB8_driver     FULLDIB8_getDriverAddr()
#define FULLDIB16_driver    FULLDIB16_getDriverAddr()
#define FULLDIB24_driver    FULLDIB24_getDriverAddr()
#define FULLDIB32_driver    FULLDIB32_getDriverAddr()
#define DDRAW8_driver       DDRAW8_getDriverAddr()
#define DDRAW16_driver      DDRAW16_getDriverAddr()
#define DDRAW24_driver      DDRAW24_getDriverAddr()
#define DDRAW32_driver      DDRAW32_getDriverAddr()
#define OPENGL_driver       OPENGL_getDriverAddr()
#define FSOGL8_driver       FSOGL8_getDriverAddr()
#define FSOGL16_driver      FSOGL16_getDriverAddr()
#define FSOGL24_driver      FSOGL24_getDriverAddr()
#define FSOGL32_driver      FSOGL32_getDriverAddr()
#endif
#endif
#define PACKED1_driver      PACKED1_getDriverAddr()
#define PACKED4_driver      PACKED4_getDriverAddr()
#define PACKED8_driver      PACKED8_getDriverAddr()
#define PACKED16_driver     PACKED16_getDriverAddr()
#define PACKED24_driver     PACKED24_getDriverAddr()
#define PACKED32_driver		PACKED32_getDriverAddr()
#else
extern int _VARAPI VGA4_driver[];
extern int _VARAPI VGAX_driver[];
extern int _VARAPI SVGA4_driver[];
extern int _VARAPI SVGA8_driver[];
extern int _VARAPI SVGA16_driver[];
extern int _VARAPI SVGA24_driver[];
extern int _VARAPI SVGA32_driver[];
#if !defined(__16BIT__)
extern int _VARAPI VGA8_driver[];
extern int _VARAPI LINEAR8_driver[];
extern int _VARAPI LINEAR16_driver[];
extern int _VARAPI LINEAR24_driver[];
extern int _VARAPI LINEAR32_driver[];
extern int _VARAPI ACCEL8_driver[];
extern int _VARAPI ACCEL16_driver[];
extern int _VARAPI ACCEL24_driver[];
extern int _VARAPI ACCEL32_driver[];
#if defined(MGLWIN) || defined(__WINDOWS__)
extern int _VARAPI FULLDIB8_driver[];
extern int _VARAPI FULLDIB16_driver[];
extern int _VARAPI FULLDIB24_driver[];
extern int _VARAPI FULLDIB32_driver[];
extern int _VARAPI DDRAW8_driver[];
extern int _VARAPI DDRAW16_driver[];
extern int _VARAPI DDRAW24_driver[];
extern int _VARAPI DDRAW32_driver[];
extern int _VARAPI OPENGL_driver[];
extern int _VARAPI FSOGL8_driver[];
extern int _VARAPI FSOGL16_driver[];
extern int _VARAPI FSOGL24_driver[];
extern int _VARAPI FSOGL32_driver[];
#endif
#endif
extern int _VARAPI PACKED1_driver[];
extern int _VARAPI PACKED4_driver[];
extern int _VARAPI PACKED8_driver[];
extern int _VARAPI PACKED16_driver[];
extern int _VARAPI PACKED24_driver[];
extern int _VARAPI PACKED32_driver[];
#endif
#endif

/*---------------------------------------------------------------------------
 * Memory allocation and utility functions.
 *-------------------------------------------------------------------------*/

#ifndef	__16BIT__
#define	_MGL_HUGE
#else
#define	_MGL_HUGE	_huge
#endif

void MGL_availableMemory(ulong *physical,ulong *total);
void MGL_useLocalMalloc(void _MGL_HUGE * (*my_malloc)(long size),void (*my_free)(void _MGL_HUGE *p));
void * MGLAPI MGL_malloc(long size);
void * MGLAPI MGL_calloc(long size,long n);
void MGLAPI MGL_free(void _MGL_HUGE *p);
void MGLAPI MGL_memset(void _MGL_HUGE *s,int c,long n);
void MGLAPI MGL_memsetw(void _MGL_HUGE *s,int c,long n);
void MGLAPI MGL_memsetl(void _MGL_HUGE *s,long c,long n);

/*---------------------------------------------------------------------------
 * Set a fullscreen suspend application callback function. This is used in
 * fullscreen video modes to allow switching back to the normal operating
 * system graphical shell (such as Windows GDI, OS/2 PM etc).
 *-------------------------------------------------------------------------*/

/* {partOf:MGL_setSuspendAppCallback} */
typedef int (ASMAPI *MGL_suspend_cb_t)(MGLDC *dc,int flags);
void	MGLAPI MGL_setSuspendAppCallback(MGL_suspend_cb_t staveState);

/*---------------------------------------------------------------------------
 * Tell the MGL to use a pre-loaded VBEAF.DRV driver file. This allows
 * you to link with the SciTech UVBELib/Accel device support libraries and
 * tell the MGL to use the device support drivers. If the user has a real
 * VBEAF.DRV driver file in the standard location on their machine, this
 * driver file will still be used.
 *-------------------------------------------------------------------------*/

void	MGLAPI MGL_setACCELDriver(void *driver);

/*---------------------- Inline functions as Macros -----------------------*/

#define	MGL_equalPoint(p1,p2)	((p1).x == (p2).x && (p1).y == (p2).y)

#define	MGL_equalRect(r1,r2)	((r1).left == (r2).left &&			\
								 (r1).top == (r2).top &&			\
								 (r1).right == (r2).right &&		\
								 (r1).bottom == (r2).bottom)

#define	MGL_emptyRect(r)		((r).bottom <= (r).top || 			\
								 (r).right <= (r).left)

#define	MGL_disjointRect(r1,r2)	((r1).right <= (r2).left ||			\
								 (r1).left >= (r2).right ||			\
								 (r1).bottom <= (r2).top ||			\
								 (r1).top >= (r2).bottom)

#define	MGL_sectRect(s1,s2,d)										\
   ((d)->left = MAX((s1).left,(s2).left),                           \
	(d)->right = MIN((s1).right,(s2).right),                        \
	(d)->top = MAX((s1).top,(s2).top),                              \
	(d)->bottom = MIN((s1).bottom,(s2).bottom),                     \
	!MGL_emptyRect(*d))

#define	MGL_sectRectFast(s1,s2,d)									\
	(d)->left = MAX((s1).left,(s2).left);                           \
	(d)->right = MIN((s1).right,(s2).right);                        \
	(d)->top = MAX((s1).top,(s2).top);                              \
	(d)->bottom = MIN((s1).bottom,(s2).bottom)

#define	MGL_sectRectCoord(l1,t1,r1,b1,l2,t2,r2,b2,d)				\
   ((d)->left = MAX(l1,l2),											\
	(d)->right = MIN(r1,r2),										\
	(d)->top = MAX(t1,t2),											\
	(d)->bottom = MIN(b1,b2),										\
	!MGL_emptyRect(*d))

#define	MGL_sectRectFastCoord(l1,t1,r1,b1,l2,t2,r2,b2,d)			\
	(d)->left = MAX(l1,l2);											\
	(d)->right = MIN(r1,r2);										\
	(d)->top = MAX(t1,t2);											\
	(d)->bottom = MIN(b1,b2)

#define	MGL_unionRect(s1,s2,d)										\
	(d)->left = MIN((s1).left,(s2).left);                           \
	(d)->right = MAX((s1).right,(s2).right);                        \
	(d)->top = MIN((s1).top,(s2).top);                              \
	(d)->bottom = MAX((s1).bottom,(s2).bottom)

#define MGL_unionRectCoord(l1,t1,r1,b1,l2,t2,r2,b2,d)				\
	(d)->left = MIN(l1,l2);                                 		\
	(d)->right = MAX(r1,r2);                              			\
	(d)->top = MIN(t1,t2);                                    		\
	(d)->bottom = MAX(b1,b2)

#define	MGL_offsetRect(r,dx,dy)										\
	{	(r).left += dx; (r).right += dx;							\
		(r).top += dy; (r).bottom += dy; }

#define	MGL_insetRect(r,dx,dy)										\
	{	(r).left += dx; (r).right -= dx;							\
		(r).top += dy; (r).bottom -= dy;							\
		if (MGL_emptyRect(r))										\
			(r).left = (r).right = (r).top = (r).bottom = 0; }

#define	MGL_ptInRect(p,r)		((p).x >= (r).left &&				\
								 (p).x < (r).right &&				\
								 (p).y >= (r).top &&				\
								 (p).y < (r).bottom)

#define	MGL_ptInRectCoord(x,y,r)	((x) >= (r).left &&				\
									 (x) < (r).right &&				\
									 (y) >= (r).top &&				\
									 (y) < (r).bottom)

#define	MGL_ptInRegion(p,r)		MGL_ptInRegionCoord((p).x,(p).y,r)

#define	MGL_pixel(p)			MGL_pixelCoord((p).x,(p).y)
#define	MGL_getPixel(p)			MGL_getPixelCoord((p).x,(p).y)
#define	MGL_pixelFast(p)		MGL_pixelCoordFast((p).x,(p).y)
#define	MGL_getPixelFast(p)		MGL_getPixelCoordFast((p).x,(p).y)
#define	MGL_moveTo(p)			MGL_moveToCoord((p).x,(p).y)
#define MGL_moveRel(p)			MGL_moveRelCoord((p).x,(p).y)
#define	MGL_line(p1,p2)			MGL_lineCoord((p1).x,(p1).y,(p2).x,(p2).y)
#define	MGL_lineFX(p1,p2)		MGL_lineCoordFX((p1).x,(p1).y,(p2).x,(p2).y)
#define	MGL_lineTo(p)			MGL_lineToCoord((p).x,(p).y)
#define	MGL_lineRel(p)			MGL_lineRelCoord((p).x,(p).y);
#define	MGL_rectPt(lt,rb)		MGL_rectCoord((lt).x,(lt).y,(rb).x,(rb).y)
#define	MGL_rect(r)				MGL_rectCoord((r).left,(r).top,			\
									(r).right,(r).bottom)
#define	MGL_drawBorder(r,s,t)	MGL_drawBorderCoord((r).left,(r).top,	\
									(r).right,(r).bottom,(s),(t))
#define	MGL_fillRectPt(lt,rb)	MGL_fillRectCoord((lt).x,(lt).y,	\
									(rb).x,(rb).y)
#define	MGL_fillRect(r)			MGL_fillRectCoord((r).left,(r).top,	\
									(r).right,(r).bottom)
#define	MGL_bitBlt(d,s,r,dl,dt,op) 	MGL_bitBltCoord((d),(s),(r).left,		\
									(r).top,(r).right,(r).bottom,dl,dt,op)
#define	MGL_bitBltLin(d,s,so,r,op) MGL_bitBltLinCoord((d),(s),so,			\
									(r).left,(r).top,(r).right,(r).bottom,op)
#define	MGL_stretchBlt(d,s,sr,dr) 	MGL_stretchBltCoord((d),(s),(sr).left,		\
									(sr).top,(sr).right,(sr).bottom,	\
									(dr).left,(dr).top,(dr).right,(dr).bottom)
#define	MGL_transBlt(d,s,r,dl,dt,c,st)	MGL_transBltCoord((d),(s),(r).left,		\
									(r).top,(r).right,(r).bottom,dl,dt,c,st)
#define	MGL_transBltLin(d,s,so,r,c,st) MGL_transBltLinCoord((d),(s),so,	\
									(r).left,(r).top,(r).right,(r).bottom,c,st)
#define	MGL_getDivot(dc,r,divot) MGL_getDivotCoord(dc,(r).left,(r).top,	\
									(r).right,(r).bottom,divot)
#define	MGL_divotSize(dc,r)		MGL_divotSizeCoord(dc,(r).left,(r).top,\
									(r).right,(r).bottom)
#define	MGL_isSimpleRegion(r)	(((region_t*)(r))->spans == NULL)
#define	MGL_rgnLine(p1,p2,p)	MGL_rgnLineCoord((p1).x,(p1).y,(p2).x,(p2).y,p)
#define	MGL_rgnLineFX(p1,p2,p)	MGL_rgnLineCoordFX((p1).x,(p1).y,(p2).x,(p2).y,p)
#define	MGL_rgnSolidRectPt(lt,rb)	MGL_rgnSolidRectCoord((lt).x,(lt).y,	\
									(rb).x,(rb).y)
#define	MGL_rgnSolidRect(r)			MGL_rgnSolidRectCoord((r).left,(r).top,	\
									(r).right,(r).bottom)

/* Fast color packing/unpacking routines implemented as macros */

#define	MGL_packColorFast(pf,R,G,B)													\
 ((ulong)(((uchar)(R) >> (pf)->redAdjust) & (pf)->redMask) << (pf)->redPos)			\
 | ((ulong)(((uchar)(G) >> (pf)->greenAdjust) & (pf)->greenMask) << (pf)->greenPos)	\
 | ((ulong)(((uchar)(B) >> (pf)->blueAdjust) & (pf)->blueMask) << (pf)->bluePos)

#define	MGL_unpackColorFast(pf,c,R,G,B)												\
{																					\
 (R) = (uchar)((((ulong)(c) >> (pf)->redPos) & (pf)->redMask) << (pf)->redAdjust);		\
 (G) = (uchar)((((ulong)(c) >> (pf)->greenPos) & (pf)->greenMask) << (pf)->greenAdjust);\
 (B) = (uchar)((((ulong)(c) >> (pf)->bluePos) & (pf)->blueMask) << (pf)->blueAdjust);	\
}

/* Macros to access the RGB components in a packed 24 bit RGB tuple */

#define	MGL_rgbRed(c)	(((uchar*)&(c))[2])
#define	MGL_rgbGreen(c)	(((uchar*)&(c))[1])
#define	MGL_rgbBlue(c)	(((uchar*)&(c))[0])

/* Fast 24 bit color packing/unpacking routines implemented as macros */

#define	MGL_packColorRGBFast(R,G,B)											\
 (((ulong)((uchar)(R)) << 16) | ((ulong)((uchar)(G)) << 8) | (uchar)(B))

#define	MGL_packColorRGBFast2(c,R,G,B)										\
{																			\
 MGL_rgbRed(c) = (uchar)(R);												\
 MGL_rgbGreen(c) = (uchar)(G);												\
 MGL_rgbBlue(c) = (uchar)(B);												\
}

#define	MGL_unpackColorRGBFast(c,R,G,B)										\
{																			\
 (R) = MGL_rgbRed(c);														\
 (G) = MGL_rgbGreen(c);														\
 (B) = MGL_rgbBlue(c);														\
}

#ifdef	__cplusplus
}						/* End of "C" linkage for C++	*/

#include "mglrect.hpp"	/* Include C++ point/rectangle classes			*/

#endif	/* __cplusplus */

/* Include soon to be obsolete 3D functions definitions */
// this is not not needed in Hexen2
//#include "mgl3d.h"

/* Include appropriate platform specific bindings */

#if defined(MGLWIN) || defined(__WINDOWS__)
#include "mglwin.h"
#elif defined(MGLPM) || defined(__OS2__)
/*#include "mglpm.h"*/
#elif defined(MGLX) || defined(__UNIX__)
/*#include "mglx.h"*/
#else
#include "mgldos.h"
#endif

/* MGL 3.0 API compatibility functions */

#ifdef	MGL_30COMPAT

/* Begin/end functions are not longer necessary. The MGL optimizes this
 * stuff internally now and avoids problems with not correctly bracketing
 * your rendering functions.
 */

#define	MGL_beginDrawing()
#define	MGL_endDrawing()
#define	MGL_beginShadedDrawing()
#define	MGL_endShadedDrawing()
#define	MGL_beginZDrawing()
#define	MGL_endZDrawing()
#define	MGL_beginZShadedDrawing()
#define	MGL_endZShadedDrawing()

/* The following fast functions have been removed, because the above
 * begin/end functions are also removed. Just use the non-fast versions of
 * the functions from now on.
 */

#define	MGL_lineFast(p1,p2)			MGL_lineCoord((p1).x,(p1).y,(p2).x,(p2).y)
#define	MGL_lineFastFX(p1,p2)		MGL_lineCoordFX((p1).x,(p1).y,(p2).x,(p2).y)
#define	MGL_cLineFast(p1,p2)		MGL_cLineCoord((p1).p.x,(p1).p.y,(p1).c,(p2).p.x,(p2).p.y,(p2).c)
#define	MGL_rgbLineFast(p1,p2)		MGL_rgbLineCoord((p1).p.x,(p1).p.y,(p1).c.r,(p1).c.g,(p1).c.b,(p2).p.x,(p2).p.y,(p2).c.r,(p2).c.g,(p2).c.b)
#define	MGL_zLineFast(p1,p2)		MGL_zLineCoord((p1).p.x,(p1).p.y,(p1).z,(p2).p.x,(p2).p.y,(p2).z)
#define	MGL_czLineFast(p1,p2)		MGL_czLineCoord((p1).p.x,(p1).p.y,(p1).z,(p1).c,(p2).p.x,(p2).p.y,(p2).z,(p2).c)
#define	MGL_rgbzLineFast(p1,p2)		MGL_rgbzLineCoord((p1).p.x,(p1).p.y,(p1).z,(p1).c.r,(p1).c.g,(p1).c.b,(p2).p.x,(p2).p.y,(p2).z,(p2).c.r,(p2).c.g,(p2).c.b)

#define	MGL_lineCoordFast(x1,y1,x2,y2)									MGL_lineCoord(x1,y1,x2,y2)
#define	MGL_lineCoordFastFX(x1,y1,x2,y2)								MGL_lineCoordFX(x1,y1,x2,y2)
#define	MGL_cLineCoordFast(x1,y1,c1,x2,y2,c2)							MGL_cLineCoord(x1,y1,c1,x2,y2,c2)
#define	MGL_rgbLineCoordFast(x1,y1,r1,g1,b1,x2,y2,r2,g2,b2)				MGL_rgbLineCoord(x1,y1,r1,g1,b1,x2,y2,r2,g2,b2)
#define	MGL_zLineCoordFast(x1,y1,zz1,x2,y2,zz2)							MGL_zLineCoord(x1,y1,zz1,x2,y2,zz2)
#define	MGL_czLineCoordFast(x1,y1,zz1,c1,x2,y2,zz2,c2)					MGL_czLineCoord(x1,y1,zz1,c1,x2,y2,zz2,c2)
#define	MGL_rgbzLineCoordFast(x1,y1,zz1,r1,g1,b1,x2,y2,zz2,r2,g2,b2)	MGL_rgbzLineCoord(x1,y1,zz1,r1,g1,b1,x2,y2,zz2,r2,g2,b2)
#define	MGL_triFast(v1,v2,v3,xOffset,yOffset)							MGL_tri(v1,v2,v3,xOffset,yOffset)
#define	MGL_cTriFast(v1,v2,v3,xOffset,yOffset)							MGL_cTri(v1,v2,v3,xOffset,yOffset)
#define	MGL_rgbTriFast(v1,v2,v3,xOffset,yOffset)						MGL_rgbTri(v1,v2,v3,xOffset,yOffset)
#define	MGL_zTriFast(v1,v2,v3,xOffset,yOffset,zzOffset)					MGL_zTri(v1,v2,v3,xOffset,yOffset,zzOffset)
#define	MGL_czTriFast(v1,v2,v3,xOffset,yOffset,zzOffset)				MGL_czTri(v1,v2,v3,xOffset,yOffset,zzOffset)
#define	MGL_rgbzTriFast(v1,v2,v3,xOffset,yOffset,zzOffset)				MGL_rgbzTri(v1,v2,v3,xOffset,yOffset,zzOffset)
#define	MGL_quadFast(v1,v2,v3,v4,xOffset,yOffset)						MGL_quad(v1,v2,v3,v4,xOffset,yOffset)
#define	MGL_cQuadFast(v1,v2,v3,v4,xOffset,yOffset)						MGL_cQuad(v1,v2,v3,v4,xOffset,yOffset)
#define	MGL_rgbQuadFast(v1,v2,v3,v4,xOffset,yOffset)					MGL_rgbQuad(v1,v2,v3,v4,xOffset,yOffset)
#define	MGL_zQuadFast(v1,v2,v3,v4,xOffset,yOffset,zzOffset)				MGL_zQuad(v1,v2,v3,v4,xOffset,yOffset,zzOffset)
#define	MGL_czQuadFast(v1,v2,v3,v4,xOffset,yOffset,zzOffset)			MGL_czQuad(v1,v2,v3,v4,xOffset,yOffset,zzOffset)
#define	MGL_rgbzQuadFast(v1,v2,v3,v4,xOffset,yOffset,zzOffset)			MGL_rgbzQuad(v1,v2,v3,v4,xOffset,yOffset,zzOffset)

/* The following polygon functions have been renamed to Cnvx versions */

#define	MGL_fillPolygonFast(count,vArray,xOffset,yOffset)			MGL_fillPolygonCnvx(count,vArray,xOffset,yOffset)
#define	MGL_fillPolygonFastFX(count,vArray,vinc,xOffset,yOffset)	MGL_fillPolygonCnvxFX(count,vArray,vinc,xOffset,yOffset)

#endif	/* MGL_30COMPAT */

#pragma pack()

#endif	/* __MGRAPH_H */

