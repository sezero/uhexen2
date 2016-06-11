/*
 * pmove.c -- player movement
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


movevars_t	movevars;

playermove_t	pmove;

int			onground;
int			waterlevel;
int			watertype;

static float	frametime;

static vec3_t	forward, right, up;

vec3_t	player_mins = {-16, -16, 0};
vec3_t	player_maxs = {16, 16, 56};

vec3_t  player_maxs_crouch = {16, 16, 28};

vec3_t	beast_mins = {-48, -48, 0};
vec3_t	beast_maxs = {48, 48, 100};
// #define	PM_GRAVITY		800
// #define	PM_STOPSPEED		100
// #define	PM_MAXSPEED		320
// #define	PM_SPECTATORMAXSPEED	500
// #define	PM_ACCELERATE		10
// #define	PM_AIRACCELERATE	0.7
// #define	PM_WATERACCELERATE	10
// #define	PM_FRICTION		6
// #define	PM_WATERFRICTION	1

extern void PM_InitBoxHull (void);

void Pmove_Init (void)
{
	PM_InitBoxHull ();
}

#define	STEPSIZE	18

#define	BUTTON_JUMP	2


/*
==================
PM_ClipVelocity

Slide off of the impacting object
returns the blocked flags (1 = floor, 2 = step / wall)
==================
*/
#define	STOP_EPSILON	0.1

static int PM_ClipVelocity (const vec3_t in, const vec3_t normal, vec3_t out, float overbounce)
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
PM_FlyMove

The basic solid body movement clip that slides along multiple planes
============
*/
#define	MAX_CLIP_PLANES	5

static int PM_FlyMove (void)
{
	int			bumpcount, numbumps;
	vec3_t		dir;
	float		d;
	int			numplanes;
	vec3_t		planes[MAX_CLIP_PLANES];
	vec3_t		primal_velocity, original_velocity;
	int			i, j;
	pmtrace_t		trace;
	vec3_t		end;
	float		time_left;
	int			blocked;

	numbumps = 4;

	blocked = 0;
	VectorCopy (pmove.velocity, original_velocity);
	VectorCopy (pmove.velocity, primal_velocity);
	numplanes = 0;

	time_left = frametime;

	for (bumpcount = 0; bumpcount < numbumps; bumpcount++)
	{
		for (i = 0; i < 3; i++)
			end[i] = pmove.origin[i] + time_left * pmove.velocity[i];

		trace = PM_PlayerMove (pmove.origin, end);

		if (trace.startsolid || trace.allsolid)
		{	// entity is trapped in another solid
			VectorClear (pmove.velocity);
			return 3;
		}

		if (trace.fraction > 0)
		{	// actually covered some distance
			VectorCopy (trace.endpos, pmove.origin);
			numplanes = 0;
		}

		if (trace.fraction == 1)
			 break;		// moved the entire distance

		// save entity for contact
		pmove.touchindex[pmove.numtouch] = trace.ent;
		pmove.numtouch++;

		if (trace.plane.normal[2] > 0.7)
		{
			blocked |= 1;	// floor
		}
		if (!trace.plane.normal[2])
		{
			blocked |= 2;	// step
		}

		time_left -= time_left * trace.fraction;

		// cliped to another plane
		if (numplanes >= MAX_CLIP_PLANES)
		{	// this shouldn't really happen
			VectorClear (pmove.velocity);
			break;
		}

		VectorCopy (trace.plane.normal, planes[numplanes]);
		numplanes++;

//
// modify original_velocity so it parallels all of the clip planes
//
		for (i = 0; i < numplanes; i++)
		{
			PM_ClipVelocity (original_velocity, planes[i], pmove.velocity, 1);
			for (j = 0; j < numplanes; j++)
			{
				if (j != i)
				{
					if (DotProduct (pmove.velocity, planes[j]) < 0)
						break;	// not ok
				}
			}

			if (j == numplanes)
				break;
		}

		if (i != numplanes)
		{	// go along this plane
		}
		else
		{	// go along the crease
			if (numplanes != 2)
			{
//				Con_Printf ("clip velocity, numplanes == %i\n",numplanes);
				VectorClear (pmove.velocity);
				break;
			}
			CrossProduct (planes[0], planes[1], dir);
			d = DotProduct (dir, pmove.velocity);
			VectorScale (dir, d, pmove.velocity);
		}

//
// if original velocity is against the original velocity, stop dead
// to avoid tiny occilations in sloping corners
//
		if (DotProduct (pmove.velocity, primal_velocity) <= 0)
		{
			VectorClear (pmove.velocity);
			break;
		}
	}

	if (pmove.waterjumptime)
	{
		VectorCopy (primal_velocity, pmove.velocity);
	}
	return blocked;
}

