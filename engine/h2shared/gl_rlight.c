/*
 * r_light.c
 *
 * $Id: gl_rlight.c,v 1.17 2007-09-14 14:10:01 sezero Exp $
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

static int	r_dlightframecount;


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

DYNAMIC LIGHTS BLEND RENDERING

=============================================================================
*/

static void AddLightBlend (float r, float g, float b, float a2)
{
	float	a;

	v_blend[3] = a = v_blend[3] + a2*(1-v_blend[3]);

	a2 = a2/a;

	v_blend[0] = v_blend[1]*(1-a2) + r*a2;
	v_blend[1] = v_blend[1]*(1-a2) + g*a2;
	v_blend[2] = v_blend[2]*(1-a2) + b*a2;
}


#define DLIGHT_BUBBLE_WEDGES		16
static float	bubble_sintable[DLIGHT_BUBBLE_WEDGES+1];
static float	bubble_costable[DLIGHT_BUBBLE_WEDGES+1];

void R_InitBubble (void)
{
	float	a;
	int			i;
	float	*bub_sin, *bub_cos;

	bub_sin = bubble_sintable;
	bub_cos = bubble_costable;

	for (i = DLIGHT_BUBBLE_WEDGES; i >= 0; i--)
	{
		a = i / ((float)DLIGHT_BUBBLE_WEDGES) * M_PI * 2;
		*bub_sin++ = sin(a);
		*bub_cos++ = cos(a);
	}
}


static void R_RenderDlight (dlight_t *light)
{
	int		i, j;
	vec3_t	v;
	float	rad;
	float	*bub_sin, *bub_cos;

	bub_sin = bubble_sintable;
	bub_cos = bubble_costable;
	rad = light->radius * 0.35;

	VectorSubtract (light->origin, r_origin, v);
	if (VectorLength (v) < rad)
	{	// view is inside the dlight
		AddLightBlend (1, 0.5, 0, light->radius * 0.0003);
		return;
	}

	glBegin_fp (GL_TRIANGLE_FAN);

	if (light->color[0] || light->color[1] || light->color[2])
	{
		glColor4fv_fp (light->color);
	}
	else
	{
#ifndef H2W
		glColor3f_fp (0.2,0.1,0.0);
#else
		glColor3f_fp (0.2,0.1,0.05); // changed dimlight effect
#endif
	}

	for (i = 0; i < 3; i++)
		v[i] = light->origin[i] - vpn[i]*rad;

	glVertex3fv_fp (v);
	glColor3f_fp (0,0,0);

	for (i = DLIGHT_BUBBLE_WEDGES; i >= 0; i--)
	{
		for (j = 0; j < 3; j++)
			v[j] = light->origin[j] + (vright[j] * (*bub_cos) + vup[j] * (*bub_sin)) * rad;

		bub_sin++;
		bub_cos++;

		glVertex3fv_fp (v);
	}
	glEnd_fp ();
}

