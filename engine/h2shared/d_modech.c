/*
 * d_modech.c -- called when mode has just changed.
 * $Id$
 *
 * Copyright (C) 1996-1997  Id Software, Inc.
 * Copyright (C) 1997-1998  Raven Software Corp.
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
#include "d_local.h"

int	d_vrectx, d_vrecty, d_vrectright_particle, d_vrectbottom_particle;

int	d_y_aspect_shift, d_y_aspect_rshift, d_pix_min, d_pix_max, d_pix_shift;

int	d_scantable[MAXHEIGHT];
short	*zspantable[MAXHEIGHT];

/*
================
D_Patch
================
*/
#if id386
void D_Patch (void)
{
	static qboolean protectset8 = false;

	if (!protectset8)
	{
		Sys_MakeCodeWriteable ((int)D_PolysetAff8Start, (int)D_PolysetAff8End - (int)D_PolysetAff8Start);
		Sys_MakeCodeWriteable ((int)D_PolysetAff8StartT, (int)D_PolysetAff8EndT - (int)D_PolysetAff8StartT);
		Sys_MakeCodeWriteable ((int)D_PolysetAff8StartT2, (int)D_PolysetAff8EndT2 - (int)D_PolysetAff8StartT2);
		Sys_MakeCodeWriteable ((int)D_PolysetAff8StartT3, (int)D_PolysetAff8EndT3 - (int)D_PolysetAff8StartT3);
		Sys_MakeCodeWriteable ((int)D_PolysetAff8StartT5, (int)D_PolysetAff8EndT5 - (int)D_PolysetAff8StartT5);
		protectset8 = true;
	}

}
#endif	/* id386 */


/*
================
D_ViewChanged
================
*/
void D_ViewChanged (void)
{
	int	i, rowbytes;

	if (r_dowarp)
		rowbytes = WARP_WIDTH;
	else
		rowbytes = vid.rowbytes;

	scale_for_mip = xscale;
	if (yscale > xscale)
		scale_for_mip = yscale;

	d_zrowbytes = vid.width * 2;
	d_zwidth = vid.width;

	d_pix_min = r_refdef.vrect.width / 320;
	if (d_pix_min < 1)
		d_pix_min = 1;

	d_pix_max = (int)((float)r_refdef.vrect.width / (320.0 / 4.0) + 0.5);
	d_pix_shift = 8 - (int)((float)r_refdef.vrect.width / 320.0 + 0.5);
	if (d_pix_max < 1)
		d_pix_max = 1;

	if (pixelAspect > 1.4)
	{
		d_y_aspect_shift = 1;
		d_y_aspect_rshift = 0;
	}
	else
	{
		d_y_aspect_shift = 0;
		if (pixelAspect < 0.27)
			d_y_aspect_rshift = 2;
		else if (pixelAspect < 0.54)
			d_y_aspect_rshift = 1;
		else
			d_y_aspect_rshift = 0;
	}

	d_vrectx = r_refdef.vrect.x;
	d_vrecty = r_refdef.vrect.y;
	d_vrectright_particle = r_refdef.vrectright - d_pix_max;
#if id386
	d_vrectbottom_particle =
			r_refdef.vrectbottom - (d_pix_max << d_y_aspect_shift);
#else
	d_vrectbottom_particle =
			r_refdef.vrectbottom - ((d_pix_max << d_y_aspect_shift) >> d_y_aspect_rshift);
#endif

	for (i = 0; i < vid.height; i++)
	{
		d_scantable[i] = i*rowbytes;
		zspantable[i] = d_pzbuffer + i*d_zwidth;
	}

#if id386
	D_Patch ();
#endif
}