/*
=============
PM_GroundMove

Player is on ground, with no upwards velocity
=============
*/
static void PM_GroundMove (void)
{
	vec3_t	start, dest;
	pmtrace_t	trace;
	vec3_t	original, originalvel, down, downvel;
	//vec3_t	up;
	float	downdist, updist;

	pmove.velocity[2] = 0;
	if (!pmove.velocity[0] && !pmove.velocity[1] && !pmove.velocity[2])
		return;

	// first try just moving to the destination
	dest[0] = pmove.origin[0] + pmove.velocity[0]*frametime;
	dest[1] = pmove.origin[1] + pmove.velocity[1]*frametime;
	dest[2] = pmove.origin[2];

	// first try moving directly to the next spot
	VectorCopy (dest, start);
	(void) start; /* variable set but not used */
	trace = PM_PlayerMove (pmove.origin, dest);
	if (trace.fraction == 1)
	{
		VectorCopy (trace.endpos, pmove.origin);
		return;
	}

	// try sliding forward both on ground and up 16 pixels
	// take the move that goes farthest
	VectorCopy (pmove.origin, original);
	VectorCopy (pmove.velocity, originalvel);

	// slide move
	PM_FlyMove ();

	VectorCopy (pmove.origin, down);
	VectorCopy (pmove.velocity, downvel);

	VectorCopy (original, pmove.origin);
	VectorCopy (originalvel, pmove.velocity);

// move up a stair height
	VectorCopy (pmove.origin, dest);
	dest[2] += STEPSIZE;
	trace = PM_PlayerMove (pmove.origin, dest);
	if (!trace.startsolid && !trace.allsolid)
	{
		VectorCopy (trace.endpos, pmove.origin);
	}

// slide move
	PM_FlyMove ();

// press down the stepheight
	VectorCopy (pmove.origin, dest);
	dest[2] -= STEPSIZE;
	trace = PM_PlayerMove (pmove.origin, dest);
	if ( trace.plane.normal[2] < 0.7)
		goto usedown;
	if (!trace.startsolid && !trace.allsolid)
	{
		VectorCopy (trace.endpos, pmove.origin);
	}
	VectorCopy (pmove.origin, up);

	// decide which one went farther
	downdist = (down[0] - original[0])*(down[0] - original[0])
			+ (down[1] - original[1])*(down[1] - original[1]);
	updist = (up[0] - original[0])*(up[0] - original[0])
			+ (up[1] - original[1])*(up[1] - original[1]);

	if (downdist > updist)
	{
usedown:
		VectorCopy (down, pmove.origin);
		VectorCopy (downvel, pmove.velocity);
	} else	// copy z value from slide move
		pmove.velocity[2] = downvel[2];

// if at a dead stop, retry the move with nudges to get around lips

}


/*
==================
PM_Friction

Handles both ground friction and water friction
==================
*/
static void PM_Friction (void)
{
	float	*vel;
	float	speed, newspeed, control;
	float	friction;
	float	drop;

	if (pmove.waterjumptime)
		return;

	vel = pmove.velocity;

	speed = sqrt(vel[0]*vel[0] +vel[1]*vel[1] + vel[2]*vel[2]);
	if (speed < 1)
	{
		vel[0] = 0;
		vel[1] = 0;
		return;
	}

	drop = 0;

// apply ground friction
	if ((onground != -1) || (pmove.movetype == MOVETYPE_FLY))
	{
		friction = movevars.friction;
		control = speed < movevars.stopspeed ? movevars.stopspeed : speed;
		drop += control*friction*frametime;
	}

// apply water friction
	if (waterlevel)
		drop += speed*movevars.waterfriction*waterlevel*frametime;

// scale the velocity
	newspeed = speed - drop;
	if (newspeed < 0)
		newspeed = 0;
	newspeed /= speed;

	vel[0] = vel[0] * newspeed;
	vel[1] = vel[1] * newspeed;
	vel[2] = vel[2] * newspeed;
}


