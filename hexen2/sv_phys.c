/*
	sv_phys.c
	sv physics

	$Header: /home/ozzie/Download/0000/uhexen2/hexen2/sv_phys.c,v 1.23 2007-05-13 11:58:30 sezero Exp $
*/

#include "quakedef.h"

/*

pushmove objects do not obey gravity, and do not interact with
each other or trigger fields, but block normal movement and push
normal objects when they move.

onground is set for toss objects when they come to a complete rest.
it is set for steping or walking objects

doors, plats, etc are SOLID_BSP, and MOVETYPE_PUSH
bonus items are SOLID_TRIGGER touch, and MOVETYPE_TOSS
corpses are SOLID_NOT and MOVETYPE_TOSS
crates are SOLID_BBOX and MOVETYPE_TOSS
walking monsters are SOLID_SLIDEBOX and MOVETYPE_STEP
flying/floating monsters are SOLID_SLIDEBOX and MOVETYPE_FLY

solid_edge items only clip against bsp models.

*/

cvar_t	sv_maxvelocity		= { "sv_maxvelocity", "2000", CVAR_NONE };
cvar_t	sv_nostep		= { "sv_nostep", "0", CVAR_NONE };
cvar_t	sv_gravity		= { "sv_gravity", "800", CVAR_NOTIFY|CVAR_SERVERINFO };
cvar_t	sv_stopspeed		= { "sv_stopspeed", "100", CVAR_NONE };
cvar_t	sv_friction		= { "sv_friction", "4", CVAR_NOTIFY|CVAR_SERVERINFO };
cvar_t	sv_flypitch		= { "sv_flypitch", "20", CVAR_NONE };
cvar_t	sv_walkpitch		= { "sv_walkpitch", "0", CVAR_NONE };

#ifdef QUAKE2
static	vec3_t	vec_origin = {0.0, 0.0, 0.0};
#endif

#define	MOVE_EPSILON	0.01

static void SV_Physics_Toss (edict_t *ent);

/*
================
SV_CheckAllEnts
================
*/
#if 0	// all uses of it are commented out
static void SV_CheckAllEnts (void)
{
	int			e;
	edict_t		*check;

	// see if any solid entities are inside the final position
	check = NEXT_EDICT(sv.edicts);
	for (e = 1; e < sv.num_edicts; e++, check = NEXT_EDICT(check))
	{
		if (check->free)
			continue;
		if (check->v.movetype == MOVETYPE_PUSH
				|| check->v.movetype == MOVETYPE_NONE
#ifdef QUAKE2
				|| check->v.movetype == MOVETYPE_FOLLOW
#endif
				|| check->v.movetype == MOVETYPE_NOCLIP)
			continue;

		if (SV_TestEntityPosition (check))
			Con_Printf ("entity in invalid position\n");
	}
}
#endif


/*
================
SV_CheckVelocity
================
*/
static void SV_CheckVelocity (edict_t *ent)
{
	int		i;
	float		w;

//
// bound velocity
//
	for (i = 0; i < 3; i++)
	{
		if (IS_NAN(ent->v.velocity[i]))
		{
			Con_DPrintf ("Got a NaN velocity on %s\n", PR_GetString(ent->v.classname));
			ent->v.velocity[i] = 0;
		}
		if (IS_NAN(ent->v.origin[i]))
		{
			Con_DPrintf ("Got a NaN origin on %s\n", PR_GetString(ent->v.classname));
			ent->v.origin[i] = 0;
		}
	}

	w = VectorLength(ent->v.velocity);
	if (w > sv_maxvelocity.value)
	{	// sv_maxvelocity fix by Maddes
		VectorScale (ent->v.velocity, sv_maxvelocity.value/w, ent->v.velocity);
	}
}


/*
=============
SV_RunThink

Runs thinking code if time.  There is some play in the exact time the think
function will be called, because it is called before any movement is done
in a frame.  Not used for pushmove objects, because they must be exact.
Returns false if the entity removed itself.
=============
*/
static qboolean SV_RunThink (edict_t *ent)
{
	float	thinktime;

	thinktime = ent->v.nextthink;
	if (thinktime <= 0 || thinktime > sv.time + host_frametime)
		return true;

	if (thinktime < sv.time)
		thinktime = sv.time;	// don't let things stay in the past.
					// it is possible to start that way
					// by a trigger with a local time.
	ent->v.nextthink = 0;
	if (old_progdefs)
	{
		pr_global_struct_v111->time = thinktime;
		pr_global_struct_v111->self = EDICT_TO_PROG(ent);
		pr_global_struct_v111->other = EDICT_TO_PROG(sv.edicts);
	}
	else
	{
		pr_global_struct->time = thinktime;
		pr_global_struct->self = EDICT_TO_PROG(ent);
		pr_global_struct->other = EDICT_TO_PROG(sv.edicts);
	}
	PR_ExecuteProgram (ent->v.think);

	return !ent->free;
}


/*
==================
SV_Impact

Two entities have touched, so run their touch functions
==================
*/
static void SV_Impact (edict_t *e1, edict_t *e2)
{
	int		old_self, old_other;

	if (old_progdefs)
	{
		old_self = pr_global_struct_v111->self;
		old_other = pr_global_struct_v111->other;

		pr_global_struct_v111->time = sv.time;
		if (e1->v.touch && e1->v.solid != SOLID_NOT)
		{
			pr_global_struct_v111->self = EDICT_TO_PROG(e1);
			pr_global_struct_v111->other = EDICT_TO_PROG(e2);
			PR_ExecuteProgram (e1->v.touch);
		}

		if (e2->v.touch && e2->v.solid != SOLID_NOT)
		{
			pr_global_struct_v111->self = EDICT_TO_PROG(e2);
			pr_global_struct_v111->other = EDICT_TO_PROG(e1);
			PR_ExecuteProgram (e2->v.touch);
		}

		pr_global_struct_v111->self = old_self;
		pr_global_struct_v111->other = old_other;

		return;
	}

	old_self = pr_global_struct->self;
	old_other = pr_global_struct->other;

	pr_global_struct->time = sv.time;
	if (e1->v.touch && e1->v.solid != SOLID_NOT)
	{
		pr_global_struct->self = EDICT_TO_PROG(e1);
		pr_global_struct->other = EDICT_TO_PROG(e2);
		PR_ExecuteProgram (e1->v.touch);
	}

	if (e2->v.touch && e2->v.solid != SOLID_NOT)
	{
		pr_global_struct->self = EDICT_TO_PROG(e2);
		pr_global_struct->other = EDICT_TO_PROG(e1);
		PR_ExecuteProgram (e2->v.touch);
	}

	pr_global_struct->self = old_self;
	pr_global_struct->other = old_other;
}


/*
==================
ClipVelocity

Slide off of the impacting object
returns the blocked flags (1 = floor, 2 = step / wall)
==================
*/
#define	STOP_EPSILON	0.1

static int ClipVelocity (vec3_t in, vec3_t normal, vec3_t out, float overbounce)
{
	float	backoff;
	float	change;
	int		i, blocked;

	blocked = 0;
	if (normal[2] > 0)
		blocked |= 1;	// floor
	if (!normal[2])
		blocked |= 2;	// step

	backoff = DotProduct (in, normal) * overbounce;

	for (i = 0; i < 3; i++)
	{
		change = normal[i]*backoff;
		out[i] = in[i] - change;
		if (out[i] > -STOP_EPSILON && out[i] < STOP_EPSILON)
			out[i] = 0;
	}

	return blocked;
}


