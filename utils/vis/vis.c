/*
 * vis.c
 * $Id: vis.c,v 1.18 2009-05-12 14:23:13 sezero Exp $
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
#include "q_endian.h"
#include "byteordr.h"
#include "pathutil.h"
#include "mathlib.h"
#include "bspfile.h"
#include "threads.h"
#include "vis.h"


int			GilMode = 1;
int			numportals, portalleafs;
int			c_portaltest, c_portalpass, c_portalcheck;

portal_t		*portals;
leaf_t			*leafs;

int			leafon;		// the next leaf to be given
					// to a thread to process

static byte	*vismap, *vismap_p, *vismap_end;        // past visfile
static int		originalvismapsize;

byte		*uncompressed;			// [bitbytes*portalleafs]

int			bitbytes;		// (portalleafs+63)>>3
int			bitlongs;

static qboolean		fastvis;
qboolean		verbose;
int			testlevel = 2;


#if 0
void NormalizePlane (plane_t *dp)
{
	double	ax, ay, az;

	if (dp->normal[0] == -1.0)
	{
		dp->normal[0] = 1.0;
		dp->dist = -dp->dist;
		return;
	}
	if (dp->normal[1] == -1.0)
	{
		dp->normal[1] = 1.0;
		dp->dist = -dp->dist;
		return;
	}
	if (dp->normal[2] == -1.0)
	{
		dp->normal[2] = 1.0;
		dp->dist = -dp->dist;
		return;
	}

	ax = fabs(dp->normal[0]);
	ay = fabs(dp->normal[1]);
	az = fabs(dp->normal[2]);

	if (ax >= ay && ax >= az)
	{
		if (dp->normal[0] < 0)
		{
			VectorNegate (dp->normal, dp->normal);
			dp->dist = -dp->dist;
		}
		return;
	}

	if (ay >= ax && ay >= az)
	{
		if (dp->normal[1] < 0)
		{
			VectorNegate (dp->normal, dp->normal);
			dp->dist = -dp->dist;
		}
		return;
	}

	if (dp->normal[2] < 0)
	{
		VectorNegate (dp->normal, dp->normal);
		dp->dist = -dp->dist;
	}
}
#endif

static void PlaneFromWinding (winding_t *w, plane_t *plane)
{
	vec3_t		v1, v2;

// calc plane
	VectorSubtract (w->points[2], w->points[1], v1);
	VectorSubtract (w->points[0], w->points[1], v2);
	CrossProduct (v2, v1, plane->normal);
	VectorNormalize (plane->normal);
	plane->dist = (float) DotProduct (w->points[0], plane->normal);
}

//=============================================================================

/*
==================
NewWinding
==================
*/

winding_t *NewWinding (int points)
{
	winding_t	*w;
	size_t			size;

	if (points > MAX_POINTS_ON_WINDING)
		COM_Error ("%s: %i points", __thisfunc__, points);

	size = (size_t)((winding_t *)0)->points[points];
	w = (winding_t *) SafeMalloc (size);

	return w;
}

void FreeWinding (winding_t *w)
{
	if (!w->original)
		free (w);
}


#if 0	/* not used */
void pw (winding_t *w)
{
	int			i;

	for (i = 0 ; i < w->numpoints ; i++)
		printf ("(%5.1f, %5.1f, %5.1f)\n",w->points[i][0], w->points[i][1],w->points[i][2]);
}

void prl(leaf_t *l)
{
	int			i;
	portal_t	*p;
	plane_t		pl;

	for (i = 0 ; i < l->numportals ; i++)
	{
		p = l->portals[i];
		pl = p->plane;
		printf ("portal %4i to leaf %4i : %7.1f : (%4.1f, %4.1f, %4.1f)\n",(int)(p-portals),p->leaf,pl.dist, pl.normal[0], pl.normal[1], pl.normal[2]);
	}
}
#endif	/* not used */


/*
==================
CopyWinding
==================
*/
winding_t *CopyWinding (winding_t *w)
{
	size_t			size;
	winding_t	*c;

	size = (size_t)((winding_t *)0)->points[w->numpoints];

//	c = (winding_t *) SafeMalloc (size);
	c = NewWinding(w->numpoints);
	memcpy (c, w, size);
	c->original = false;
	return c;
}


