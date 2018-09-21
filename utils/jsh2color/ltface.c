/*
 * ltface.c
 * $Id: ltface.c,v 1.13 2010-10-24 08:35:47 sezero Exp $
 *
 * Copyright (C) 1996-1997  Id Software, Inc.
 * Modifications by Kevin Shanahan, 1999-2000
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

#include "q_stdinc.h"
#include "compiler.h"
#include "arch_def.h"
#include "cmdlib.h"
#include "mathlib.h"
#include "bspfile.h"
#include "entities.h"
#include "threads.h"
#include "tyrlite.h"
#include "jscolor.h"

/*
============
CastRay

Returns the distance between the points, or -1 if blocked
=============
*/
static vec_t CastRay (vec3_t p1, vec3_t p2)
{
	int		i;
	vec_t	t;
	qboolean	trace;

	trace = TestLine (p1, p2);

	if (!trace)
		return -1;		// ray was blocked

	t = 0;
	for (i = 0 ; i < 3 ; i++)
		t += (p2[i]-p1[i]) * (p2[i]-p1[i]);

	if (t <= 0)
		t = 1;		// don't blow up...

	return sqrt(t);
}


/*
===============================================================================

SAMPLE POINT DETERMINATION

void SetupBlock (dface_t *f) Returns with surfpt[] set

This is a little tricky because the lightmap covers more area than the face.
If done in the straightforward fashion, some of the
sample points will be inside walls or on the other side of walls, causing
false shadows and light bleeds.

To solve this, I only consider a sample point valid if a line can be drawn
between it and the exact midpoint of the face.  If invalid, it is adjusted
towards the center until it is valid.

(this doesn't completely work)

===============================================================================
*/

/* FIXME: HACK HACK HACK:  TEX_SPECIAL cases don't fit into 18*18 lightmap
 * size and the code still wants to play them (see TestLightFace() calling
 * CalcFaceExtents() with a no-failure flag), so I bumped the SINGLEMAP
 * definition from 18*18*4 to 64*64*4.  Otherwise the surf pointer goes out
 * of bounds in CalcPoints() and also probably in SingleLightFace(),
 * and TestSingleLightFace() and overwrites other data ==> stack corruption.
 * See the Error() statements down below.  */
#define	SINGLEMAP	(64*64*4)	// (18*18*4)

typedef struct
{
	vec_t	lightmaps[MAXLIGHTMAPS][SINGLEMAP];
	int		numlightstyles;
	vec_t	*light;
	vec_t	facedist;
	vec3_t	facenormal;

	int		numsurfpt;
	vec3_t	surfpt[SINGLEMAP];

	vec3_t	texorg;
	vec3_t	worldtotex[2];	// s = (world - texorg) . worldtotex[0]
	vec3_t	textoworld[2];	// world = texorg + s * textoworld[0]

	vec_t	exactmins[2], exactmaxs[2];

	int		texmins[2], texsize[2];
	int		lightstyles[256];
	int		surfnum;
	dface_t	*face;

	// colored lighting
	vec3_t	lightmapcolors[MAXLIGHTMAPS][SINGLEMAP];

	// texture light color modification
	char texname[16];
} lightinfo_t;