/*
=============
R_RenderDlights
=============
*/
void R_RenderDlights (void)
{
	int		i;
	dlight_t	*l;

	if (!gl_flashblend.integer)
		return;

	r_dlightframecount = r_framecount + 1;	// because the count hasn't
						//  advanced yet for this frame
	glDepthMask_fp (0);
	glDisable_fp (GL_TEXTURE_2D);
	glShadeModel_fp (GL_SMOOTH);
	glEnable_fp (GL_BLEND);
	glBlendFunc_fp (GL_ONE, GL_ONE);

	l = cl_dlights;
	for (i = 0; i < MAX_DLIGHTS; i++, l++)
	{
		if (l->die < cl.time || !l->radius)
			continue;
		R_RenderDlight (l);
	}

	glColor3f_fp (1,1,1);
	glDisable_fp (GL_BLEND);
	glEnable_fp (GL_TEXTURE_2D);
	glBlendFunc_fp (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask_fp (1);
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
#if 0	/* the original version from ID */
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
#else	/* the major speedup version by Lord Havoc */
void R_MarkLights (dlight_t *light, int bit, mnode_t *node)
{
	mplane_t	*splitplane;
	float		l, dist, maxdist;
	msurface_t	*surf;
	int		i, j, s, t;
	vec3_t		impact;

loc0:
	if (node->contents < 0)
		return;

	splitplane = node->plane;
	if (splitplane->type < 3)
		dist = light->origin[splitplane->type] - splitplane->dist;
	else
		dist = DotProduct (light->origin, splitplane->normal) - splitplane->dist;

	if (dist > light->radius)
	{
		node = node->children[0];
		goto loc0;
	}
	if (dist < -light->radius)
	{
		node = node->children[1];
		goto loc0;
	}

	maxdist = light->radius * light->radius;

// mark the polygons
	surf = cl.worldmodel->surfaces + node->firstsurface;
	for (i = 0; i < node->numsurfaces; i++, surf++)
	{	// eliminates marking of surfaces too far away from light,
		// thus preventing unnecessary renders and uploads
		for (j = 0; j < 3; j++)
			impact[j] = light->origin[j] - surf->plane->normal[j]*dist;

		// clamp center of light to corner and check brightness
		l = DotProduct (impact, surf->texinfo->vecs[0]) + surf->texinfo->vecs[0][3] - surf->texturemins[0];
		s = l + 0.5;
		if (s < 0)
			s = 0;
		else if (s > surf->extents[0])
			s = surf->extents[0];
		s = l - s;
		l = DotProduct (impact, surf->texinfo->vecs[1]) + surf->texinfo->vecs[1][3] - surf->texturemins[1];
		t = l + 0.5;
		if (t < 0)
			t = 0;
		else if (t > surf->extents[1])
			t = surf->extents[1];
		t = l - t;

		// compare to minimum light
		if ((s*s + t*t + dist*dist) < maxdist)
		{
			if (surf->dlightframe != r_dlightframecount)
			{	// not dynamic until now
				surf->dlightbits = bit;
				surf->dlightframe = r_dlightframecount;
			}
			else	// already dynamic
				surf->dlightbits |= bit;
		}
	}

	if (node->children[0]->contents >= 0)
		R_MarkLights (light, bit, node->children[0]);
	if (node->children[1]->contents >= 0)
		R_MarkLights (light, bit, node->children[1]);
}
#endif	/* end of 2 R_MarkLights versions */


/*
=============
R_PushDlights
=============
*/
void R_PushDlights (void)
{
	int		i;
	dlight_t	*l;

	if (gl_flashblend.integer)
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

vec3_t			lightspot;

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

	if (node->contents < 0)
		return -1;		// didn't hit anything

// calculate mid point

// FIXME: optimize for axial
	plane = node->plane;
	front = DotProduct (start, plane->normal) - plane->dist;
	back = DotProduct (end, plane->normal) - plane->dist;
	side = front < 0;

	if ( (back < 0) == side)
		return RecursiveLightPoint (node->children[side], start, end);

	frac = front / (front-back);
	mid[0] = start[0] + (end[0] - start[0])*frac;
	mid[1] = start[1] + (end[1] - start[1])*frac;
	mid[2] = start[2] + (end[2] - start[2])*frac;

// go down front side
	r = RecursiveLightPoint (node->children[side], start, mid);
	if (r >= 0)
		return r;		// hit something

	if ( (back < 0) == side )
		return -1;		// didn't hit anything

// check for impact on this node
	VectorCopy (mid, lightspot);

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
	return RecursiveLightPoint (node->children[!side], mid, end);
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

	return r;
}


static int RecursiveLightPointColor (vec3_t color, mnode_t *node, vec3_t start, vec3_t end)
{
	float		front, back, frac;
	vec3_t		mid;

loc0:
	if (node->contents < 0)
		return false;		// didn't hit anything

// calculate mid point
	if (node->plane->type < 3)
	{
		front = start[node->plane->type] - node->plane->dist;
		back = end[node->plane->type] - node->plane->dist;
	}
	else
	{
		front = DotProduct(start, node->plane->normal) - node->plane->dist;
		back = DotProduct(end, node->plane->normal) - node->plane->dist;
	}
	// LordHavoc: optimized recursion
	if ((back < 0) == (front < 0))
	{
		node = node->children[front < 0];
		goto loc0;
	}

	frac = front / (front-back);
	mid[0] = start[0] + (end[0] - start[0])*frac;
	mid[1] = start[1] + (end[1] - start[1])*frac;
	mid[2] = start[2] + (end[2] - start[2])*frac;

// go down front side
	if (RecursiveLightPointColor (color, node->children[front < 0], start, mid))
		return true;	// hit something
	else
	{
		int		i, ds, dt;
		msurface_t	*surf;
// check for impact on this node
		VectorCopy (mid, lightspot);
		surf = cl.worldmodel->surfaces + node->firstsurface;
		for (i = 0; i < node->numsurfaces; i++, surf++)
		{
			if (surf->flags & SURF_DRAWTILED)
				continue;	// no lightmaps
			ds = (int) ((float) DotProduct(mid, surf->texinfo->vecs[0]) + surf->texinfo->vecs[0][3]);
			dt = (int) ((float) DotProduct(mid, surf->texinfo->vecs[1]) + surf->texinfo->vecs[1][3]);
			if (ds < surf->texturemins[0] || dt < surf->texturemins[1])
				continue;

			ds -= surf->texturemins[0];
			dt -= surf->texturemins[1];

			if (ds > surf->extents[0] || dt > surf->extents[1])
				continue;
			if (surf->samples)
			{
				// LordHavoc: enhanced to interpolate lighting
				byte	*lightmap;
				float	scale;
				int	maps, line3,
					dsfrac = ds & 15,
					dtfrac = dt & 15,
					r00 = 0, g00 = 0, b00 = 0,
					r01 = 0, g01 = 0, b01 = 0,
					r10 = 0, g10 = 0, b10 = 0,
					r11 = 0, g11 = 0, b11 = 0;

				line3 = ((surf->extents[0]>>4) + 1) * 3;
				lightmap = surf->samples + ((dt>>4) * ((surf->extents[0]>>4) + 1) + (ds>>4)) * 3;
				for (maps = 0; maps < MAXLIGHTMAPS && surf->styles[maps] != 255; maps++)
				{
					scale = (float) d_lightstylevalue[surf->styles[maps]] * 1.0 / 256.0;
					r00 += (float) lightmap[0] * scale;
					g00 += (float) lightmap[1] * scale;
					b00 += (float) lightmap[2] * scale;
					r01 += (float) lightmap[3] * scale;
					g01 += (float) lightmap[4] * scale;
					b01 += (float) lightmap[5] * scale;
					r10 += (float) lightmap[line3+0] * scale;
					g10 += (float) lightmap[line3+1] * scale;
					b10 += (float) lightmap[line3+2] * scale;
					r11 += (float) lightmap[line3+3] * scale;
					g11 += (float) lightmap[line3+4] * scale;
					b11 += (float) lightmap[line3+5] * scale;
					lightmap += ((surf->extents[0]>>4) + 1) * ((surf->extents[1]>>4) + 1) * 3;
				}
				color[0] += (float) ((int) ((((((((r11-r10) * dsfrac) >> 4) + r10)-((((r01-r00) * dsfrac) >> 4) + r00)) * dtfrac) >> 4) + ((((r01-r00) * dsfrac) >> 4) + r00)));
				color[1] += (float) ((int) ((((((((g11-g10) * dsfrac) >> 4) + g10)-((((g01-g00) * dsfrac) >> 4) + g00)) * dtfrac) >> 4) + ((((g01-g00) * dsfrac) >> 4) + g00)));
				color[2] += (float) ((int) ((((((((b11-b10) * dsfrac) >> 4) + b10)-((((b01-b00) * dsfrac) >> 4) + b00)) * dtfrac) >> 4) + ((((b01-b00) * dsfrac) >> 4) + b00)));
			}
			return true; // success
		}
	// go down back side
		return RecursiveLightPointColor (color, node->children[front >= 0], mid, end);
	}
}

vec3_t			lightcolor;

float R_LightPointColor (vec3_t p)
{
	vec3_t		end;

	if (!cl.worldmodel->lightdata)
	{
		lightcolor[0] = lightcolor[1] = lightcolor[2] = 255.0;
		return 255.0;
	}

	end[0] = p[0];
	end[1] = p[1];
	end[2] = p[2] - 2048;

	lightcolor[0] = lightcolor[1] = lightcolor[2] = 0;
	RecursiveLightPointColor (lightcolor, cl.worldmodel->nodes, p, end);
	return (lightcolor[0] + lightcolor[1] + lightcolor[2]) / 3.0;
}

