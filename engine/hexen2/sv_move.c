/*
 * sv_move.c -- monster movement
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

#define	STEPSIZE	18

/*
=============
SV_CheckBottom

Returns false if any part of the bottom of the entity is off an edge that
is not a staircase.
=============
*/
qboolean SV_CheckBottom (edict_t *ent)
{
	// By this point, ent has been moved to its new position after the
	// move, and adjusted for steps
/*	qmodel_t	*model;
	hull_t	*wclip_hull;
	int	index;
	vec3_t	check, size; */
	vec3_t	mins, maxs, start, stop;
	trace_t	trace;
	int	x, y;
	float	mid, bottom;
	float	save_hull;

	VectorAdd (ent->v.origin, ent->v.mins, mins);
	VectorAdd (ent->v.origin, ent->v.maxs, maxs);

/*	// Make it use the clipping hull's size, not their bounding box...
	model = sv.models[ (int)sv.edicts->v.modelindex ];
	VectorSubtract (ent->v.maxs, ent->v.mins, size);
	if (ent->v.hull)
	{
		index = ent->v.hull-1;
		wclip_hull = &model->hulls[index];
		if (!wclip_hull) // Invalid hull
		{
			Con_Printf ("ERROR: hull %d is null.\n",wclip_hull);
			wclip_hull = &model->hulls[0];
		}
	}
	else  // Using the old way uses size to determine hull to use
	{
		if (size[0] < 3) // Point
			wclip_hull = &model->hulls[0];
		else if (size[0] <= 8)  // Pentacles
			wclip_hull = &model->hulls[4];
		else if (size[0] <= 32 && size[2] <= 28)  // Half Player
			wclip_hull = &model->hulls[3];
		else if (size[0] <= 32)  // Full Player
			wclip_hull = &model->hulls[1];
		else // Golumn
			wclip_hull = &model->hulls[5];
	}
	VectorAdd (ent->v.origin, wclip_hull->clip_mins, mins);
	VectorAdd (ent->v.origin, wclip_hull->clip_maxs, maxs);
*/
	// if all of the points under the corners are solid world, don't bother
	// with the tougher checks
	// the corners must be within 16 of the midpoint
	start[2] = mins[2] - 1;
	for (x = 0; x < 2; x++)
	{
		for (y = 0; y < 2; y++)
		{
		//	start[0] = x ? maxs[0] : mins[0];
		//	start[1] = y ? maxs[1] : mins[1];
			if (x)
				start[0] = maxs[0];
			else
				start[0] = mins[0];
			if (y)
				start[1] = maxs[1];
			else
				start[1] = mins[1];

			if (SV_PointContents (start) != CONTENTS_SOLID)
				goto realcheck;
		}
	}

	return true;	// we got out easy

realcheck:	// check it for real...

	start[2] = mins[2];

	// the midpoint must be within 16 of the bottom
	start[0] = stop[0] = (mins[0] + maxs[0]) * 0.5;
	start[1] = stop[1] = (mins[1] + maxs[1]) * 0.5;
	stop[2] = start[2] - 2*STEPSIZE;

	// do a trace from the bottom center of the ent down
	// to 36 below the bottom center of the ent, using a point hull
	// the "true" in this function is telling SV_Move to consider
	// this ent's movement as MOVE_NOMONSTERS - means it will
	// not clip against entities in this move
	// NOTE: these don't check for trace_allsolid of trace_startsolid.
	// Technically, these can't possibly be a valid result since
	// the start point is in the ent, but what about imprecision?

	save_hull = ent->v.hull;//temp hack so it HullForEntity doesn't calculate the wrong offset
	ent->v.hull = 0;
	trace = SV_Move (start, vec3_origin, vec3_origin, stop, true, ent);
	ent->v.hull = save_hull;

/*	if ((int)ent->v.flags & FL_MONSTER)
	{
		if (trace.allsolid)
			Con_DPrintf("Checkbottom midpoint check was all solid!!!\n");
		else if (trace.startsolid)
			Con_DPrintf("Checkbottom midpoint check started solid!!!\n");
	}
*/
	if (trace.fraction == 1.0)
		return false;

	// trace did not reach full 36 below, so set mid and bottom
	// to whatever distance it did get to below the ent's
	// bottom centerpoint (start[2])
	mid = bottom = trace.endpos[2];

	// the corners must be within 16 of the midpoint
	for (x = 0; x < 2; x++)
	{
		for (y = 0; y < 2; y++)
		{
			// check 4 corners, in this order:
			//	x = 0, y = 0 (NE)
			//	x = 0, y = 1 (SE)
			//	x = 1, y = 0 (NW)
			//	x = 1, y = 1 (SW)

		//	start[0] = stop[0] = x ? maxs[0] : mins[0];
		//	start[1] = stop[1] = y ? maxs[1] : mins[1];
			if (x)
				start[0] = stop[0] = maxs[0];
			else
				start[0] = stop[0] = mins[0];
			if (y)
				start[1] = stop[1] = maxs[1];
			else
				start[1] = stop[1] = mins[1];

			// same check as above, just from the 4 corners down 36
			save_hull = ent->v.hull;//temp hack so it HullForEntity doesn't calculate the wrong offset
			ent->v.hull = 0;
			trace = SV_Move (start, vec3_origin, vec3_origin, stop, true, ent);
			ent->v.hull = save_hull;

		/*	if ((int)ent->v.flags & FL_MONSTER)
			{
				if (trace.allsolid)
					Con_DPrintf("Checkbottom (x=%d,y=%d) check was all solid!!!\n",x,y);
				else if (trace.startsolid)
					Con_DPrintf("Checkbottom (x=%d,y=%d) check started solid!!!\n",x,y);
			}*/

			// Hit a closer surface than did when checked center,
			// so set the "bottom" to the new, closer z height
			// we hit
			if (trace.fraction != 1.0 && trace.endpos[2] > bottom)
				bottom = trace.endpos[2];

			// one of the corners does not have a surface within
			// 36 below it, or the surface it did hit is more than
			// 54 below this corner. This is a really stupid check,
			// because if it hits a surface more than 54 below the
			// ent,the trace_fraction will be 1
			if (trace.fraction == 1.0 || mid - trace.endpos[2] > STEPSIZE)
				return false;
		}
	}

	return true;
}


