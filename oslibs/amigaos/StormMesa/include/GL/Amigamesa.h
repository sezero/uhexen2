/* Amigamesa.h */

/*
 * Mesa 3-D graphics library
 * Version:  1.2
 * Copyright (C) 1995  Brian Paul  (brianp@ssec.wisc.edu)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */


/* Important note (03.01.1998 by Sam Jordan)

When using windows you should use window->BorderBottom for AMA_Bottom,
not window->BorderBottom+1! Please change your existing MESA application
when using this implementation here. The reason for this change is that
the current implementation doesn't decrease the viewport size by 2 to
get an additional border (which doesn't look nice and which didn't work
correctly either).

When using the TK window layer you don't have to change anything.

*/


/*
Implementions of new drawing rutines:

you implement a own init for your rutines/hardware
and make some test and calls it from AmigaMesaCreateContext()
(look in the file src/amigamesa.c I'll thing you get it)
Be sure to fill this three ponters out:
	void (*InitDD)( void );
	void (*Dispose) (struct amigamesa_context *c);
	void (*SwapBuffer) (void);
where InitDD sets the DD structure in orginal mesa with pointers to drawing rutines
Dispose is called when someone quits/closes down your made inits
SwapBuffer is called when one is changing buffer in dubble buffering mode

Write nice drawing rutines like those in src/amigamesa.c on make sure
that it's those you set in your InitDD rutine.

Add enum for your drawingmode for the taglist and if you need more tags also implement them
If posible some autodetection code in AmigaMesaCreateContext
Add enums and error codes if you neads 

PUT ALL YOUR NEADED DATA IN amigamesa_context->(void *)data in for your gfx driver
private structure.

Send the code to me and I will include it in the main code.
*/

/*
$Id: Amigamesa.h 1.4 1996/10/13 20:54:31 StefanZ Exp StefanZ $

$Log: Amigamesa.h $
 * Revision 1.4  1996/10/13  20:54:31  StefanZ
 * Updated to reflect amigamesa 1.5
 *
 * Revision 1.3  1996/10/06  20:35:11  StefanZ
 * Source bump before Mesa 2.0
 *
 * Revision 1.2  1996/08/14  22:16:31  StefanZ
 * New Api to amigaspecific functions, Added suport for gfx-cards
 *
 * Revision 1.1  1996/06/02  00:15:03   StefanZ
 * Initial revision
 *
 * Revision 1.0  1996/02/21  11:09:45   brianp
 * A copy of amesa.h version 1.4 in a brave atempt to make a amiga interface
 *
 */


/* Example usage:

1. Make a window using Intuition calls

2. Call AMesaCreateContext() to make a rendering context and attach it
	to the window made in step 1.

3. Call AMesaMakeCurrent() to make the context the active one.

4. Make gl* calls to render your graphics.

5. When exiting, call AMesaDestroyContext().

*/



#ifndef AMIGAMESA_H
#define AMIGAMESA_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef APIENTRY
#ifndef NOSAVEDS
#define APIENTRY __saveds
#else
#define APIENTRY
#endif
#endif

#include <exec/libraries.h>
#include <intuition/intuition.h>
#include <utility/tagitem.h>
#include "GL/gl.h"



struct amigamesa_visual
{
   void *gl_visual;
   GLboolean db_flag;           /* double buffered? */
   GLboolean rgb_flag;          /* RGB mode? */
   GLboolean alpha_flag;   /* Alphacolor? */
   GLuint        depth;                         /* bits per pixel (1, 8, 24, etc) */
   GLuint        stencil;
   GLuint        accum;
};



struct amigamesa_buffer
{
   void *gl_buffer;    /* The depth, stencil, accum, etc buffers */
   /* your window handle, etc */
};


/*
 * This is the Amiga/Mesa context structure.  This usually contains
 * info about what window/buffer we're rendering too, the current
 * drawing color, etc.
 */

/* 5.9.98 sj
   this structure must not be accessed directly by applications
   anymore */

struct amigamesa_context
{
	void *gl_ctx;                   /* the core library context */
	struct amigamesa_visual  *visual; /* the visual context */
	struct amigamesa_buffer  *buffer; /* the buffer context */

	struct amigamesa_context *share;

	unsigned long flags;                    /*0x1 = own visuel, 0x2 = own buffer
						  0x4 = forbid 3D HW, 0x8 = fullscreen mode */


