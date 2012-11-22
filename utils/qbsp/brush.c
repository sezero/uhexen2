/*
 * brush.c
 * $Id: brush.c,v 1.11 2007-12-14 16:41:23 sezero Exp $
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

#include "q_stdinc.h"
#include "compiler.h"
#include "arch_def.h"
#include "cmdlib.h"
#include "mathlib.h"
#include "bspfile.h"
#include "bsp5.h"

int			numbrushplanes;
plane_t		planes[MAX_MAP_PLANES];

int			numbrushfaces;
mface_t		faces[128];		// beveled clipping hull can generate many extra


/*
=================
CheckFace

Note: this will not catch 0 area polygons
=================
*/
static void CheckFace (face_t *f)
{
	int		i, j;
	double	*p1, *p2;
	double	d, edgedist;
	vec3_t	dir, edgenormal, facenormal;

	if (f->numpoints < 3)
		COM_Error ("%s: %i points", __thisfunc__, f->numpoints);

	VectorCopy (planes[f->planenum].normal, facenormal);
	if (f->planeside)
	{
		VectorNegate (facenormal, facenormal);
	}

	for (i = 0 ; i < f->numpoints ; i++)
	{
		p1 = f->pts[i];

		for (j = 0 ; j < 3 ; j++)
			if (p1[j] > BOGUS_RANGE || p1[j] < -BOGUS_RANGE)
				COM_Error ("%s: BUGUS_RANGE: %f", __thisfunc__, p1[j]);

		j = (i+1 == f->numpoints) ? 0 : i+1;

	// check the point is on the face plane
/*		d = DotProduct (p1, planes[f->planenum].normal) - planes[f->planenum].dist;
		if (d < -ON_EPSILON || d > ON_EPSILON)
		//	printf ("%s: point #%i off plane\n", __thisfunc__, i);
			COM_Error ("%s: point #%i off plane", __thisfunc__, i);
*/

	// check the edge isn't degenerate
		p2 = f->pts[j];
		VectorSubtract (p2, p1, dir);

		if (VectorLength (dir) < ON_EPSILON)
			COM_Error ("%s: degenerate edge", __thisfunc__);

		CrossProduct (facenormal, dir, edgenormal);
		VectorNormalize (edgenormal);
		edgedist = DotProduct (p1, edgenormal);
		edgedist += ON_EPSILON;

	// all other points must be on front side
		for (j = 0 ; j < f->numpoints ; j++)
		{
			if (j == i)
				continue;
			d = DotProduct (f->pts[j], edgenormal);
			if (d > edgedist)
				COM_Error ("%s: non-convex", __thisfunc__);
		}
	}
}


//===========================================================================

/*
=================
ClearBounds
=================
*/
void ClearBounds (brushset_t *bs)
{
	int		i, j;

	for (j = 0 ; j < NUM_HULLS ; j++)
	{
		for (i = 0 ; i < 3 ; i++)
		{
			bs->mins[i] = 99999;
			bs->maxs[i] = -99999;
		}
	}
}

/*
=================
AddToBounds
=================
*/
void AddToBounds (brushset_t *bs, vec3_t v)
{
	int		i;

	for (i = 0 ; i < 3 ; i++)
	{
		if (v[i] < bs->mins[i])
			bs->mins[i] = v[i];
		if (v[i] > bs->maxs[i])
			bs->maxs[i] = v[i];
	}
}

//===========================================================================

int PlaneTypeForNormal (vec3_t normal)
{
	float	ax, ay, az;

// NOTE: should these have an epsilon around 1.0?
	if (normal[0] == 1.0)
		return PLANE_X;
	if (normal[1] == 1.0)
		return PLANE_Y;
	if (normal[2] == 1.0)
		return PLANE_Z;
	if (normal[0] == -1.0 || normal[1] == -1.0 || normal[2] == -1.0)
		COM_Error ("%s: not a canonical vector", __thisfunc__);

	ax = fabs(normal[0]);
	ay = fabs(normal[1]);
	az = fabs(normal[2]);

	if (ax >= ay && ax >= az)
		return PLANE_ANYX;
	if (ay >= ax && ay >= az)
		return PLANE_ANYY;
	return PLANE_ANYZ;
}