/*
==============
PM_Accelerate
==============
*/
static void PM_Accelerate (const vec3_t wishdir, float wishspeed, float accel)
{
	int		i;
	float		addspeed, accelspeed, currentspeed;

	if (pmove.dead)
		return;
	if (pmove.waterjumptime)
		return;

	currentspeed = DotProduct (pmove.velocity, wishdir);
	addspeed = wishspeed - currentspeed;
	if (addspeed <= 0)
		return;
	accelspeed = accel*frametime*wishspeed;
	if (accelspeed > addspeed)
		accelspeed = addspeed;

	for (i = 0; i < 3; i++)
		pmove.velocity[i] += accelspeed*wishdir[i];
}

static void PM_AirAccelerate (const vec3_t wishdir, float wishspeed, float accel)
{
	int			i;
	float		addspeed, accelspeed, currentspeed, wishspd = wishspeed;

	if (pmove.dead)
		return;
	if (pmove.waterjumptime)
		return;

	if (wishspd > 30)
		wishspd = 30;
	currentspeed = DotProduct (pmove.velocity, wishdir);
	addspeed = wishspd - currentspeed;
	if (addspeed <= 0)
		return;
	accelspeed = accel * wishspeed * frametime;
	if (accelspeed > addspeed)
		accelspeed = addspeed;

	for (i = 0; i < 3; i++)
		pmove.velocity[i] += accelspeed*wishdir[i];
}


/*
===================
PM_WaterMove

===================
*/
static void PM_WaterMove (void)
{
	int		i;
	vec3_t	wishvel;
	float	wishspeed;
	vec3_t	wishdir;
	vec3_t	start, dest;
	pmtrace_t	trace;
	float	fmove, smove, clamp;

//
// user intentions
//
	fmove = pmove.cmd.forwardmove;
	smove = pmove.cmd.sidemove;

	// client 400 (assasin running) is scaled to maxspeed
	// clamp is tested seperately so strafe running works
	clamp = movevars.maxspeed * pmove.hasted;
	if (fabs(fmove) > clamp)
	{
		if (fmove < 0)
			fmove = -clamp;
		else
			fmove = clamp;
	}

	if (fabs(smove) > clamp)
	{
		if (smove < 0)
			smove = -clamp;
		else
			smove = clamp;
	}

	if (pmove.crouched)
	{
		fmove = fmove / 600 * movevars.maxspeed;
		smove = smove / 600 * movevars.maxspeed;
	}
	else
	{
		fmove = fmove / 400 * movevars.maxspeed;
		smove = smove / 400 * movevars.maxspeed;
	}

	for (i = 0; i < 3; i++)
		wishvel[i] = forward[i]*fmove + right[i]*smove;

	if ((!pmove.cmd.forwardmove && !pmove.cmd.sidemove && !pmove.cmd.upmove)&&pmove.crouched)
		wishvel[2] -= 120;	// drift towards bottom
	else if ((!pmove.cmd.forwardmove && !pmove.cmd.sidemove && !pmove.cmd.upmove)||pmove.crouched)
		wishvel[2] -= 60;	// drift towards bottom
	else
		wishvel[2] += pmove.cmd.upmove;

	VectorCopy (wishvel, wishdir);
	wishspeed = VectorNormalize(wishdir);

	if (wishspeed > (movevars.maxspeed * pmove.hasted))
	{
		VectorScale (wishvel, (movevars.maxspeed * pmove.hasted)/wishspeed, wishvel);
		wishspeed = (movevars.maxspeed * pmove.hasted);
	}
	wishspeed *= 0.7;

	if (wishspeed > (movevars.maxspeed * pmove.hasted))
	{
		VectorScale (wishvel, (movevars.maxspeed * pmove.hasted)/wishspeed, wishvel);
		wishspeed = (movevars.maxspeed * pmove.hasted);
	}
//
// water acceleration
//
//	if (pmove.waterjumptime)
//		Con_Printf ("wm->%f, %f, %f\n", pmove.velocity[0], pmove.velocity[1], pmove.velocity[2]);
	PM_Accelerate (wishdir, wishspeed, movevars.wateraccelerate);

// assume it is a stair or a slope, so press down from stepheight above
	VectorMA (pmove.origin, frametime, pmove.velocity, dest);
	VectorCopy (dest, start);
	start[2] += STEPSIZE + 1;
	trace = PM_PlayerMove (start, dest);
	if (!trace.startsolid && !trace.allsolid)	// FIXME: check steep slope?
	{	// walked up the step
		VectorCopy (trace.endpos, pmove.origin);
		return;
	}

	PM_FlyMove ();
//	if (pmove.waterjumptime)
//		Con_Printf ("<-wm%f, %f, %f\n", pmove.velocity[0], pmove.velocity[1], pmove.velocity[2]);
}