	unsigned long pixel;                    /* current color index or RGBA pixel value */
	unsigned long clearpixel;       /* pixel for clearing the color buffers */

	/* etc... */
	struct Window *window;                  /* Not neaded if in dubbelbuff needed */                /* the Intuition window */
	struct RastPort *front_rp;      /* front rastport */
	struct RastPort *back_rp;               /* back rastport */
	int    SwapCounter;                     /* which buffer is active */
	UBYTE* FrontArray;                                      /* for multibuffering */
	UBYTE* BackArray;                                       /*a pen Array big as drawing area for use in dubelbuff mode*/
	struct RastPort *rp;                    /* current rastport */
	struct Screen *Screen;                  /* current screen*/
	struct TmpRas *tmpras;                  /* tmpras rastport */
	struct RastPort *temprp;
	struct DBufInfo *dbufinfo;              /* OS3.0 multibuffering */

	GLuint depth;                                           /* bits per pixel (1, 8, 24, etc) */
	GLuint bppix;                           /* bytes per pixel */
	GLuint bprow;                           /* bytes per row */
	GLuint fmt;                             /* color format */

	GLuint  width, height;                          /* drawable area */
	GLint           left, bottom;                           /* offsets due to window border */
	GLint           right, top;             /* right, top offsets, needed for correct resizing */
	GLint           RealWidth,RealHeight;   /* the drawingareas real size*/
	GLint           FixedWidth,FixedHeight; /* The internal buffer real size speeds up the drawing a bit*/
	long*           ColorTable;             /* LUT8 display: ARGB -> real pen conversion */
						/* ARGB display: ARGB -> GL pen conversion */
	long*           ColorTable2;            /* LUT8 display: table of allocated pens */
						/* ARGB display: GL pen -> ARGB conversion */
	UBYTE penconv[256];                     /* pen conversion from GL color to real color */
	UBYTE penconvinv[256];                  /* pen conversion from real color to GL color */
	UBYTE dtable[256];
	UBYTE bugfix[256];
	UBYTE *imageline;                                       /* One Line for WritePixelRow renders */
	GLuint *rgb_buffer;                                     /*back buffer when in RGBA mode OLD DElete?*/

	void (*InitDD)(void *  );                                                                  /* keep track of witch drawing rutines should be used */
	void (*Dispose) (struct amigamesa_context *c);          /* Use this when AmigaMesaDestroyContext is called */
	void (*SwapBuffer) (struct amigamesa_context *c);       /* Use this when AmigaMesaSwapBuffers is called */
	ULONG ColTable[256];
	ULONG ColTable2[256];
	ULONG oldFPU;                           /* old rounding mode */
	UBYTE pixelargb[4];
	ULONG specialalloc;
	UBYTE dmatrix[128+8];
	struct ScreenBuffer *sbuf1;              /* OS3.0 multibuffering */
	struct ScreenBuffer *sbuf2;              /* OS3.0 multibuffering */
	struct MsgPort* dbport;                  /* OS3.0 multibuffering */
	int drawbufferflag;                     /* GL double buffering support  */
	int readbufferflag;                     /*              "               */
	int backarrayflag;                      /*              "               */
	UBYTE* DrawBuffer;                      /*              "               */
	UBYTE* ReadBuffer;                      /*              "               */
	struct RastPort *back2_rp;              /* second back rastport */
	struct RastPort *draw_rp;
	void *hwdriver;                         /* hwdriver private structure */
	struct ScreenBuffer *sbuf3;             /* third screen buffer */
	struct ScreenBuffer *sbuf_initial;      /* initial screen buffer */
};

/* possible values for flags */

#define FLAG_OWNVISUAL    0x01             /*see AMA_Visual*/
#define FLAG_OWNBUFFER    0x02             /*see AMA_Buffer*/
#define FLAG_FORBID3DHW   0x04             /*forbid using 3D-HW*/
#define FLAG_FULLSCREEN   0x08             /*fullscreen mode*/
#define FLAG_DIRECTRENDER 0x10             /*direct rendering to gfx RAM*/
#define FLAG_TWOBUFFERS   0x20             /*two color buffers*/
#define FLAG_FAST         0x40             /*less quality, more speed*/
#define FLAG_VERYFAST     0x80             /*AFAP*/
#define FLAG_RGBA         0x100            /*private*/
#define FLAG_SYNC         0x200            /*synchronize window refresh*/
#define FLAG_TRIPLE       0x400            /*triple buffering*/

typedef struct amigamesa_context *AmigaMesaContext;

