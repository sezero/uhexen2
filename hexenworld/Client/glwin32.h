/*
	glwin32.h
	glquake header for win32/wgl

	$Id: glwin32.h,v 1.7 2005-09-12 08:05:42 sezero Exp $
*/


#if !defined(__GNUC__)
// disable data conversion warnings
#pragma warning(disable : 4244)     // MIPS
#pragma warning(disable : 4136)     // X86
#pragma warning(disable : 4051)     // ALPHA
#endif

#include <windows.h>

#include <GL/gl.h>
#include <GL/glu.h>

// whether to dlsym gl and wgl function calls
// the define GL_DLSYM is decided in Makefile.mingw
#ifdef GL_DLSYM
#include "gl_func.h"
#else
#include "gl_func_nondl.h"
#endif