/*
================
CalcFaceVectors

Fills in texorg, worldtotex. and textoworld
================
*/
static void CalcFaceVectors (lightinfo_t *l)
{
	texinfo_t	*tex;
	int			i, j;
	vec3_t	texnormal;
	float	distscale;
	vec_t	dist, len;

	tex = &texinfo[l->face->texinfo];

// convert from float to vec_t
	for (i = 0 ; i < 2 ; i++)
	{
		for (j = 0 ; j < 3 ; j++)
			l->worldtotex[i][j] = tex->vecs[i][j];
	}

// calculate a normal to the texture axis.  points can be moved along this
// without changing their S/T
	texnormal[0] = tex->vecs[1][1]*tex->vecs[0][2] - tex->vecs[1][2]*tex->vecs[0][1];
	texnormal[1] = tex->vecs[1][2]*tex->vecs[0][0] - tex->vecs[1][0]*tex->vecs[0][2];
	texnormal[2] = tex->vecs[1][0]*tex->vecs[0][1] - tex->vecs[1][1]*tex->vecs[0][0];
	VectorNormalize (texnormal);

// flip it towards plane normal
	distscale = DotProduct (texnormal, l->facenormal);
	if (!distscale)
		COM_Error ("Texture axis perpendicular to face");
	if (distscale < 0)
	{
		distscale = -distscale;
		VectorNegate (texnormal, texnormal);
	}

// distscale is the ratio of the distance along the texture normal to
// the distance along the plane normal
	distscale = 1/distscale;

	for (i = 0 ; i < 2 ; i++)
	{
		len = VectorLength (l->worldtotex[i]);
		dist = DotProduct (l->worldtotex[i], l->facenormal);
		dist *= distscale;
		VectorMA (l->worldtotex[i], -dist, texnormal, l->textoworld[i]);
		VectorScale (l->textoworld[i], (1/len)*(1/len), l->textoworld[i]);
	}

// calculate texorg on the texture plane
	for (i = 0 ; i < 3 ; i++)
		l->texorg[i] = -tex->vecs[0][3]* l->textoworld[0][i] - tex->vecs[1][3] * l->textoworld[1][i];

// project back to the face plane
	dist = DotProduct (l->texorg, l->facenormal) - l->facedist - 1;
	dist *= distscale;
	VectorMA (l->texorg, -dist, texnormal, l->texorg);
}

/*
================
CalcFaceExtents

Fills in s->texmins[] and s->texsize[]
also sets exactmins[] and exactmaxs[]
================
*/
static void CalcFaceExtents (lightinfo_t *l, const vec3_t faceoffset, qboolean fail)
{
	dface_t	*s;
	vec_t	mins[2], maxs[2], val;
	int		i, j, e;
	dvertex_t	*v;
	texinfo_t	*tex;

	s = l->face;

	mins[0] = mins[1] = 999999;
	maxs[0] = maxs[1] = -99999;

	tex = &texinfo[s->texinfo];

	for (i = 0 ; i < s->numedges ; i++)
	{
		e = dsurfedges[s->firstedge+i];
		if (e >= 0)
			v = dvertexes + dedges[e].v[0];
		else
			v = dvertexes + dedges[-e].v[1];

		for (j = 0 ; j < 2 ; j++)
		{
			val =	((double)v->point[0] + faceoffset[0]) * (double)tex->vecs[j][0] +
				((double)v->point[1] + faceoffset[1]) * (double)tex->vecs[j][1] +
				((double)v->point[2] + faceoffset[2]) * (double)tex->vecs[j][2] +
				(double)tex->vecs[j][3];

			if (val < mins[j])
				mins[j] = val;
			if (val > maxs[j])
				maxs[j] = val;
		}
	}

	for (i = 0 ; i < 2 ; i++)
	{
		l->exactmins[i] = mins[i];
		l->exactmaxs[i] = maxs[i];

		mins[i] = floor(mins[i]/16);
		maxs[i] = ceil(maxs[i]/16);

		/*
		l->texmins[i] = (int)floor(mins[i]);
		l->texsize[i] = (int)floor(maxs[i] - mins[i]);
		*/
		l->texmins[i] = mins[i];
		l->texsize[i] = maxs[i] - mins[i];

		if (fail && l->texsize[i] > 17)
			COM_Error ("Bad surface extents");
	}
}