/* support functions in separate link libs */

struct Library* OpenGLLibrary(void);
struct Library* OpenGLULibrary(struct Library*);
struct Library* OpenGLUTLibrary(struct Library*, struct Library* );
struct Library* OpenGLELibrary(struct Library*, struct Library* );
struct Library* OpenGLSMAPLibrary(struct Library*, struct Library* );
void CloseGLLibrary(struct Library* );
void CloseGLULibrary(struct Library* );
void CloseGLUTLibrary(struct Library* );
void CloseGLELibrary(struct Library* );
void CloseGLSMAPLibrary(struct Library* );

/**********************************************************************/
/*****                                  Amiga/Mesa API Functions                                                        *****/
/**********************************************************************/
struct amigamesa_visual *AmigaMesaCreateVisualTags(long Tag1, ...);
struct amigamesa_context *AmigaMesaCreateContextTags(long Tag1, ...);
void AmigaMesaSetRastTags(struct amigamesa_context* c, long Tag1, ...);

#ifndef __STORM__
#ifdef __VBCC__
struct amigamesa_visual* APIENTRY AmigaMesaCreateVisual(struct TagItem *tagList);
struct amigamesa_context* APIENTRY AmigaMesaCreateContext(struct TagItem *tagList );
void APIENTRY AmigaMesaDestroyContext(struct amigamesa_context *c );
void APIENTRY AmigaMesaDestroyVisual(struct amigamesa_visual *v );
void APIENTRY AmigaMesaMakeCurrent(struct amigamesa_context *c ,struct amigamesa_buffer *b);
void APIENTRY AmigaMesaSwapBuffers(struct amigamesa_context *amesa);
void APIENTRY AmigaMesaSetOneColor(struct amigamesa_context *c,int index,float r,float g,float b);
void APIENTRY AmigaMesaSetRast(struct amigamesa_context *c, struct TagItem *tagList );
void APIENTRY AmigaMesaGetConfig(struct amigamesa_visual *v, GLenum pname, GLint* params );
#if defined(GL_INLINE) || defined(__PPC__)
#include <proto/amigamesa.h>
#endif
#else
#ifdef __GNUC__ /* if under EGCS/GCC environement */
#ifndef __cplusplus /* Just to prevent if we include this file in g++ mode */
APIENTRY struct amigamesa_visual *AmigaMesaCreateVisual(struct TagItem *tagList __asm("a0"));
APIENTRY struct amigamesa_context *AmigaMesaCreateContext(struct TagItem *tagList __asm("a0"));
APIENTRY void AmigaMesaDestroyVisual(struct amigamesa_visual *v __asm("a0"));
APIENTRY void AmigaMesaDestroyContext(struct amigamesa_context *c __asm("a0"));
APIENTRY void AmigaMesaMakeCurrent(struct amigamesa_context *c __asm("a0"), struct amigamesa_buffer *b __asm("a1"));
APIENTRY void AmigaMesaSwapBuffers(struct amigamesa_context *amesa __asm("a0"));
APIENTRY void AmigaMesaSetOneColor(struct amigamesa_context *c __asm("a0"), int index __asm("d0"), float r __asm("fp0"), float g __asm("fp1"), float b __asm("fp2"));
APIENTRY void AmigaMesaSetRast(struct amigamesa_context *c __asm("a0"), struct TagItem *tagList __asm("a1"));
APIENTRY void AmigaMesaGetConfig(struct amigamesa_visual *v __asm("a0"), GLenum pname __asm("d0"), GLint* params __asm("a1"));
#endif
#else
__asm APIENTRY struct amigamesa_visual *AmigaMesaCreateVisual(register __a0 struct TagItem *tagList);
__asm APIENTRY struct amigamesa_context *AmigaMesaCreateContext(register __a0 struct TagItem *tagList );
__asm APIENTRY void AmigaMesaDestroyVisual(register __a0 struct amigamesa_visual *v );
__asm APIENTRY void AmigaMesaDestroyContext(register __a0 struct amigamesa_context *c );
__asm APIENTRY void AmigaMesaMakeCurrent(register __a0 struct amigamesa_context *c ,register __a1    struct amigamesa_buffer *b);
__asm APIENTRY void AmigaMesaSwapBuffers(register __a0 struct amigamesa_context *amesa);
__asm APIENTRY void AmigaMesaSetOneColor(register __a0 struct amigamesa_context *c, register __d0 int index, register __fp0 float r, register __fp1 float g, register __fp2 float b);
__asm APIENTRY void AmigaMesaSetRast(register __a0 struct amigamesa_context *c, register __a1 struct TagItem *tagList );
__asm APIENTRY void AmigaMesaGetConfig(register __a0 struct amigamesa_visual *v, register __d0 GLenum pname, register __a1 GLint* params );
#endif
#endif
#else
#ifndef __PPC__
struct amigamesa_visual* APIENTRY AmigaMesaCreateVisual(register __a0 struct TagItem *tagList);
struct amigamesa_context* APIENTRY AmigaMesaCreateContext(register __a0 struct TagItem *tagList );
void APIENTRY AmigaMesaDestroyContext(register __a0 struct amigamesa_context *c );
void APIENTRY AmigaMesaDestroyVisual(register __a0 struct amigamesa_visual *v );
void APIENTRY AmigaMesaMakeCurrent(register __a0 struct amigamesa_context *c ,register __a1    struct amigamesa_buffer *b);
void APIENTRY AmigaMesaSwapBuffers(register __a0 struct amigamesa_context *amesa);
void APIENTRY AmigaMesaSetOneColor(register __a0 struct amigamesa_context *c, register __d0 int index, register __fp0 float r, register __fp1 float g, register __fp2 float b);
void APIENTRY AmigaMesaSetRast(register __a0 struct amigamesa_context *c, register __a1 struct TagItem *tagList );
void APIENTRY AmigaMesaGetConfig(register __a0 struct amigamesa_visual *v, register __d0 GLenum pname, register __a1 GLint* params );
#else
struct amigamesa_visual* APIENTRY AmigaMesaCreateVisual(struct TagItem *tagList);
struct amigamesa_context* APIENTRY AmigaMesaCreateContext(struct TagItem *tagList );
void APIENTRY AmigaMesaDestroyContext(struct amigamesa_context *c );
void APIENTRY AmigaMesaDestroyVisual(struct amigamesa_visual *v );
void APIENTRY AmigaMesaMakeCurrent(struct amigamesa_context *c ,struct amigamesa_buffer *b);
void APIENTRY AmigaMesaSwapBuffers(struct amigamesa_context *amesa);
void APIENTRY AmigaMesaSetOneColor(struct amigamesa_context *c,int index,float r,float g,float b);
void APIENTRY AmigaMesaSetRast(struct amigamesa_context *c, struct TagItem *tagList );
void APIENTRY AmigaMesaGetConfig(struct amigamesa_visual *v, GLenum pname, GLint* params );
#endif
#endif