/*
==================
ClipWinding

Clips the winding to the plane, returning the new winding on the positive side
Frees the input winding.
If keepon is true, an exactly on-plane winding will be saved, otherwise
it will be clipped away.
==================
*/
winding_t *ClipWinding (winding_t *in, plane_t *split, qboolean keepon)
{
	double	dists[MAX_POINTS_ON_WINDING];
	int		sides[MAX_POINTS_ON_WINDING];
	int		counts[3];
	double	dot;
	int		i, j;
	double	*p1, *p2;
	vec3_t	mid;
	winding_t	*neww;
	int		maxpts;

	counts[0] = counts[1] = counts[2] = 0;

// determine sides for each point
	sides[0] = SIDE_ON; dists[0] = 0; // make static analyzers happy
	for (i = 0 ; i < in->numpoints ; i++)
	{
		dot = DotProduct (in->points[i], split->normal);
		dot -= split->dist;
		dists[i] = dot;
		if (dot > ON_EPSILON)
			sides[i] = SIDE_FRONT;
		else if (dot < -ON_EPSILON)
			sides[i] = SIDE_BACK;
		else
		{
			sides[i] = SIDE_ON;
		}
		counts[sides[i]]++;
	}
	sides[i] = sides[0];
	dists[i] = dists[0];

	if (keepon && !counts[0] && !counts[1])
		return in;

	if (!counts[0])
	{
		FreeWinding (in);
		return NULL;
	}
	if (!counts[1])
		return in;

	maxpts = in->numpoints + 4;	// can't use counts[0]+2 because
					// of fp grouping errors
	neww = NewWinding (maxpts);

	for (i = 0 ; i < in->numpoints ; i++)
	{
		p1 = in->points[i];

		if (sides[i] == SIDE_ON)
		{
			VectorCopy (p1, neww->points[neww->numpoints]);
			neww->numpoints++;
			continue;
		}

		if (sides[i] == SIDE_FRONT)
		{
			VectorCopy (p1, neww->points[neww->numpoints]);
			neww->numpoints++;
		}

		if (sides[i+1] == SIDE_ON || sides[i+1] == sides[i])
			continue;

	// generate a split point
		p2 = in->points[(i+1)%in->numpoints];

		dot = dists[i] / (dists[i]-dists[i+1]);
		for (j = 0 ; j < 3 ; j++)
		{	// avoid round off error when possible
			if (split->normal[j] == 1)
				mid[j] = split->dist;
			else if (split->normal[j] == -1)
				mid[j] = -split->dist;
			else
				mid[j] = p1[j] + dot*(p2[j]-p1[j]);
		}

		VectorCopy (mid, neww->points[neww->numpoints]);
		neww->numpoints++;
	}

	if (neww->numpoints > maxpts)
		COM_Error ("%s: points exceeded estimate", __thisfunc__);

// free the original winding
	FreeWinding (in);

	return neww;
}


//=============================================================================

/*
=============
GetNextPortal

Returns the next portal for a thread to work on
Returns the portals from the least complex, so the later ones can reuse
the earlier information.
=============
*/
static portal_t *GetNextPortal (void)
{
	int		j;
	portal_t	*p, *tp;
	int		minsee;
	const int	num2 = numportals * 2;

	ThreadLock();

	minsee = 99999;
	p = NULL;

	for (j = 0, tp = portals ; j < num2 ; j++, tp++)
	{
		if (tp->nummightsee < minsee && tp->status == stat_none)
		{
			minsee = tp->nummightsee;
			p = tp;
		}
	}

	if (p)
		p->status = stat_working;

	ThreadUnlock();

	return p;
}

/*
==============
LeafThread
==============
*/
static	void	LeafThread (void *junk)
{
	portal_t	*p;

	printf ("Begining %s: %i\n", __thisfunc__, (int)(intptr_t)junk);
	do
	{
		p = GetNextPortal ();
		if (!p)
			break;

		PortalFlow (p);
		if (verbose) {
			printf ("portal:%4i  mightsee:%4i  cansee:%4i\n",
				(int)(p - portals), p->nummightsee, p->numcansee);
		}
	} while (1);

	printf ("Completed %s: %i\n", __thisfunc__, (int)(intptr_t)junk);
}

