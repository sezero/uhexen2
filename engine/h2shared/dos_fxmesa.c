/* gl_fxmesa.c -- DOS OpenGL refresh using fxMesa api:
 * for use with Mesa library built against 3dfx glide.
 * Copyright (C) 1997-2001 Id Software, Inc.
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

/* fxMesa api changes across Mesa versions:
 *
 * Mesa-3.1 .. 3.4: seems no change.
 * Mesa-3.4 -> 3.5: fxMesaSetNearFar() removed.
 * Mesa-3.5 .. 5.0: seems no change.
 * Mesa-5.0 -> 5.1:
 * - new FXMESA_COLORDEPTH and FXMESA_SHARE_CONTEXT attributes added.
 * - new fxGetScreenGeometry() added.
 * - fxQueryHardware() semantics changed: it returned success (bool),
 *   instead of hardware type (int).
 * Mesa-5.1 -> 6.x:
 * - fxQueryHardware() became a private function.
 *
 * NOTE:
 * Direct use of fxMesa api from Mesa >= 5.1 does NOT seem to work.
 */

#include "quakedef.h"
#include "gl_dos.h"
#include "sys_dxe.h"

#if !defined(REFGL_FXMESA)
int FXMESA_LoadAPI (void *handle)
{
	return -1;
}

#else

#include <GL/fxmesa.h>

#ifndef FXMESA_COLORDEPTH
#define FXMESA_COLORDEPTH 20
#endif

#if defined(GL_DLSYM)
typedef fxMesaContext (*fxMesaCreateContext_f) (GLuint, GrScreenResolution_t, GrScreenRefresh_t, const GLint attribList[]);
typedef fxMesaContext (*fxMesaCreateBestContext_f) (GLuint, GLint, GLint, const GLint attribList[]);
typedef void (*fxMesaMakeCurrent_f) (fxMesaContext);
typedef void (*fxMesaDestroyContext_f) (fxMesaContext);
typedef void (*fxMesaSwapBuffers_f) (void);
typedef void (*FXMESAPROC) ();
typedef FXMESAPROC (*fxMesaGetProcAddress_f) (const char *);
static fxMesaCreateContext_f fxMesaCreateContext_fp;
static fxMesaCreateBestContext_f fxMesaCreateBestContext_fp;
static fxMesaMakeCurrent_f fxMesaMakeCurrent_fp;
static fxMesaDestroyContext_f fxMesaDestroyContext_fp;
static fxMesaSwapBuffers_f fxMesaSwapBuffers_fp;
static fxMesaGetProcAddress_f fxMesaGetProcAddress_fp;
#else
#define fxMesaCreateContext_fp fxMesaCreateContext
#define fxMesaCreateBestContext_fp fxMesaCreateBestContext
#define fxMesaMakeCurrent_fp fxMesaMakeCurrent
#define fxMesaDestroyContext_fp fxMesaDestroyContext
#define fxMesaSwapBuffers_fp fxMesaSwapBuffers
/*#define fxMesaGetProcAddress_fp fxMesaGetProcAddress*/
#endif

static fxMesaContext fc = NULL;

typedef struct {
	int width, height;
	GrScreenResolution_t fxmode;
} fxmode_t;

static fxmode_t fx_modes[] = {
	{ 320,200,  GR_RESOLUTION_320x200 },
	{ 320,240,  GR_RESOLUTION_320x240 },
	{ 400,256,  GR_RESOLUTION_400x256 },
	{ 400,300,  GR_RESOLUTION_400x300 },
	{ 512,384,  GR_RESOLUTION_512x384 },
	{ 640,200,  GR_RESOLUTION_640x200 },
	{ 640,350,  GR_RESOLUTION_640x350 },
	{ 640,400,  GR_RESOLUTION_640x400 },
	{ 640,480,  GR_RESOLUTION_640x480 },
	{ 800,600,  GR_RESOLUTION_800x600 },
	{ 960,720,  GR_RESOLUTION_960x720 },
	{ 856,480,  GR_RESOLUTION_856x480 },
	{ 512,256,  GR_RESOLUTION_512x256 },
	{ 1024,768, GR_RESOLUTION_1024x768 },
	{ 1280,1024,GR_RESOLUTION_1280x1024 },
	{ 1600,1200,GR_RESOLUTION_1600x1200 },
};
#define MAX_FXMODES (int)(sizeof(fx_modes) / sizeof(fx_modes[0]))