/*
============
SV_FlyMove

The basic solid body movement clip that slides along multiple planes
Returns the clipflags if the velocity was modified (hit something solid)
1 = floor
2 = wall / step
4 = dead stop
If steptrace is not NULL, the trace of any vertical wall hit will be stored
============
*/
#define	MAX_CLIP_PLANES	5
static int SV_FlyMove (edict_t *ent, float time, trace_t *steptrace)
{
	int			bumpcount, numbumps;
	vec3_t		dir;
	float		d;
	int			numplanes;
	vec3_t		planes[MAX_CLIP_PLANES];
	vec3_t		primal_velocity, original_velocity, new_velocity;
//rjr	vec3_t		before_velocity;
	int			i, j;
	trace_t		trace;
	vec3_t		end;
	float		time_left;
	int			blocked;

	numbumps = 4;

	blocked = 0;
	VectorCopy (ent->v.velocity, original_velocity);
	VectorCopy (ent->v.velocity, primal_velocity);
	numplanes = 0;

	time_left = time;

	for (bumpcount = 0; bumpcount < numbumps; bumpcount++)
	{
		if (!ent->v.velocity[0] && !ent->v.velocity[1] && !ent->v.velocity[2])
			break;

		for (i = 0; i < 3; i++)
			end[i] = ent->v.origin[i] + time_left * ent->v.velocity[i];

		trace = SV_Move (ent->v.origin, ent->v.mins, ent->v.maxs, end, false, ent);

		if (trace.allsolid)
		{	// entity is trapped in another solid
			VectorClear (ent->v.velocity);
			return 3;
		}

		if (trace.fraction > 0)
		{	// actually covered some distance
			VectorCopy (trace.endpos, ent->v.origin);
			VectorCopy (ent->v.velocity, original_velocity);
			numplanes = 0;
		}

		if (trace.fraction == 1)
			break;		// moved the entire distance

		if (!trace.ent)
			Sys_Error ("%s: !trace.ent", __thisfunc__);

		if (trace.plane.normal[2] > 0.7)
		{
			blocked |= 1;		// floor
			if (trace.ent->v.solid == SOLID_BSP)
			{
				ent->v.flags =	(int)ent->v.flags | FL_ONGROUND;
				ent->v.groundentity = EDICT_TO_PROG(trace.ent);
			}
		}
		if (!trace.plane.normal[2])
		{
			blocked |= 2;		// step
			if (steptrace)
				*steptrace = trace;	// save for player extrafriction
		}

//rjr		VectorCopy (ent->v.velocity, before_velocity);

//
// run the impact function
//
		SV_Impact (ent, trace.ent);
		if (ent->free)
			break;		// removed by the impact function

/*rjr		if (!VectorCompare(ent->v.velocity, before_velocity))
		{
			break;
		}
*/

		time_left -= time_left * trace.fraction;

	// cliped to another plane
		if (numplanes >= MAX_CLIP_PLANES)
		{	// this shouldn't really happen
			VectorClear (ent->v.velocity);
			return 3;
		}

		VectorCopy (trace.plane.normal, planes[numplanes]);
		numplanes++;

//
// modify original_velocity so it parallels all of the clip planes
//
		for (i = 0; i < numplanes; i++)
		{
			ClipVelocity (original_velocity, planes[i], new_velocity, 1);
			for (j = 0; j < numplanes; j++)
			{
				if (j != i)
				{
					if (DotProduct (new_velocity, planes[j]) < 0)
						break;	// not ok
				}
			}

			if (j == numplanes)
				break;
		}

		if (i != numplanes)
		{	// go along this plane
			VectorCopy (new_velocity, ent->v.velocity);
		}
		else
		{	// go along the crease
			if (numplanes != 2)
			{
			//	Con_Printf ("clip velocity, numplanes == %i\n",numplanes);
				VectorClear (ent->v.velocity);
				return 7;
			}
			CrossProduct (planes[0], planes[1], dir);
			d = DotProduct (dir, ent->v.velocity);
			VectorScale (dir, d, ent->v.velocity);
		}

//
// if original velocity is against the original velocity, stop dead
// to avoid tiny occilations in sloping corners
//
		if (DotProduct (ent->v.velocity, primal_velocity) <= 0)
		{
			VectorClear (ent->v.velocity);
			return blocked;
		}
	}

	return blocked;
}


/*
============
SV_FlyExtras

============
*/
static const float hoverinc = 0.4;
static void SV_FlyExtras (edict_t *ent, float time, trace_t *steptrace)
{
	// Jumping makes you loose this flag so reset it
	ent->v.flags = (int) ent->v.flags | FL_ONGROUND;

	if ((ent->v.velocity[2] <= 6) && (ent->v.velocity[2] >= -6))
	{
		ent->v.velocity[2] += ent->v.hoverz;

		if (ent->v.velocity[2] >= 6)
		{
			ent->v.hoverz = -hoverinc;
			ent->v.velocity[2] += ent->v.hoverz;
		}
		else if (ent->v.velocity[2] <= -6)
		{
			ent->v.hoverz = hoverinc;
			ent->v.velocity[2] += ent->v.hoverz;
		}
	}
	else	// friction for upward or downward progress once key is released
	{
		ent->v.velocity[2] -= sv_player->v.velocity[2] * .1;
	}
}


/*
============
SV_AddGravity

============
*/
static void SV_AddGravity (edict_t *ent)
{
	float	ent_gravity;

#ifdef QUAKE2
	if (ent->v.gravity)
		ent_gravity = ent->v.gravity;
	else
		ent_gravity = 1.0;
#else
	eval_t	*val;

	val = GetEdictFieldValue(ent, "gravity");
	if (val && val->_float)
		ent_gravity = val->_float;
	else
		ent_gravity = 1.0;
#endif

	ent->v.velocity[2] -= ent_gravity * sv_gravity.value * host_frametime;
}


/*
===============================================================================

PUSHMOVE

===============================================================================
*/

/*
============
SV_PushEntity

Does not change the entities velocity at all
============
*/
static trace_t SV_PushEntity (edict_t *ent, vec3_t push)
{
	trace_t	trace;
	vec3_t	start,end, impact;
	edict_t *impact_e;

	VectorCopy (ent->v.origin, start);
	VectorAdd (ent->v.origin, push, end);

	if (ent->v.movetype == MOVETYPE_FLYMISSILE  || ent->v.movetype == MOVETYPE_BOUNCEMISSILE)
		trace = SV_Move (ent->v.origin, ent->v.mins, ent->v.maxs, end, MOVE_MISSILE, ent);
	else if (ent->v.solid == SOLID_TRIGGER || ent->v.solid == SOLID_NOT)
// only clip against bmodels
		trace = SV_Move (ent->v.origin, ent->v.mins, ent->v.maxs, end, MOVE_NOMONSTERS, ent);
	else if (ent->v.movetype == MOVETYPE_SWIM)
		trace = SV_Move (ent->v.origin, ent->v.mins, ent->v.maxs, end, MOVE_WATER, ent);
	else
		trace = SV_Move (ent->v.origin, ent->v.mins, ent->v.maxs, end, MOVE_NORMAL, ent);

	if (ent->v.solid != SOLID_PHASE)
	{
		if (ent->v.movetype != MOVETYPE_BOUNCE || (trace.allsolid == 0 && trace.startsolid == 0))
		{
			VectorCopy (trace.endpos, ent->v.origin);   // Macro - watchout
		}
		else
		{
			trace.fraction = 0;

			return trace;
		}
	}
	else	// Entity is PHASED so bounce off walls and other entities, go through monsters and players
	{
		if (trace.ent)
		{	// Go through MONSTERS and PLAYERS, can't use FL_CLIENT cause rotating brushes do
			if (((int) trace.ent->v.flags & FL_MONSTER) || (trace.ent->v.movetype == MOVETYPE_WALK))
			{
				VectorCopy (trace.endpos, impact);
				impact_e = trace.ent;

				trace = SV_Move (ent->v.origin, ent->v.mins, ent->v.maxs, end, MOVE_PHASE, ent);

				VectorCopy (impact, ent->v.origin);
				SV_Impact (ent, impact_e);

				VectorCopy (trace.endpos, ent->v.origin);
			}
			else
			{
				VectorCopy (trace.endpos, ent->v.origin);
			}
		}
		else
		{
			VectorCopy (trace.endpos, ent->v.origin);
		}
	}

	SV_LinkEdict (ent, true);

	if (trace.ent)
		SV_Impact (ent, trace.ent);

	return trace;
}


/*
============
SV_PushMove

============
*/
static void SV_PushMove (edict_t *pusher, float movetime, qboolean update_time)
{
	int			i, e;
	edict_t		*check, *block;
	vec3_t		mins, maxs, move;
	vec3_t		entorig, pushorig;
	int			num_moved;
	edict_t		*moved_edict[MAX_EDICTS];
	vec3_t		moved_from[MAX_EDICTS];

	if (!pusher->v.velocity[0] && !pusher->v.velocity[1] && !pusher->v.velocity[2])
	{
		if (update_time)
			pusher->v.ltime += movetime;
		return;
	}

	for (i = 0; i < 3; i++)
	{
		move[i] = pusher->v.velocity[i] * movetime;
		mins[i] = pusher->v.absmin[i] + move[i];
		maxs[i] = pusher->v.absmax[i] + move[i];
	}

	VectorCopy (pusher->v.origin, pushorig);

	// move the pusher to it's final position
	VectorAdd (pusher->v.origin, move, pusher->v.origin);
	if (update_time)
		pusher->v.ltime += movetime;
	SV_LinkEdict (pusher, false);

	// see if any solid entities are inside the final position
	num_moved = 0;
	check = NEXT_EDICT(sv.edicts);
	for (e = 1; e < sv.num_edicts; e++, check = NEXT_EDICT(check))
	{
		if (check->free)
			continue;
		if (check->v.movetype == MOVETYPE_PUSH
				|| check->v.movetype == MOVETYPE_NONE
#ifdef QUAKE2
				|| check->v.movetype == MOVETYPE_FOLLOW
#endif
				|| check->v.movetype == MOVETYPE_NOCLIP)
			continue;

	// if the entity is standing on the pusher, it will definately be moved
		if ( ! ( ((int)check->v.flags & FL_ONGROUND)
			&& PROG_TO_EDICT(check->v.groundentity) == pusher) )
		{
			if ( check->v.absmin[0] >= maxs[0]
					|| check->v.absmin[1] >= maxs[1]
					|| check->v.absmin[2] >= maxs[2]
					|| check->v.absmax[0] <= mins[0]
					|| check->v.absmax[1] <= mins[1]
					|| check->v.absmax[2] <= mins[2] )
				continue;

		// see if the ent's bbox is inside the pusher's final position
			if (!SV_TestEntityPosition (check))
				continue;
		}

		// remove the onground flag for non-players
		if (check->v.movetype != MOVETYPE_WALK)
			check->v.flags = (int)check->v.flags & ~FL_ONGROUND;

		VectorCopy (check->v.origin, entorig);
		VectorCopy (check->v.origin, moved_from[num_moved]);
		moved_edict[num_moved] = check;
		num_moved++;

		// try moving the contacted entity
		pusher->v.solid = SOLID_NOT;
		SV_PushEntity (check, move);
		pusher->v.solid = SOLID_BSP;

		// if it is still inside the pusher, block
		block = SV_TestEntityPosition (check);
		if (block)
		{	// fail the move
			if (check->v.mins[0] == check->v.maxs[0])
				continue;
			if (check->v.solid == SOLID_NOT || check->v.solid == SOLID_TRIGGER)
			{	// corpse
				check->v.mins[0] = check->v.mins[1] = 0;
				VectorCopy (check->v.mins, check->v.maxs);
				continue;
			}

			VectorCopy (entorig, check->v.origin);
			SV_LinkEdict (check, true);

			VectorCopy (pushorig, pusher->v.origin);
			SV_LinkEdict (pusher, false);
			if (update_time)
				pusher->v.ltime -= movetime;

			// if the pusher has a "blocked" function, call it
			// otherwise, just stay in place until the obstacle is gone
			if (pusher->v.blocked)
			{
				if (old_progdefs)
				{
					pr_global_struct_v111->self = EDICT_TO_PROG(pusher);
					pr_global_struct_v111->other = EDICT_TO_PROG(check);
				}
				else
				{
					pr_global_struct->self = EDICT_TO_PROG(pusher);
					pr_global_struct->other = EDICT_TO_PROG(check);
				}
				PR_ExecuteProgram (pusher->v.blocked);
			}

			// move back any entities we already moved
			for (i = 0; i < num_moved; i++)
			{
				VectorCopy (moved_from[i], moved_edict[i]->v.origin);
				SV_LinkEdict (moved_edict[i], false);
			}
			return;
		}
	}
}


