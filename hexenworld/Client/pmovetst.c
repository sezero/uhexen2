/*
	pmovetst.c
	player movement testing

	$Id: pmovetst.c,v 1.11 2007-05-13 16:14:53 sezero Exp $
*/

#include "quakedef.h"

static	hull_t		box_hull;
static	dclipnode_t	box_clipnodes[6];
static	mplane_t	box_planes[6];

extern	vec3_t player_mins;
extern	vec3_t player_maxs;
extern	vec3_t player_maxs_crouch;
extern	vec3_t beast_mins;
extern	vec3_t beast_maxs;


/*
===================
PM_InitBoxHull

Set up the planes and clipnodes so that the six floats of a bounding box
can just be stored out and get a proper hull_t structure.
===================
*/
void PM_InitBoxHull (void)
{
	int		i;
	int		side;

	box_hull.clipnodes = box_clipnodes;
	box_hull.planes = box_planes;
	box_hull.firstclipnode = 0;
	box_hull.lastclipnode = 5;

	for (i = 0; i < 6; i++)
	{
		box_clipnodes[i].planenum = i;

		side = i & 1;

		box_clipnodes[i].children[side] = CONTENTS_EMPTY;
		if (i != 5)
			box_clipnodes[i].children[side^1] = i + 1;
		else
			box_clipnodes[i].children[side^1] = CONTENTS_SOLID;

		box_planes[i].type = i>>1;
		box_planes[i].normal[i>>1] = 1;
	}
}


/*
===================
PM_HullForBox

To keep everything totally uniform, bounding boxes are turned into small
BSP trees instead of being compared directly.
===================
*/
static hull_t	*PM_HullForBox (vec3_t mins, vec3_t maxs)
{
	box_planes[0].dist = maxs[0];
	box_planes[1].dist = mins[0];
	box_planes[2].dist = maxs[1];
	box_planes[3].dist = mins[1];
	box_planes[4].dist = maxs[2];
	box_planes[5].dist = mins[2];

	return &box_hull;
}


/*
==================
PM_HullPointContents

==================
*/
int PM_HullPointContents (hull_t *hull, int num, vec3_t p)
{
	float		d;
	dclipnode_t	*node;
	mplane_t	*plane;

	while (num >= 0)
	{
		if (num < hull->firstclipnode || num > hull->lastclipnode)
			Sys_Error ("%s: bad node number", __thisfunc__);

		node = hull->clipnodes + num;
		plane = hull->planes + node->planenum;

		if (plane->type < 3)
			d = p[plane->type] - plane->dist;
		else
			d = DotProduct (plane->normal, p) - plane->dist;
		if (d < 0)
			num = node->children[1];
		else
			num = node->children[0];
	}

	return num;
}

/*
==================
PM_PointContents

==================
*/
int PM_PointContents (vec3_t p)
{
	float		d;
	dclipnode_t	*node;
	mplane_t	*plane;
	hull_t		*hull;
	int			num;

	hull = &pmove.physents[0].model->hulls[0];

	num = hull->firstclipnode;

	while (num >= 0)
	{
		if (num < hull->firstclipnode || num > hull->lastclipnode)
			Sys_Error ("%s: bad node number", __thisfunc__);

		node = hull->clipnodes + num;
		plane = hull->planes + node->planenum;

		if (plane->type < 3)
			d = p[plane->type] - plane->dist;
		else
			d = DotProduct (plane->normal, p) - plane->dist;
		if (d < 0)
			num = node->children[1];
		else
			num = node->children[0];
	}

	return num;
}

/*
===============================================================================

LINE TESTING IN HULLS

===============================================================================
*/

// 1/32 epsilon to keep floating point happy
#define	DIST_EPSILON	(0.03125)

