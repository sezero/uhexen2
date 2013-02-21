/*
 * d_part.c -- software driver module for drawing particles.
 * $Id$
 *
 * Copyright (C) 1996-1997  Id Software, Inc.
 * Copyright (C) 1997-1998  Raven Software Corp.
 * Copyright (C) 2007-2012  O.Sezer <sezero@users.sourceforge.net>
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


/*
==============
D_EndParticles
==============
*/
void D_EndParticles (void)
{
// not used by software driver
}


/*
==============
D_StartParticles
==============
*/
void D_StartParticles (void)
{
// not used by software driver
}


#if	!id386

/*
==============
D_DrawParticle
==============
*/
void D_DrawParticle (particle_t *pparticle)
{
	vec3_t	local, transformed;
	float	zi;
	byte	*pdest;
	short	*pz;
	int		i, izi, pix, count, u, v;
	int		color;
	qboolean	is_trans;

// transform point
	VectorSubtract (pparticle->org, r_origin, local);

	transformed[0] = DotProduct(local, r_pright);
	transformed[1] = DotProduct(local, r_pup);
	transformed[2] = DotProduct(local, r_ppn);

	if (transformed[2] < PARTICLE_Z_CLIP)
		return;

// project the point
// FIXME: preadjust xcenter and ycenter
	zi = 1.0 / transformed[2];
	u = (int)(xcenter + zi * transformed[0] + 0.5);
	v = (int)(ycenter - zi * transformed[1] + 0.5);

	if ((v > d_vrectbottom_particle) || (u > d_vrectright_particle) ||
	    (v < d_vrecty) || (u < d_vrectx))
	{
		return;
	}

	pz = d_pzbuffer + (d_zwidth * v) + u;
	pdest = d_viewbuffer + d_scantable[v] + u;
	izi = (int)(zi * 0x8000);

	pix = izi >> d_pix_shift;
	if (pix < d_pix_min)
		pix = d_pix_min;
	else if (pix > d_pix_max)
		pix = d_pix_max;

	/* clamp color to 0-511: particle->type 10 and 11 (pt_c_explode
	 * and pt_c_explode2, e.g. Crusader's ice particles hitting a
	 * wall) lead to negative values, because R_UpdateParticles ()
	 * decrements their color against time. */
	color = ((int) pparticle->color) & 0x01ff;
	if ((is_trans = (color >= 256)))
		color = (color - 256) << 8;	/* << 8 for transTable idx */

	switch (pix)
	{
	case 1:
		count = 1 << d_y_aspect_shift;

		for ( ; count ; count--, pz += d_zwidth, pdest += screenwidth)
		{
			if (pz[0] <= izi)
			{
				pz[0] = izi;
				pdest[0] = (!is_trans) ? color : transTable[color + pdest[0]];
			}
		}
		break;

	case 2:
		count = 2 << d_y_aspect_shift;

		for ( ; count ; count--, pz += d_zwidth, pdest += screenwidth)
		{
			if (pz[0] <= izi)
			{
				pz[0] = izi;
				pdest[0] = (!is_trans) ? color : transTable[color + pdest[0]];
			}
			if (pz[1] <= izi)
			{
				pz[1] = izi;
				pdest[1] = (!is_trans) ? color : transTable[color + pdest[1]];
			}
		}
		break;

	case 3:
		count = 3 << d_y_aspect_shift;

		for ( ; count ; count--, pz += d_zwidth, pdest += screenwidth)
		{
			if (pz[0] <= izi)
			{
				pz[0] = izi;
				pdest[0] = (!is_trans) ? color : transTable[color + pdest[0]];
			}
			if (pz[1] <= izi)
			{
				pz[1] = izi;
				pdest[1] = (!is_trans) ? color : transTable[color + pdest[1]];
			}
			if (pz[2] <= izi)
			{
				pz[2] = izi;
				pdest[2] = (!is_trans) ? color : transTable[color + pdest[2]];
			}
		}
		break;

	case 4:
		count = 4 << d_y_aspect_shift;

		for ( ; count ; count--, pz += d_zwidth, pdest += screenwidth)
		{
			if (pz[0] <= izi)
			{
				pz[0] = izi;
				pdest[0] = (!is_trans) ? color : transTable[color + pdest[0]];
			}
			if (pz[1] <= izi)
			{
				pz[1] = izi;
				pdest[1] = (!is_trans) ? color : transTable[color + pdest[1]];
			}
			if (pz[2] <= izi)
			{
				pz[2] = izi;
				pdest[2] = (!is_trans) ? color : transTable[color + pdest[2]];
			}
			if (pz[3] <= izi)
			{
				pz[3] = izi;
				pdest[3] = (!is_trans) ? color : transTable[color + pdest[3]];
			}
		}
		break;

	default:
		count = pix << d_y_aspect_shift;

		for ( ; count ; count--, pz += d_zwidth, pdest += screenwidth)
		{
			for (i = 0 ; i < pix ; i++)
			{
				if (pz[i] <= izi)
				{
					pz[i] = izi;
					pdest[i] = (!is_trans) ? color : transTable[color + pdest[i]];
				}
			}
		}
		break;
	}
}

#endif	/* !id386 */