/*
============
SV_PushRotate
Pre-Mission Pack fix
============
*/
#if 0	// Pre-Mission Pack fix
static void SV_PushRotate (edict_t *pusher, float movetime)
{
	int			i, e;
	edict_t		*check, *block;
	vec3_t		move, a, amove;
	vec3_t		entorig, pushorig;
	int			num_moved;
	edict_t		*moved_edict[MAX_EDICTS];
	vec3_t		moved_from[MAX_EDICTS];
	vec3_t		org, org2;
	vec3_t		forward, right, up;
	edict_t		*ground;
	edict_t		*master;
	edict_t		*slave;
	int			slaves_moved;
	qboolean	moveit;

#   if 0
	Con_DPrintf("%s entity %i (time=%f)\n", __thisfunc__, NUM_FOR_EDICT(pusher), movetime);
	Con_DPrintf("%f %f %f (avelocity)\n", pusher->v.avelocity[0], pusher->v.avelocity[1], pusher->v.avelocity[2]);
	Con_DPrintf("%f %f %f\n", pusher->v.angles[0], pusher->v.angles[1], pusher->v.angles[2]);
#   endif

	for (i = 0; i < 3; i++)
		amove[i] = pusher->v.avelocity[i] * movetime;

	VectorNegate (amove, a);
	AngleVectors (a, forward, right, up);

	VectorCopy (pusher->v.angles, pushorig);

	// move the pusher to it's final position
	VectorAdd (pusher->v.angles, amove, pusher->v.angles);

	pusher->v.ltime += movetime;
	SV_LinkEdict (pusher, false);

	master = pusher;
	slaves_moved = 0;
/*	while (master->v.aiment)
	{
		slave = PROG_TO_EDICT(master->v.aiment);

	//	Con_DPrintf("%f %f %f   slave entity %i\n", slave->v.angles[0], slave->v.angles[1], slave->v.angles[2], NUM_FOR_EDICT(slave));

		slaves_moved++;
		VectorCopy (slave->v.angles, moved_from[MAX_EDICTS - slaves_moved]);
		moved_edict[MAX_EDICTS - slaves_moved] = slave;

		if (slave->v.movedir[PITCH])
			slave->v.angles[PITCH] = master->v.angles[PITCH];
		else
			slave->v.angles[PITCH] += slave->v.avelocity[PITCH] * movetime;

		if (slave->v.movedir[YAW])
			slave->v.angles[YAW] = master->v.angles[YAW];
		else
			slave->v.angles[YAW] += slave->v.avelocity[YAW] * movetime;

		if (slave->v.movedir[ROLL])
			slave->v.angles[ROLL] = master->v.angles[ROLL];
		else
			slave->v.angles[ROLL] += slave->v.avelocity[ROLL] * movetime;

		slave->v.ltime = master->v.ltime;
		SV_LinkEdict (slave, false);

		master = slave;
	}
*/

	// see if any solid entities are inside the final position
	num_moved = 0;
	check = NEXT_EDICT(sv.edicts);
	for (e = 1; e < sv.num_edicts; e++, check = NEXT_EDICT(check))
	{
		if (check->free)
			continue;
		if (check->v.movetype == MOVETYPE_PUSH
				|| check->v.movetype == MOVETYPE_NONE
				|| check->v.movetype == MOVETYPE_FOLLOW
				|| check->v.movetype == MOVETYPE_NOCLIP)
			continue;

		// if the entity is standing on the pusher, it will definitely be moved
		moveit = false;
		ground = PROG_TO_EDICT(check->v.groundentity);
		if ((int)check->v.flags & FL_ONGROUND)
		{
			if (ground == pusher)
			{
				moveit = true;
			}
			else
			{
				for (i = 0; i < slaves_moved; i++)
				{
					if (ground == moved_edict[MAX_EDICTS - i - 1])
					{
						moveit = true;
						break;
					}
				}
			}
		}

		if (!moveit)
		{
			if ( check->v.absmin[0] >= pusher->v.absmax[0]
					|| check->v.absmin[1] >= pusher->v.absmax[1]
					|| check->v.absmin[2] >= pusher->v.absmax[2]
					|| check->v.absmax[0] <= pusher->v.absmin[0]
					|| check->v.absmax[1] <= pusher->v.absmin[1]
					|| check->v.absmax[2] <= pusher->v.absmin[2] )
			{
				for (i = 0; i < slaves_moved; i++)
				{
					slave = moved_edict[MAX_EDICTS - i - 1];
					if ( check->v.absmin[0] >= slave->v.absmax[0]
							|| check->v.absmin[1] >= slave->v.absmax[1]
							|| check->v.absmin[2] >= slave->v.absmax[2]
							|| check->v.absmax[0] <= slave->v.absmin[0]
							|| check->v.absmax[1] <= slave->v.absmin[1]
							|| check->v.absmax[2] <= slave->v.absmin[2] )
						continue;
				}
				if (i == slaves_moved)
					continue;
			}

		// see if the ent's bbox is inside the pusher's final position
			if (!SV_TestEntityPosition (check))
				continue;
		}

		// remove the onground flag for non-players
		if (check->v.movetype != MOVETYPE_WALK)
			check->v.flags = (int)check->v.flags & ~FL_ONGROUND;

		VectorCopy (check->v.origin, entorig);
		VectorCopy (check->v.origin, moved_from[num_moved]);
		moved_edict[num_moved] = check;
		num_moved++;

		// calculate destination position
		VectorSubtract (check->v.origin, pusher->v.origin, org);
		org2[0] = DotProduct (org, forward);
		org2[1] = -DotProduct (org, right);
		org2[2] = DotProduct (org, up);
		VectorSubtract (org2, org, move);

		// try moving the contacted entity
		pusher->v.solid = SOLID_NOT;
		SV_PushEntity (check, move);
	//@@TODO: do we ever want to do anybody's angles?  maybe just yaw???
	//	if (!((int)check->v.flags & (FL_CLIENT | FL_MONSTER)))
	//		VectorAdd (check->v.angles, amove, check->v.angles);
		check->v.angles[YAW] += amove[YAW];

		pusher->v.solid = SOLID_BSP;

		// if it is still inside the pusher, block
		block = SV_TestEntityPosition (check);
		if (block)
		{	// fail the move
			if (check->v.mins[0] == check->v.maxs[0])
				continue;
			if (check->v.solid == SOLID_NOT || check->v.solid == SOLID_TRIGGER)
			{	// corpse
				check->v.mins[0] = check->v.mins[1] = 0;
				VectorCopy (check->v.mins, check->v.maxs);
				continue;
			}

			VectorCopy (entorig, check->v.origin);
			SV_LinkEdict (check, true);

			VectorCopy (pushorig, pusher->v.angles);
			SV_LinkEdict (pusher, false);
			pusher->v.ltime -= movetime;

			for (i = 0; i < slaves_moved; i++)
			{
				slave = moved_edict[MAX_EDICTS - i - 1];
				VectorCopy (moved_from[MAX_EDICTS - i - 1], slave->v.angles);
				SV_LinkEdict (slave, false);
				slave->v.ltime -= movetime;
			}

			// if the pusher has a "blocked" function, call it
			// otherwise, just stay in place until the obstacle is gone
			if (pusher->v.blocked)
			{
				if (old_progdefs)
				{
					pr_global_struct_v111->self = EDICT_TO_PROG(pusher);
					pr_global_struct_v111->other = EDICT_TO_PROG(check);
				}
				else
				{
					pr_global_struct->self = EDICT_TO_PROG(pusher);
					pr_global_struct->other = EDICT_TO_PROG(check);
				}
				PR_ExecuteProgram (pusher->v.blocked);
			}

			// move back any entities we already moved
			for (i = 0; i < num_moved; i++)
			{
				VectorCopy (moved_from[i], moved_edict[i]->v.origin);
			//@@TODO:: see above
			//	if (!((int)moved_edict[i]->v.flags & (FL_CLIENT | FL_MONSTER)))
			//		VectorSubtract (moved_edict[i]->v.angles, amove, moved_edict[i]->v.angles);
				moved_edict[i]->v.angles[YAW] -= amove[YAW];

				SV_LinkEdict (moved_edict[i], false);
			}
			return;
		}
	}

#   if 0
	Con_DPrintf("result:\n");
	Con_DPrintf("%f %f %f\n", pusher->v.angles[0], pusher->v.angles[1], pusher->v.angles[2]);
	for (i = 0; i < slaves_moved; i++)
	{
		slave = moved_edict[MAX_EDICTS - i - 1];
		Con_DPrintf("%f %f %f   slave entity %i\n", slave->v.angles[0], slave->v.angles[1], slave->v.angles[2], NUM_FOR_EDICT(slave));
	}
	Con_DPrintf("\n");
#   endif
}
#endif	// end of Pre-Mission Pack fix