/*
 * Amiga Mesa Attribute tag ID's.  These are used in the ti_Tag field of
 * TagItem arrays passed to AmigaMesaSetDefs() and AmigaMesaCreateContext()
 */
#define AMA_Dummy       (TAG_USER + 32)

/*
Offset to use. WARNING AMA_Left, AMA_Bottom Specifies the low left corner
of the drawing area in deltapixles from the lowest left corner
typical AMA_Left,window->BorderLeft
		  AMA_Bottom,window->BorderBottom + 1

03.01.1998 Note by Sam Jordan: Use window->BorderBottom for this current
implementation. Now the viewport size isn't decreased by 2 as in previous
implementations.


This is since ALL gl drawing actions is specified with this point as 0,0
and with y positive uppwards (like in real graphs).

Untuched (defult) will result in 
AMA_Left=0;
AMA_Bottom=0;
*/
#define AMA_Left                (AMA_Dummy + 0x0001)
#define AMA_Bottom      (AMA_Dummy + 0x0002)

/*
Size in pixels of drawing area if others than the whole rastport.
All internal drawingbuffers will be in this size

Untuched (defult) will result in 
AMA_Width =rp->BitMap->BytesPerRow*8;
AMA_Height=rp->BitMap->Rows;
*/
#define AMA_Width       (AMA_Dummy + 0x0003)
#define AMA_Height      (AMA_Dummy + 0x0004)

/*
AMA_DrawMode: Specifies the drawing hardware and should be one of
				  AGA,(CYBERGFX,RETINA)
				  Defult value: AGA
if AMESA_AGA Amiga native drawigns
	this has to be filled with data
		AMA_Window = (ptr) Window to draw on
	or
		AMA_Screen =(ptr) Screen to draw on.
		AMA_RastPort =(ptr) RastPort to draw on.
if AMESA_AGA_C2P Amiga native drawing using a chunky buffer
		 thats converted when switching drawbuffer
		 only works on doublebuffered drawings.
	this has to be filled with data
		AMA_DoubleBuf = GL_TRUE
		AMA_Window = (ptr) Window to draw on
	or
		AMA_DoubleBuf = GL_TRUE
		AMA_Screen =(ptr) Screen to draw on.
		AMA_RastPort =(ptr) RastPort to draw on.

else
   here should all needed gfx-card tagitem be specified
*/