#define	DISTEPSILON		0.01
#define	ANGLEEPSILON		0.00001

static void NormalizePlane (plane_t *dp)
{
	double	ax, ay, az;

	if (dp->normal[0] == -1.0)
	{
		dp->normal[0] = 1.0;
		dp->dist = -dp->dist;
	}
	if (dp->normal[1] == -1.0)
	{
		dp->normal[1] = 1.0;
		dp->dist = -dp->dist;
	}
	if (dp->normal[2] == -1.0)
	{
		dp->normal[2] = 1.0;
		dp->dist = -dp->dist;
	}

	if (dp->normal[0] == 1.0)
	{
		dp->type = PLANE_X;
		return;
	}
	if (dp->normal[1] == 1.0)
	{
		dp->type = PLANE_Y;
		return;
	}
	if (dp->normal[2] == 1.0)
	{
		dp->type = PLANE_Z;
		return;
	}

	ax = fabs(dp->normal[0]);
	ay = fabs(dp->normal[1]);
	az = fabs(dp->normal[2]);

	if (ax >= ay && ax >= az)
		dp->type = PLANE_ANYX;
	else if (ay >= ax && ay >= az)
		dp->type = PLANE_ANYY;
	else
		dp->type = PLANE_ANYZ;
	if (dp->normal[dp->type-PLANE_ANYX] < 0)
	{
		VectorNegate (dp->normal, dp->normal);
		dp->dist = -dp->dist;
	}
}

/*
===============
FindPlane

Returns a global plane number and the side that will be the front
===============
*/
int FindPlane (plane_t *dplane, int *side)
{
	int			i;
	plane_t		*dp, pl;
	double		dot;

	dot = VectorLength(dplane->normal);
	if (dot < 1.0 - ANGLEEPSILON || dot > 1.0 + ANGLEEPSILON)
		COM_Error ("%s: normalization error", __thisfunc__);

	pl = *dplane;
	NormalizePlane (&pl);
	if (DotProduct(pl.normal, dplane->normal) > 0)
		*side = 0;
	else
		*side = 1;

	dp = planes;
	for (i = 0 ; i < numbrushplanes ; i++, dp++)
	{
		dot = DotProduct (dp->normal, pl.normal);
		if (dot > 1.0 - ANGLEEPSILON && fabs(dp->dist - pl.dist) < DISTEPSILON)
		{	// regular match
			return i;
		}
	}

	if (numbrushplanes == MAX_MAP_PLANES)
		COM_Error ("numbrushplanes == MAX_MAP_PLANES");

	planes[numbrushplanes] = pl;

	numbrushplanes++;

	return numbrushplanes-1;
}


