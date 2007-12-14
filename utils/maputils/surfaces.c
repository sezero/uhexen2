/*
	surfaces.c (divide.h)
	$Id: surfaces.c,v 1.9 2007-12-14 16:41:28 sezero Exp $
*/

#include "q_stdinc.h"
#include "compiler.h"
#include "arch_def.h"
#include "cmdlib.h"
#include "mathlib.h"
#include "bspfile.h"
#include "bsp5.h"


/*
a surface has all of the faces that could be drawn on a given plane

the outside filling stage can remove some of them so a better bsp can
be generated

*/

static int	subdivides;

#if 0
/*
===============
SubdivideFace

If the face is >256 in either texture direction, carve a valid sized
piece off and insert the remainder in the next link
===============
*/
void SubdivideFace (face_t *f, face_t **prevptr)
{
	vec3_t		mins, maxs;
	double		v;
	int			i, j;
	plane_t		plane;
	face_t		*front, *back, *next;
	float		size[3];

// special (non-surface cached) faces don't need subdivision
	if ( texinfo[f->texturenum].flags & TEX_SPECIAL)
		return;

	do
	{
		mins[0] = mins[1] = mins[2] = 9999;
		maxs[0] = maxs[1] = maxs[2] = -9999;

		for (i = 0 ; i < f->numpoints ; i++)
		{
			for (j = 0 ; j < 3 ; j++)
			{
				v = f->pts[i][j];
				if (v < mins[j])
					mins[j] = v;
				if (v > maxs[j])
					maxs[j] = v;
			}
		}

		for (i = 0 ; i < 3 ; i++)
			size[i] = maxs[i] - mins[i];

#	if 0
		if ( !size[0] && size[1] <= 496 && size[2] <= 496 && size[1]*size[2]<0x10000)
			return;
		if ( !size[1] && size[0] <= 496 && size[2] <= 496 && size[0]*size[2]<0x10000)
			return;
		if ( !size[2] && size[0] <= 496 && size[1] <= 496 && size[0]*size[1]<0x10000)
			return;
#	endif

		for (i = 0 ; i < 3 ; i++)
		{
			v = maxs[i] - mins[i];
			if (v <= 240)	// NOT 256, because of 16 pixel edge crossings
				continue;

		// split it
			subdivides++;

			plane.normal[0] = plane.normal[1] = plane.normal[2] = 0;
			plane.normal[i] = 1;
			plane.dist = maxs[i] - 224;	// not 240, because of epsilons
			next = f->next;
			SplitFace (f, &plane, &front, &back);
			if (!front || !back)
				Error ("%s: didn't split the polygon", __thisfunc__);
			*prevptr = front;
			front->next = back;
			back->next = next;
			f = front;
			break;
		}

	} while (i < 3);
}
#endif

#if 1

/*
===============
SubdivideFace

If the face is >256 in either texture direction, carve a valid sized
piece off and insert the remainder in the next link
===============
*/
void SubdivideFace (face_t *f, face_t **prevptr)
{
	float		mins, maxs;
	double		v;
	int			axis, i;
	plane_t		plane;
	face_t		*front, *back, *next;
	texinfo_t	*tex;

// special (non-surface cached) faces don't need subdivision
	tex = &texinfo[f->texturenum];

	if ( tex->flags & TEX_SPECIAL)
		return;

	for (axis = 0 ; axis < 2 ; axis++)
	{
		while (1)
		{
			mins = 9999;
			maxs = -9999;

			for (i = 0 ; i < f->numpoints ; i++)
			{
				v = DotProduct (f->pts[i], tex->vecs[axis]);
				if (v < mins)
					mins = v;
				if (v > maxs)
					maxs = v;
			}

			if (maxs - mins <= 240)
				break;

		// split it
			subdivides++;

			VectorCopy (tex->vecs[axis], plane.normal);
			v = VectorLength (plane.normal);
			VectorNormalize (plane.normal);
			plane.dist = (mins + 224)/v;
			next = f->next;
			SplitFace (f, &plane, &front, &back);
			if (!front || !back)
				Error ("%s: didn't split the polygon", __thisfunc__);
			*prevptr = back;
			back->next = front;
			front->next = next;
			f = back;
		}
	}
}
#endif


/*
================
SubdivideFaces
================
*/
// actually, this has no users.
void SubdivideFaces (surface_t *surfhead)
{
	surface_t	*surf;
	face_t		*f , **prevptr;

	qprintf ("--- SubdivideFaces ---\n");

	subdivides = 0;

	for (surf = surfhead ; surf ; surf = surf->next)
	{
		prevptr = &surf->faces;
		while (1)
		{
			f = *prevptr;
			if (!f)
				break;
			SubdivideFace (f, prevptr);
			f = *prevptr;
			prevptr = &f->next;
		}
	}

	qprintf ("%i faces added by subdivision\n", subdivides);
}