/*
============
SV_PushRotate
NEW
============
*/
static void SV_PushRotate (edict_t *pusher, float movetime)
{
	int			i, e, t;
	edict_t		*check, *block;
	vec3_t		move, a, amove, mins, maxs, move2, move3, testmove;
//	vec3_t		amove_norm;
	vec3_t		entorig, pushorig, pushorigangles;
	int			num_moved;
	edict_t		*moved_edict[MAX_EDICTS];
	vec3_t		moved_from[MAX_EDICTS];
	vec3_t		org, org2, check_center;
	vec3_t		forward, right, up;
	edict_t		*ground;
	edict_t		*master;
	edict_t		*slave;
	int			slaves_moved;
	qboolean	moveit;
//	float		turn_away, amove_mag;

#   if 0
	Con_DPrintf("%s entity %i (time=%f)\n", __thisfunc__, NUM_FOR_EDICT(pusher), movetime);
	Con_DPrintf("%f %f %f (avelocity)\n", pusher->v.avelocity[0], pusher->v.avelocity[1], pusher->v.avelocity[2]);
	Con_DPrintf("%f %f %f\n", pusher->v.angles[0], pusher->v.angles[1], pusher->v.angles[2]);
#   endif

	for (i = 0; i < 3; i++)
	{
		amove[i] = pusher->v.avelocity[i] * movetime;
		move[i] = pusher->v.velocity[i] * movetime;
		mins[i] = pusher->v.absmin[i] + move[i];
		maxs[i] = pusher->v.absmax[i] + move[i];
	}

	VectorNegate (amove, a);
	AngleVectors (a, forward, right, up);

	VectorCopy (pusher->v.origin, pushorig);
	VectorCopy (pusher->v.angles, pushorigangles);

	// move the pusher to it's final position
	VectorAdd (pusher->v.origin, move, pusher->v.origin);
	VectorAdd (pusher->v.angles, amove, pusher->v.angles);

	pusher->v.ltime += movetime;
	SV_LinkEdict (pusher, false);

	master = pusher;
	slaves_moved = 0;
/*	while (master->v.aiment)
	{
		slave = PROG_TO_EDICT(master->v.aiment);

	//	Con_DPrintf("%f %f %f   slave entity %i\n", slave->v.angles[0], slave->v.angles[1], slave->v.angles[2], NUM_FOR_EDICT(slave));

		slaves_moved++;
		VectorCopy (slave->v.angles, moved_from[MAX_EDICTS - slaves_moved]);
		moved_edict[MAX_EDICTS - slaves_moved] = slave;

		if (slave->v.movedir[PITCH])
			slave->v.angles[PITCH] = master->v.angles[PITCH];
		else
			slave->v.angles[PITCH] += slave->v.avelocity[PITCH] * movetime;

		if (slave->v.movedir[YAW])
			slave->v.angles[YAW] = master->v.angles[YAW];
		else
			slave->v.angles[YAW] += slave->v.avelocity[YAW] * movetime;

		if (slave->v.movedir[ROLL])
			slave->v.angles[ROLL] = master->v.angles[ROLL];
		else
			slave->v.angles[ROLL] += slave->v.avelocity[ROLL] * movetime;

		slave->v.ltime = master->v.ltime;
		SV_LinkEdict (slave, false);

		master = slave;
	}
*/

	// see if any solid entities are inside the final position
	num_moved = 0;
	check = NEXT_EDICT(sv.edicts);
	for (e = 1; e < sv.num_edicts; e++, check = NEXT_EDICT(check))
	{
		if (check->free)
			continue;
		if (check->v.movetype == MOVETYPE_PUSH
				|| check->v.movetype == MOVETYPE_NONE
				|| check->v.movetype == MOVETYPE_FOLLOW
				|| check->v.movetype == MOVETYPE_NOCLIP)
			continue;

		// if the entity is standing on the pusher, it will definitely be moved
		moveit = false;
		ground = PROG_TO_EDICT(check->v.groundentity);
		if ((int)check->v.flags & FL_ONGROUND)
		{
			if (ground == pusher)
			{
				moveit = true;
			}
			else
			{
				for (i = 0; i < slaves_moved; i++)
				{
					if (ground == moved_edict[MAX_EDICTS - i - 1])
					{
						moveit = true;
						break;
					}
				}
			}
		}

		if (!moveit)
		{
			if ( check->v.absmin[0] >= maxs[0]
				|| check->v.absmin[1] >= maxs[1]
				|| check->v.absmin[2] >= maxs[2]
				|| check->v.absmax[0] <= mins[0]
				|| check->v.absmax[1] <= mins[1]
				|| check->v.absmax[2] <= mins[2] )
			{
				for (i = 0; i < slaves_moved; i++)
				{
					slave = moved_edict[MAX_EDICTS - i - 1];
					if ( check->v.absmin[0] >= slave->v.absmax[0]
							|| check->v.absmin[1] >= slave->v.absmax[1]
							|| check->v.absmin[2] >= slave->v.absmax[2]
							|| check->v.absmax[0] <= slave->v.absmin[0]
							|| check->v.absmax[1] <= slave->v.absmin[1]
							|| check->v.absmax[2] <= slave->v.absmin[2] )
						continue;
				}
				if (i == slaves_moved)
					continue;
			}

		// see if the ent's bbox is inside the pusher's final position
			if (!SV_TestEntityPosition (check))
				continue;
		}

		// remove the onground flag for non-players
		if (check->v.movetype != MOVETYPE_WALK)
			check->v.flags = (int)check->v.flags & ~FL_ONGROUND;

		VectorCopy (check->v.origin, entorig);
		VectorCopy (check->v.origin, moved_from[num_moved]);
		moved_edict[num_moved] = check;
		num_moved++;

		// put check in first move spot
		VectorAdd (check->v.origin, move, check->v.origin);
		// Use center of model, like in QUAKE!!!!
		// Our origins are on the bottom!!!
		for (i = 0; i < 3; i++)
			check_center[i] = (check->v.absmin[i] + check->v.absmax[i]) / 2;
		// calculate destination position
		VectorSubtract (check_center, pusher->v.origin, org);
		// put check back
		VectorSubtract (check->v.origin, move, check->v.origin);
		org2[0] = DotProduct (org, forward);
		org2[1] = -DotProduct (org, right);
		org2[2] = DotProduct (org, up);
		VectorSubtract (org2, org, move2);

	//	Con_DPrintf("%f %f %f (move2)\n", move2[0], move2[1], move2[2]);

	//	VectorAdd (check->v.origin, move2, check->v.origin);

		// Add all moves together
		VectorAdd(move,move2,move3);

		// Find the angle of rotation as compared to vector from pusher
		// origin to check center
	//	turn_away = DotProduct(org,a);

		// try moving the contacted entity
		for (t = 0; t < 13; t++)
		{
			switch (t)
			{
			case 0:
			//try x, y and z
				VectorCopy(move3,testmove);
				break;
			case 1:
			//Try xy only
				VectorSubtract(check->v.origin,testmove,check->v.origin);
				testmove[0] = move3[0];
				testmove[1] = move3[1];
				testmove[2] = 0;
				break;
			case 2:
			//Try z only
				VectorSubtract(check->v.origin,testmove,check->v.origin);
				testmove[0] = 0;
				testmove[1] = 0;
				testmove[2] = move3[2];
				break;
			case 3:
			//Try none
				VectorSubtract(check->v.origin,testmove,check->v.origin);
				testmove[0] = 0;
				testmove[1] = 0;
				testmove[2] = 0;
				break;
			case 4:
			//Try xy in opposite dir
				testmove[0] = move3[0] * -1;
				testmove[1] = move3[1] * -1;
				testmove[2] = move3[2];
				break;
			case 5:
			//Try z in opposite dir
				VectorSubtract(check->v.origin,testmove,check->v.origin);
				testmove[0] = move3[0];
				testmove[1] = move3[1];
				testmove[2] = move3[2] * -1;
				break;
			case 6:
			//Try xyz in opposite dir
				VectorSubtract(check->v.origin,testmove,check->v.origin);
				testmove[0] = move3[0] * -1;
				testmove[1] = move3[1] * -1;
				testmove[2] = move3[2] * -1;
				break;
			case 7:
			//Try move3 times 2
				VectorSubtract(check->v.origin,testmove,check->v.origin);
				VectorScale(move3,2,testmove);
				break;
			case 8:
			//Try normalized org
				VectorSubtract(check->v.origin,testmove,check->v.origin);

			//	VectorCopy(amove,amove_norm);
			//	amove_mag = VectorNormalize(amove_norm);
			//	//VectorNormalize(org);
			//	VectorScale(org,amove_mag,org);

			//	VectorNormalize(org);
				VectorScale(org,movetime,org);//movetime*20?
				VectorCopy(org,testmove);
				break;
			case 9:
			//Try normalized org z * 3 only
				VectorSubtract(check->v.origin,testmove,check->v.origin);
				testmove[0] = 0;
				testmove[1] = 0;
				testmove[2] = org[2] * 3;//was: +org[2]*(fabs(org[1])+fabs(org[2]));
				break;
			case 10:
			//Try normalized org xy * 2 only
				VectorSubtract(check->v.origin,testmove,check->v.origin);
				testmove[0] = org[0] * 2;//was: +org[0]*fabs(org[2]);
				testmove[1] = org[1] * 2;//was: +org[1]*fabs(org[2]);
				testmove[2] = 0;
				break;
			case 11:
			//Try xy in opposite org dir
				VectorSubtract(check->v.origin,testmove,check->v.origin);
				testmove[0] = org[0] * -2;
				testmove[1] = org[1] * -2;
				testmove[2] = org[2];
				break;
			case 12:
			//Try z in opposite dir
				VectorSubtract(check->v.origin,testmove,check->v.origin);
				testmove[0] = org[0];
				testmove[1] = org[1];
				testmove[2] = org[2] * -3;
				break;
			}

			if (t != 3)
			{
			//THIS IS VERY BAD BAD HACK...
				pusher->v.solid = SOLID_NOT;
				SV_PushEntity (check, move3);
			//@@TODO: do we ever want to do anybody's angles?  maybe just yaw???
			//	if (!((int)check->v.flags & (FL_CLIENT | FL_MONSTER)))
			//		VectorAdd (check->v.angles, amove, check->v.angles);
				check->v.angles[YAW] += amove[YAW];
				pusher->v.solid = SOLID_BSP;
			}
			// if it is still inside the pusher, block
			block = SV_TestEntityPosition (check);
			if (!block)
				break;
		}

	//	Con_DPrintf("t: %i\n",t);

	//	if (turn_away > 0)
	//	{
			if (block)
			{	// fail the move
			//	Con_DPrintf("Check blocked\n");
				if (check->v.mins[0] == check->v.maxs[0])
					continue;
				if (check->v.solid == SOLID_NOT || check->v.solid == SOLID_TRIGGER)
				{	// corpse
					check->v.mins[0] = check->v.mins[1] = 0;
					VectorCopy (check->v.mins, check->v.maxs);
					continue;
				}

				VectorCopy (entorig, check->v.origin);
				SV_LinkEdict (check, true);

				VectorCopy (pushorig, pusher->v.origin);
				VectorCopy (pushorigangles, pusher->v.angles);
				SV_LinkEdict (pusher, false);
				pusher->v.ltime -= movetime;

				for (i = 0; i < slaves_moved; i++)
				{
					slave = moved_edict[MAX_EDICTS - i - 1];
					VectorCopy (moved_from[MAX_EDICTS - i - 1], slave->v.angles);
					SV_LinkEdict (slave, false);
					slave->v.ltime -= movetime;
				}

				// if the pusher has a "blocked" function, call it
				// otherwise, just stay in place until the obstacle is gone
				if (pusher->v.blocked)
				{
					if (old_progdefs)
					{
						pr_global_struct_v111->self = EDICT_TO_PROG(pusher);
						pr_global_struct_v111->other = EDICT_TO_PROG(check);
					}
					else
					{
						pr_global_struct->self = EDICT_TO_PROG(pusher);
						pr_global_struct->other = EDICT_TO_PROG(check);
					}
					PR_ExecuteProgram (pusher->v.blocked);
				}

				// move back any entities we already moved
				for (i = 0; i < num_moved; i++)
				{
					VectorCopy (moved_from[i], moved_edict[i]->v.origin);
				//@@TODO:: see above
				//	if (!((int)moved_edict[i]->v.flags & (FL_CLIENT | FL_MONSTER)))
				//		VectorSubtract (moved_edict[i]->v.angles, amove, moved_edict[i]->v.angles);
					moved_edict[i]->v.angles[YAW] -= amove[YAW];

					SV_LinkEdict (moved_edict[i], false);
				}
				return;
			}
	//	}
	//	else if (block)	// undo last move
	//		VectorCopy (entorig, check->v.origin);
	}

#   if 0
	Con_DPrintf("result:\n");
	Con_DPrintf("%f %f %f\n", pusher->v.angles[0], pusher->v.angles[1], pusher->v.angles[2]);
	for (i = 0; i < slaves_moved; i++)
	{
		slave = moved_edict[MAX_EDICTS - i - 1];
		Con_DPrintf("%f %f %f   slave entity %i\n", slave->v.angles[0], slave->v.angles[1], slave->v.angles[2], NUM_FOR_EDICT(slave));
	}
	Con_DPrintf("\n");
#   endif
}