/*
===================
PM_AirMove

===================
*/
static void PM_AirMove (void)
{
	int			i;
	vec3_t		wishvel;
	float		fmove, smove;
	vec3_t		wishdir;
	float		wishspeed, clamp;

	if (pmove.teleport_time < realtime)
	{	//no Mario Bros. stop in mid air!
		fmove = pmove.cmd.forwardmove;
		smove = pmove.cmd.sidemove;
	}
	else
		fmove = smove = 0;

	// client 400 (assasin running) is scaled to maxspeed
	// clamp is tested seperately so strafe running works
	clamp = movevars.maxspeed * pmove.hasted;
	if (fabs(fmove) > clamp)
	{
		if (fmove < 0)
			fmove = -clamp;
		else
			fmove = clamp;
	}

	if (fabs(smove) > clamp)
	{
		if (smove < 0)
			smove = -clamp;
		else
			smove = clamp;
	}

	fmove = fmove / 400 * movevars.maxspeed;
	smove = smove / 400 * movevars.maxspeed;

//	Con_Printf("fmove %f\n", fmove);

	forward[2] = 0;
	right[2] = 0;
	VectorNormalize (forward);
	VectorNormalize (right);

	for (i = 0; i < 2; i++)
		wishvel[i] = forward[i]*fmove*pmove.hasted + right[i]*smove*pmove.hasted;
	wishvel[2] = 0;

	VectorCopy (wishvel, wishdir);
	wishspeed = VectorNormalize(wishdir);

//
// clamp to server defined max speed
//
	if (wishspeed > (movevars.maxspeed * pmove.hasted))
	{
		VectorScale (wishvel, (movevars.maxspeed * pmove.hasted)/wishspeed, wishvel);
		wishspeed = (movevars.maxspeed * pmove.hasted);
	}

//	if (pmove.waterjumptime)
//		Con_Printf ("am->%f, %f, %f\n", pmove.velocity[0], pmove.velocity[1], pmove.velocity[2]);

	if ( onground != -1)
	{
		pmove.velocity[2] = 0;
		PM_Accelerate (wishdir, wishspeed, movevars.accelerate);
		PM_GroundMove ();
	}
	else
	{	// not on ground, so little effect on velocity
		PM_AirAccelerate (wishdir, wishspeed, movevars.accelerate);
		PM_FlyMove ();
		// add gravity
		pmove.velocity[2] -= movevars.entgravity * movevars.gravity * frametime;
	}

//	Con_Printf("airmove:vec: %4.2f %4.2f %4.2f\n",
//			pmove.velocity[0], pmove.velocity[1], pmove.velocity[2]);

//	if (pmove.waterjumptime)
//		Con_Printf ("<-am%f, %f, %f\n", pmove.velocity[0], pmove.velocity[1], pmove.velocity[2]);
}


