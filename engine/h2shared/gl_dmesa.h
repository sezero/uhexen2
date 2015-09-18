/*
 * gl_dmesa.h -- DMesa functions we may need to link to
 * make sure NOT to protect this file against multiple inclusions!
 *
 * Copyright (C) 2015  O.Sezer <sezero@users.sourceforge.net>
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

#ifdef GL_DLSYM

#ifndef GL_FUNCTION
#define UNDEF_GL_FUNCTION
#define GL_FUNCTION(ret, func, params) \
typedef ret (*func##_f) params; \
func##_f func##_fp;
#endif

GL_FUNCTION(DMesaProc, DMesaGetProcAddress, (const char *))
GL_FUNCTION(DMesaContext, DMesaCreateContext, (DMesaVisual, DMesaContext))
GL_FUNCTION(void, DMesaDestroyContext, (DMesaContext))
GL_FUNCTION(GLboolean, DMesaMakeCurrent, (DMesaContext, DMesaBuffer))
GL_FUNCTION(DMesaVisual, DMesaCreateVisual, (GLint, GLint, GLint, GLint, GLboolean, GLboolean, GLint, GLint, GLint, GLint))
GL_FUNCTION(void, DMesaDestroyVisual, (DMesaVisual))
GL_FUNCTION(DMesaBuffer, DMesaCreateBuffer, (DMesaVisual, GLint, GLint, GLint, GLint))
GL_FUNCTION(void, DMesaDestroyBuffer, (DMesaBuffer))
GL_FUNCTION(void, DMesaSwapBuffers, (DMesaBuffer))

#ifdef UNDEF_GL_FUNCTION
#undef GL_FUNCTION
#undef UNDEF_GL_FUNCTION
#endif

#else

#define DMesaGetProcAddress_fp	DMesaGetProcAddress
#define DMesaCreateContext_fp	DMesaCreateContext
#define DMesaDestroyContext_fp	DMesaDestroyContext
#define DMesaMakeCurrent_fp	DMesaMakeCurrent
#define DMesaCreateVisual_fp	DMesaCreateVisual
#define DMesaDestroyVisual_fp	DMesaDestroyVisual
#define DMesaCreateBuffer_fp	DMesaCreateBuffer
#define DMesaDestroyBuffer_fp	DMesaDestroyBuffer
#define DMesaSwapBuffers_fp	DMesaSwapBuffers

#endif