/*
================
SV_Physics_Pusher

================
*/
static void SV_Physics_Pusher (edict_t *ent)
{
	float	thinktime;
	float	oldltime;
	float	movetime;

	oldltime = ent->v.ltime;

	thinktime = ent->v.nextthink;
	if (thinktime < ent->v.ltime + host_frametime)
	{
		movetime = thinktime - ent->v.ltime;
		if (movetime < 0)
			movetime = 0;
	}
	else
		movetime = host_frametime;

	if (movetime)
	{
		if (ent->v.avelocity[0] || ent->v.avelocity[1] || ent->v.avelocity[2])
		{
			//SV_PushMove (ent, movetime, false);
			SV_PushRotate (ent, movetime);
		}
		else
			SV_PushMove (ent, movetime, true);	// advances ent->v.ltime if not blocked
	}

	if (thinktime > oldltime && thinktime <= ent->v.ltime)
	{
		ent->v.nextthink = 0;
		if (old_progdefs)
		{
			pr_global_struct_v111->time = sv.time;
			pr_global_struct_v111->self = EDICT_TO_PROG(ent);
			pr_global_struct_v111->other = EDICT_TO_PROG(sv.edicts);
		}
		else
		{
			pr_global_struct->time = sv.time;
			pr_global_struct->self = EDICT_TO_PROG(ent);
			pr_global_struct->other = EDICT_TO_PROG(sv.edicts);
		}
		PR_ExecuteProgram (ent->v.think);
		if (ent->free)
			return;
	}
}


/*
===============================================================================

CLIENT MOVEMENT

===============================================================================
*/

/*
=============
SV_CheckStuck

This is a big hack to try and fix the rare case of getting stuck in the world
clipping hull.
=============
*/
static void SV_CheckStuck (edict_t *ent)
{
	int		i, j;
	int		z;
	vec3_t	org;

	if (!SV_TestEntityPosition(ent))
	{
		VectorCopy (ent->v.origin, ent->v.oldorigin);
		return;
	}

	VectorCopy (ent->v.origin, org);
	VectorCopy (ent->v.oldorigin, ent->v.origin);
	if (!SV_TestEntityPosition(ent))
	{
		Con_DPrintf ("moved back Unstuck.\n");
		SV_LinkEdict (ent, true);
		return;
	}

	for (z = 0; z < 18; z++)
	{
		for (i = -1; i <= 1; i++)
		{
			for (j = -1; j <= 1; j++)
			{
				ent->v.origin[0] = org[0] + i;
				ent->v.origin[1] = org[1] + j;
				ent->v.origin[2] = org[2] + z;
				if (!SV_TestEntityPosition(ent))
				{
					Con_DPrintf ("scooted Unstuck.\n");
					SV_LinkEdict (ent, true);
					return;
				}
			}
		}
	}

	VectorCopy (org, ent->v.origin);
	if (ent->v.oldorigin!=ent->v.origin)
		Con_DPrintf ("player is stuck.\n");
}


