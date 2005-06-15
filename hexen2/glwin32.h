/*
	glwin32.h
	glquake header for win32/wgl

	$Id: glwin32.h,v 1.4 2005-06-15 21:32:41 sezero Exp $
*/


// disable data conversion warnings
#pragma warning(disable : 4244)     // MIPS
#pragma warning(disable : 4136)     // X86
#pragma warning(disable : 4051)     // ALPHA
  
#include <windows.h>

#include <gl\gl.h>
#include <gl\glu.h>

#include "gl_func_nondl.h"