/*
=============================================================================

GatherNodeFaces

Frees the current node tree and returns a new chain of the surfaces that
have inside faces.
=============================================================================
*/
static void GatherNodeFaces_r (node_t *node)
{
	face_t	*f, *next;

	if (node->planenum != PLANENUM_LEAF)
	{
//
// decision node
//
		for (f = node->faces ; f ; f = next)
		{
			next = f->next;
			if (!f->numpoints)
			{	// face was removed outside
				FreeFace (f);
			}
			else
			{
				f->next = validfaces[f->planenum];
				validfaces[f->planenum] = f;
			}
		}

		GatherNodeFaces_r (node->children[0]);
		GatherNodeFaces_r (node->children[1]);

		free (node);
	}
	else
	{
//
// leaf node
//
		free (node);
	}
}

/*
================
GatherNodeFaces

================
*/
surface_t *GatherNodeFaces (node_t *headnode)
{
	memset (validfaces, 0, sizeof(validfaces));
	GatherNodeFaces_r (headnode);
	return BuildSurfaces ();
}

//===========================================================================

typedef struct hashvert_s
{
	struct hashvert_s	*next;
	vec3_t	point;
	int		num;
	int		numplanes;		// for corner determination
	int		planenums[2];
	int		numedges;
} hashvert_t;

#define	POINT_EPSILON	0.01

static hashvert_t	hvertex[MAX_MAP_VERTS];
static hashvert_t	*hvert_p;

int		c_cornerverts;
face_t		*edgefaces[MAX_MAP_EDGES][2];
int		firstmodeledge = 1;
int		firstmodelface;

//============================================================================

#define	NUM_HASH	4096

static	hashvert_t	*hashverts[NUM_HASH];
static	vec3_t	hash_min, hash_scale;

static void InitHash (void)
{
	vec3_t	size;
	double	volume;
	double	scale;
	int		newsize[2];
	int		i;

	memset (hashverts, 0, sizeof(hashverts));

	for (i = 0 ; i < 3 ; i++)
	{
		hash_min[i] = -8000;
		size[i] = 16000;
	}

	volume = size[0]*size[1];

	scale = sqrt(volume / NUM_HASH);

	newsize[0] = size[0] / scale;
	newsize[1] = size[1] / scale;

	hash_scale[0] = newsize[0] / size[0];
	hash_scale[1] = newsize[1] / size[1];
	hash_scale[2] = newsize[1];

	hvert_p = hvertex;
}

static unsigned int HashVec (vec3_t vec)
{
	unsigned int	h;

	h = hash_scale[0] * (vec[0] - hash_min[0]) * hash_scale[2]
			+ hash_scale[1] * (vec[1] - hash_min[1]);
	if ( h >= NUM_HASH)
		return NUM_HASH - 1;
	return h;
}


/*
=============
GetVertex
=============
*/
static int GetVertex (vec3_t in, int planenum)
{
	int			h;
	int			i;
	hashvert_t	*hv;
	vec3_t		vert;

	for (i = 0 ; i < 3 ; i++)
	{
		if ( fabs(in[i] - Q_rint(in[i])) < 0.001)
			vert[i] = Q_rint(in[i]);
		else
			vert[i] = in[i];
	}

	h = HashVec (vert);

	for (hv = hashverts[h] ; hv ; hv = hv->next)
	{
		if ( fabs(hv->point[0]-vert[0]) < POINT_EPSILON
			&& fabs(hv->point[1]-vert[1]) < POINT_EPSILON
			&& fabs(hv->point[2]-vert[2]) < POINT_EPSILON )
		{
			hv->numedges++;
			if (hv->numplanes == 3)
				return hv->num;		// already known to be a corner
			for (i = 0 ; i < hv->numplanes ; i++)
			{
				if (hv->planenums[i] == planenum)
					return hv->num;	// already know this plane
			}
			if (hv->numplanes == 2)
				c_cornerverts++;
			else
				hv->planenums[hv->numplanes] = planenum;
			hv->numplanes++;
			return hv->num;
		}
	}

	hv = hvert_p;
	hv->numedges = 1;
	hv->numplanes = 1;
	hv->planenums[0] = planenum;
	hv->next = hashverts[h];
	hashverts[h] = hv;
	VectorCopy (vert, hv->point);
	hv->num = numvertexes;
	if (hv->num == MAX_MAP_VERTS)
		Error ("%s: MAX_MAP_VERTS", __thisfunc__);
	hvert_p++;

// emit a vertex
	if (numvertexes == MAX_MAP_VERTS)
		Error ("numvertexes == MAX_MAP_VERTS");

	dvertexes[numvertexes].point[0] = vert[0];
	dvertexes[numvertexes].point[1] = vert[1];
	dvertexes[numvertexes].point[2] = vert[2];
	numvertexes++;

	return hv->num;
}

//===========================================================================