static GrScreenResolution_t findres (int *width, int *height)
{
	int i;

	for (i = 0; i < MAX_FXMODES; i++) {
		if (*width <= fx_modes[i].width && *height <= fx_modes[i].height) {
			*width = fx_modes[i].width;
			*height = fx_modes[i].height;
			return fx_modes[i].fxmode;
		}
	}

	*width = 640;
	*height = 480;
	return GR_RESOLUTION_640x480;
}

static int FXMESA_InitCtx (int *width, int *height, int *bpp)
{
	GLint attribs[32];

	attribs[0] = FXMESA_DOUBLEBUFFER;
	attribs[1] = FXMESA_ALPHA_SIZE;
	attribs[2] = 1;
	attribs[3] = FXMESA_DEPTH_SIZE;
	attribs[4] = 1;
	attribs[5] = FXMESA_NONE;

	if (*bpp != 16) {
		Con_SafePrintf("ignoring %d bpp request, using 16 bpp.\n", *bpp);
		*bpp = 16;
	}

//	fc = fxMesaCreateBestContext_fp(0, *width, *height, attribs);
	fc = fxMesaCreateContext_fp(0, findres(width, height), GR_REFRESH_60Hz, attribs);
	if (!fc)
		return -1;

	fxMesaMakeCurrent_fp(fc);

	return 0;
}

static void FXMESA_Shutdown (void)
{
	if (fc) {
		fxMesaDestroyContext_fp(fc);
		fc = NULL;
	}
}

static void FXMESA_EndFrame (void)
{
	glFlush_fp();
	fxMesaSwapBuffers_fp();
}

#ifdef GL_DLSYM
static void *FXMESA_GetProcAddress (const char *sym)
{
	if (fxMesaGetProcAddress_fp)
		return (void *) fxMesaGetProcAddress_fp (sym);
	return NULL;
}
#else /* assume the function is NOT present */
static void *FXMESA_GetProcAddress (const char *sym) {
	return NULL;
}
#endif

static const char *FXMESA_APIName (void)
{
	return "fxMesa";
}

int FXMESA_LoadAPI (void *handle)
{
#ifdef GL_DLSYM
	DOSGL_InitCtx  = NULL;
	DOSGL_Shutdown = NULL;
	DOSGL_EndFrame = NULL;
	DOSGL_GetProcAddress = NULL;
	DOSGL_APIName = NULL;
	fxMesaCreateContext_fp = (fxMesaCreateContext_f) Sys_dlsym(handle,"_fxMesaCreateContext");
	fxMesaCreateBestContext_fp = (fxMesaCreateBestContext_f) Sys_dlsym(handle,"_fxMesaCreateBestContext");
	fxMesaMakeCurrent_fp = (fxMesaMakeCurrent_f) Sys_dlsym(handle,"_fxMesaMakeCurrent");
	fxMesaDestroyContext_fp = (fxMesaDestroyContext_f) Sys_dlsym(handle,"_fxMesaDestroyContext");
	fxMesaSwapBuffers_fp = (fxMesaSwapBuffers_f) Sys_dlsym(handle,"_fxMesaSwapBuffers");
	fxMesaGetProcAddress_fp = (fxMesaGetProcAddress_f) Sys_dlsym(handle,"_fxMesaGetProcAddress");
	if (!fxMesaCreateContext_fp || !fxMesaCreateBestContext_fp ||
	    !fxMesaMakeCurrent_fp || !fxMesaDestroyContext_fp ||
	    !fxMesaSwapBuffers_fp) {
		return -1;
	}
#endif

	DOSGL_InitCtx  = FXMESA_InitCtx;
	DOSGL_Shutdown = FXMESA_Shutdown;
	DOSGL_EndFrame = FXMESA_EndFrame;
	DOSGL_GetProcAddress = FXMESA_GetProcAddress;
	DOSGL_APIName = FXMESA_APIName;

	return 0;
}

#endif