/*
===============
FindPlane_old

Returns a global plane number and the side that will be the front
===============
*/
#if 0
int FindPlane_old (plane_t *dplane, int *side)
{
	int			i;
	plane_t		*dp;
	double		dot, ax, ay, az;

	dot = VectorLength(dplane->normal);
	if (dot < 1.0 - ANGLEEPSILON || dot > 1.0 + ANGLEEPSILON)
		COM_Error ("%s: normalization error", __thisfunc__);

	dp = planes;

	for (i = 0 ; i < numbrushplanes ; i++, dp++)
	{
		dot = DotProduct (dplane->normal, dp->normal);
		if (dot > 1.0 - ANGLEEPSILON && fabs(dplane->dist - dp->dist) < DISTEPSILON)
		{	// regular match
			*side = 0;
			return i;
		}
		if (dot < -1.0+ANGLEEPSILON && fabs(dplane->dist + dp->dist) < DISTEPSILON)
		{	// inverse of vector
			*side = 1;
			return i;
		}
	}

// allocate a new plane, flipping normal to a consistant direction
// if needed
	*dp = *dplane;

	if (numbrushplanes == MAX_MAP_PLANES)
		COM_Error ("numbrushplanes == MAX_MAP_PLANES");
	numbrushplanes++;

	*side = 0;

// NOTE: should these have an epsilon around 1.0?
	if (dplane->normal[0] == 1.0)
		dp->type = PLANE_X;
	else if (dplane->normal[1] == 1.0)
		dp->type = PLANE_Y;
	else if (dplane->normal[2] == 1.0)
		dp->type = PLANE_Z;
	else if (dplane->normal[0] == -1.0)
	{
		dp->type = PLANE_X;
		dp->normal[0] = 1.0;
		dp->dist = -dp->dist;
		*side = 1;
	}
	else if (dplane->normal[1] == -1.0)
	{
		dp->type = PLANE_Y;
		dp->normal[1] = 1.0;
		dp->dist = -dp->dist;
		*side = 1;
	}
	else if (dplane->normal[2] == -1.0)
	{
		dp->type = PLANE_Z;
		dp->normal[2] = 1.0;
		dp->dist = -dp->dist;
		*side = 1;
	}
	else
	{
		ax = fabs(dplane->normal[0]);
		ay = fabs(dplane->normal[1]);
		az = fabs(dplane->normal[2]);

		if (ax >= ay && ax >= az)
			dp->type = PLANE_ANYX;
		else if (ay >= ax && ay >= az)
			dp->type = PLANE_ANYY;
		else
			dp->type = PLANE_ANYZ;
		if (dplane->normal[dp->type-PLANE_ANYX] < 0)
		{
			VectorNegate (dp->normal, dp->normal);
			dp->dist = -dp->dist;
			*side = 1;
		}
	}

	return i;
}
#endif	// end of FindPlane_old


/*
=============================================================================

TURN BRUSHES INTO GROUPS OF FACES

=============================================================================
*/

static vec3_t	brush_mins, brush_maxs;
static face_t	*brush_faces;

/*
=================
CreateBrushFaces
=================
*/
#define	ZERO_EPSILON	0.001
static void CreateBrushFaces (void)
{
	int			i, j, k;
	double		r;
	face_t		*f;
	winding_t	*w;
	plane_t		plane;
	mface_t		*mf;

	brush_mins[0] = brush_mins[1] = brush_mins[2] = 99999;
	brush_maxs[0] = brush_maxs[1] = brush_maxs[2] = -99999;

	brush_faces = NULL;

	for (i = 0 ; i < numbrushfaces ; i++)
	{
		mf = &faces[i];

		w = BaseWindingForPlane (&mf->plane);

		for (j = 0 ; j < numbrushfaces && w ; j++)
		{
			if (j == i)
				continue;
		// flip the plane, because we want to keep the back side
			VectorNegate (faces[j].plane.normal, plane.normal);
			plane.dist = -faces[j].plane.dist;

			w = ClipWinding (w, &plane, false);
		}

		if (!w)
			continue;	// overcontrained plane

	// this face is a keeper
		f = AllocFace ();
		f->numpoints = w->numpoints;
		if (f->numpoints > MAXEDGES)
			COM_Error ("f->numpoints > MAXEDGES");

		for (j = 0 ; j < w->numpoints ; j++)
		{
			for (k = 0 ; k < 3 ; k++)
			{
				r = Q_rint (w->points[j][k]);
				if ( fabs(w->points[j][k] - r) < ZERO_EPSILON)
					f->pts[j][k] = r;
				else
					f->pts[j][k] = w->points[j][k];

				if (f->pts[j][k] < brush_mins[k])
					brush_mins[k] = f->pts[j][k];
				if (f->pts[j][k] > brush_maxs[k])
					brush_maxs[k] = f->pts[j][k];
			}
		}
		FreeWinding (w);
		f->texturenum = mf->texinfo;
		f->planenum = FindPlane (&mf->plane, &f->planeside);
		f->next = brush_faces;
		brush_faces = f;
		CheckFace (f);
	}
}


/*
==============================================================================

BEVELED CLIPPING HULL GENERATION

This is done by brute force, and could easily get a lot faster if anyone cares.
==============================================================================
*/

typedef vec3_t hull_size_t[2];