/*
=============
SV_CheckWater
=============
*/
static qboolean SV_CheckWater (edict_t *ent)
{
	vec3_t	point;
	int		cont;
#ifdef QUAKE2
	int		truecont;
#endif

	point[0] = ent->v.origin[0];
	point[1] = ent->v.origin[1];
	point[2] = ent->v.origin[2] + ent->v.mins[2] + 1;

	ent->v.waterlevel = 0;
	ent->v.watertype = CONTENTS_EMPTY;
	cont = SV_PointContents (point);
	if (cont <= CONTENTS_WATER)
	{
#ifdef QUAKE2
		truecont = SV_TruePointContents (point);
#endif
		ent->v.watertype = cont;
		ent->v.waterlevel = 1;
		point[2] = ent->v.origin[2] + (ent->v.mins[2] + ent->v.maxs[2])*0.5;
		cont = SV_PointContents (point);
		if (cont <= CONTENTS_WATER)
		{
			ent->v.waterlevel = 2;
			point[2] = ent->v.origin[2] + ent->v.view_ofs[2];
			cont = SV_PointContents (point);
			if (cont <= CONTENTS_WATER)
				ent->v.waterlevel = 3;
		}
#ifdef QUAKE2
		if (truecont <= CONTENTS_CURRENT_0 && truecont >= CONTENTS_CURRENT_DOWN)
		{
			static vec3_t current_table[] =
			{
				{1, 0, 0},
				{0, 1, 0},
				{-1, 0, 0},
				{0, -1, 0},
				{0, 0, 1},
				{0, 0, -1}
			};

			VectorMA (ent->v.basevelocity, 150.0*ent->v.waterlevel/3.0, current_table[CONTENTS_CURRENT_0 - truecont], ent->v.basevelocity);
		}
#endif
	}

	return ent->v.waterlevel > 1;
}


/*
============
SV_WallFriction

============
*/
static void SV_WallFriction (edict_t *ent, trace_t *trace)
{
	vec3_t		forward, right, up;
	float		d, i;
	vec3_t		into, side;

	AngleVectors (ent->v.v_angle, forward, right, up);
	d = DotProduct (trace->plane.normal, forward);

	d += 0.5;
	if (d >= 0)
		return;

// cut the tangential velocity
	i = DotProduct (trace->plane.normal, ent->v.velocity);
	VectorScale (trace->plane.normal, i, into);
	VectorSubtract (ent->v.velocity, into, side);

	ent->v.velocity[0] = side[0] * (1 + d);
	ent->v.velocity[1] = side[1] * (1 + d);
}


/*
=====================
SV_TryUnstick

Player has come to a dead stop, possibly due to the problem with limited
float precision at some angle joins in the BSP hull.

Try fixing by pushing one pixel in each direction.

This is a hack, but in the interest of good gameplay...
======================
*/
static int SV_TryUnstick (edict_t *ent, vec3_t oldvel)
{
	int		i;
	vec3_t	oldorg;
	vec3_t	dir;
	int		clip;
	trace_t	steptrace;

	VectorCopy (ent->v.origin, oldorg);
	VectorClear (dir);

	for (i = 0; i < 8; i++)
	{
	// try pushing a little in an axial direction
		switch (i)
		{
		case 0:
			dir[0] = 2;
			dir[1] = 0;
			break;
		case 1:
			dir[0] = 0;
			dir[1] = 2;
			break;
		case 2:
			dir[0] = -2;
			dir[1] = 0;
			break;
		case 3:
			dir[0] = 0;
			dir[1] = -2;
			break;
		case 4:
			dir[0] = 2;
			dir[1] = 2;
			break;
		case 5:
			dir[0] = -2;
			dir[1] = 2;
			break;
		case 6:
			dir[0] = 2;
			dir[1] = -2;
			break;
		case 7:
			dir[0] = -2;
			dir[1] = -2;
			break;
		}

		SV_PushEntity (ent, dir);

	// retry the original move
		ent->v.velocity[0] = oldvel[0];
		ent->v. velocity[1] = oldvel[1];
		ent->v. velocity[2] = 0;
		clip = SV_FlyMove (ent, 0.1, &steptrace);

		if ( fabs(oldorg[1] - ent->v.origin[1]) > 4
			|| fabs(oldorg[0] - ent->v.origin[0]) > 4 )
		{
		//	Con_DPrintf ("unstuck!\n");
			return clip;
		}

	// go back to the original pos and try again
		VectorCopy (oldorg, ent->v.origin);
	}

	VectorClear (ent->v.velocity);
	return 7;		// still not moving
}


/*
=====================
SV_WalkMove

Only used by players
======================
*/
#define	STEPSIZE	18
static void SV_WalkMove (edict_t *ent)
{
	vec3_t		upmove, downmove;
	vec3_t		oldorg, oldvel;
	vec3_t		nosteporg, nostepvel;
	int			clip;
	int			oldonground;
	trace_t		steptrace, downtrace;

//
// do a regular slide move unless it looks like you ran into a step
//
	oldonground = (int)ent->v.flags & FL_ONGROUND;
	ent->v.flags = (int)ent->v.flags & ~FL_ONGROUND;

	VectorCopy (ent->v.origin, oldorg);
	VectorCopy (ent->v.velocity, oldvel);

	clip = SV_FlyMove (ent, host_frametime, &steptrace);

	if ( !(clip & 2) )
		return;		// move didn't block on a step

	if (!oldonground && ent->v.waterlevel == 0)
		return;		// don't stair up while jumping

	if (ent->v.movetype != MOVETYPE_WALK)
		return;		// gibbed by a trigger

	if (sv_nostep.integer)
		return;

	if ( (int)sv_player->v.flags & FL_WATERJUMP )
		return;

	VectorCopy (ent->v.origin, nosteporg);
	VectorCopy (ent->v.velocity, nostepvel);

//
// try moving up and forward to go up a step
//
	VectorCopy (oldorg, ent->v.origin);	// back to start pos

	VectorClear (upmove);
	VectorClear (downmove);
	upmove[2] = STEPSIZE;
	downmove[2] = -STEPSIZE + oldvel[2]*host_frametime;

// move up
	SV_PushEntity (ent, upmove);	// FIXME: don't link?

// move forward
	ent->v.velocity[0] = oldvel[0];
	ent->v. velocity[1] = oldvel[1];
	ent->v. velocity[2] = 0;
	clip = SV_FlyMove (ent, host_frametime, &steptrace);

// check for stuckness, possibly due to the limited precision of floats
// in the clipping hulls
	if (clip)
	{
		if ( fabs(oldorg[1] - ent->v.origin[1]) < 0.03125
			&& fabs(oldorg[0] - ent->v.origin[0]) < 0.03125 )
		{	// stepping up didn't make any progress
			clip = SV_TryUnstick (ent, oldvel);
		}
	}

// extra friction based on view angle
	if ( clip & 2 )
		SV_WallFriction (ent, &steptrace);

// move down
	downtrace = SV_PushEntity (ent, downmove);	// FIXME: don't link?

	if (downtrace.plane.normal[2] > 0.7)
	{
		if (ent->v.solid == SOLID_BSP)
		{
			ent->v.flags =	(int)ent->v.flags | FL_ONGROUND;
			ent->v.groundentity = EDICT_TO_PROG(downtrace.ent);
		}
	}
	else
	{
// if the push down didn't end up on good ground, use the move without
// the step up.  This happens near wall / slope combinations, and can
// cause the player to hop up higher on a slope too steep to climb
		VectorCopy (nosteporg, ent->v.origin);
		VectorCopy (nostepvel, ent->v.velocity);
	}
}


/*
================
SV_Physics_Client

Player character actions
================
*/
static void SV_Physics_Client (edict_t *ent, int num)
{
	if ( ! svs.clients[num-1].active )
		return;		// unconnected slot

//
// call standard client pre-think
//
	if (old_progdefs)
	{
		pr_global_struct_v111->time = sv.time;
		pr_global_struct_v111->self = EDICT_TO_PROG(ent);
		PR_ExecuteProgram (pr_global_struct_v111->PlayerPreThink);
	}
	else
	{
		pr_global_struct->time = sv.time;
		pr_global_struct->self = EDICT_TO_PROG(ent);
		PR_ExecuteProgram (pr_global_struct->PlayerPreThink);
	}

//
// do a move
//
	SV_CheckVelocity (ent);

//
// decide which move function to call
//
	switch ((int)ent->v.movetype)
	{
	case MOVETYPE_NONE:
		if (!SV_RunThink (ent))
			return;
		break;

	case MOVETYPE_WALK:
		if (!SV_RunThink (ent))
			return;
		if (!SV_CheckWater (ent) && ! ((int)ent->v.flags & FL_WATERJUMP) )
			SV_AddGravity (ent);
		SV_CheckStuck (ent);
#ifdef QUAKE2
		VectorAdd (ent->v.velocity, ent->v.basevelocity, ent->v.velocity);
#endif
		SV_WalkMove (ent);

#ifdef QUAKE2
		VectorSubtract (ent->v.velocity, ent->v.basevelocity, ent->v.velocity);
#endif
		break;

	case MOVETYPE_TOSS:
	case MOVETYPE_BOUNCE:
		SV_Physics_Toss (ent);
		break;

	case MOVETYPE_FLY:
	case MOVETYPE_SWIM:
		if (!SV_RunThink (ent))
			return;
		SV_CheckWater (ent);
		SV_FlyMove (ent, host_frametime, NULL);
		SV_FlyExtras (ent, host_frametime, NULL);  // Hover & friction
		break;

	case MOVETYPE_NOCLIP:
		if (!SV_RunThink (ent))
			return;
		VectorMA (ent->v.origin, host_frametime, ent->v.velocity, ent->v.origin);
		break;

	default:
		Sys_Error ("%s: bad movetype %i", __thisfunc__, (int)ent->v.movetype);
	}

//
// call standard player post-think
//
	SV_LinkEdict (ent, true);

	if (old_progdefs)
	{
		pr_global_struct_v111->time = sv.time;
		pr_global_struct_v111->self = EDICT_TO_PROG(ent);
		PR_ExecuteProgram (pr_global_struct_v111->PlayerPostThink);
	}
	else
	{
		pr_global_struct->time = sv.time;
		pr_global_struct->self = EDICT_TO_PROG(ent);
		PR_ExecuteProgram (pr_global_struct->PlayerPostThink);
	}
}


