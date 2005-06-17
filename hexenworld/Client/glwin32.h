/*
	glwin32.h
	glquake header for win32/wgl

	$Id: glwin32.h,v 1.4 2005-06-17 14:46:30 sezero Exp $
*/


// disable data conversion warnings
#pragma warning(disable : 4244)     // MIPS
#pragma warning(disable : 4136)     // X86
#pragma warning(disable : 4051)     // ALPHA
  
#include <windows.h>

#include <gl\gl.h>
#include <gl\glu.h>

// whether to dlsym gl and wgl function calls
// dlsymming is broken for now, dont use it.
//#define GL_DLSYM	1
#undef GL_GLSYM

#ifdef GL_DLSYM
#include "gl_func.h"
#else
#include "gl_func_nondl.h"
#endif