static const hull_size_t	hull_size_h2[6] = {	// original H2 sizes for #5 and #6
	{  {  0,    0,   0 },  {  0,    0,   0 }  },
	{  { -16, -16, -32 },  {  16,  16,  24 }  },
	{  { -24, -24, -20 },  {  24,  24,  20 }  },
	{  { -16, -16, -16 },  {  16,  16,  12 }  },
	{  { -40, -40, -42 },  {  40,  40,  42 }  },	// called hyrda, but not really
	{  { -48, -48, -50 },  {  48,  48,  50 }  }
};

static const hull_size_t	hull_size_mp[6] = {	// mission pack sizes for #5 and #6
	{  {  0,    0,   0 },  {  0,    0,   0 }  },
	{  { -16, -16, -32 },  {  16,  16,  24 }  },
	{  { -24, -24, -20 },  {  24,  24,  20 }  },
	{  { -16, -16, -16 },  {  16,  16,  12 }  },
	{  {  -8,  -8,  -8 },  {   8,   8,   8 }  },
	{  { -28, -28, -40 },  {  28,  28,  40 }  }
};

static const hull_size_t	*hull_size;


#define	MAX_HULL_POINTS	32
#define	MAX_HULL_EDGES	64

static int	num_hull_points;
static vec3_t	hull_points[MAX_HULL_POINTS];
static vec3_t	hull_corners[MAX_HULL_POINTS*8];
static int	num_hull_edges;
static int	hull_edges[MAX_HULL_EDGES][2];

/*
============
AddBrushPlane
=============
*/
static void AddBrushPlane (plane_t *plane)
{
	int		i;
	plane_t	*pl;
	float	l;

	if (numbrushfaces == MAX_FACES)
		printf ("%s: numbrushfaces == MAX_FACES\n", __thisfunc__);
//		COM_Error ("%s: numbrushfaces == MAX_FACES", __thisfunc__);

	l = VectorLength (plane->normal);
	if (l < 0.999 || l > 1.001)
		COM_Error ("%s: bad normal", __thisfunc__);

	for (i = 0 ; i < numbrushfaces ; i++)
	{
		pl = &faces[i].plane;
		if (VectorCompare (pl->normal, plane->normal) && fabs(pl->dist - plane->dist) < ON_EPSILON)
			return;
	}
	faces[i].plane = *plane;
	faces[i].texinfo = faces[0].texinfo;
	numbrushfaces++;
}


/*
============
TestAddPlane

Adds the given plane to the brush description if all of the original brush
vertexes can be put on the front side
=============
*/
static void TestAddPlane (plane_t *plane)
{
	int		i, c;
	double	d;
	double	*corner;
	plane_t	flip;
	vec3_t	inv;
	int		counts[3];
	plane_t	*pl;

// see if the plane has already been added
	for (i = 0 ; i < numbrushfaces ; i++)
	{
		pl = &faces[i].plane;
		if (VectorCompare (plane->normal, pl->normal) && fabs(plane->dist - pl->dist) < ON_EPSILON)
			return;
		VectorNegate (plane->normal, inv);
		if (VectorCompare (inv, pl->normal) && fabs(plane->dist + pl->dist) < ON_EPSILON)
			return;
	}

// check all the corner points
	counts[0] = counts[1] = counts[2] = 0;
	c = num_hull_points * 8;

	corner = hull_corners[0];
	for (i = 0 ; i < c ; i++, corner += 3)
	{
		d = DotProduct (corner, plane->normal) - plane->dist;
		if (d < -ON_EPSILON)
		{
			if (counts[0])
				return;
			counts[1]++;
		}
		else if (d > ON_EPSILON)
		{
			if (counts[1])
				return;
			counts[0]++;
		}
		else
			counts[2]++;
	}

// the plane is a seperator

	if (counts[0])
	{
		VectorNegate (plane->normal, flip.normal);
		flip.dist = -plane->dist;
		plane = &flip;
	}

	AddBrushPlane (plane);
}