/*
=================
CalcPoints

For each texture aligned grid point, back project onto the plane
to get the world xyz value of the sample point
=================
*/
static void CalcPoints (lightinfo_t *l)
{
	int		i;
	int		s, t, j;
	int		w, h, step;
	vec_t	starts, startt, us, ut;
	vec_t	*surf;
	vec_t	mids, midt;
	vec3_t	facemid, move;

//
// fill in surforg
// the points are biased towards the center of the surface
// to help avoid edge cases just inside walls
//
	surf = l->surfpt[0];
	mids = (l->exactmaxs[0] + l->exactmins[0])/2;
	midt = (l->exactmaxs[1] + l->exactmins[1])/2;

	for (j = 0 ; j < 3 ; j++)
		facemid[j] = l->texorg[j] + l->textoworld[0][j]*mids + l->textoworld[1][j]*midt;

	if (extrasamples)
	{	// extra filtering
		h = (l->texsize[1]+1)*2;
		w = (l->texsize[0]+1)*2;
		starts = (l->texmins[0]-0.5)*16;
		startt = (l->texmins[1]-0.5)*16;
		step = 8;
	}
	else
	{
		h = l->texsize[1]+1;
		w = l->texsize[0]+1;
		starts = l->texmins[0]*16;
		startt = l->texmins[1]*16;
		step = 16;
	}

	l->numsurfpt = w * h;
	for (t = 0 ; t < h ; t++)
	{
		for (s = 0 ; s < w ; s++, surf+=3)
		{
			if (surf > l->surfpt[SINGLEMAP - 1])
				COM_Error ("%s: surf out of bounds (numsurfpt=%d)", __thisfunc__, l->numsurfpt);
			us = starts + s*step;
			ut = startt + t*step;

		// if a line can be traced from surf to facemid, the point is good
			for (i = 0 ; i < 6 ; i++)
			{
			// calculate texture point
				for (j = 0 ; j < 3 ; j++)
					surf[j] = l->texorg[j] + l->textoworld[0][j]*us + l->textoworld[1][j]*ut;

				if (CastRay (facemid, surf) != -1)
					break;	// got it

				if (i & 1)
				{
					if (us > mids)
					{
						us -= 8;
						if (us < mids)
							us = mids;
					}
					else
					{
						us += 8;
						if (us > mids)
							us = mids;
					}
				}
				else
				{
					if (ut > midt)
					{
						ut -= 8;
						if (ut < midt)
							ut = midt;
					}
					else
					{
						ut += 8;
						if (ut > midt)
							ut = midt;
					}
				}

				// move surf 8 pixels towards the center
				VectorSubtract (facemid, surf, move);
				VectorNormalize (move);
				VectorMA (surf, 8, move, surf);
			}
		}
	}
}


/*
===============================================================================

FACE LIGHTING

===============================================================================
*/

/*
==============================================
TYRLITE: Attenuation formulae setup functions
==============================================
*/
static vec_t scaledDistance (vec_t distance, entity_t *light)
{
	switch (light->formula)
	{
	case 1:
	case 2:
	case 3:
	/* Return a small distance to prevent culling these lights, since we */
	/* know these formulae won't fade to nothing.			     */
		return (distance<=0) ? -0.25 : 0.25;
	case 0:
		return scaledist * light->atten * distance;
	default:
		return 1;	/* shut up compiler warnings */
	}
}

static vec_t scaledLight (vec_t distance, entity_t *light)
{
	vec_t tmp = scaledist * light->atten * distance;
	switch (light->formula)
	{
	case 3:
		return light->light;
	case 1:
		return light->light / (tmp / 128);
	case 2:
		return light->light / ((tmp * tmp) / 16384);
	case 0:
		if (light->light > 0)
			return (light->light - tmp > 0) ? light->light - tmp : 0;
		else	return (light->light + tmp < 0) ? light->light + tmp : 0;
	default:
		return 1;	/* shut up compiler warnings */
	}
}

