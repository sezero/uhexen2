/*  Copyright (C) 1996-1997  Id Software, Inc.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

    See file, 'COPYING', for details.
*/

/*
	trace.c
	$Id: trace.c,v 1.8 2007-07-08 17:01:16 sezero Exp $

	Modifications by Kevin Shanahan, 1999-2000
*/

#include "util_inc.h"
#include "cmdlib.h"
#include "mathlib.h"
#include "bspfile.h"
#include "entities.h"
#include "threads.h"
#include "tyrlite.h"

typedef struct tnode_s
{
	int		type;
	vec3_t	normal;
	float	dist;
	int		children[2];
	int		pad;
} tnode_t;

static tnode_t		*tnodes, *tnode_p;

/*
==============
MakeTnode

Converts the disk node structure into the efficient tracing structure
==============
*/
static void MakeTnode (int nodenum)
{
	tnode_t			*t;
	dplane_t		*plane;
	int				i;
	dnode_t 		*node;

	t = tnode_p++;

	node = dnodes + nodenum;
	plane = dplanes + node->planenum;

	t->type = plane->type;
	VectorCopy (plane->normal, t->normal);
	t->dist = plane->dist;

	for (i = 0 ; i < 2 ; i++)
	{
		if (node->children[i] < 0)
			t->children[i] = dleafs[-node->children[i] - 1].contents;
		else
		{
			t->children[i] = tnode_p - tnodes;
			MakeTnode (node->children[i]);
		}
	}
}


/*
=============
MakeTnodes

Loads the node structure out of a .bsp file to be used for light occlusion
=============
*/
void MakeTnodes (dmodel_t *bm)
{
	tnode_p = tnodes = (tnode_t *) malloc(numnodes * sizeof(tnode_t));

	MakeTnode (0);
}


/*
==============================================================================

LINE TRACING

The major lighting operation is a point to point visibility test, performed
by recursive subdivision of the line by the BSP tree.

==============================================================================
*/

typedef struct
{
	vec3_t	backpt;
	int		side;
	int		node;
} tracestack_t;


/*
==============
TestLineOrSky
TYR - modified TestLine (a bit of a hack job...)
==============
*/
//qboolean TestLine (vec3_t start, vec3_t stop)
static qboolean TestLineOrSky (vec3_t start, vec3_t stop, qboolean sky_test)
{
	int			node, side;
/*	With vec_t defined as double, these front and back stuff need to
	be vec_t as well:  While creating LIT files for demo2 or castle4,
	the gcc compiled program segfaults under either of these conditions:
	- the program is not compiled with the -ffast-math flag.
	- compiled with gcc v4 for arches >= pentiumpro: i686, pentium2,
	or pentium3 without the -msse -mfpmath=sse extra flags.		*/ 
//	float		front, back;
//	float		frontx, fronty, frontz, backx, backy, backz;
	vec_t		front, back;
	vec_t		frontx, fronty, frontz, backx, backy, backz;
	tracestack_t	*tstack_p;
	tracestack_t	tracestack[64];
	tnode_t			*tnode;

	frontx = start[0];
	fronty = start[1];
	frontz = start[2];
	backx = stop[0];
	backy = stop[1];
	backz = stop[2];

	tstack_p = tracestack;
	node = 0;

	while (1)
	{
		while (node < 0 && node != CONTENTS_SOLID && (node != CONTENTS_SKY || !sky_test))
		{
		// pop up the stack for a back side

		// we can modify this to check if a vector hits a light casting
		// node before hitting a one that doesn't, and include sky as a
		// potential light casting node. in order to do this, we need
		// to find a way of identifying a texture that may be on a node
			tstack_p--;
			if (tstack_p < tracestack) /* if sky_test is...			*/
				return !sky_test;  /*	true => We didn't hit sky	*/
					/*	false => no solid obstructions		*/
			node = tstack_p->node;

		// set the hit point for this plane

			frontx = backx;
			fronty = backy;
			frontz = backz;

		// go down the back side

			backx = tstack_p->backpt[0];
			backy = tstack_p->backpt[1];
			backz = tstack_p->backpt[2];

			node = tnodes[tstack_p->node].children[!tstack_p->side];
		}

		if (node == CONTENTS_SOLID)
			return false;	// DONE!
		else if (node == CONTENTS_SKY && sky_test)
			return true;	// DONE!

		tnode = &tnodes[node];

		switch (tnode->type)
		{
		case PLANE_X:
			front = frontx - tnode->dist;
			back = backx - tnode->dist;
			break;
		case PLANE_Y:
			front = fronty - tnode->dist;
			back = backy - tnode->dist;
			break;
		case PLANE_Z:
			front = frontz - tnode->dist;
			back = backz - tnode->dist;
			break;
		default:
			front = (frontx*tnode->normal[0] + fronty*tnode->normal[1]
							 + frontz*tnode->normal[2]) - tnode->dist;
			back = (backx*tnode->normal[0] + backy*tnode->normal[1]
							 + backz*tnode->normal[2]) - tnode->dist;
			break;
		}

	//	if (front > 0 && back > 0)
		if (front > -ON_EPSILON && back > -ON_EPSILON)
		{
			node = tnode->children[0];
			continue;
		}

	//	if (front <= 0 && back <= 0)
		if (front < ON_EPSILON && back < ON_EPSILON)
		{
			node = tnode->children[1];
			continue;
		}

	//	side = front < 0;
		side = (front < 0.0f) ? 1 : 0;

	//	front = front / (front-back);
		front /= (front - back);

		tstack_p->node = node;
		tstack_p->side = side;
		tstack_p->backpt[0] = backx;
		tstack_p->backpt[1] = backy;
		tstack_p->backpt[2] = backz;

		tstack_p++;

		backx = frontx + front*(backx-frontx);
		backy = fronty + front*(backy-fronty);
		backz = frontz + front*(backz-frontz);

		node = tnode->children[side];
	}
}

/*
================
TestSky  -- TYR

Returns true if the ray cast from point 'start' in the
direction of vector 'dirn' hits a CONTENTS_SKY node before
a CONTENTS_SOLID node.
this is major buggy - we really should be testing in a number of
directions - up, down, right, left, in, out, and various variations
in between...

Wrapper functions for testing LOS between two points (TestLine)
and testing LOS to a sky brush along a direction vector (TestSky)
================
*/
qboolean TestLine(vec3_t start, vec3_t stop)
{
	return TestLineOrSky(start, stop, false);
}

qboolean TestSky (vec3_t start, vec3_t dirn)
{
	vec3_t	stop;

	VectorAdd(dirn, start, stop);
	return TestLineOrSky(start, stop, true);
}