/*
==================
PM_RecursiveHullCheck

==================
*/
static qboolean PM_RecursiveHullCheck (hull_t *hull, int num, float p1f, float p2f, vec3_t p1, vec3_t p2, pmtrace_t *trace)
{
	dclipnode_t	*node;
	mplane_t	*plane;
	float		t1, t2;
	float		frac;
	int			i;
	vec3_t		mid;
	int			side;
	float		midf;

// check for empty
	if (num < 0)
	{
		if (num != CONTENTS_SOLID)
		{
			trace->allsolid = false;
			if (num == CONTENTS_EMPTY)
				trace->inopen = true;
			else
				trace->inwater = true;
		}
		else
			trace->startsolid = true;
		return true;		// empty
	}

	if (num < hull->firstclipnode || num > hull->lastclipnode)
		Sys_Error ("%s: bad node number", __thisfunc__);

//
// find the point distances
//
	node = hull->clipnodes + num;
	plane = hull->planes + node->planenum;

	if (plane->type < 3)
	{
		t1 = p1[plane->type] - plane->dist;
		t2 = p2[plane->type] - plane->dist;
	}
	else
	{
		t1 = DotProduct (plane->normal, p1) - plane->dist;
		t2 = DotProduct (plane->normal, p2) - plane->dist;
	}

#if 1
	if (t1 >= 0 && t2 >= 0)
		return PM_RecursiveHullCheck (hull, node->children[0], p1f, p2f, p1, p2, trace);
	if (t1 < 0 && t2 < 0)
		return PM_RecursiveHullCheck (hull, node->children[1], p1f, p2f, p1, p2, trace);
#else
	if ( (t1 >= DIST_EPSILON && t2 >= DIST_EPSILON) || (t2 > t1 && t1 >= 0) )
		return PM_RecursiveHullCheck (hull, node->children[0], p1f, p2f, p1, p2, trace);
	if ( (t1 <= -DIST_EPSILON && t2 <= -DIST_EPSILON) || (t2 < t1 && t1 <= 0) )
		return PM_RecursiveHullCheck (hull, node->children[1], p1f, p2f, p1, p2, trace);
#endif

// put the crosspoint DIST_EPSILON pixels on the near side
	if (t1 < 0)
		frac = (t1 + DIST_EPSILON)/(t1-t2);
	else
		frac = (t1 - DIST_EPSILON)/(t1-t2);
	if (frac < 0)
		frac = 0;
	if (frac > 1)
		frac = 1;

	midf = p1f + (p2f - p1f)*frac;
	for (i = 0; i < 3; i++)
		mid[i] = p1[i] + frac*(p2[i] - p1[i]);

	side = (t1 < 0);

// move up to the node
	if (!PM_RecursiveHullCheck (hull, node->children[side], p1f, midf, p1, mid, trace) )
		return false;

#ifdef PARANOID
	if (PM_HullPointContents (pm_hullmodel, mid, node->children[side]) == CONTENTS_SOLID)
	{
		Con_Printf ("mid PointInHullSolid\n");
		return false;
	}
#endif

	if (PM_HullPointContents (hull, node->children[side^1], mid) != CONTENTS_SOLID)
		// go past the node
		return PM_RecursiveHullCheck (hull, node->children[side^1], midf, p2f, mid, p2, trace);

	if (trace->allsolid)
		return false;		// never got out of the solid area

//==================
// the other side of the node is solid, this is the impact point
//==================
	if (!side)
	{
		VectorCopy (plane->normal, trace->plane.normal);
		trace->plane.dist = plane->dist;
	}
	else
	{
		VectorNegate (plane->normal, trace->plane.normal);
		trace->plane.dist = -plane->dist;
	}

	while (PM_HullPointContents (hull, hull->firstclipnode, mid) == CONTENTS_SOLID)
	{ // shouldn't really happen, but does occasionally
		frac -= 0.1;
		if (frac < 0)
		{
			trace->fraction = midf;
			VectorCopy (mid, trace->endpos);
			Con_DPrintf ("backup past 0\n");
			return false;
		}
		midf = p1f + (p2f - p1f)*frac;
		for (i = 0; i < 3; i++)
			mid[i] = p1[i] + frac*(p2[i] - p1[i]);
	}

	trace->fraction = midf;
	VectorCopy (mid, trace->endpos);

	return false;
}


/*
================
PM_TestPlayerPosition

Returns false if the given player position is not valid (in solid)
================
*/
qboolean PM_TestPlayerPosition (vec3_t pos)
{
#if 0	/* FIXME: Unreachable code. See also: NudgePosition(). Compare QW and HW codes someday. */
	int			i;
	physent_t	*pe;
	vec3_t		mins, maxs, test;
	hull_t		*hull;
#endif	/* Unreachable code */
	pmtrace_t	trace;

	trace = PM_PlayerMove (pos, pos);
	if (trace.allsolid || trace.startsolid)
	{
		return false;
	}

	return true;

#if 0	/* Unreachable code: see above. */
	for (i = 0; i < pmove.numphysent; i++)
	{
		pe = &pmove.physents[i];
	// get the clipping hull
		if (0){}/*shitbox
			 pmove.hasted == 1.666) //hacky- beast speed
		{
			VectorCopy (beast_maxs, maxs);
			VectorCopy (beast_mins, mins);
			hull = &pmove.physents[i].model->hulls[5];
		}*/
		else if (pe->model)
		{
			if (pmove.crouched)
			{
				hull = &pmove.physents[i].model->hulls[3];
			}
			else
			{
				hull = &pmove.physents[i].model->hulls[1];
			}
		}
		else
		{
			if (pmove.crouched)
			{
				VectorSubtract (pe->mins, player_maxs_crouch, mins);
			}
			else
			{
				VectorSubtract (pe->mins, player_maxs, mins);
			}
			VectorSubtract (pe->maxs, player_mins, maxs);
			hull = PM_HullForBox (mins, maxs);
		}

		VectorSubtract (pos, pe->origin, test);

	// rjr will need to adjust for player when going into different hulls
		test[2] -= hull->clip_mins[2];

		if (PM_HullPointContents (hull, hull->firstclipnode, test) == CONTENTS_SOLID)
			return false;
	}

	return true;
#endif	/* Unreachable code */
}


