/* gl_dmesa.c -- DOS OpenGL refresh using DMesa api.
 * for use with Mesa library version 5.x or 6.x possibly built
 * against 3dfx glide.
 * Copyright (C) 2015 O.Sezer <sezero@users.sourceforge.net>
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

#include "quakedef.h"
#include "gl_dos.h"
#include "sys_dxe.h"

#if !defined(REFGL_MESA)
int DMESA_LoadAPI (void *handle)
{
	return -1;
}

#else

#include <GL/dmesa.h>

#if defined(GL_DLSYM)
typedef DMesaVisual (*DMesaCreateVisual_f) (GLint, GLint, GLint, GLint, GLboolean, GLboolean, GLint, GLint, GLint, GLint);
typedef void (*DMesaDestroyVisual_f) (DMesaVisual);
typedef DMesaContext (*DMesaCreateContext_f) (DMesaVisual, DMesaContext);
typedef void (*DMesaDestroyContext_f) (DMesaContext);
typedef DMesaBuffer (*DMesaCreateBuffer_f) (DMesaVisual, GLint, GLint, GLint, GLint);
typedef void (*DMesaDestroyBuffer_f) (DMesaBuffer b);
typedef void (*DMesaSwapBuffers_f) (DMesaBuffer);
typedef GLboolean (*DMesaMakeCurrent_f) (DMesaContext, DMesaBuffer);
typedef DMesaProc (*DMesaGetProcAddress_f) (const char *);
static DMesaCreateVisual_f DMesaCreateVisual_fp;
static DMesaDestroyVisual_f DMesaDestroyVisual_fp;
static DMesaCreateContext_f DMesaCreateContext_fp;
static DMesaDestroyContext_f DMesaDestroyContext_fp;
static DMesaCreateBuffer_f DMesaCreateBuffer_fp;
static DMesaDestroyBuffer_f DMesaDestroyBuffer_fp;
static DMesaSwapBuffers_f DMesaSwapBuffers_fp;
static DMesaMakeCurrent_f DMesaMakeCurrent_fp;
static DMesaGetProcAddress_f DMesaGetProcAddress_fp;
#else
#define DMesaCreateVisual_fp DMesaCreateVisual
#define DMesaDestroyVisual_fp DMesaDestroyVisual
#define DMesaCreateContext_fp DMesaCreateContext
#define DMesaDestroyContext_fp DMesaDestroyContext
#define DMesaCreateBuffer_fp DMesaCreateBuffer
#define DMesaDestroyBuffer_fp DMesaDestroyBuffer
#define DMesaSwapBuffers_fp DMesaSwapBuffers
#define DMesaMakeCurrent_fp DMesaMakeCurrent
#define DMesaGetProcAddress_fp DMesaGetProcAddress
#endif

static DMesaVisual dv;
static DMesaContext dc;
static DMesaBuffer db;

static int DMESA_InitCtx (int *width, int *height, int *bpp)
{
	/* request either 16 or 32 bpp visual */
	if (*bpp <= 16) /* 565 */
		dv = DMesaCreateVisual_fp(*width, *height, 16, 0, true, true, 0, 16, 0, 0);
	else	dv = DMesaCreateVisual_fp(*width, *height, 32, 0, true, true, 8, 24, 8, 0);
	if (!dv) return -1;

	dc = DMesaCreateContext_fp(dv, NULL);
	if (!dc) return -1;

	db = DMesaCreateBuffer_fp(dv, 0, 0, *width, *height);
	if (!db) return -1;

	DMesaMakeCurrent_fp(dc, db);

	return 0;
}

static void DMESA_Shutdown (void)
{
	if (db) {
		DMesaDestroyBuffer_fp(db);
		db = NULL;
	}
	if (dc) {
		DMesaDestroyContext_fp(dc);
		dc = NULL;
	}
	if (dv) {
		DMesaDestroyVisual_fp(dv);
		dv = NULL;
	}
}

static void DMESA_EndFrame (void)
{
	glFlush_fp();
	DMesaSwapBuffers_fp(db);
}

#ifdef GL_DLSYM
static void *DMESA_GetProcAddress (const char *sym)
{
	if (DMesaGetProcAddress_fp)
		return (void *) DMesaGetProcAddress_fp (sym);
	return NULL;
}
#else /* assume the function is present */
static void *DMESA_GetProcAddress (const char *sym) {
	return (void *) DMesaGetProcAddress (sym);
}
#endif

static const char *DMESA_APIName (void)
{
	return "DMesa";
}

int DMESA_LoadAPI (void *handle)
{
#ifdef GL_DLSYM
	DOSGL_InitCtx  = NULL;
	DOSGL_Shutdown = NULL;
	DOSGL_EndFrame = NULL;
	DOSGL_GetProcAddress = NULL;
	DOSGL_APIName = NULL;
	DMesaCreateVisual_fp = (DMesaCreateVisual_f) Sys_dlsym(handle,"_DMesaCreateVisual");
	DMesaDestroyVisual_fp = (DMesaDestroyVisual_f) Sys_dlsym(handle,"_DMesaDestroyVisual");
	DMesaCreateContext_fp = (DMesaCreateContext_f) Sys_dlsym(handle,"_DMesaCreateContext");
	DMesaDestroyContext_fp = (DMesaDestroyContext_f) Sys_dlsym(handle,"_DMesaDestroyContext");
	DMesaCreateBuffer_fp = (DMesaCreateBuffer_f) Sys_dlsym(handle,"_DMesaCreateBuffer");
	DMesaDestroyBuffer_fp = (DMesaDestroyBuffer_f) Sys_dlsym(handle,"_DMesaDestroyBuffer");
	DMesaSwapBuffers_fp = (DMesaSwapBuffers_f) Sys_dlsym(handle,"_DMesaSwapBuffers");
	DMesaMakeCurrent_fp = (DMesaMakeCurrent_f) Sys_dlsym(handle,"_DMesaMakeCurrent");
	DMesaGetProcAddress_fp = (DMesaGetProcAddress_f) Sys_dlsym(handle,"_DMesaGetProcAddress");
	if (!DMesaCreateVisual_fp || !DMesaDestroyVisual_fp ||
	    !DMesaCreateContext_fp || !DMesaDestroyContext_fp ||
	    !DMesaCreateBuffer_fp || !DMesaDestroyBuffer_fp ||
	    !DMesaSwapBuffers_fp || !DMesaMakeCurrent_fp) {
		return -1;
	}
#endif

	DOSGL_InitCtx  = DMESA_InitCtx;
	DOSGL_Shutdown = DMESA_Shutdown;
	DOSGL_EndFrame = DMESA_EndFrame;
	DOSGL_GetProcAddress = DMESA_GetProcAddress;
	DOSGL_APIName = DMESA_APIName;

	return 0;
}

#endif
