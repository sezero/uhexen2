/*
 * fx_gamma.h
 * $Id: fx_gamma.h,v 1.1 2007-07-06 11:01:58 sezero Exp $
 *
 * Small library providing gamma control functions for 3Dfx Voodoo1/2
 * cards by abusing the exposed glide symbols when using fxMesa.
 * Author: O. Sezer <sezero@users.sourceforge.net>   License: GPL
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the accompanying COPYING file for more details.
 */

#ifndef __FXGAMMA_H
#define __FXGAMMA_H

#ifdef __cplusplus
extern "C" {
#endif

extern void Shutdown_3dfxGamma (void);

extern int Init_3dfxGammaCtrl (void);
	/* returns 0 for failure, 2 for glide2 or 3 for glide3 api. */

extern void do3dfxGammaCtrl (float value);
	/* gamma correction procedure. takes the gamma value as its
	   arg (must be between 1 and 0.333) */

#if USE_GAMMA_RAMPS

extern int glSetDeviceGammaRamp3DFX (void *arrays);
	/* returns 1 for success, 0 for failure. */

extern int glGetDeviceGammaRamp3DFX (void *arrays);
	/* sends a 1.0 gamma table. use for querying the lib.
	   returns 1 for success, 0 for failure. */

#endif	/* USE_GAMMA_RAMPS */

#ifdef __cplusplus
}
#endif

#endif	/* __FXGAMMA_H */

