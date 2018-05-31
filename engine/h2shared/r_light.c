/*
 * r_light.c
 * $Id: r_light.c,v 1.6 2007-09-14 14:10:02 sezero Exp $
 *
 * Copyright (C) 1996-1997  Id Software, Inc.
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
#include "r_local.h"

int	r_dlightframecount;


/*
==================
R_AnimateLight
==================
*/
void R_AnimateLight (void)
{
	int	i, c, v;
	int	defaultLocus;
	int	locusHz[3];

	defaultLocus = locusHz[0] = (int)(cl.time*10);
	locusHz[1] = (int)(cl.time*20);
	locusHz[2] = (int)(cl.time*30);
	for (i = 0; i < MAX_LIGHTSTYLES; i++)
	{
		if (!cl_lightstyle[i].length)
		{ // No style def
			d_lightstylevalue[i] = 256;
			continue;
		}
		c = cl_lightstyle[i].map[0];
		if (c == '1' || c == '2' || c == '3')
		{ // Explicit anim rate
			if (cl_lightstyle[i].length == 1)
			{ // Bad style def
				d_lightstylevalue[i] = 256;
				continue;
			}
			v = locusHz[c-'1'] % (cl_lightstyle[i].length-1);
			d_lightstylevalue[i] = (cl_lightstyle[i].map[v+1]-'a')*22;
			continue;
		}
		// Default anim rate (10 Hz)
		v = defaultLocus % cl_lightstyle[i].length;
		d_lightstylevalue[i] = (cl_lightstyle[i].map[v]-'a')*22;
	}
}


/*
=============================================================================

DYNAMIC LIGHTS

=============================================================================
*/

/*
=============
R_MarkLights
=============
*/
void R_MarkLights (dlight_t *light, int bit, mnode_t *node)
{
	mplane_t	*splitplane;
	float		dist;
	msurface_t	*surf;
	int			i;

	if (node->contents < 0)
		return;

	splitplane = node->plane;
	dist = DotProduct (light->origin, splitplane->normal) - splitplane->dist;

	if (light->radius > 0)
	{
		if (dist > light->radius)
		{
			R_MarkLights (light, bit, node->children[0]);
			return;
		}
		if (dist < -light->radius)
		{
			R_MarkLights (light, bit, node->children[1]);
			return;
		}
	}
	else
	{
		if (dist > -light->radius)
		{
			R_MarkLights (light, bit, node->children[0]);
			return;
		}
		if (dist < light->radius)
		{
			R_MarkLights (light, bit, node->children[1]);
			return;
		}
	}

// mark the polygons
	surf = cl.worldmodel->surfaces + node->firstsurface;
	for (i = 0; i < node->numsurfaces; i++, surf++)
	{
		if (surf->dlightframe != r_dlightframecount)
		{
			surf->dlightbits = 0;
			surf->dlightframe = r_dlightframecount;
		}
		surf->dlightbits |= bit;
	}

	R_MarkLights (light, bit, node->children[0]);
	R_MarkLights (light, bit, node->children[1]);
}


/*
=============
R_PushDlights
=============
*/
void R_PushDlights (void)
{
	int		i;
	dlight_t	*l;

	if (!r_dynamic.integer)
		return;

	r_dlightframecount = r_framecount + 1;	// because the count hasn't
						//  advanced yet for this frame
	l = cl_dlights;

	for (i = 0; i < MAX_DLIGHTS; i++, l++)
	{
		if (l->die < cl.time || !l->radius)
			continue;
		R_MarkLights ( l, 1<<i, cl.worldmodel->nodes );
	}
}


/*
=============================================================================

LIGHT SAMPLING

=============================================================================
*/

static int RecursiveLightPoint (mnode_t *node, vec3_t start, vec3_t end)
{
	int			r;
	float		front, back, frac;
	int			side;
	mplane_t	*plane;
	vec3_t		mid;
	msurface_t	*surf;
	int			s, t, ds, dt;
	int			i;
	mtexinfo_t	*tex;
	byte		*lightmap;
	unsigned int	scale;
	int			maps;

loc0: // surgeon: optimized recursion

	if (node->contents < 0)
		return -1;		// didn't hit anything

// calculate mid point

// optimization borrowed from LordHavoc (darkplaces engine)
	plane = node->plane;
	if (plane->type < 3)
	{
		front = start[plane->type] - plane->dist;
		back = end[plane->type] - plane->dist;
	}
	else
	{
		front = DotProduct (start, plane->normal) - plane->dist;
		back = DotProduct (end, plane->normal) - plane->dist;
	}
	side = front < 0;

	/*if ( (back < 0) == side)
		return RecursiveLightPoint (node->children[side], start, end);*/

// optimization borrowed from LordHavoc (darkplaces engine)
	if ((back < 0) == side)
	{
		node = node->children[side];
		goto loc0;
	}

	frac = front / (front-back);
	mid[0] = start[0] + (end[0] - start[0])*frac;
	mid[1] = start[1] + (end[1] - start[1])*frac;
	mid[2] = start[2] + (end[2] - start[2])*frac;

// go down front side
	r = RecursiveLightPoint (node->children[side], start, mid);
	if (r >= 0)
		return r;		// hit something

	if ( (back < 0) == side )
		return -1;		// didn't hit anuthing

// check for impact on this node

	surf = cl.worldmodel->surfaces + node->firstsurface;
	for (i = 0; i < node->numsurfaces; i++, surf++)
	{
		if (surf->flags & SURF_DRAWTILED)
			continue;	// no lightmaps

		tex = surf->texinfo;

		/* added double casts so that 64 bit/sse2 builds' precision
		 * matches that of x87 floating point. took from QuakeSpasm,
		 * patch by Eric Wasylishen.  */
		s = DotProductDBL(mid, tex->vecs[0]) + (double)tex->vecs[0][3];
		t = DotProductDBL(mid, tex->vecs[1]) + (double)tex->vecs[1][3];

		if (s < surf->texturemins[0] || t < surf->texturemins[1])
			continue;

		ds = s - surf->texturemins[0];
		dt = t - surf->texturemins[1];

		if (ds > surf->extents[0] || dt > surf->extents[1])
			continue;

		if (!surf->samples)
			return 0;

		ds >>= 4;
		dt >>= 4;

		lightmap = surf->samples;
		r = 0;
		if (lightmap)
		{
			lightmap += dt * ((surf->extents[0] >> 4) + 1) + ds;

			for (maps = 0; maps < MAXLIGHTMAPS && surf->styles[maps] != 255; maps++)
			{
				scale = d_lightstylevalue[surf->styles[maps]];
				r += *lightmap * scale;
				lightmap += ((surf->extents[0] >> 4) + 1) * ((surf->extents[1] >> 4) + 1);
			}

			r >>= 8;
		}

		return r;
	}

// go down back side
	//return RecursiveLightPoint (node->children[!side], mid, end);

// surgeon: optimized recursion
	node = node->children[!side];
	VectorCopy(start, mid);
	goto loc0;
}

int R_LightPoint (vec3_t p)
{
	vec3_t		end;
	int			r;

	if (!cl.worldmodel->lightdata)
		return 255;

	end[0] = p[0];
	end[1] = p[1];
	end[2] = p[2] - 2048;

	r = RecursiveLightPoint (cl.worldmodel->nodes, p, end);

	if (r == -1)
		r = 0;

	if (r < r_refdef.ambientlight)
		r = r_refdef.ambientlight;

	return r;
}

