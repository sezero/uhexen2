/* wgl_func.h -- WGL functions we may need to link to
 * make sure NOT to protect this file against multiple inclusions!
 *
 * Copyright (C) 2005-2016  O.Sezer <sezero@users.sourceforge.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

/* core wgl functions
 */
#ifdef GL_DLSYM

#ifndef GL_FUNCTION
#define GL_FUNCTION(ret, func, params) \
typedef ret (WINAPI *func##_f) params; \
static func##_f func##_fp;
#endif

GL_FUNCTION(PROC, wglGetProcAddress, (LPCSTR))
GL_FUNCTION(HGLRC, wglCreateContext, (HDC))
GL_FUNCTION(BOOL, wglDeleteContext, (HGLRC))
GL_FUNCTION(BOOL, wglMakeCurrent, (HDC, HGLRC))
GL_FUNCTION(HGLRC, wglGetCurrentContext, (VOID))
GL_FUNCTION(HDC, wglGetCurrentDC, (VOID))

#else

#ifndef WGL_FUNC_H
#define WGL_FUNC_H

#define wglGetProcAddress_fp	wglGetProcAddress
#define wglCreateContext_fp	wglCreateContext
#define wglDeleteContext_fp	wglDeleteContext
#define wglMakeCurrent_fp	wglMakeCurrent
#define wglGetCurrentContext_fp	wglGetCurrentContext
#define wglGetCurrentDC_fp	wglGetCurrentDC

#endif	/* WGL_FUNC_H */

#endif	/* !defined(GL_DLSYM) */

#undef GL_FUNCTION


/* wgl funcs needed when using a standalone (minigl) driver
 */
#ifdef GL_DLSYM

#ifndef GL_FUNCTION_OPT
#define GL_FUNCTION_OPT(ret, func, def, params) \
typedef ret (WINAPI *func##_f) params; \
static func##_f func##_fp;
#endif

GL_FUNCTION_OPT(int, wglChoosePixelFormat, ChoosePixelFormat, (HDC, CONST PIXELFORMATDESCRIPTOR *))
GL_FUNCTION_OPT(int, wglDescribePixelFormat, DescribePixelFormat, (HDC, int, UINT, LPPIXELFORMATDESCRIPTOR))
GL_FUNCTION_OPT(int, wglGetPixelFormat, GetPixelFormat, (HDC))
GL_FUNCTION_OPT(BOOL, wglSetPixelFormat, SetPixelFormat, (HDC, int, CONST PIXELFORMATDESCRIPTOR *))
GL_FUNCTION_OPT(BOOL, wglSwapBuffers, SwapBuffers, (HDC))

#else

#ifndef WGL_OPTFUNC_H
#define WGL_OPTFUNC_H

#define wglChoosePixelFormat_fp		ChoosePixelFormat
#define wglDescribePixelFormat_fp	DescribePixelFormat
#define wglGetPixelFormat_fp		GetPixelFormat
#define wglSetPixelFormat_fp		SetPixelFormat
#define wglSwapBuffers_fp		SwapBuffers

#endif	/* WGL_OPTFUNC_H */

#endif	/* !defined(GL_DLSYM) */

#undef GL_FUNCTION_OPT


/* typedefs for wgl functions linked to at runtime
 */
#ifndef WGL_FUNC_TYPEDEFS
#define WGL_FUNC_TYPEDEFS

typedef BOOL (WINAPI *wglSwapIntervalEXT_f) (int);

#endif /* WGL_FUNC_TYPEDEFS */