/*
===================
PM_FlyingMove - for when you are using the ring of flight

===================
*/
static void PM_FlyingMove (void)
{
	int		i;
	vec3_t		wishvel;
	float		fmove, smove, umove;
	vec3_t		wishdir;
	float		wishspeed, clamp;

	fmove = pmove.cmd.forwardmove;
	smove = pmove.cmd.sidemove;
	umove = pmove.cmd.upmove;

	// client 400 (assasin running) is scaled to maxspeed
	// clamp is tested seperately so strafe running works
	clamp = movevars.maxspeed * pmove.hasted;
	if (fabs(fmove) > clamp)
	{
		if (fmove < 0)
			fmove = -clamp;
		else
			fmove = clamp;
	}

	if (fabs(smove) > clamp)
	{
		if (smove < 0)
			smove = -clamp;
		else
			smove = clamp;
	}
	if (fabs(umove) > clamp)
	{
		if (umove < 0)
			umove = -clamp;
		else
			umove = clamp;
	}

	fmove = fmove / 400 * movevars.maxspeed;
	smove = smove / 400 * movevars.maxspeed;
	umove = umove / 400 * movevars.maxspeed;

//	Con_Printf("fmove %f\n", fmove);

	VectorNormalize (forward);
	VectorNormalize (right);
	VectorNormalize (up);

	for (i = 0; i < 3; i++)
		wishvel[i] = forward[i]*fmove + right[i]*smove + up[i]*umove;

	VectorCopy (wishvel, wishdir);
	wishspeed = VectorNormalize(wishdir);

//
// clamp to server defined max speed
//
/*	if (wishspeed > (movevars.maxspeed * pmove.hasted))
	{
		VectorScale (wishvel, (movevars.maxspeed * pmove.hasted)/wishspeed, wishvel);
		wishspeed = (movevars.maxspeed * pmove.hasted);
	}
*/
//	if (pmove.waterjumptime)
//		Con_Printf ("am->%f, %f, %f\n", pmove.velocity[0], pmove.velocity[1], pmove.velocity[2]);

	PM_Accelerate (wishdir, wishspeed, movevars.accelerate);
	PM_FlyMove ();

//	Con_Printf("airmove:vec: %4.2f %4.2f %4.2f\n",
//			pmove.velocity[0], pmove.velocity[1], pmove.velocity[2]);

//	if (pmove.waterjumptime)
//		Con_Printf ("<-am%f, %f, %f\n", pmove.velocity[0], pmove.velocity[1], pmove.velocity[2]);
}


/*
=============
PM_CatagorizePosition
=============
*/
static void PM_CatagorizePosition (void)
{
	vec3_t		point;
	int			cont;
	pmtrace_t		tr;

// if the player hull point one unit down is solid, the player
// is on ground

// see if standing on something solid
	point[0] = pmove.origin[0];
	point[1] = pmove.origin[1];
	point[2] = pmove.origin[2] - 1;
	if (pmove.velocity[2] > 180)
	{
		onground = -1;
	}
	else
	{
		tr = PM_PlayerMove (pmove.origin, point);
		if ( tr.plane.normal[2] < 0.7)
			onground = -1;	// too steep
		else
			onground = tr.ent;

		if (onground != -1)
		{
			pmove.waterjumptime = 0;
			if (!tr.startsolid && !tr.allsolid)
				VectorCopy (tr.endpos, pmove.origin);
		}

		// standing on an entity other than the world
		if (tr.ent > 0)
		{
			pmove.touchindex[pmove.numtouch] = tr.ent;
			pmove.numtouch++;
		}
	}

//
// get waterlevel
//
	waterlevel = 0;
	watertype = CONTENTS_EMPTY;

	point[2] = pmove.origin[2] + player_mins[2] + 1;
	cont = PM_PointContents (point);

	if (cont <= CONTENTS_WATER)
	{
		watertype = cont;
		waterlevel = 1;
		point[2] += 26;
		cont = PM_PointContents (point);
		if (cont <= CONTENTS_WATER)
		{
			waterlevel = 2;
			point[2] += 22;
			cont = PM_PointContents (point);
			if (cont <= CONTENTS_WATER)
				waterlevel = 3;
		}
	}
}


