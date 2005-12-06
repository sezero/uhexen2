/*
 * Small library providing gamma control functions for 3Dfx Voodoo1/2
 * cards by abusing the exposed glide symbols when using fxMesa.
 * Author: O. Sezer <sezero@users.sourceforge.net>	License: GPL
 *
 * Compile using:
 * gcc -o lib3dfxgamma.so -Wall -O2 -march=i586 -shared lib3dfxgamma.c
 *
 * How to use:
 * If you are linking to the opengl library at compile time (-lGL),
 * not much is necessary. If you dlopen() the opengl library, then
 * RTLD_GLOBAL flag is necessary so that the library's symbols would be
 * available to you: SDL_GL_LoadLibrary() is just fine in this regard.
 * In either case, if the gllib is an fxMesa library, then you will have
 * the necessary glide symbols exposed on you. Decide whether you have
 * a Voodoo1/2, dlopen() this lib and then use the functions here.
 *
 * Issues:
 * glSetDeviceGammaRamp3DFX works nicely with Voodoo2, but it crashes
 * Voodoo1. The ramp functions are added for completeness sake anyway.
 * do3dfxGammaCtrl works just fine for both Voodoo1 and Voodoo2.
 *
 * Revision history:
 * v0.0.1, 2005-06-04:	Initial version, do3dfxGammaCtrl works fine,
 *			glGetDeviceGammaRamp3DFX & co need more care
 * v0.0.2, 2005-06-13:	tried following the exact win32 versions for
 *			glGetDeviceGammaRamp3DFX/glSetDeviceGammaRamp3DFX
 * v0.0.3, 2005-12-05:	Updated documentation about the RTLD_GLOBAL flag.
 */

#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

/**********************************************************************/

// 3dfx glide2 func for gamma correction
void (*FX_GammaControl2)(float) = NULL;
// 3dfx glide3 func for gamma correction
void (*FX_GammaControl3)(float,float,float) = NULL;

// 3dfx glide3 funcs to make a replacement wglSetDeviceGammaRamp3DFX
#define GR_GAMMA_TABLE_ENTRIES	0x05
unsigned int (*FX_Get)(unsigned int, unsigned int, unsigned int *) = NULL;
void (*FX_LoadGammaTable)(unsigned int, unsigned int*, unsigned int*, unsigned int*) = NULL;

/**********************************************************************/

/*
 * Check_3DfxGammaCtrl
 * Sends 0 for failure, 2 for glide2 or 3 for glide3 api.
 */
int Check_3DfxGammaCtrl (void)
{
	void *prjobj;
	int ret = 0;

	if (FX_GammaControl2 != NULL)
		return 2;
	if (FX_GammaControl3 != NULL)
		return 3;

	prjobj = dlopen(NULL, RTLD_LAZY);
	if (prjobj != NULL)
	{
		if ((FX_GammaControl2 = dlsym(prjobj, "grGammaCorrectionValue")) != NULL)
			ret = 2;
		else if ((FX_GammaControl3 = dlsym(prjobj, "guGammaCorrectionRGB")) != NULL)
			ret = 3;

		dlclose(prjobj);
	}

	return ret;
}

/*
 * do3dfxGammaCtrl
 *
 * Receiving -1 as the value shall be interpreted as querying
 * and the result from Check_3DfxGammaCtrl will be returned.
 * If, at that time, our functions weren't initialized, they
 * will then be.
 */
int do3dfxGammaCtrl(float value)
{
	if (value < 0)	// someone wants to query us
		return Check_3DfxGammaCtrl();

	if (FX_GammaControl2)
		FX_GammaControl2 (value);
	else if (FX_GammaControl3)
		FX_GammaControl3 (value,value,value);

	return 0;
}

/**********************************************************************/

int Check_3DfxGammaRamp (void)
{
	void *prjobj;

	if (FX_LoadGammaTable != NULL)
		return 1;

	prjobj = dlopen(NULL, RTLD_LAZY);
	if (prjobj != NULL)
	{
		FX_Get = dlsym(prjobj, "grGet");
		FX_LoadGammaTable = dlsym(prjobj, "grLoadGammaTable");
		if ((FX_LoadGammaTable != NULL) && (FX_Get != NULL))
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
	int tableSize = 0;
	int i, inc, idx;
	unsigned short *red, *green, *blue;
	unsigned int gammaTableR[256], gammaTableG[256], gammaTableB[256];

	if ((FX_LoadGammaTable == NULL) || (FX_Get == NULL))
		return 0;

	FX_Get (GR_GAMMA_TABLE_ENTRIES, 4, &tableSize);
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
	int	i;
	unsigned short	gammaTable[3][256];

	if ((FX_LoadGammaTable == NULL) || (FX_Get == NULL))
	{
		if (Check_3DfxGammaRamp() == 0)
			return 0;
	}

	for (i=0 ; i<256 ; i++)
	{
		 gammaTable[0][i] = i << 8;
		 gammaTable[1][i] = i << 8;
		 gammaTable[2][i] = i << 8;
	}

	memcpy(arrays, gammaTable, 3*256*sizeof(unsigned short));

	return 1;
}