static void set_move_trace(trace_t *trace)
{
	*sv_globals.trace_allsolid = trace->allsolid;
	*sv_globals.trace_startsolid = trace->startsolid;
	*sv_globals.trace_fraction = trace->fraction;
	*sv_globals.trace_inwater = trace->inwater;
	*sv_globals.trace_inopen = trace->inopen;
	VectorCopy (trace->endpos, *sv_globals.trace_endpos);
	VectorCopy (trace->plane.normal, *sv_globals.trace_plane_normal);
	*sv_globals.trace_plane_dist =  trace->plane.dist;
	if (trace->ent)
		*sv_globals.trace_ent = EDICT_TO_PROG(trace->ent);
	else
		*sv_globals.trace_ent = EDICT_TO_PROG(sv.edicts);
}


/*
=============
SV_movestep

Called by monster program code.
The move will be adjusted for slopes and stairs, but if the move isn't
possible, no move is done, false is returned, and
*sv_globals.trace_normal is set to the normal of the blocking wall
=============
*/
qboolean SV_movestep (edict_t *ent, vec3_t move, qboolean relink, qboolean noenemy, qboolean set_trace)
{
	float		dz;
	vec3_t		oldorg, neworg, end;
	trace_t		trace;
	int			i;
	edict_t		*enemy = NULL;	// avoid compiler warning.

	// try the move
	VectorCopy (ent->v.origin, oldorg);
	VectorAdd (ent->v.origin, move, neworg);

	// flying monsters don't step up
	// unless no_z turned on
	if ( ((int)ent->v.flags & (FL_SWIM|FL_FLY))
		&& !((int)ent->v.flags & FL_HUNTFACE)
		&& !((int)ent->v.flags & FL_NOZ) )
	{
		// try one move with vertical motion, then one without
		for (i = 0; i < 2; i++)
		{
			VectorAdd (ent->v.origin, move, neworg);
			if (!noenemy)
			{
				enemy = PROG_TO_EDICT(ent->v.enemy);
				if (i == 0 && enemy != sv.edicts)
				{
					dz = ent->v.origin[2] - PROG_TO_EDICT(ent->v.enemy)->v.origin[2];
					// This is utterly broken: we already made sure
					// that FL_HUNTFACE is not set just above. -O.S
					if ((int)ent->v.flags & FL_HUNTFACE)//Go for face
						dz += PROG_TO_EDICT(ent->v.enemy)->v.view_ofs[2];

					if (dz > 40)
						neworg[2] -= 8;
					else if (dz < 30)
						neworg[2] += 8;
				}
			}

			if ( ((int)ent->v.flags & FL_SWIM) && SV_PointContents(neworg) == CONTENTS_EMPTY )
			{	//Would end up out of water, don't do z move
				neworg[2] = ent->v.origin[2];
				trace = SV_Move (ent->v.origin, ent->v.mins, ent->v.maxs, neworg, false, ent);
				if (set_trace)
					set_move_trace(&trace);
				if (trace.fraction < 1 || SV_PointContents(trace.endpos) == CONTENTS_EMPTY )
					return false;	// swim monster left water
			}
			else
			{
				trace = SV_Move (ent->v.origin, ent->v.mins, ent->v.maxs, neworg, false, ent);
				if (set_trace)
					set_move_trace(&trace);
			}

			if (trace.fraction == 1)
			{
				VectorCopy (trace.endpos, ent->v.origin);
				if (relink)
					SV_LinkEdict (ent, true);

				return true;
			}

			if (noenemy || enemy == sv.edicts)
				break;
		}

		return false;
	}

	// push down from a step height above the wished position
	neworg[2] += STEPSIZE;
	VectorCopy (neworg, end);
	end[2] -= STEPSIZE*2;

	trace = SV_Move (neworg, ent->v.mins, ent->v.maxs, end, false, ent);

	if (set_trace)
	{
		set_move_trace(&trace);
	}

	if (trace.allsolid)
	{
		return false;
	}

	if (trace.startsolid)
	{
		neworg[2] -= STEPSIZE;
		trace = SV_Move (neworg, ent->v.mins, ent->v.maxs, end, false, ent);
		if (set_trace)
		{
			set_move_trace(&trace);
		}
		if (trace.allsolid || trace.startsolid)
		{
			return false;
		}
	}
	if (trace.fraction == 1)
	{
		// if monster had the ground pulled out, go ahead and fall
		if ( (int)ent->v.flags & FL_PARTIALGROUND )
		{
			VectorAdd (ent->v.origin, move, ent->v.origin);
			if (relink)
				SV_LinkEdict (ent, true);
			ent->v.flags = (int)ent->v.flags & ~FL_ONGROUND;
		//	Con_Printf ("fall down\n");
			return true;
		}

		return false;	// walked off an edge
	}

	// check point traces down for dangling corners
	VectorCopy (trace.endpos, ent->v.origin);

	if (!SV_CheckBottom (ent))
	{
		if ( (int)ent->v.flags & FL_PARTIALGROUND )
		{	// entity had floor mostly pulled out from underneath it
			// and is trying to correct
			if (relink)
				SV_LinkEdict (ent, true);
			return true;
		}

		VectorCopy (oldorg, ent->v.origin);
		return false;
	}

	if ( (int)ent->v.flags & FL_PARTIALGROUND )
	{
	//	Con_Printf ("back on ground\n");
		ent->v.flags = (int)ent->v.flags & ~FL_PARTIALGROUND;
	}
	ent->v.groundentity = EDICT_TO_PROG(trace.ent);

	// the move is ok
	if (relink)
		SV_LinkEdict (ent, true);
	return true;
}