/*
================
SingleLightFace
================
*/
static void SingleLightFace (entity_t *light, lightinfo_t *l, const vec3_t faceoffset)
{
	vec_t	dist;
	vec3_t	incoming;
	vec_t	angle;
	vec_t	add;
	vec_t	*surf;
	qboolean	hit;
	int		mapnum;
	int		size;
	int		c, i;
	vec3_t	rel;
	vec3_t	spotvec;
	vec_t	falloff;
	vec_t	*lightsamp;
	/* Colored lighting */
	vec3_t	*lightcolorsamp;

	VectorSubtract (light->origin, bsp_origin, rel);
	dist = scaledDistance((DotProduct(rel, l->facenormal) - l->facedist), light);

// don't bother with lights behind the surface
	if (dist <= 0)
		return;

// don't bother with light too far away
	//if (dist > light->light)
	if (dist > abs(light->light))
	{
		return;
	}

	if (light->targetent)
	{
		VectorSubtract (light->targetent->origin, light->origin, spotvec);
		VectorNormalize (spotvec);
		if (!light->angle)
			falloff = -cos(20*Q_PI/180);
		else
			falloff = -cos(light->angle/2*Q_PI/180);
	}
	else if (light->use_mangle)
	{
		VectorCopy (light->mangle, spotvec);
		if (!light->angle)
			falloff = -cos(20*Q_PI/180);
		else	falloff = -cos(light->angle/2*Q_PI/180);
	}
	else
	{
		falloff = 0;	// shut up compiler warnings
		VectorClear (spotvec); // shut up static analyzers
	}

	for (mapnum = 0; mapnum < l->numlightstyles; mapnum++)
	{
		if (l->lightstyles[mapnum] == light->style)
			break;
	}
	lightsamp = l->lightmaps[mapnum];
	lightcolorsamp = l->lightmapcolors[mapnum];
	if (mapnum == l->numlightstyles)
	{	// init a new light map
		if (mapnum == MAXLIGHTMAPS)
		{
			printf ("WARNING: Too many light styles on a face\n");
			return;
		}
		size = (l->texsize[1]+1)*(l->texsize[0]+1);
		for (i = 0 ; i < size ; i++)
		{
			lightcolorsamp[i][0] = 0;
			lightcolorsamp[i][1] = 0;
			lightcolorsamp[i][2] = 0;
			lightsamp[i] = 0;
		}
	}

//
// check it for real
//
	hit = false;

	surf = l->surfpt[0];
	for (c = 0 ; c < l->numsurfpt ; c++, surf+=3)
	{
		if (surf > l->surfpt[SINGLEMAP - 1])
			COM_Error ("%s: surf out of bounds (numsurfpt=%d)", __thisfunc__, l->numsurfpt);
		dist = scaledDistance(CastRay(light->origin, surf), light);
		if (dist < 0)
			continue;	// light doesn't reach

		VectorSubtract (light->origin, surf, incoming);
		VectorNormalize (incoming);
		angle = DotProduct (incoming, l->facenormal);
		if (light->targetent || light->use_mangle)
		{	// spotlight cutoff
			if (DotProduct (spotvec, incoming) > falloff)
				continue;
		}

		angle = (1.0-scalecos) + scalecos*angle;
		add = scaledLight(CastRay(light->origin, surf), light);
		add *= angle;
		lightsamp[c] += add;
		if (lightsamp[c] > 255)
			lightsamp[c] = 255;

		add /= 255.0;
		lightcolorsamp[c][0] += add * light->lightcolor[0];
		lightcolorsamp[c][1] += add * light->lightcolor[1];
		lightcolorsamp[c][2] += add * light->lightcolor[2];

		if (abs((int) lightsamp[c]) > 1)	// ignore really tiny lights
			hit = true;
	}

	if (mapnum == l->numlightstyles && hit)
	{
		if (mapnum == MAXLIGHTMAPS-1)
		{
			printf ("WARNING: Too many light styles on a face\n");
			return;
		}
		l->lightstyles[mapnum] = light->style;
		l->numlightstyles++;	// the style has some real data now
	}
}

/*
============
FixMinlight
============
*/
static void FixMinlight (lightinfo_t *l)
{
	int		i, j, k;
	vec_t	tmp;

	if (!worldminlight)
		return;

	for (i = 0 ; i < l->numlightstyles ; i++)
	{
		if (l->lightstyles[i] == 0)
			break;
	}
	if (i == l->numlightstyles)
	{
		if (l->numlightstyles == MAXLIGHTMAPS)
			return;		// oh well..
		for (j = 0 ; j < l->numsurfpt ; j++)
			l->lightmaps[i][j] = worldminlight;

		for (j = 0 ; j < l->numsurfpt ; j++)
		{
			l->lightmapcolors[i][j][0] = (worldminlight * minlight_color[0]) /255;
			l->lightmapcolors[i][j][1] = (worldminlight * minlight_color[1]) /255;
			l->lightmapcolors[i][j][2] = (worldminlight * minlight_color[2]) /255;
		}

		l->lightstyles[i] = 0;
		l->numlightstyles++;
	}
	else
	{
		for (j = 0 ; j < l->numsurfpt ; j++)
		{
			if ( l->lightmaps[i][j] < worldminlight)
				l->lightmaps[i][j] = worldminlight;
			for (k = 0 ; k < 3 ; k++)
			{
				tmp = (vec_t)(worldminlight * minlight_color[k]) / 255.0;
				if (l->lightmapcolors[i][j][k] < tmp )
					l->lightmapcolors[i][j][k] = tmp;
			}
		}
	}
}


