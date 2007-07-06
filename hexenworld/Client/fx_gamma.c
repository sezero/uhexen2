/*
 * fx_gamma.c
 * $Id: fx_gamma.c,v 1.6 2007-07-06 12:45:42 sezero Exp $
 *
 * Small library providing gamma control functions for 3Dfx Voodoo1/2
 * cards by abusing the exposed glide symbols when using fxMesa.
 *
 * Author: O. Sezer <sezero@users.sourceforge.net>   License: GPL
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the accompanying COPYING file for more details.
 *
 * Compiling as a shared library:
 * gcc lib3dfxgamma.c -O2 -fPIC -Wall  -o lib3dfxgamma.so -shared
 *
 * How to use:
 * If you are linking to the opengl library at compile time (-lGL),
 * not much is necessary. If you dlopen() the opengl library, then
 * RTLD_GLOBAL flag is necessary so that the library's symbols would be
 * available to you: SDL_GL_LoadLibrary() is just fine in this regard.
 * In either case, if the gllib is an fxMesa library, then you will have
 * the necessary glide symbols exposed on you. Decide whether you have
 * a Voodoo1/2 and then use the functions here.
 *
 * Issues:
 * glSetDeviceGammaRamp3DFX works nicely with Voodoo2, but it crashes
 * Voodoo1. The ramp functions are added for completeness sake anyway.
 * do3dfxGammaCtrl works just fine for both Voodoo1 and Voodoo2.
 * Besides, the GammaRamp3DFX functions are only available for Glide3:
 * Glide2 users cannot benefit them, but the gamma control option is
 * available for both Glide2 and Glide3. Therefore employing the gamma
 * control option seems more beneficial.
 *
 * Revision history:
 * v0.0.1, 2005-06-04:	Initial version, do3dfxGammaCtrl works fine,
 *			glGetDeviceGammaRamp3DFX & co need more care
 * v0.0.2, 2005-06-13:	tried following the exact win32 versions for
 *			glGetDeviceGammaRamp3DFX/glSetDeviceGammaRamp3DFX
 * v0.0.3, 2005-12-05:	Updated documentation about the RTLD_GLOBAL flag.
 * v0.0.4, 2006-03-16:	Fixed incorrect prototype for the grGet function
 *			(it takes a signed int param*, not unsigned).
 *			Also renamed FX_Get to FX_GetInteger to be more
 *			explicit.
 */

#include <stdlib.h>
#include <string.h>

#if defined(USE_3DFXGAMMA)

#include <dlfcn.h>

#define USE_GAMMA_RAMPS		1	/* actual define is in gl_vidsdl.c */

#include "fx_gamma.h"

/**********************************************************************/

/**	PRIVATE STUFF			**/

/* 3dfx glide2 func for gamma correction */
static void (*FX_GammaControl2)(float) = NULL;
/* 3dfx glide3 func for gamma correction */
static void (*FX_GammaControl3)(float, float, float) = NULL;

#if USE_GAMMA_RAMPS
/* 3dfx glide3 funcs to make a replacement wglSetDeviceGammaRamp3DFX */
#define GR_GAMMA_TABLE_ENTRIES	0x05
static unsigned int (*FX_GetInteger)(unsigned int, unsigned int, signed int *) = NULL;
static void (*FX_LoadGammaTable)(unsigned int, unsigned int *, unsigned int *, unsigned int *) = NULL;
#endif

/**********************************************************************/

/*
 * Init_3dfxGammaCtrl
 * Sends 0 for failure, 2 for glide2 or 3 for glide3 api.
 */