//============================================================================

/*
=============
SV_Physics_None

Non moving objects can only think
=============
*/
static void SV_Physics_None (edict_t *ent)
{
// regular thinking
	SV_RunThink (ent);
}


#ifdef QUAKE2
/*
=============
SV_Physics_Follow

Entities that are "stuck" to another entity
=============
*/
static void SV_Physics_Follow (edict_t *ent)
{
// regular thinking
	SV_RunThink (ent);
	VectorAdd (PROG_TO_EDICT(ent->v.aiment)->v.origin, ent->v.v_angle, ent->v.origin);
	SV_LinkEdict (ent, true);
}
#endif


/*
=============
SV_Physics_Noclip

A moving object that doesn't obey physics
=============
*/
static void SV_Physics_Noclip (edict_t *ent)
{
// regular thinking
	if (!SV_RunThink (ent))
		return;

	VectorMA (ent->v.angles, host_frametime, ent->v.avelocity, ent->v.angles);
	VectorMA (ent->v.origin, host_frametime, ent->v.velocity, ent->v.origin);

	SV_LinkEdict (ent, false);
}


/*
==============================================================================

TOSS / BOUNCE

==============================================================================
*/

/*
=============
SV_CheckWaterTransition

=============
*/
static void SV_CheckWaterTransition (edict_t *ent)
{
	int		cont;
#ifdef QUAKE2
	vec3_t	point;

	point[0] = ent->v.origin[0];
	point[1] = ent->v.origin[1];
	point[2] = ent->v.origin[2] + ent->v.mins[2] + 1;
	cont = SV_PointContents (point);
#else
	cont = SV_PointContents (ent->v.origin);
#endif
	if (!ent->v.watertype)
	{	// just spawned here
		ent->v.watertype = cont;
		ent->v.waterlevel = 1;
		return;
	}

	if (cont <= CONTENTS_WATER)
	{
		if (ent->v.watertype == CONTENTS_EMPTY)
		{	// just crossed into water
			SV_StartSound (ent, 0, "misc/hith2o.wav", 255, 1);
		}
		ent->v.watertype = cont;
		ent->v.waterlevel = 1;
	}
	else
	{
		if (ent->v.watertype != CONTENTS_EMPTY)
		{	// just crossed into water
			SV_StartSound (ent, 0, "misc/hith2o.wav", 255, 1);
		}
		ent->v.watertype = CONTENTS_EMPTY;
		ent->v.waterlevel = cont;
	}
}


/*
=============
SV_Physics_Toss

Toss, bounce, and fly movement.  When onground, do nothing.
=============
*/
static void SV_Physics_Toss (edict_t *ent)
{
	trace_t	trace;
	vec3_t	move;
	float	backoff;
#ifdef QUAKE2
	edict_t	*groundentity;

	groundentity = PROG_TO_EDICT(ent->v.groundentity);
	if ((int)groundentity->v.flags & FL_CONVEYOR)
		VectorScale(groundentity->v.movedir, groundentity->v.speed, ent->v.basevelocity);
	else
		VectorCopy(vec_origin, ent->v.basevelocity);
	SV_CheckWater (ent);
#endif
	// regular thinking
	if (!SV_RunThink (ent))
		return;

#ifdef QUAKE2
	if (ent->v.velocity[2] > 0)
		ent->v.flags = (int)ent->v.flags & ~FL_ONGROUND;

	if ( ((int)ent->v.flags & FL_ONGROUND) )
//@@
		if (VectorCompare(ent->v.basevelocity, vec_origin))
			return;

	SV_CheckVelocity (ent);

// add gravity
	if (! ((int)ent->v.flags & FL_ONGROUND)
			&& ent->v.movetype != MOVETYPE_FLY
			&& ent->v.movetype != MOVETYPE_BOUNCEMISSILE
			&& ent->v.movetype != MOVETYPE_FLYMISSILE
			&& ent->v.movetype != MOVETYPE_SWIM)
		SV_AddGravity (ent);
#else
// if onground, return without moving
	if ( ((int)ent->v.flags & FL_ONGROUND) )
		return;

	SV_CheckVelocity (ent);

// add gravity
	if (ent->v.movetype != MOVETYPE_FLY
			&& ent->v.movetype != MOVETYPE_BOUNCEMISSILE
			&& ent->v.movetype != MOVETYPE_FLYMISSILE
			&& ent->v.movetype != MOVETYPE_SWIM)
		SV_AddGravity (ent);
#endif

// move angles
	VectorMA (ent->v.angles, host_frametime, ent->v.avelocity, ent->v.angles);

// move origin
#ifdef QUAKE2
	VectorAdd (ent->v.velocity, ent->v.basevelocity, ent->v.velocity);
#endif
	VectorScale (ent->v.velocity, host_frametime, move);
//	VectorCopy(vec_origin,move);
	trace = SV_PushEntity (ent, move);
#ifdef QUAKE2
	VectorSubtract (ent->v.velocity, ent->v.basevelocity, ent->v.velocity);
#endif
	if (trace.fraction == 1)
		return;
	if (ent->free)
		return;

	if (ent->v.movetype == MOVETYPE_BOUNCE)
		backoff = 1.5;
//#ifdef QUAKE2
	else if (ent->v.movetype == MOVETYPE_BOUNCEMISSILE)
	{	// Solid phased missiles don't bounce on monsters or players
		if ( (ent->v.solid == SOLID_PHASE) && (((int)trace.ent->v.flags & FL_MONSTER) || ((int)trace.ent->v.movetype == MOVETYPE_WALK)) )
		{
			return;
		}
		backoff = 2.0;
	}
//#endif
	else
		backoff = 1;

	ClipVelocity (ent->v.velocity, trace.plane.normal, ent->v.velocity, backoff);

// stop if on ground
	if ((trace.plane.normal[2] > 0.7) && (ent->v.movetype != MOVETYPE_BOUNCEMISSILE))
	{
//#ifdef QUAKE2
		if (ent->v.velocity[2] < 60 || ent->v.movetype != MOVETYPE_BOUNCE)
//#else
//		if (ent->v.velocity[2] < 60 || ent->v.movetype != MOVETYPE_BOUNCE)
//#endif
		{
			ent->v.flags = (int)ent->v.flags | FL_ONGROUND;
			ent->v.groundentity = EDICT_TO_PROG(trace.ent);
			VectorClear (ent->v.velocity);
			VectorClear (ent->v.avelocity);
		}
	}

// check for in water
	SV_CheckWaterTransition (ent);
}


/*
===============================================================================

STEPPING MOVEMENT

===============================================================================
*/