/*
============
AddHullPoint

Doesn't add if duplicated
=============
*/
static int AddHullPoint (vec3_t p, int hullnumber)
{
	int		i;
	double	*c;
	int		x, y, z;

	for (i = 0 ; i < num_hull_points ; i++)
		if (VectorCompare (p, hull_points[i]))
			return i;

	VectorCopy (p, hull_points[num_hull_points]);

	c = hull_corners[i*8];

	for (x = 0 ; x < 2 ; x++)
	{
		for (y = 0 ; y < 2 ; y++)
		{
			for (z = 0 ; z < 2 ; z++)
			{
				c[0] = p[0] + hull_size[hullnumber][x][0];
				c[1] = p[1] + hull_size[hullnumber][y][1];
				c[2] = p[2] + hull_size[hullnumber][z][2];
				c += 3;
			}
		}
	}

	if (num_hull_points == MAX_HULL_POINTS)
		COM_Error ("MAX_HULL_POINTS");

	num_hull_points++;

	return i;
}


/*
============
AddHullEdge

Creates all of the hull planes around the given edge, if not done already
=============
*/
static void AddHullEdge (vec3_t p1, vec3_t p2, int hullnumber)
{
	int		pt1, pt2;
	int		i;
	int		a, b, c, d, e;
	vec3_t	edgevec, planeorg, planevec;
	plane_t	plane;
	double	l;

	pt1 = AddHullPoint (p1, hullnumber);
	pt2 = AddHullPoint (p2, hullnumber);

	for (i = 0 ; i < num_hull_edges ; i++)
	{
		if ( (hull_edges[i][0] == pt1 && hull_edges[i][1] == pt2)
				|| (hull_edges[i][0] == pt2 && hull_edges[i][1] == pt1) )
			return;	// already added
	}

	if (num_hull_edges == MAX_HULL_EDGES)
		COM_Error ("MAX_HULL_EDGES");

	hull_edges[i][0] = pt1;
	hull_edges[i][1] = pt2;
	num_hull_edges++;

	VectorSubtract (p1, p2, edgevec);
	VectorNormalize (edgevec);

	for (a = 0 ; a < 3 ; a++)
	{
		b = (a + 1) % 3;
		c = (a + 2) % 3;
		for (d = 0 ; d <= 1 ; d++)
		{
			for (e = 0 ; e <= 1 ; e++)
			{
				VectorCopy (p1, planeorg);
				planeorg[b] += hull_size[hullnumber][d][b];
				planeorg[c] += hull_size[hullnumber][e][c];

				VectorClear (planevec);
				planevec[a] = 1;

				CrossProduct (planevec, edgevec, plane.normal);
				l = VectorLength (plane.normal);
				if (l < 1-ANGLEEPSILON || l > 1+ANGLEEPSILON)
					continue;
				plane.dist = DotProduct (planeorg, plane.normal);
				TestAddPlane (&plane);
			}
		}
	}
}		


/*
============
ExpandBrush
=============
*/
static void ExpandBrush (int hullnumber)
{
	int		i, x, s;
	vec3_t	corner;
	face_t	*f;
	plane_t	plane, *p;

	num_hull_points = 0;
	num_hull_edges = 0;

// create all the hull points
	for (f = brush_faces ; f ; f = f->next)
	{
		for (i = 0 ; i < f->numpoints ; i++)
			AddHullPoint (f->pts[i], hullnumber);
	}

// expand all of the planes
	for (i = 0 ; i < numbrushfaces ; i++)
	{
		p = &faces[i].plane;
		VectorClear (corner);
		for (x = 0 ; x < 3 ; x++)
		{
			if (p->normal[x] > 0)
				corner[x] = hull_size[hullnumber][1][x];
			else if (p->normal[x] < 0)
				corner[x] = hull_size[hullnumber][0][x];
		}
		p->dist += DotProduct (corner, p->normal);
	}

// add any axis planes not contained in the brush to bevel off corners
	for (x = 0 ; x < 3 ; x++)
		for (s = -1 ; s <= 1 ; s+=2)
		{
		// add the plane
			VectorClear (plane.normal);
			plane.normal[x] = s;
			if (s == -1)
				plane.dist = -brush_mins[x] + -hull_size[hullnumber][0][x];
			else
				plane.dist = brush_maxs[x] + hull_size[hullnumber][1][x];
			AddBrushPlane (&plane);
		}

// add all of the edge bevels
	for (f = brush_faces ; f ; f = f->next)
	{
		for (i = 0 ; i < f->numpoints ; i++)
			AddHullEdge (f->pts[i], f->pts[(i+1)%f->numpoints], hullnumber);
	}
}