//============================================================================

/*
======================
SV_StepDirection

Turns to the movement direction, and walks the current distance if
facing it.
======================
*/
extern void PF_changeyaw (void);
static qboolean SV_StepDirection (edict_t *ent, float yaw, float dist)
{
	vec3_t		move, oldorigin;
	float		delta;
	qboolean	set_trace_plane;

	ent->v.ideal_yaw = yaw;
	PF_changeyaw();

	yaw = yaw * M_PI * 2 / 360;
	move[0] = cos(yaw) * dist;
	move[1] = sin(yaw) * dist;
	move[2] = 0;	//FIXME: Make wallcrawlers and flying monsters use this!

	VectorCopy (ent->v.origin, oldorigin);
	if ((int)ent->v.flags & FL_SET_TRACE)
		set_trace_plane = true;
	else
		set_trace_plane = false;

	if (SV_movestep (ent, move, false, false, set_trace_plane))
	{
		delta = ent->v.angles[YAW] - ent->v.ideal_yaw;
		if (delta > 45 && delta < 315)
		{	// not turned far enough, so don't take the step
			VectorCopy (oldorigin, ent->v.origin);
		}
		SV_LinkEdict (ent, true);
		return true;
	}
	SV_LinkEdict (ent, true);

	return false;
}


/*
======================
SV_FixCheckBottom

======================
*/
static void SV_FixCheckBottom (edict_t *ent)
{
//	Con_Printf ("SV_FixCheckBottom\n");
	ent->v.flags = (int)ent->v.flags | FL_PARTIALGROUND;
}