/*
=============
SV_Physics_Step

Monsters freefall when they don't have a ground entity, otherwise
all movement is done with discrete steps.

This is also used for objects that have become still on the ground, but
will fall if the floor is pulled out from under them.
=============
*/
#ifdef QUAKE2
static void SV_Physics_Step (edict_t *ent)
{
	qboolean	wasonground;
	qboolean	inwater;
	qboolean	hitsound = false;
	float		*vel;
	float		speed, newspeed, control;
	float		friction;
	edict_t		*groundentity;

	groundentity = PROG_TO_EDICT(ent->v.groundentity);
	if ((int)groundentity->v.flags & FL_CONVEYOR)
		VectorScale(groundentity->v.movedir, groundentity->v.speed, ent->v.basevelocity);
	else
		VectorCopy(vec_origin, ent->v.basevelocity);
//@@
	if (old_progdefs)
	{
		pr_global_struct_v111->time = sv.time;
		pr_global_struct_v111->self = EDICT_TO_PROG(ent);
	}
	else
	{
		pr_global_struct->time = sv.time;
		pr_global_struct->self = EDICT_TO_PROG(ent);
	}
	PF_WaterMove();

	SV_CheckVelocity (ent);

	wasonground = (int)ent->v.flags & FL_ONGROUND;
//	ent->v.flags = (int)ent->v.flags & ~FL_ONGROUND;

	// add gravity except:
	//   flying monsters
	//   swimming monsters who are in the water
	inwater = SV_CheckWater(ent);
	if (! wasonground)
	{
		if ( !((int)ent->v.flags & FL_FLY) )
		{
			if ( !( ((int)ent->v.flags & FL_SWIM) && (ent->v.waterlevel > 0) ) )
			{
				if (ent->v.velocity[2] < sv_gravity.value*-0.1)
					hitsound = true;
				if (!inwater)
					SV_AddGravity (ent);
			}
		}
	}

	if (!VectorCompare(ent->v.velocity, vec_origin) || !VectorCompare(ent->v.basevelocity, vec_origin))
	{
		ent->v.flags = (int)ent->v.flags & ~FL_ONGROUND;
		// apply friction
		// let dead monsters who aren't completely onground slide
		if (wasonground)
		{
			if (!(ent->v.health <= 0.0 && !SV_CheckBottom(ent)))
			{
				vel = ent->v.velocity;
				speed = sqrt(vel[0]*vel[0] +vel[1]*vel[1]);
				if (speed)
				{
					friction = sv_friction.value;

					control = speed < sv_stopspeed.value ? sv_stopspeed.value : speed;
					newspeed = speed - host_frametime*control*friction;

					if (newspeed < 0)
						newspeed = 0;
					newspeed /= speed;

					vel[0] = vel[0] * newspeed;
					vel[1] = vel[1] * newspeed;
				}
			}
		}

		VectorAdd (ent->v.velocity, ent->v.basevelocity, ent->v.velocity);
		SV_FlyMove (ent, host_frametime, NULL);
		VectorSubtract (ent->v.velocity, ent->v.basevelocity, ent->v.velocity);

		// determine if it's on solid ground at all
		{
			vec3_t	mins, maxs, point;
			int		x, y;

			VectorAdd (ent->v.origin, ent->v.mins, mins);
			VectorAdd (ent->v.origin, ent->v.maxs, maxs);

			point[2] = mins[2] - 1;
			for (x = 0; x <= 1; x++)
			{
				for (y = 0; y <= 1; y++)
				{
					point[0] = x ? maxs[0] : mins[0];
					point[1] = y ? maxs[1] : mins[1];
					if (SV_PointContents (point) == CONTENTS_SOLID)
					{
						ent->v.flags = (int)ent->v.flags | FL_ONGROUND;
						break;
					}
				}
			}
		}

		SV_LinkEdict (ent, true);

		if (((int)ent->v.flags & FL_ONGROUND) && (!ent->v.flags & FL_MONSTER))
		{
			if (!wasonground)
				if (hitsound)
					SV_StartSound (ent, 0, "fx/thngland.wav", 255, 1);
		}
	}

// regular thinking
	SV_RunThink (ent);
	SV_CheckWaterTransition (ent);
}
#else
static void SV_Physics_Step (edict_t *ent)
{
	qboolean	hitsound;

// freefall if not onground
	if ( ! ((int)ent->v.flags & (FL_ONGROUND | FL_FLY | FL_SWIM) ) )
	{
		if (ent->v.velocity[2] < sv_gravity.value*-0.1)
			hitsound = true;
		else
			hitsound = false;

		SV_AddGravity (ent);
		SV_CheckVelocity (ent);
		SV_FlyMove (ent, host_frametime, NULL);
		SV_LinkEdict (ent, true);

	//	if ( (int)ent->v.flags & FL_ONGROUND )	// just hit ground
		if (((int)ent->v.flags & FL_ONGROUND) && (!ent->v.flags & FL_MONSTER))
		{
			if (hitsound)
				SV_StartSound (ent, 0, "fx/thngland.wav", 255, 1);
		}
	}

// regular thinking
	SV_RunThink (ent);

	SV_CheckWaterTransition (ent);
}
#endif


//============================================================================

/*
================
SV_Physics

================
*/
void SV_Physics (void)
{
	int		i, c, originMoved;
	edict_t	*ent, *ent2;
	vec3_t	oldOrigin, oldAngle;

// let the progs know that a new frame has started
	if (old_progdefs)
	{
		pr_global_struct_v111->self = EDICT_TO_PROG(sv.edicts);
		pr_global_struct_v111->other = EDICT_TO_PROG(sv.edicts);
		pr_global_struct_v111->time = sv.time;
		PR_ExecuteProgram (pr_global_struct_v111->StartFrame);
	}
	else
	{
		pr_global_struct->self = EDICT_TO_PROG(sv.edicts);
		pr_global_struct->other = EDICT_TO_PROG(sv.edicts);
		pr_global_struct->time = sv.time;
		PR_ExecuteProgram (pr_global_struct->StartFrame);
	}

	//SV_CheckAllEnts ();

//
// treat each object in turn
//
	ent = sv.edicts;
	for (i = 0; i < sv.num_edicts; i++, ent = NEXT_EDICT(ent))
	{
		if (ent->free)
			continue;

		ent2 = PROG_TO_EDICT(ent->v.movechain);
		if (ent2 != sv.edicts)
		{
			VectorCopy(ent->v.origin,oldOrigin);
			VectorCopy(ent->v.angles,oldAngle);
		}

		if (PR_GLOBAL_STRUCT(force_retouch))
		{
			SV_LinkEdict (ent, true);	// force retouch even for stationary
		}

		if (i > 0 && i <= svs.maxclients)
			SV_Physics_Client (ent, i);
		else if (ent->v.movetype == MOVETYPE_PUSH)
			SV_Physics_Pusher (ent);
		else if (ent->v.movetype == MOVETYPE_NONE)
			SV_Physics_None (ent);
#ifdef QUAKE2
		else if (ent->v.movetype == MOVETYPE_FOLLOW)
			SV_Physics_Follow (ent);
#endif
		else if (ent->v.movetype == MOVETYPE_NOCLIP)
			SV_Physics_Noclip (ent);
		else if ((ent->v.movetype == MOVETYPE_STEP) ||
				(ent->v.movetype == MOVETYPE_PUSHPULL))
			SV_Physics_Step (ent);
		else if (ent->v.movetype == MOVETYPE_TOSS
				|| ent->v.movetype == MOVETYPE_BOUNCE
//#ifdef QUAKE2
				|| ent->v.movetype == MOVETYPE_BOUNCEMISSILE
//#endif
				|| ent->v.movetype == MOVETYPE_FLY
				|| ent->v.movetype == MOVETYPE_FLYMISSILE
				|| ent->v.movetype == MOVETYPE_SWIM)
			SV_Physics_Toss (ent);
		else
			Sys_Error ("%s: bad movetype %i", __thisfunc__, (int)ent->v.movetype);

		if (ent2 != sv.edicts)
		{
			originMoved = !VectorCompare(ent->v.origin,oldOrigin);
			if (originMoved || !VectorCompare(ent->v.angles,oldAngle))
			{
				VectorSubtract(ent->v.origin,oldOrigin,oldOrigin);
				VectorSubtract(ent->v.angles,oldAngle,oldAngle);

				for (c = 0; c < 10; c++)
				{	// chain a max of 10 objects
					if (ent2->free)
						break;

					VectorAdd(oldOrigin,ent2->v.origin,ent2->v.origin);
					if ((int)ent2->v.flags & FL_MOVECHAIN_ANGLE)
					{
						VectorAdd(oldAngle,ent2->v.angles,ent2->v.angles);
					}

					if (originMoved && ent2->v.chainmoved)
					{	// callback function
						if (old_progdefs)
						{
							pr_global_struct_v111->self = EDICT_TO_PROG(ent2);
							pr_global_struct_v111->other = EDICT_TO_PROG(ent);
						}
						else
						{
							pr_global_struct->self = EDICT_TO_PROG(ent2);
							pr_global_struct->other = EDICT_TO_PROG(ent);
						}
						PR_ExecuteProgram(ent2->v.chainmoved);
					}

					ent2 = PROG_TO_EDICT(ent2->v.movechain);
					if (ent2 == sv.edicts)
						break;
				}
			}
		}
	}

	if (old_progdefs)
	{
		if (pr_global_struct_v111->force_retouch)
			pr_global_struct_v111->force_retouch--;
	}
	else
	{
		if (pr_global_struct->force_retouch)
			pr_global_struct->force_retouch--;
	}

	sv.time += host_frametime;
}


#ifdef QUAKE2
trace_t SV_Trace_Toss (edict_t *ent, edict_t *ignore)
{
	edict_t	tempent, *tent;
	trace_t	trace;
	vec3_t	move;
	vec3_t	end;
	double	save_frametime;
//	extern particle_t	*active_particles, *free_particles;
//	particle_t	*p;

	save_frametime = host_frametime;
	host_frametime = 0.05;

	memcpy(&tempent, ent, sizeof(edict_t));
	tent = &tempent;

	while (1)
	{
		SV_CheckVelocity (tent);
		SV_AddGravity (tent);
		VectorMA (tent->v.angles, host_frametime, tent->v.avelocity, tent->v.angles);
		VectorScale (tent->v.velocity, host_frametime, move);
		VectorAdd (tent->v.origin, move, end);
		trace = SV_Move (tent->v.origin, tent->v.mins, tent->v.maxs, end, MOVE_NORMAL, tent);
		VectorCopy (trace.endpos, tent->v.origin);

//		p = free_particles;
//		if (p)
//		{
//			free_particles = p->next;
//			p->next = active_particles;
//			active_particles = p;
//
//			p->die = 256;
//			p->color = 15;
//			p->type = pt_static;
//			VectorClear (p->vel);
//			VectorCopy (tent->v.origin, p->org);
//		}

		if (trace.ent)
			if (trace.ent != ignore)
				break;
	}
//	p->color = 224;
	host_frametime = save_frametime;
	return trace;
}
#endif