/*
================
PM_PlayerMove
================
*/
pmtrace_t PM_PlayerMove (vec3_t start, vec3_t end)
{
	pmtrace_t	trace, total;
	vec3_t		offset;
	vec3_t		start_l, end_l;
	hull_t		*hull;
	int			i;
	physent_t	*pe;
	vec3_t		mins, maxs;

// fill in a default trace
	memset (&total, 0, sizeof(pmtrace_t));
	total.fraction = 1;
	total.ent = -1;
	VectorCopy (end, total.endpos);

	for (i = 0; i < pmove.numphysent; i++)
	{
		pe = &pmove.physents[i];
	// get the clipping hull
		if (0){}/*shitbox
			 pmove.hasted == 1.666) //hacky- beast speed
		{
			VectorCopy (beast_maxs, maxs);
			VectorCopy (beast_mins, mins);
			hull = &pmove.physents[i].model->hulls[5];
		}*/
		else if (pe->model)
		{
			if (pmove.crouched)
			{
				hull = &pmove.physents[i].model->hulls[3];
			}
			else
			{
				hull = &pmove.physents[i].model->hulls[1];
			}
		}
		else
		{
			if (pmove.crouched)
			{
				VectorSubtract (pe->mins, player_maxs_crouch, mins);
			}
			else
			{
				VectorSubtract (pe->mins, player_maxs, mins);
			}
			VectorSubtract (pe->maxs, player_mins, maxs);
			hull = PM_HullForBox (mins, maxs);
		}

	// PM_HullForEntity (ent, mins, maxs, offset);
		VectorCopy (pe->origin, offset);

		VectorSubtract (start, offset, start_l);
		VectorSubtract (end, offset, end_l);

		if (pe->model && (fabs(pe->angles[0]) > 1 || fabs(pe->angles[1]) > 1 || fabs(pe->angles[2]) > 1) )
		{
			vec3_t	forward, right, up;
			vec3_t	temp;

			AngleVectors (pe->angles, forward, right, up);

			VectorCopy (start_l, temp);
			start_l[0] = DotProduct (temp, forward);
			start_l[1] = -DotProduct (temp, right);
			start_l[2] = DotProduct (temp, up);

			VectorCopy (end_l, temp);
			end_l[0] = DotProduct (temp, forward);
			end_l[1] = -DotProduct (temp, right);
			end_l[2] = DotProduct (temp, up);
		}

	// rjr will need to adjust for player when going into different hulls
		start_l[2] -= hull->clip_mins[2];
		end_l[2] -= hull->clip_mins[2];

	// fill in a default trace
		memset (&trace, 0, sizeof(pmtrace_t));
		trace.fraction = 1;
		trace.allsolid = true;
		//trace.startsolid = true;
		VectorCopy (end, trace.endpos);
	// rjr will need to adjust for player when going into different hulls
		trace.endpos[2] -= hull->clip_mins[2];

	// trace a line through the apropriate clipping hull
		PM_RecursiveHullCheck (hull, hull->firstclipnode, 0, 1, start_l, end_l, &trace);

	// rjr will need to adjust for player when going into different hulls
		trace.endpos[2] += hull->clip_mins[2];

		if (pe->model && (fabs(pe->angles[0]) > 1 || fabs(pe->angles[1]) > 1 || fabs(pe->angles[2]) > 1) )
		{
			vec3_t	a;
			vec3_t	forward, right, up;
			vec3_t	temp;

			if (trace.fraction != 1)
			{
				VectorNegate (pe->angles, a);
				AngleVectors (a, forward, right, up);

				VectorCopy (trace.endpos, temp);
				trace.endpos[0] = DotProduct (temp, forward);
				trace.endpos[1] = -DotProduct (temp, right);
				trace.endpos[2] = DotProduct (temp, up);

				VectorCopy (trace.plane.normal, temp);
				trace.plane.normal[0] = DotProduct (temp, forward);
				trace.plane.normal[1] = -DotProduct (temp, right);
				trace.plane.normal[2] = DotProduct (temp, up);
			}
		}

		if (trace.allsolid)
			trace.startsolid = true;
		if (trace.startsolid)
			trace.fraction = 0;

	// did we clip the move?
		if (trace.fraction < total.fraction)
		{
			// fix trace up by the offset
			VectorAdd (trace.endpos, offset, trace.endpos);
			total = trace;
			total.ent = i;
		}
	}

	return total;
}


