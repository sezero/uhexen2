/*
 * glheader.h: opengl system includes
 * $Id$
 */


#ifndef __GLHEADER_H
#define __GLHEADER_H

#if defined(PLATFORM_WINDOWS)
#include <windows.h>
#include <GL/gl.h>

#elif defined(PLATFORM_OSX)
#include <OpenGL/gl.h>

#elif defined(PLATFORM_MAC)
#include <gl.h>

#elif defined(__MORPHOS__)
#include <proto/tinygl.h>
#include <tgl/gl.h>

#elif defined(__AROS__)
/*#include <GL/arosmesa.h>*/
#include <GL/gl.h>

#elif defined(__amigaos4__)
/*#include <mgl/gl.h>*/
#include <GL/gl.h>

#elif defined(__AMIGA__) /* AOS3 */
#if defined(REFGL_MINIGL)		/* Hyperion's MiniGL 1.2 */
#define USE_MGLAPI 1
#include <mgl/gl.h>
#elif defined(REFGL_AMESA)			/* StormMesa */
#include <GL/gl.h>
#else
#error Which Amiga GL API to use not specified
#endif

#else	/* other unix */
#include <GL/gl.h>
#endif

#ifndef APIENTRY
#define	APIENTRY
#endif

/* include our function pointers */
#include "gl_func.h"

#ifndef	GLX_3DFX_WINDOW_MODE_MESA
#define	GLX_3DFX_WINDOW_MODE_MESA		0x1
#endif

#ifndef	GLX_3DFX_FULLSCREEN_MODE_MESA
#define	GLX_3DFX_FULLSCREEN_MODE_MESA		0x2
#endif

#ifndef	GL_TEXTURE0_ARB
#define	GL_TEXTURE0_ARB				0x84C0
#define	GL_TEXTURE1_ARB				0x84C1
#define	GL_TEXTURE2_ARB				0x84C2
#define	GL_TEXTURE3_ARB				0x84C3
#define	GL_TEXTURE4_ARB				0x84C4
#define	GL_TEXTURE5_ARB				0x84C5

#define	GL_ACTIVE_TEXTURE_ARB			0x84E0
#define	GL_CLIENT_ACTIVE_TEXTURE_ARB		0x84E1
#define	GL_MAX_TEXTURE_UNITS_ARB		0x84E2
#endif

#ifndef	GL_MULTISAMPLE_ARB
#define	GL_MULTISAMPLE_ARB			0x809D
#endif

#ifndef	GL_SHARED_TEXTURE_PALETTE_EXT
#define	GL_SHARED_TEXTURE_PALETTE_EXT		0x81FB
#endif

#ifndef	GL_COLOR_INDEX8_EXT
#define	GL_COLOR_INDEX8_EXT			0x80E5
#endif

#ifndef	GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT
#define	GL_TEXTURE_MAX_ANISOTROPY_EXT		0x84FE
#define	GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT	0x84FF
#endif

#endif	/* __GLHEADER_H */