/*
============
LightFace
============
*/
void LightFaceLIT (int surfnum, const vec3_t faceoffset)
{
	dface_t	*f;
	lightinfo_t	l;
	int		s, t;
	int		i, j, c;
	int		size;
	int		lightmapwidth;
	byte	*out;
	int	w;
	/* TYR - temp vars */
	vec_t		maxc;
	int		x1, x2, x3, x4;
	/* TYR - colored lights */
	vec3_t		*lightcolor;
	vec3_t		totalcolors;
	vec3_t		point;


	f = dfaces + surfnum;

	// this version already has the light offsets calculated from
	// the original lighting, so we will just reuse them.
	if (f->lightofs == -1)
		return;

	for (j = 0 ; j < MAXLIGHTMAPS ; j++)
		f->styles[j] = 255;

	if ( texinfo[f->texinfo].flags & TEX_SPECIAL)
	{	// non-lit texture
		return;
	}

	memset (&l, 0, sizeof(l));
	l.surfnum = surfnum;
	l.face = f;

//
// rotate plane
//
	VectorCopy (dplanes[f->planenum].normal, l.facenormal);
	l.facedist = dplanes[f->planenum].dist;
	VectorScale (l.facenormal, l.facedist, point);
	VectorAdd (point, faceoffset, point);
	l.facedist = DotProduct( point, l.facenormal );

	if (f->side)
	{
		VectorNegate (l.facenormal, l.facenormal);
		l.facedist = -l.facedist;
	}

	CalcFaceVectors (&l);
	CalcFaceExtents (&l, faceoffset, true);
	CalcPoints (&l);

	lightmapwidth = l.texsize[0]+1;

	size = lightmapwidth*(l.texsize[1]+1);
	if (size > SINGLEMAP)
		COM_Error ("Bad lightmap size");

	for (i = 0 ; i < MAXLIGHTMAPS ; i++)
		l.lightstyles[i] = 255;

	l.numlightstyles = 0;

	strcpy (l.texname, miptex[texinfo[f->texinfo].miptex].name);

	for (i = 0 ; i < num_entities ; i++)
	{
		if (entities[i].light)
			SingleLightFace (&entities[i], &l, faceoffset);
	}

// minimum lighting
	FixMinlight (&l);

	if (!l.numlightstyles)
	{	// no light hitting it
		return;
	}

//
// save out the values
//
	for (i = 0 ; i < MAXLIGHTMAPS ; i++)
		f->styles[i] = l.lightstyles[i];

	// we have to store the new light data at
	// the same offset as the old stuff...
	out = &newdlightdata[faces_ltoffset[surfnum]];

// extra filtering
	w = (l.texsize[0] + 1) * 2;

	for (i = 0 ; i < l.numlightstyles ; i++)
	{
		if (l.lightstyles[i] == 0xff)
			COM_Error ("Wrote empty lightmap");

		lightcolor = l.lightmapcolors[i];
		c = 0;

		for (t = 0 ; t <= l.texsize[1] ; t++)
		{
			for (s = 0 ; s <= l.texsize[0] ; s++, c++)
			{
				if (extrasamples)
				{
					x1 = t*2*w + s*2;
					x2 = x1 + 1;
					x3 = (t*2 + 1)*w + s*2;
					x4 = x3 + 1;

					// calculate the color
					totalcolors[0] = lightcolor[x1][0] + lightcolor[x2][0] + lightcolor[x3][0] + lightcolor[x4][0];
					totalcolors[0] *= 0.25;
					totalcolors[1] = lightcolor[x1][1] + lightcolor[x2][1] + lightcolor[x3][1] + lightcolor[x4][1];
					totalcolors[1] *= 0.25;
					totalcolors[2] = lightcolor[x1][2] + lightcolor[x2][2] + lightcolor[x3][2] + lightcolor[x4][2];
					totalcolors[2] *= 0.25;
				}
				else
				{
					VectorCopy (lightcolor[c], totalcolors);
				}

				// CSL - Scale back intensity, instead
				//	 of capping individual colors
				VectorScale (totalcolors, rangescale, totalcolors);
				maxc = 0;

				for (j = 0; j < 3; j++)
				{
					if (totalcolors[j] > maxc)
					{
						maxc = totalcolors[j];
					}
					else if (totalcolors[j] < 0)
					{
						totalcolors[j] = 0;	// this used to be an error!!!!
					}
				}
				if (maxc > 255.0)
					VectorScale (totalcolors, 255.0 / maxc, totalcolors);

				// write out the lightmap in RGBA format
				*out++ = totalcolors[0];
				*out++ = totalcolors[1];
				*out++ = totalcolors[2];
			}
		}
	}
}