/*
===============
CompressRow

===============
*/
static int CompressRow (byte *vis, byte *dest)
{
	int		j;
	int		rep;
	int		visrow;
	byte	*dest_p;

	dest_p = dest;
	visrow = (portalleafs + 7)>>3;

	for (j = 0 ; j < visrow ; j++)
	{
		*dest_p++ = vis[j];
		if (vis[j])
			continue;

		rep = 1;
		for (j++; j < visrow ; j++)
		{
			if (vis[j] || rep == 255)
				break;
			else
				rep++;
		}

		*dest_p++ = rep;
		j--;
	}

	return dest_p - dest;
}


/*
===============
LeafFlow

Builds the entire visibility list for a leaf
===============
*/
static int		totalvis;

static void LeafFlow (int leafnum)
{
	leaf_t		*leaf;
	byte		*outbuffer;
	byte		compressed[MAX_MAP_LEAFS/8];
	int			i, j;
	int			numvis;
	byte		*dest;
	portal_t	*p;

//
// flow through all portals, collecting visible bits
//
	outbuffer = uncompressed + leafnum*bitbytes;
	leaf = &leafs[leafnum];
	for (i = 0 ; i < leaf->numportals ; i++)
	{
		p = leaf->portals[i];
		if (p->status != stat_done)
			COM_Error ("portal not done");
		for (j = 0 ; j < bitbytes ; j++)
			outbuffer[j] |= p->visbits[j];
	}

	if (outbuffer[leafnum>>3] & (1<<(leafnum&7)))
		COM_Error ("Leaf portals saw into leaf");

	outbuffer[leafnum>>3] |= (1<<(leafnum&7));

	numvis = 0;
	for (i = 0 ; i < portalleafs ; i++)
	{
		if ( outbuffer[i>>3] & (1<<(i & 3)) )
			numvis++;
	}

//
// compress the bit string
//
	if (verbose)
		printf ("leaf %4i : %4i visible\n", leafnum, numvis);
	totalvis += numvis;

#if 0
	i = (portalleafs + 7) >> 3;
	memcpy (compressed, outbuffer, i);
#else
	i = CompressRow (outbuffer, compressed);
#endif

	dest = vismap_p;
	vismap_p += i;

	if (vismap_p > vismap_end)
		COM_Error ("Vismap expansion overflow");

	dleafs[leafnum+1].visofs = dest-vismap;	// leaf 0 is a common solid

	memcpy (dest, compressed, i);
}


/*
==================
CalcPortalVis
==================
*/
static void CalcPortalVis (void)
{
	int		i;

// fastvis just uses mightsee for a very loose bound
	if (fastvis)
	{
		for (i = 0 ; i < numportals*2 ; i++)
		{
			portals[i].visbits = portals[i].mightsee;
			portals[i].status = stat_done;
		}
		return;
	}

	leafon = 0;

	RunThreadsOn (LeafThread);

	if (verbose)
	{
		printf ("portalcheck: %i  portaltest: %i  portalpass: %i\n",c_portalcheck, c_portaltest, c_portalpass);
		printf ("c_vistest: %i  c_mighttest: %i\n",c_vistest, c_mighttest);
	}
}


/*
==================
CalcVis
==================
*/
static void CalcVis (void)
{
	int		i;

	BasePortalVis ();

	CalcPortalVis ();

//
// assemble the leaf vis lists by oring and compressing the portal lists
//
	for (i = 0 ; i < portalleafs ; i++)
		LeafFlow (i);

	printf ("average leafs visible: %i\n", totalvis / portalleafs);
}


/*
==============================================================================

PASSAGE CALCULATION (not used yet...)

==============================================================================
*/
#if 0
int		count_sep;

qboolean PlaneCompare (plane_t *p1, plane_t *p2)
{
	int		i;

	if ( fabs(p1->dist - p2->dist) > 0.01)
		return false;

	for (i = 0 ; i < 3 ; i++)
	{
		if ( fabs(p1->normal[i] - p2->normal[i] ) > 0.001)
			return false;
	}

	return true;
}

