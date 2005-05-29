/*
	glxunix.h
	glquake header for unix/sdl

	$Id: glxunix.h,v 1.2 2005-05-29 11:23:13 sezero Exp $
*/


#include <GL/gl.h>
#include <GL/glu.h>
#include "gl_func.h"

#ifndef	GLX_3DFX_WINDOW_MODE_MESA
#define	GLX_3DFX_WINDOW_MODE_MESA	0x1
#endif

#ifndef	GLX_3DFX_FULLSCREEN_MODE_MESA
#define	GLX_3DFX_FULLSCREEN_MODE_MESA	0x2
#endif

#define	bindTexFunc(a, b) glfunc.glBindTexture_fp((a), (b))