int Init_3dfxGammaCtrl (void)
{
	void	*symslist;
	int	ret = 0;

	if (FX_GammaControl2 != NULL)
		return 2;
	if (FX_GammaControl3 != NULL)
		return 3;

	symslist = (void *) dlopen(NULL, RTLD_LAZY);
	if (symslist != NULL)
	{
		if ((FX_GammaControl2 = (void (*) (float)) dlsym(symslist, "grGammaCorrectionValue")) != NULL)
			ret = 2;
		else if ((FX_GammaControl3 = (void (*) (float, float, float)) dlsym(symslist, "guGammaCorrectionRGB")) != NULL)
			ret = 3;

		dlclose(symslist);
	}
	else
	{	/* shouldn't happen. */
		ret = -1;
	}

	return ret;
}

void Shutdown_3dfxGamma (void)
{
	FX_GammaControl2 = NULL;
	FX_GammaControl3 = NULL;
#if USE_GAMMA_RAMPS
	FX_GetInteger = NULL;
	FX_LoadGammaTable = NULL;
#endif
}

/*
 * do3dfxGammaCtrl
 */
void do3dfxGammaCtrl (float value)
{
	if (FX_GammaControl2)
		FX_GammaControl2 (value);
	else if (FX_GammaControl3)
		FX_GammaControl3 (value, value, value);
}

/**********************************************************************/
#if USE_GAMMA_RAMPS
static int Check_3DfxGammaRamp (void)
{
	void	*symslist;

	if (FX_LoadGammaTable != NULL)
		return 1;

	symslist = (void *) dlopen(NULL, RTLD_LAZY);
	if (symslist != NULL)
	{
		FX_GetInteger = (unsigned int (*) (unsigned int, unsigned int, signed int *)) dlsym(symslist, "grGet");
		FX_LoadGammaTable = (void (*) (unsigned int, unsigned int *, unsigned int *, unsigned int *)) dlsym(symslist, "grLoadGammaTable");
		if ((FX_LoadGammaTable != NULL) && (FX_GetInteger != NULL))
			return 1;
	}

	return 0;
}

/*
 * glSetDeviceGammaRamp3DFX is adapted from Mesa-6.x
 *
 * glSetDeviceGammaRamp3DFX crashes Voodoo1, at least
 * currently, so it is not recommended yet.
 */
int glSetDeviceGammaRamp3DFX (void *arrays)
{
	int		tableSize = 0;
	int		i, inc, idx;
	unsigned short	*red, *green, *blue;
	unsigned int	gammaTableR[256], gammaTableG[256], gammaTableB[256];

	if ((FX_LoadGammaTable == NULL) || (FX_GetInteger == NULL))
		return 0;

	FX_GetInteger (GR_GAMMA_TABLE_ENTRIES, 4, &tableSize);
	if (!tableSize)
		return 0;

	inc = 256 / tableSize;

	red = (unsigned short *)arrays;
	green = (unsigned short *)arrays + 256;
	blue = (unsigned short *)arrays + 512;

	for (i = 0, idx = 0; i < tableSize; i++, idx += inc)
	{
		gammaTableR[i] = red[idx] >> 8;
		gammaTableG[i] = green[idx] >> 8;
		gammaTableB[i] = blue[idx] >> 8;
	}

	FX_LoadGammaTable(tableSize, gammaTableR, gammaTableG, gammaTableB);

	return 1;
}

/*
 * glGetDeviceGammaRamp3DFX
 * Sends a 1.0 gamma table. Also to be used for querying the lib.
 */
int glGetDeviceGammaRamp3DFX (void *arrays)
{
	int		i;
	unsigned short	gammaTable[3][256];

	if ((FX_LoadGammaTable == NULL) || (FX_GetInteger == NULL))
	{
		if (Check_3DfxGammaRamp() == 0)
			return 0;
	}

	for (i = 0; i < 256; i++)
	{
		 gammaTable[0][i] = i << 8;
		 gammaTable[1][i] = i << 8;
		 gammaTable[2][i] = i << 8;
	}

	memcpy (arrays, gammaTable, 3 * 256 * sizeof(unsigned short));

	return 1;
}
#endif	/* USE_GAMMA_RAMPS */

#endif	/* USE_3DFXGAMMA */