sep_t *Findpassages (winding_t *source, winding_t *pass)
{
	int			i, j, k, l;
	plane_t		plane;
	vec3_t		v1, v2;
	float		d;
	double		length;
	int			counts[3];
	qboolean		fliptest;
	sep_t		*sep, *list;

	list = NULL;

// check all combinations
	for (i = 0 ; i < source->numpoints ; i++)
	{
		l = (i+1)%source->numpoints;
		VectorSubtract (source->points[l] , source->points[i], v1);

	// fing a vertex of pass that makes a plane that puts all of the
	// vertexes of pass on the front side and all of the vertexes of
	// source on the back side
		for (j = 0 ; j < pass->numpoints ; j++)
		{
			VectorSubtract (pass->points[j], source->points[i], v2);

			plane.normal[0] = v1[1]*v2[2] - v1[2]*v2[1];
			plane.normal[1] = v1[2]*v2[0] - v1[0]*v2[2];
			plane.normal[2] = v1[0]*v2[1] - v1[1]*v2[0];

		// if points don't make a valid plane, skip it

			length = plane.normal[0] * plane.normal[0]
					+ plane.normal[1] * plane.normal[1]
					+ plane.normal[2] * plane.normal[2];

			if (length < ON_EPSILON)
				continue;

			length = 1 / sqrt(length);

			plane.normal[0] *= length;
			plane.normal[1] *= length;
			plane.normal[2] *= length;

			plane.dist = (float) DotProduct (pass->points[j], plane.normal);

		//
		// find out which side of the generated seperating plane has the
		// source portal
		//
			fliptest = false;
			for (k = 0 ; k < source->numpoints ; k++)
			{
				if (k == i || k == l)
					continue;
				d = (float) DotProduct (source->points[k], plane.normal) - plane.dist;
				if (d < -ON_EPSILON)
				{	// source is on the negative side, so we want all
					// pass and target on the positive side
					fliptest = false;
					break;
				}
				else if (d > ON_EPSILON)
				{	// source is on the positive side, so we want all
					// pass and target on the negative side
					fliptest = true;
					break;
				}
			}
			if (k == source->numpoints)
				continue;	// planar with source portal

		//
		// flip the normal if the source portal is backwards
		//
			if (fliptest)
			{
				VectorNegate (plane.normal, plane.normal);
				plane.dist = -plane.dist;
			}

		//
		// if all of the pass portal points are now on the positive side,
		// this is the seperating plane
		//
			counts[0] = counts[1] = counts[2] = 0;
			for (k = 0 ; k < pass->numpoints ; k++)
			{
				if (k == j)
					continue;
				d = (float) DotProduct (pass->points[k], plane.normal) - plane.dist;
				if (d < -ON_EPSILON)
					break;
				else if (d > ON_EPSILON)
					counts[0]++;
				else
					counts[2]++;
			}
			if (k != pass->numpoints)
				continue;	// points on negative side, not a seperating plane

			if (!counts[0])
				continue;	// planar with pass portal

		//
		// save this out
		//
			count_sep++;

			sep = (sep_t *) SafeMalloc(sizeof(*sep));
			sep->next = list;
			list = sep;
			sep->plane = plane;
		}
	}

	return list;
}
#endif

//=============================================================================