/*
=============
JumpButton
=============
*/
static void JumpButton (void)
{
	if (pmove.dead)
	{
		pmove.oldbuttons |= BUTTON_JUMP;	// don't jump again until released
		return;
	}

	if (pmove.waterjumptime)
	{
		pmove.waterjumptime -= frametime;
		if (pmove.waterjumptime < 0)
			pmove.waterjumptime = 0;
		return;
	}

	if (waterlevel >= 2)
	{	// swimming, not jumping
		onground = -1;

		if (watertype == CONTENTS_WATER)
			pmove.velocity[2] = 100;
		else if (watertype == CONTENTS_SLIME)
			pmove.velocity[2] = 80;
		else
			pmove.velocity[2] = 50;
		return;
	}

	if (onground == -1)
		return;		// in air, so no effect

	if ( pmove.oldbuttons & BUTTON_JUMP )
		return;		// don't pogo stick

	onground = -1;
	pmove.velocity[2] += 270;

	pmove.oldbuttons |= BUTTON_JUMP;	// don't jump again until released
}

/*
=============
CheckWaterJump
=============
*/
static void CheckWaterJump (void)
{
	vec3_t	spot;
	int		cont;
	vec3_t	flatforward;

	if (pmove.waterjumptime)
		return;

//	Con_Printf("wjvel: %4.2f %4.2f %4.2f: ",
//		pmove.velocity[0], pmove.velocity[1], pmove.velocity[2]);

	// ZOID, don't hop out if we just jumped in
	if (pmove.velocity[2] < -180)
	{
		//Con_Printf("downcut\n");
		return;	// only hop out if we are moving up
	}

	// see if near an edge
	flatforward[0] = forward[0];
	flatforward[1] = forward[1];
	flatforward[2] = 0;
	VectorNormalize (flatforward);

	VectorMA (pmove.origin, 24, flatforward, spot);
	spot[2] += 32;
	cont = PM_PointContents (spot);
	if (cont != CONTENTS_SOLID) 
	{
		//Con_Printf("notsolid\n");
		return;
	}
	spot[2] += 24;
	cont = PM_PointContents (spot);
	if (cont != CONTENTS_EMPTY)
	{
		//Con_Printf("notempty\n");
		return;
	}
	// jump out of water
	VectorScale (forward, 200, pmove.velocity);
	pmove.velocity[2] = 275;
	pmove.waterjumptime = 2;	// safety net
	pmove.oldbuttons |= BUTTON_JUMP;	// don't jump again until released

//	Con_Printf ("Waterjump (%6.2f,%6.2f,%6.2f) (%6.2f,%6.2f,%6.2f)\n",
//		pmove.origin[0], pmove.origin[1], pmove.origin[2],
//		pmove.velocity[0],pmove.velocity[1],pmove.velocity[2]);
}


/*
=================
NudgePosition

If pmove.origin is in a solid position,
try nudging slightly on all axis to
allow for the cut precision of the net coordinates
=================
*/
static void NudgePosition (void)
{
	vec3_t	base;
	int		x, y, z;
	int		i;

	VectorCopy (pmove.origin, base);

	for (i = 0; i < 3; i++)
		pmove.origin[i] = ((int)(pmove.origin[i]*8)) * 0.125;
	pmove.origin[2] += 0.124;
//	pmove.origin[2] += 1.124;

//	if (pmove.dead)
//		return;		// might be a squished point, so don'y bother
	if (PM_TestPlayerPosition (pmove.origin) )
		return;

	for (z = -1; z <= 1; z++)
	{
		for (x = -1; x <= 1; x++)
		{
			for (y = -1; y <= 1; y++)
			{
				pmove.origin[0] = base[0] + x * 1.0/8;
				pmove.origin[1] = base[1] + y * 1.0/8;
				pmove.origin[2] = base[2] + z * 1.0/8;
				if (PM_TestPlayerPosition (pmove.origin))
					return;
			}
		}
	}
	VectorCopy (base, pmove.origin);
//	Con_DPrintf ("NudgePosition: stuck\n");
}