/*
==================
GetEdge

Don't allow four way edges
==================
*/
int	c_tryedges;

static int GetEdge (vec3_t p1, vec3_t p2, face_t *f)
{
	int		v1, v2;
	dedge_t	*edge;
	int		i;

	if (!f->contents[0])
		Error ("%s: 0 contents", __thisfunc__);

	c_tryedges++;
	v1 = GetVertex (p1, f->planenum);
	v2 = GetVertex (p2, f->planenum);
	for (i = firstmodeledge ; i < numedges ; i++)
	{
		edge = &dedges[i];
		if (v1 == edge->v[1] && v2 == edge->v[0]
			&& !edgefaces[i][1]
			&& edgefaces[i][0]->contents[0] == f->contents[0])
		{
			edgefaces[i][1] = f;
			return -i;
		}
	}

// emit an edge
	if (numedges == MAX_MAP_EDGES)
		Error ("numedges == MAX_MAP_EDGES");
	edge = &dedges[numedges];
	numedges++;
	edge->v[0] = v1;
	edge->v[1] = v2;
	edgefaces[i][0] = f;

	return i;
}


/*
==================
FindFaceEdges
==================
*/
static void FindFaceEdges (face_t *face)
{
	int		i;

	face->outputnumber = -1;
	if (face->numpoints > MAXEDGES)
		Error ("%s: %i points", __thisfunc__, face->numpoints);

	for (i = 0; i < face->numpoints ; i++)
		face->edges[i] = GetEdge (face->pts[i], face->pts[(i+1)%face->numpoints], face);
}

/*
=============
CheckVertexes
// debugging
=============
*/
#if 0	// all uses are commented out
static void CheckVertexes (void)
{
	int		cb, c0, c1, c2, c3;
	hashvert_t	*hv;

	cb = c0 = c1 = c2 = c3 = 0;
	for (hv = hvertex ; hv != hvert_p ; hv++)
	{
		if (hv->numedges < 0 || hv->numedges & 1)
			cb++;
		else if (!hv->numedges)
			c0++;
		else if (hv->numedges == 2)
			c1++;
		else if (hv->numedges == 4)
			c2++;
		else
			c3++;
	}

	qprintf ("%5i bad edge points\n", cb);
	qprintf ("%5i 0 edge points\n", c0);
	qprintf ("%5i 2 edge points\n", c1);
	qprintf ("%5i 4 edge points\n", c2);
	qprintf ("%5i 6+ edge points\n", c3);
}

/*
=============
CheckEdges
// debugging
=============
*/
static void CheckEdges (void)
{
	dedge_t	*edge;
	int		i;
	dvertex_t	*d1, *d2;
	face_t		*f1, *f2;
	int		c_nonconvex;
	int		c_multitexture;

	c_nonconvex = c_multitexture = 0;

//	CheckVertexes ();

	for (i = 1 ; i < numedges ; i++)
	{
		edge = &dedges[i];
		if (!edgefaces[i][1])
		{
			d1 = &dvertexes[edge->v[0]];
			d2 = &dvertexes[edge->v[1]];
			qprintf ("unshared edge at: (%8.2f, %8.2f, %8.2f) (%8.2f, %8.2f, %8.2f)\n",d1->point[0], d1->point[1], d1->point[2], d2->point[0], d2->point[1], d2->point[2]); 
		}
		else
		{
			f1 = edgefaces[i][0];
			f2 = edgefaces[i][1];
			if (f1->planeside != f2->planeside)
				continue;
			if (f1->planenum != f2->planenum)
				continue;

			// on the same plane, might be discardable
			if (f1->texturenum == f2->texturenum)
			{
				hvertex[edge->v[0]].numedges-=2;
				hvertex[edge->v[1]].numedges-=2;
				c_nonconvex++;
			}
			else
				c_multitexture++;
		}
	}

//	qprintf ("%5i edges\n", i);
//	qprintf ("%5i c_nonconvex\n", c_nonconvex);
//	qprintf ("%5i c_multitexture\n", c_multitexture);

//	CheckVertexes ();
}
#endif


/*
================
MakeFaceEdges_r
================
*/
static void MakeFaceEdges_r (node_t *node)
{
	face_t	*f;

	if (node->planenum == PLANENUM_LEAF)
		return;

	for (f = node->faces ; f ; f = f->next)
		FindFaceEdges (f);

	MakeFaceEdges_r (node->children[0]);
	MakeFaceEdges_r (node->children[1]);
}

/*
================
MakeFaceEdges
================
*/
void MakeFaceEdges (node_t *headnode)
{
	qprintf ("----- MakeFaceEdges -----\n");

	InitHash ();
	c_tryedges = 0;
	c_cornerverts = 0;

	MakeFaceEdges_r (headnode);

//	CheckEdges ();

	GrowNodeRegions (headnode);

	firstmodeledge = numedges;
	firstmodelface = numfaces;
}