/*
============
LoadPortals
============
*/
static void LoadPortals (char *name)
{
	int			i, j;
	portal_t	*p;
	leaf_t		*l;
	char		magic[80];
	FILE		*f;
	int			numpoints;
	winding_t	*w;
	int			leafnums[2];
	plane_t		plane;

	if (!strcmp(name,"-"))
		f = stdin;
	else
	{
		f = fopen(name, "r");
		if (!f)
		{
			printf ("%s: couldn't read %s\n", __thisfunc__, name);
			printf ("No vising performed.\n");
			exit (1);
		}
	}

	if (fscanf (f,"%79s\n%i\n%i\n",magic, &portalleafs, &numportals) != 3)
		COM_Error ("%s: failed to read header", __thisfunc__);
	if (strcmp(magic,PORTALFILE))
		COM_Error ("%s: not a portal file", __thisfunc__);

	printf ("%4i portalleafs\n", portalleafs);
	printf ("%4i numportals\n", numportals);

	bitbytes = ((portalleafs + 63) & ~63) >> 3;
	bitlongs = bitbytes / sizeof(long);

// each file portal is split into two memory portals
	portals = (portal_t *) SafeMalloc(2 * numportals * sizeof(portal_t));
	leafs = (leaf_t *) SafeMalloc(portalleafs * sizeof(leaf_t));

	originalvismapsize = portalleafs*((portalleafs+7)/8);

	vismap = vismap_p = dvisdata;
	vismap_end = vismap + MAX_MAP_VISIBILITY;

	for (i = 0, p = portals ; i < numportals ; i++)
	{
		if (fscanf (f, "%i %i %i ", &numpoints, &leafnums[0], &leafnums[1]) != 3)
			COM_Error ("%s: Error reading portal %i", __thisfunc__, i);
		if (numpoints > MAX_POINTS_ON_WINDING)
			COM_Error ("%s: portal %i has too many points", __thisfunc__, i);
		if ( leafnums[0] > portalleafs || leafnums[1] > portalleafs)
			COM_Error ("%s: portal %i, leafnums > portalleafs", __thisfunc__, i);

		w = p->winding = NewWinding (numpoints);
		w->original = true;
		w->numpoints = numpoints;

		for (j = 0 ; j < numpoints ; j++)
		{
			if (fscanf (f, "(%lf %lf %lf ) ",
					&w->points[j][0], &w->points[j][1], &w->points[j][2]) != 3)
				COM_Error ("%s: reading portal %i", __thisfunc__, i);
		}
		fscanf (f, "\n");

	// calc plane
		PlaneFromWinding (w, &plane);

	// create forward portal
		l = &leafs[leafnums[0]];
		if (l->numportals == MAX_PORTALS_ON_LEAF)
			COM_Error ("Leaf with too many portals");
		l->portals[l->numportals] = p;
		l->numportals++;

		p->winding = w;
		VectorNegate (plane.normal, p->plane.normal);
		p->plane.dist = -plane.dist;
		p->leaf = leafnums[1];
		p++;

	// create backwards portal
		l = &leafs[leafnums[1]];
		if (l->numportals == MAX_PORTALS_ON_LEAF)
			COM_Error ("Leaf with too many portals");
		l->portals[l->numportals] = p;
		l->numportals++;

		p->winding = w;
		p->plane = plane;
		p->leaf = leafnums[0];
		p++;
	}

	fclose (f);
}


/*
===========
main
===========
*/
int main (int argc, char **argv)
{
	char	portalfile[1024];
	char	source[1024];
	int		i;
	int		wantthreads;
	double	start, end;

	printf ("---- vis ----\n");

	ValidateByteorder ();

	wantthreads = 1;	// default to single threaded

	for (i = 1 ; i < argc ; i++)
	{
		if (!strcmp(argv[i],"-threads"))
		{
			if (i >= argc - 1)
				COM_Error("Missing argument to \"%s\"", argv[i]);
			wantthreads = atoi (argv[++i]);
		}
		else if (!strcmp(argv[i], "-nogil"))
		{
			printf ("Gil's optimizations disabled\n");
			GilMode = 0;
		}
		else if (!strcmp(argv[i], "-fast"))
		{
			printf ("fastvis = true\n");
			fastvis = true;
		}
		else if (!strcmp(argv[i], "-level"))
		{
			if (i >= argc - 1)
				COM_Error("Missing argument to \"%s\"", argv[i]);
			testlevel = atoi(argv[++i]);
			printf ("testlevel = %i\n", testlevel);
		}
		else if (!strcmp(argv[i], "-v"))
		{
			printf ("verbose = true\n");
			verbose = true;
		}
		else if (argv[i][0] == '-')
			COM_Error ("Unknown option \"%s\"", argv[i]);
		else
			break;
	}

	if (i != argc - 1)
		COM_Error ("usage: vis [-nogil] [-threads #] [-level 0-4] [-fast] [-v] bspfile");

	InitThreads (wantthreads, 0);

	start = COM_GetTime ();

	strcpy (source, argv[i]);
	StripExtension (source);
	DefaultExtension (source, ".bsp", sizeof(source));

	LoadBSPFile (source);

	strcpy (portalfile, argv[i]);
	StripExtension (portalfile);
	strcat (portalfile, ".prt");

	LoadPortals (portalfile);

	uncompressed = (byte *) SafeMalloc(bitbytes*portalleafs);

	CalcVis ();

	printf ("c_chains: %i\n", c_chains);

	visdatasize = vismap_p - dvisdata;
	printf ("visdatasize:%i  compressed from %i\n", visdatasize, originalvismapsize);

	CalcAmbientSounds ();

	WriteBSPFile (source);

//	Q_unlink (portalfile);
	if (GilMode)
		PrintStats();
	end = COM_GetTime ();
	printf ("%5.1f seconds elapsed\n", end-start);

	return 0;
}