static void TestSingleLightFace (entity_t *light, lightinfo_t *l, const vec3_t faceoffset)
{
	vec_t	dist;
	vec_t	add;
	vec_t	*surf;
	vec3_t	rel;
	int	surf_r;
	int	surf_g;
	int	surf_b;
	int	c;

	VectorSubtract (light->origin, bsp_origin, rel);
	dist = scaledDistance((DotProduct(rel, l->facenormal) - l->facedist), light);

	// don't bother with lights behind the surface
	if (dist <= 0)
		return;

	// don't bother with light too far away
	if (dist > abs(light->light))
	{
		return;
	}

	// mfah - find the light color based on the surface name
	FindTexlightColor (&surf_r, &surf_g, &surf_b, l->texname);

	surf = l->surfpt[0];

	// we could speed the whole thing up drastically by checking only
	// the first and last point of each face - trouble is, any large
	// faces may have a light that only hits the middle.
	for (c = 0 ; c < l->numsurfpt ; c++, surf+=3)
	{
		if (surf > l->surfpt[SINGLEMAP - 1])
			COM_Error ("%s: surf out of bounds (numsurfpt=%d)", __thisfunc__, l->numsurfpt);
		dist = scaledDistance(CastRay(light->origin, surf), light);

		if (dist < 0)
			continue;	// light doesn't reach

		add = scaledLight(CastRay(light->origin, surf), light);

		if (add < (light->light / 3))
			continue;

		// normal light - other lights already have a color assigned
		// to them from when they were initially loaded
		// this will give madly high color values here so we will
		// scale them down later on
		light->lightcolor[0] = light->lightcolor[0] + surf_r;
		light->lightcolor[1] = light->lightcolor[1] + surf_g;
		light->lightcolor[2] = light->lightcolor[2] + surf_b;

		// speed up the checking process some more - if we have one hit
		// on a face, all other hits on the same face are just going to
		// give the same result - so we can return now.
		return;
	}
}

void TestLightFace (int surfnum, const vec3_t faceoffset)
{
	dface_t	*f;
	lightinfo_t	l;
	int		i;
//	int		j, c;
	vec3_t		point;

	f = dfaces + surfnum;

	memset (&l, 0, sizeof(l));

	strcpy (l.texname, miptex[texinfo[f->texinfo].miptex].name);

// we can speed up the checking process by ignoring any textures
// that give white light. this hasn't been done since version 0.2,
// we can get rid of it
//	FindTexlightColor (&i, &j, &c, miptex[texinfo[f->texinfo].miptex].name);
//	if (i == 255 && j == 255 && c == 255)
//		return;

	// don't even bother with sky - although we might later on if we can
	// get some kinda good sky textures going.
	if (!strncmp (l.texname, "sky", 3))
		return;

	l.surfnum = surfnum;
	l.face = f;

	/* rotate plane */

	VectorCopy (dplanes[f->planenum].normal, l.facenormal);
	l.facedist = dplanes[f->planenum].dist;
	VectorScale (l.facenormal, l.facedist, point);
	VectorAdd (point, faceoffset, point);
	l.facedist = DotProduct( point, l.facenormal );

	if (f->side)
	{
		VectorNegate (l.facenormal, l.facenormal);
		l.facedist = -l.facedist;
	}

	CalcFaceVectors (&l);

	// use the safe version here which will not give bad surface
	// extents on special textures
	CalcFaceExtents(&l, faceoffset, false);

	CalcPoints (&l);

	for (i = 0 ; i < num_entities ; i++)
	{
		if (!strcmp (entities[i].classname, "light"))
		{
			// don't test torches, flames and globes
			// they already have their own light
			TestSingleLightFace (&entities[i], &l, faceoffset);
		}
		else if (!strncmp (entities[i].classname, "light_fluor", 11))
		{
			// test fluoros as well
			TestSingleLightFace (&entities[i], &l, faceoffset);
		}
	}
}

