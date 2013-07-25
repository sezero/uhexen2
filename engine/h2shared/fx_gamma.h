/*
 * fx_gamma.h
 * $Id$
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
	/* returns 0 for failure, non-zero for success
	  (2 glide2 or 3 for glide3 api.) */

extern int do3dfxGammaCtrl (float/* gamma value*/);
	/* gamma correction proc (1.0 ... 0.333).
	   returns 0 for failure, 1 for success. */

extern int glSetDeviceGammaRamp3DFX (void *arrays);
	/* returns 0 for failure, 1 for success. */

extern int glGetDeviceGammaRamp3DFX (void *arrays);
	/* sends an identity (1.0) gamma table.
	   use for querying the lib.
	   returns 0 for failure, 1 for success. */

#ifdef __cplusplus
}
#endif

#endif	/* __FXGAMMA_H */