enum DrawModeID {AMESA_AGA,AMESA_AGA_C2P /*,AMESA_CYBERGFX,AMESA_RETINA*/};
#define AMA_DrawMode    (AMA_Dummy + 0x0005)
#define AMA_Screen      (AMA_Dummy + 0x0006)
#define AMA_Window      (AMA_Dummy + 0x0007)
#define AMA_RastPort    (AMA_Dummy + 0x0008)

/** booleans **/
/*
AMA_DoubleBuf: If specified it uses double Buffering (change buffer with
					AmigaMesaSwapBuffers()) Turn this on as much as posible
					it will result in smother looking and faster rendering
					Defult value: GL_FALSE
AMA_RGBMode: If specified it uses 24bit when drawing (on non 24bit displays it
				 it emuletes 24bit)
				 Defult value: GL_TRUE
AMA_AlphaFlag: Alphachanel ?
				   Defule value: GL_FALSE

AMA_Forbid3DHW: if set to GL_TRUE, no 3D HW is used, but a software engine
		default = GL_FALSE (3D-HW allowed)
AMA_Fullscreen: if set to GL_TRUE, a fullscreen driver is launched (supporting
		true multibuffering). Also available in 3D-HW mode.
AMA_DirectRender: if set to GL_TRUE in non-fullscreen-mode, the frame is rendered
		directly into the gfx RAM, the frame is then copied using the blitter.
		(not available for AGA)
AMA_TwoBuffers: allocates two back buffers in double-buffered window mode
		(or AGA fullscreen mode). This is absolutely required, if the GL
		applications requires access to the front buffer
		(glDrawBuffer(GL_FRONT), glReadBuffer(GL_FRONT)). Never set this
		flag, if you don't use these special features.
AMA_Fast:       Tries to render faster as usual, but tries to maintain enough
		OpenGL compatibility for accurate results
AMA_VeryFast:   Tries to render as fast as possible and doesn't care at all
		for OpenGL compliant results.
		If both AMA_Fast and AMA_VeryFast are given, then AMA_VeryFast
		takes precedence. If none of these tags are given, then the
		graphics quality will be maintained as high as possible. Note,
		that 3D hardware might support some effects, but might also
		give slightly different results. Therefore if you want 100
		percent GL compliant results, you should disable 3D hardware
		support using AMA_Forbid3DHW.
AMA_NoDepth:    don't allocate ZBuffer if GL_TRUE
AMA_NoStencil:  don't allocate StencilBuffer if GL_TRUE
AMA_NoAccum:    don't allocate AccumulationBuffer if GL_TRUE
*/
#define AMA_DoubleBuf (AMA_Dummy + 0x0030)
#define AMA_RGBMode      (AMA_Dummy + 0x0031)
#define AMA_AlphaFlag (AMA_Dummy + 0x0032)
#define AMA_Forbid3DHW (AMA_Dummy + 0x0033)
#define AMA_Fullscreen (AMA_Dummy + 0x0034)
#define AMA_DirectRender (AMA_Dummy + 0x0035)
#define AMA_TwoBuffers (AMA_Dummy + 0x0036)
#define AMA_Fast (AMA_Dummy + 0x0037)
#define AMA_VeryFast (AMA_Dummy + 0x0038)
#define AMA_NoDepth (AMA_Dummy + 0x0039)
#define AMA_NoStencil (AMA_Dummy + 0x003a)
#define AMA_NoAccum (AMA_Dummy + 0x003b)

/** Special **/
/*
AMA_Visual:     If you want to implement your own amigamesa_visual 
AMA_Buffer: If you want to implement your own amigamesa_buffer
AMA_WindowID: A windowID to use when I alloc AMA_Buffer for you if
				  you didn't supply one.(defult=1)
*/

#define AMA_Visual                      (AMA_Dummy + 0x0041)
#define AMA_Buffer                      (AMA_Dummy + 0x0042)
#define AMA_WindowID                    (AMA_Dummy + 0x0043)

#ifdef __cplusplus
}
#endif


#endif