/*
================
SV_NewChaseDir

================
*/
#define	DI_NODIR	-1
static void SV_NewChaseDir (edict_t *actor, edict_t *enemy, float dist)
{
	float		deltax, deltay;
//	float		deltaz;
	float			d[3];
	float		tdir, olddir, turnaround;

	olddir = anglemod( (int)(actor->v.ideal_yaw / 45) * 45 );
	turnaround = anglemod(olddir - 180);

	deltax = enemy->v.origin[0] - actor->v.origin[0];
	deltay = enemy->v.origin[1] - actor->v.origin[1];

/*	if ((int)actor->v.flags & FL_FLY)	//Pentacles
		deltaz = enemy->v.origin[2] + enemy->v.view_ofs[2] - actor->v.origin[2];
	else
		deltaz = enemy->v.origin[2] - actor->v.origin[2];
*/
	if (deltax > 10)
		d[1] = 0;
	else if (deltax < -10)
		d[1] = 180;
	else
		d[1] = DI_NODIR;

	if (deltay < -10)
		d[2] = 270;
	else if (deltay > 10)
		d[2] = 90;
	else
		d[2] = DI_NODIR;

/*	if (deltaz < -10)//Below
		d[0] = ;
	else if (deltaz > 10)//above
		d[0] = ;
	else
		d[0] = DI_NODIR;
*/
	// try direct route
	if (d[1] != DI_NODIR && d[2] != DI_NODIR)
	{
		if (d[1] == 0)
			tdir = d[2] == 90 ? 45 : 315;
		else
			tdir = d[2] == 90 ? 135 : 215;

		if (tdir != turnaround && SV_StepDirection(actor, tdir, dist))
			return;
	}

	// try other directions
	if ( ((rand() & 3) & 1) || abs((int)deltay) > abs((int)deltax) )//If north/sounth diff is greater than east/west diff?!!
	{
		tdir = d[1];
		d[1] = d[2];
		d[2] = tdir;
	}

	if (d[1] != DI_NODIR && d[1] != turnaround && SV_StepDirection(actor, d[1], dist))
		return;

	if (d[2] != DI_NODIR && d[2] != turnaround && SV_StepDirection(actor, d[2], dist))
		return;

	/* there is no direct path to the player, so pick another direction */

	if (olddir != DI_NODIR && SV_StepDirection(actor, olddir, dist))
		return;

	if (rand() & 1)	/* randomly determine direction of search */
	{
		for (tdir = 0; tdir <= 315; tdir += 45)
		{
			if (tdir != turnaround && SV_StepDirection(actor, tdir, dist) )
				return;
		}
	}
	else
	{
		for (tdir = 315; tdir >= 0; tdir -= 45)
		{
			if (tdir != turnaround && SV_StepDirection(actor, tdir, dist) )
				return;
		}
	}

	if (turnaround != DI_NODIR && SV_StepDirection(actor, turnaround, dist) )
		return;

	actor->v.ideal_yaw = olddir;	// can't move

	// if a bridge was pulled out from underneath a monster, it may not have
	// a valid standing position at all

	if (!SV_CheckBottom (actor))
		SV_FixCheckBottom (actor);
}


/*
======================
SV_CloseEnough

======================
*/
static qboolean SV_CloseEnough (edict_t *ent, edict_t *goal, float dist)
{
	int		i;

	for (i = 0; i < 3; i++)
	{
		if (goal->v.absmin[i] > ent->v.absmax[i] + dist)
			return false;
		if (goal->v.absmax[i] < ent->v.absmin[i] - dist)
			return false;
	}
	return true;
}


/*
======================
SV_MoveToGoal

======================
*/
void SV_MoveToGoal (void)
{
	edict_t		*ent, *goal;
	float		dist;
#ifdef QUAKE2
	edict_t		*enemy;
#endif

	ent = PROG_TO_EDICT(*sv_globals.self);		// Entity moving
	goal = PROG_TO_EDICT(ent->v.goalentity);	// its goalentity
	dist = G_FLOAT(OFS_PARM0);			// how far to move

	// Reset trace_plane_normal
	VectorClear(*sv_globals.trace_plane_normal);

	// If not onground, flying, or swimming, return 0
	if ( !( (int)ent->v.flags & (FL_ONGROUND|FL_FLY|FL_SWIM) ) )
	{
		G_FLOAT(OFS_RETURN) = 0;
		return;
	}

	// if the next step hits the enemy, return immediately
#ifdef QUAKE2
	// Convert a progs entity to an edict?
	enemy = PROG_TO_EDICT(ent->v.enemy);
	// If edict is not world and 2 edicts close enough to touch if dist is moved
	if (enemy != sv.edicts &&  SV_CloseEnough(ent, enemy, dist) )
#else
	if ( PROG_TO_EDICT(ent->v.enemy) != sv.edicts &&  SV_CloseEnough(ent, goal, dist) )
#endif
	{
		G_FLOAT(OFS_RETURN) = 0;
		return;
	}

	// bump around...

	//If can't go in a direction (including step check) or 30% chance...
	if (!SV_StepDirection (ent, ent->v.ideal_yaw, dist))
	{
		// Find a new direction to go in instead
		SV_NewChaseDir (ent, goal, dist);
		G_FLOAT(OFS_RETURN) = 0;
	}
	else
	{
		if ((rand() & 3) == 1)
		{
			// Find a new direction to go in instead
			SV_NewChaseDir (ent, goal, dist);
		}
		G_FLOAT(OFS_RETURN) = 1;
	}
}