//============================================================================


/*
===============
LoadBrush

Converts a mapbrush to a bsp brush
===============
*/
static brush_t *LoadBrush (mbrush_t *mb, int hullnumber)
{
	brush_t		*b;
	int			contents;
	char		*name;
	mface_t		*f;
	face_t		*f2;

//
// check texture name for attributes
//
	name = miptex[texinfo[mb->faces->texinfo].miptex];

	if (!q_strcasecmp(name, "clip") && hullnumber == 0)
		return NULL;	// "clip" brushes don't show up in the draw hull

	if (name[0] == '*' && worldmodel)	// entities never use water merging
	{
		if (!q_strncasecmp(name+1,"lava",4))
			contents = CONTENTS_LAVA;
		else if (!q_strncasecmp(name+1,"slime",5))
			contents = CONTENTS_SLIME;
		else
			contents = CONTENTS_WATER;
	}
	else if (!q_strncasecmp (name, "sky",3) && worldmodel && hullnumber == 0)
		contents = CONTENTS_SKY;
	else
		contents = CONTENTS_SOLID;

	if (hullnumber && contents != CONTENTS_SOLID && contents != CONTENTS_SKY)
		return NULL;	// water brushes don't show up in clipping hulls

// no seperate textures on clip hull

//
// create the faces
//
	brush_faces = NULL;

	numbrushfaces = 0;
	for (f = mb->faces ; f ; f = f->next)
	{
		faces[numbrushfaces] = *f;
		if (hullnumber)
			faces[numbrushfaces].texinfo = 0;
		numbrushfaces++;
	}

	CreateBrushFaces ();

	if (!brush_faces)
	{
		printf ("WARNING: couldn't create brush faces\n");
		return NULL;
	}

	if (hullnumber)
	{
		ExpandBrush (hullnumber);
		CreateBrushFaces ();
	}

	for (f2 = brush_faces ; f2 ; f2 = f2->next)
	{
		f2->Light = mb->Light;
	}

//
// create the brush
//
	b = AllocBrush ();

	b->contents = contents;
	b->faces = brush_faces;
	VectorCopy (brush_mins, b->mins);
	VectorCopy (brush_maxs, b->maxs);

	return b;
}

//=============================================================================


/*
============
Brush_DrawAll

============
*/
static void Brush_DrawAll (brushset_t *bs)
{
	brush_t	*b;
	face_t	*f;

	for (b = bs->brushes ; b ; b = b->next)
	{
		for (f = b->faces ; f ; f = f->next)
			Draw_DrawFace (f);
	}
}


/*
============
Brush_LoadEntity
============
*/
brushset_t *Brush_LoadEntity (entity_t *ent, int hullnumber)
{
	brush_t		*b, *next, *water, *other;
	mbrush_t	*mbr;
	int			numbrushes;
	brushset_t	*bset;

	bset = (brushset_t *) SafeMalloc (sizeof(brushset_t));
	ClearBounds (bset);

	numbrushes = 0;
	other = water = NULL;

	qprintf ("--- Brush_LoadEntity ---\n");

	if (oldhullsize)
		hull_size = hull_size_h2;
	else	hull_size = hull_size_mp;

	for (mbr = ent->brushes ; mbr ; mbr = mbr->next)
	{
		b = LoadBrush (mbr, hullnumber);
		if (!b)
			continue;

		numbrushes++;

		if (b->contents != CONTENTS_SOLID)
		{
			b->next = water;
			water = b;
		}
		else
		{
			b->next = other;
			other = b;
		}

		AddToBounds (bset, b->mins);
		AddToBounds (bset, b->maxs);
	}

// add all of the water textures at the start
	for (b = water ; b ; b = next)
	{
		next = b->next;
		b->next = other;
		other = b;
	}

	bset->brushes = other;

	brushset = bset;
	Brush_DrawAll (bset);

	qprintf ("%i brushes read\n",numbrushes);

	return bset;
}

