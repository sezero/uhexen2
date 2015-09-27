/* gl_fxmesa.c -- DOS OpenGL refresh using fxMesa interface:
 * for use with the Mesa library built against 3dfx glide.
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

#include "quakedef.h"
#include "gl_dos.h"
#include "sys_dxe.h"

#if !defined(GL_DLSYM) && !defined(REFGL_FXMESA)
int FXMESA_ScanIFace (void)
{
	return -1;
}

#else

#include <GL/fxmesa.h>

#if defined(GL_DLSYM)
typedef fxMesaContext (*fxMesaCreateContext_f) (GLuint, GrScreenResolution_t, GrScreenRefresh_t, const GLint attribList[]);
typedef fxMesaContext (*fxMesaCreateBestContext_f) (GLuint, GLint, GLint, const GLint attribList[]);
typedef void (*fxMesaMakeCurrent_f) (fxMesaContext);
typedef void (*fxMesaDestroyContext_f) (fxMesaContext);
typedef void (*fxMesaSwapBuffers_f) (void);
static fxMesaCreateContext_f fxMesaCreateContext_fp;
static fxMesaCreateBestContext_f fxMesaCreateBestContext_fp;
static fxMesaMakeCurrent_f fxMesaMakeCurrent_fp;
static fxMesaDestroyContext_f fxMesaDestroyContext_fp;
static fxMesaSwapBuffers_f fxMesaSwapBuffers_fp;
#else
#define fxMesaCreateContext_fp fxMesaCreateContext
#define fxMesaCreateBestContext_fp fxMesaCreateBestContext
#define fxMesaMakeCurrent_fp fxMesaMakeCurrent
#define fxMesaDestroyContext_fp fxMesaDestroyContext
#define fxMesaSwapBuffers_fp fxMesaSwapBuffers
#endif

static fxMesaContext fc = NULL;

static int findres (int *width, int *height)
{
	static int fx_resolutions[][3] = {
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
		{ 0, 0, 0 }
	};

	int i;

	for (i = 0; fx_resolutions[i][0]; i++) {
		if (*width <= fx_resolutions[i][0] && *height <= fx_resolutions[i][1]) {
			*width = fx_resolutions[i][0];
			*height = fx_resolutions[i][1];
			return fx_resolutions[i][2];
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

static void *FXMESA_GetProcAddress (const char *sym)
{
	return NULL; /* no can do.. */
}

static const char *FXMESA_IFaceName (void)
{
	return "fxMesa";
}

int FXMESA_ScanIFace (void *handle)
{
#ifdef GL_DLSYM
	DOSGL_InitCtx  = NULL;
	DOSGL_Shutdown = NULL;
	DOSGL_EndFrame = NULL;
	DOSGL_GetProcAddress = NULL;
	DOSGL_IFaceName = NULL;
	fxMesaCreateContext_fp = (fxMesaCreateContext_f) Sys_dlsym(handle,"_fxMesaCreateContext");
	fxMesaCreateBestContext_fp = (fxMesaCreateBestContext_f) Sys_dlsym(handle,"_fxMesaCreateBestContext");
	fxMesaMakeCurrent_fp = (fxMesaMakeCurrent_f) Sys_dlsym(handle,"_fxMesaMakeCurrent");
	fxMesaDestroyContext_fp = (fxMesaDestroyContext_f) Sys_dlsym(handle,"_fxMesaDestroyContext");
	fxMesaSwapBuffers_fp = (fxMesaSwapBuffers_f) Sys_dlsym(handle,"_fxMesaSwapBuffers");
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
	DOSGL_IFaceName = FXMESA_IFaceName;

	return 0;
}

#endif
