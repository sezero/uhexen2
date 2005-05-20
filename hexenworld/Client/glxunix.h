/*
	glxunix.h
	glquake header for unix/sdl

	$Id: glxunix.h,v 1.1 2005-05-20 18:16:45 sezero Exp $
*/


#include <GL/gl.h>
#include <GL/glu.h>
#include "gl_func.h"

#define	bindTexFunc(a, b) glfunc.glBindTexture_fp((a), (b))

