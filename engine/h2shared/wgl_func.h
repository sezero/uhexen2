/*
 * wgl_func.h -- WGL functions we may need to link to
 * make sure NOT to protect this file against multiple inclusions!
 * $Id: wgl_func.h,v 1.4 2007-03-14 21:03:28 sezero Exp $
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

/* core wgl functions */

#ifdef GL_DLSYM
#ifndef GL_FUNCTION
#define UNDEF_GL_FUNCTION
#define GL_FUNCTION(ret, func, params) \
typedef ret (WINAPI *func##_f) params; \
func##_f func##_fp;
#endif

GL_FUNCTION(PROC, wglGetProcAddress, (LPCSTR))
GL_FUNCTION(HGLRC, wglCreateContext, (HDC))
GL_FUNCTION(BOOL, wglDeleteContext, (HGLRC))
GL_FUNCTION(BOOL, wglMakeCurrent, (HDC, HGLRC))
GL_FUNCTION(HGLRC, wglGetCurrentContext, (VOID))
GL_FUNCTION(HDC, wglGetCurrentDC, (VOID))

#ifdef UNDEF_GL_FUNCTION
#undef GL_FUNCTION
#undef UNDEF_GL_FUNCTION
#endif

#else

#ifndef WGL_FUNC_H
#define WGL_FUNC_H

#define wglGetProcAddress_fp	wglGetProcAddress
#define wglCreateContext_fp	wglCreateContext
#define wglDeleteContext_fp	wglDeleteContext
#define wglMakeCurrent_fp	wglMakeCurrent
#define wglGetCurrentContext_fp	wglGetCurrentContext
#define wglGetCurrentDC_fp	wglGetCurrentDC
/*
#define wglGetExtensionsStringARB_fp	wglGetExtensionsStringARB
#define wglSwapBuffers_fp		wglSwapBuffers
#define wglSwapIntervalEXT_fp		wglSwapIntervalEXT
*/

#endif	/* WGL_FUNC_H */

#endif	/* !defined(GL_DLSYM) */


/* optional wgl funcs */

#ifndef GL_FUNCTION_OPT
#define UNDEF_GL_FUNCTION_OPT
#define GL_FUNCTION_OPT(ret, func, params) \
typedef ret (WINAPI *func##_f) params; \
func##_f func##_fp;
#endif

/* like above, but just typedef only */
#ifndef GL_FUNCTION_OPT2
#define UNDEF_GL_FUNCTION_OPT2
#define GL_FUNCTION_OPT2(ret, func, params) \
typedef ret (APIENTRY *func##_f) params;
#endif

GL_FUNCTION_OPT2(const char *, wglGetExtensionsStringARB, (HDC hdc))
GL_FUNCTION_OPT2(BOOL, wglSwapBuffers, (HDC))
GL_FUNCTION_OPT2(BOOL, wglSwapIntervalEXT, (int))

#ifdef UNDEF_GL_FUNCTION_OPT
#undef GL_FUNCTION_OPT
#undef UNDEF_GL_FUNCTION_OPT
#endif

#ifdef UNDEF_GL_FUNCTION_OPT2
#undef GL_FUNCTION_OPT2
#undef UNDEF_GL_FUNCTION_OPT2
#endif