/*
===============
SpectatorMove
===============
*/
static void SpectatorMove (void)
{
	float	speed, drop, friction, control, newspeed;
	float	currentspeed, addspeed, accelspeed;
	int		i;
	vec3_t		wishvel;
	float		fmove, smove;
	vec3_t		wishdir;
	float		wishspeed;

	// friction
	speed = VectorLength (pmove.velocity);
	if (speed < 1)
	{
		VectorClear (pmove.velocity);
	}
	else
	{
		drop = 0;

		friction = movevars.friction*1.5;	// extra friction
		control = speed < movevars.stopspeed ? movevars.stopspeed : speed;
		drop += control*friction*frametime;

		// scale the velocity
		newspeed = speed - drop;
		if (newspeed < 0)
			newspeed = 0;
		newspeed /= speed;

		VectorScale (pmove.velocity, newspeed, pmove.velocity);
	}

	// accelerate
	fmove = pmove.cmd.forwardmove;
	smove = pmove.cmd.sidemove;

	VectorNormalize (forward);
	VectorNormalize (right);

	for (i = 0; i < 3; i++)
		wishvel[i] = forward[i]*fmove + right[i]*smove;
	wishvel[2] += pmove.cmd.upmove;

	VectorCopy (wishvel, wishdir);
	wishspeed = VectorNormalize(wishdir);

	//
	// clamp to server defined max speed
	//
	if (wishspeed > movevars.spectatormaxspeed)
	{
		VectorScale (wishvel, movevars.spectatormaxspeed/wishspeed, wishvel);
		wishspeed = movevars.spectatormaxspeed;
	}

	currentspeed = DotProduct(pmove.velocity, wishdir);
	addspeed = wishspeed - currentspeed;
	if (addspeed <= 0)
		return;
	accelspeed = movevars.accelerate*frametime*wishspeed;
	if (accelspeed > addspeed)
		accelspeed = addspeed;

	for (i = 0; i < 3; i++)
		pmove.velocity[i] += accelspeed*wishdir[i];

	// move
	VectorMA (pmove.origin, frametime, pmove.velocity, pmove.origin);
}


/*
=============
PlayerMove

Returns with origin, angles, and velocity modified in place.

Numtouch and touchindex[] will be set if any of the physents
were contacted during the move.
=============
*/
void PlayerMove (void)
{
	frametime = pmove.cmd.msec * 0.001;
	pmove.numtouch = 0;

	if (pmove.movetype == MOVETYPE_NONE)
	{
		pmove.oldbuttons = 0;
		pmove.cmd.buttons = 0;
		return;
	}

	AngleVectors (pmove.angles, forward, right, up);

	if (pmove.spectator)
	{
		SpectatorMove ();
		return;
	}

	NudgePosition ();

	// take angles directly from command
	VectorCopy (pmove.cmd.angles, pmove.angles);

	// set onground, watertype, and waterlevel
	PM_CatagorizePosition ();

	if (waterlevel == 1)
		CheckWaterJump ();

	if (pmove.velocity[2] < 0 && pmove.waterjumptime)
	{
		pmove.waterjumptime = 0;
//		Con_Printf ("Waterjump *OVER*\n");
	}

	if (pmove.cmd.buttons & BUTTON_JUMP)
		JumpButton ();
	else
		pmove.oldbuttons &= ~BUTTON_JUMP;

	PM_Friction ();

	if (waterlevel >= 2)
	{
		PM_WaterMove ();
	}
	else
	{
		if (pmove.movetype == MOVETYPE_FLY)
			PM_FlyingMove();
		else
			PM_AirMove ();
//		else//WHY THE FUCK DO I KEEP MOVING ANYWAY?!!!!  WORKED IN MP!!!
//			Con_Printf("No Air move, teleport_time > realtime\n");
	}

	// set onground, watertype, and waterlevel for final spot
	PM_CatagorizePosition ();
}

