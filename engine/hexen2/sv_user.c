/*
 * sv_user.c -- server code for moving users
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

edict_t	*sv_player = NULL;

extern	cvar_t	sv_friction;
cvar_t	sv_edgefriction = {"edgefriction", "2", CVAR_NONE};
cvar_t	sv_altnoclip = {"sv_altnoclip", "1", CVAR_ARCHIVE};
extern	cvar_t	sv_stopspeed;

static	vec3_t		forward, right, up;

static	vec3_t		wishdir;
static	float		wishspeed;

// world
static	float		*angles;
static	float		*origin;
static	float		*velocity;

static	qboolean	onground;

static	usercmd_t	cmd;

cvar_t	sv_idealpitchscale = {"sv_idealpitchscale", "0.8", CVAR_NONE};
cvar_t	sv_idealrollscale = {"sv_idealrollscale", "0.8", CVAR_NONE};

#if defined(SERVERONLY)
static	cvar_t	cl_rollspeed = {"cl_rollspeed", "200", CVAR_NONE};
static	cvar_t	cl_rollangle = {"cl_rollangle", "2.0", CVAR_NONE};
#else
extern	cvar_t	cl_rollspeed;
extern	cvar_t	cl_rollangle;
#endif


/*
===============
SV_SetIdealPitch
===============
*/
#define	MAX_FORWARD	6
void SV_SetIdealPitch (void)
{
	float	angleval, sinval, cosval;
	trace_t	tr;
	vec3_t	top, bottom;
	float	z[MAX_FORWARD];
	int		i, j;
	int		step, dir, steps;
	float	save_hull;

	if (!((int)sv_player->v.flags & FL_ONGROUND))
		return;

	if (sv_player->v.movetype == MOVETYPE_FLY)
		return;

	angleval = sv_player->v.angles[YAW] * M_PI*2 / 360;
	sinval = sin(angleval);
	cosval = cos(angleval);

	save_hull = sv_player->v.hull;
	sv_player->v.hull = 0;

	for (i = 0; i < MAX_FORWARD; i++)
	{
		top[0] = sv_player->v.origin[0] + cosval*(i+3)*12;
		top[1] = sv_player->v.origin[1] + sinval*(i+3)*12;
		top[2] = sv_player->v.origin[2] + sv_player->v.view_ofs[2];

		bottom[0] = top[0];
		bottom[1] = top[1];
		bottom[2] = top[2] - 160;

		tr = SV_Move (top, vec3_origin, vec3_origin, bottom, 1, sv_player);
		if ((tr.allsolid) || // looking at a wall, leave ideal the way is was
			(tr.fraction == 1))// near a dropoff
		{
			sv_player->v.hull = save_hull;
			return;
		}
		z[i] = top[2] + tr.fraction*(bottom[2]-top[2]);
	}

	sv_player->v.hull = save_hull;	//restore

	dir = 0;
	steps = 0;
	for (j = 1; j < i; j++)
	{
		step = z[j] - z[j-1];
		if (step > -ON_EPSILON && step < ON_EPSILON)
			continue;

		if (dir && ( step-dir > ON_EPSILON || step-dir < -ON_EPSILON ) )
			return;		// mixed changes

		steps++;
		dir = step;
	}

	if (!dir)
	{
		sv_player->v.idealpitch = 0;
		return;
	}

	if (steps < 2)
		return;

	sv_player->v.idealpitch = -dir * sv_idealpitchscale.value;
}


/*
==================
SV_UserFriction

==================
*/
static void SV_UserFriction (void)
{
	float	*vel;
	float	speed, newspeed, control;
	vec3_t	start, stop;
	float	friction;
	trace_t	trace;
	float	save_hull;

	vel = velocity;

	speed = sqrt(vel[0]*vel[0] +vel[1]*vel[1]);
	if (!speed)
		return;

	// if the leading edge is over a dropoff, increase friction
	start[0] = stop[0] = origin[0] + vel[0]/speed*16;
	start[1] = stop[1] = origin[1] + vel[1]/speed*16;
	start[2] = origin[2] + sv_player->v.mins[2];
	stop[2] = start[2] - 34;

	save_hull = sv_player->v.hull;
	sv_player->v.hull = 0;
	trace = SV_Move (start, vec3_origin, vec3_origin, stop, true, sv_player);
	sv_player->v.hull = save_hull;

// Hexen II v1.11 progs doesn't initialize the friction properly (always 0).
// In fact, applying a patch to the progs can easily solve this issue, but
// in that case several unpatched mods would be left broken. Note: Compared
// to the AoT solution, the USE_AOT_FRICTION 0  option makes pure hexen2 to
// feel slightly more slippery.
#if USE_AOT_FRICTION
	if (progs->crc != PROGS_V112_CRC)
		friction = 6;
	else
	{
		if (trace.fraction == 1.0)
			friction = sv_friction.value*sv_edgefriction.value*sv_player->v.friction;
		else
			friction = sv_friction.value*sv_player->v.friction;
	}
#else	/* not using AoT friction */
	if (progs->crc != PROGS_V112_CRC)
		sv_player->v.friction = 1.0f;

	if (trace.fraction == 1.0)
		friction = sv_friction.value*sv_edgefriction.value*sv_player->v.friction;
	else
		friction = sv_friction.value*sv_player->v.friction;
#endif

// These lines were found as commented out in Raven's H2MP source. Probably
// it was their pre-mission pack solution.  See above for the current stuff
// and for the explanation.
//	if (sv_player->v.friction != 1)	//reset their friction to 1, only a trigger touching can change it again
//		sv_player->v.friction = 1;

	// apply friction
	control = speed < sv_stopspeed.value ? sv_stopspeed.value : speed;
	newspeed = speed - host_frametime*control*friction;

	if (newspeed < 0)
		newspeed = 0;
	newspeed /= speed;

	vel[0] = vel[0] * newspeed;
	vel[1] = vel[1] * newspeed;
	vel[2] = vel[2] * newspeed;
}


/*
==============
SV_Accelerate
==============
*/

//cvar_t	sv_maxspeed = {"sv_maxspeed", "320", CVAR_NOTIFY|CVAR_SERVERINFO};
cvar_t	sv_maxspeed = {"sv_maxspeed", "640", CVAR_NOTIFY|CVAR_SERVERINFO};
cvar_t	sv_accelerate = {"sv_accelerate", "10", CVAR_NONE};

/* Old values before the id 1.07 update
cvar_t	sv_maxspeed = {"sv_maxspeed", "640", CVAR_NOTIFY|CVAR_SERVERINFO};
cvar_t	sv_accelerate = {"sv_accelerate", "100", CVAR_NONE};
*/

#if 0
static void SV_Accelerate (vec3_t wishvel)
{
	int			i;
	float		addspeed, accelspeed;
	vec3_t		pushvec;

	if (wishspeed == 0)
		return;

	VectorSubtract (wishvel, velocity, pushvec);
	addspeed = VectorNormalize (pushvec);

	accelspeed = sv_accelerate.value*host_frametime*addspeed;
	if (accelspeed > addspeed)
		accelspeed = addspeed;

	for (i = 0; i < 3; i++)
		velocity[i] += accelspeed*pushvec[i];
}
#endif
static void SV_Accelerate (void)
{
	int			i;
	float		addspeed, accelspeed, currentspeed;

	currentspeed = DotProduct (velocity, wishdir);
	addspeed = wishspeed - currentspeed;
	if (addspeed <= 0)
		return;
	accelspeed = sv_accelerate.value*host_frametime*wishspeed;
	if (accelspeed > addspeed)
		accelspeed = addspeed;

	for (i = 0; i < 3; i++)
		velocity[i] += accelspeed*wishdir[i];
}


static void SV_AirAccelerate (vec3_t wishveloc)
{
	int			i;
	float		addspeed, wishspd, accelspeed, currentspeed;

	wishspd = VectorNormalize (wishveloc);
	if (wishspd > 30)
		wishspd = 30;
	currentspeed = DotProduct (velocity, wishveloc);
	addspeed = wishspd - currentspeed;
	if (addspeed <= 0)
		return;
//	accelspeed = sv_accelerate.value * host_frametime;
	accelspeed = sv_accelerate.value*wishspeed * host_frametime;
	if (accelspeed > addspeed)
		accelspeed = addspeed;

	for (i = 0; i < 3; i++)
		velocity[i] += accelspeed*wishveloc[i];
}


static void DropPunchAngle (void)
{
	float	len;

	len = VectorNormalize (sv_player->v.punchangle);

	len -= 10*host_frametime;
	if (len < 0)
		len = 0;
	VectorScale (sv_player->v.punchangle, len, sv_player->v.punchangle);
}


/*
===================
SV_FlightMove

this is just the same as SV_WaterMove but
with a few changes to make it flight
===================
*/
static void SV_FlightMove (void)
{
	int		i;
	vec3_t	wishvel;
	float	speed, newspeed, addspeed, accelspeed;
	//float	wishspeed;

#if !defined(SERVERONLY)
	cl.nodrift = false;
	cl.driftmove = 0;
#endif

//
// user intentions
//
	AngleVectors (sv_player->v.v_angle, forward, right, up);

	for (i = 0; i < 3; i++)
		wishvel[i] = forward[i]*cmd.forwardmove + right[i]*cmd.sidemove + up[i]* cmd.upmove;

	wishspeed = VectorLength(wishvel);
	if (wishspeed > sv_maxspeed.value)
	{
		VectorScale (wishvel, sv_maxspeed.value/wishspeed, wishvel);
		wishspeed = sv_maxspeed.value;
	}

//
// water friction
//
	speed = VectorLength (velocity);
	if (speed)
	{
		newspeed = speed - host_frametime * speed * sv_friction.value;
		if (newspeed < 0)
			newspeed = 0;
		VectorScale (velocity, newspeed/speed, velocity);
	}
	else
		newspeed = 0;

//
// water acceleration
//
	if (!wishspeed)
		return;

	addspeed = wishspeed - newspeed;
	if (addspeed <= 0)
		return;

	VectorNormalize (wishvel);
	accelspeed = sv_accelerate.value * wishspeed * host_frametime;
	if (accelspeed > addspeed)
		accelspeed = addspeed;

	for (i = 0; i < 3; i++)
		velocity[i] += accelspeed * wishvel[i];
}


/*
===================
SV_WaterMove

===================
*/
static void SV_WaterMove (void)
{
	int		i;
	vec3_t	wishvel;
	float	speed, newspeed, addspeed, accelspeed;
	//float	wishspeed;

//
// user intentions
//
	AngleVectors (sv_player->v.v_angle, forward, right, up);

	for (i = 0; i < 3; i++)
		wishvel[i] = forward[i]*cmd.forwardmove + right[i]*cmd.sidemove;

	if (!cmd.forwardmove && !cmd.sidemove && !cmd.upmove)
		wishvel[2] -= 60;	// drift towards bottom
	else
		wishvel[2] += cmd.upmove;

	wishspeed = VectorLength(wishvel);
	if (wishspeed > sv_maxspeed.value)
	{
		VectorScale (wishvel, sv_maxspeed.value/wishspeed, wishvel);
		wishspeed = sv_maxspeed.value;
	}

	if (sv_player->v.playerclass == CLASS_DEMON)	// Paladin Special Ability #1 - unrestricted movement in water
		wishspeed *= 0.5;
	else if (sv_player->v.playerclass != CLASS_PALADIN) // Paladin Special Ability #1 - unrestricted movement in water
		wishspeed *= 0.7;
	else if (sv_player->v.level == 1)
		wishspeed *= 0.75;
	else if (sv_player->v.level == 2)
		wishspeed *= 0.80;
	else if ((sv_player->v.level == 3) || (sv_player->v.level == 4))
		wishspeed *= 0.85;
	else if ((sv_player->v.level == 5) || (sv_player->v.level == 6))
		wishspeed *= 0.90;
	else if ((sv_player->v.level == 7) || (sv_player->v.level == 8))
		wishspeed *= 0.95;

//
// water friction
//
	speed = VectorLength (velocity);
	if (speed)
	{
		newspeed = speed - host_frametime * speed * sv_friction.value;
		if (newspeed < 0)
			newspeed = 0;
		VectorScale (velocity, newspeed/speed, velocity);
	}
	else
		newspeed = 0;

//
// water acceleration
//
	if (!wishspeed)
		return;

	addspeed = wishspeed - newspeed;
	if (addspeed <= 0)
		return;

	VectorNormalize (wishvel);
	accelspeed = sv_accelerate.value * wishspeed * host_frametime;
	if (accelspeed > addspeed)
		accelspeed = addspeed;

	for (i = 0; i < 3; i++)
		velocity[i] += accelspeed * wishvel[i];
}


static void SV_WaterJump (void)
{
	if (sv.time > sv_player->v.teleport_time
		|| !sv_player->v.waterlevel)
	{
		sv_player->v.flags = (int)sv_player->v.flags & ~FL_WATERJUMP;
		sv_player->v.teleport_time = 0;
	}
	sv_player->v.velocity[0] = sv_player->v.movedir[0];
	sv_player->v.velocity[1] = sv_player->v.movedir[1];
}


/*
===================
SV_NoclipMove
alternate q2-style noclip from Fitzquake
old noclip still handled in SV_AirMove()
===================
*/
static void SV_NoclipMove (void)
{
	AngleVectors (sv_player->v.v_angle, forward, right, up);

	velocity[0] = forward[0]*cmd.forwardmove + right[0]*cmd.sidemove;
	velocity[1] = forward[1]*cmd.forwardmove + right[1]*cmd.sidemove;
	velocity[2] = forward[2]*cmd.forwardmove + right[2]*cmd.sidemove;
	velocity[2] += cmd.upmove*2; // doubled to match running speed

	if (VectorLength (velocity) > sv_maxspeed.value)
	{
		VectorNormalize (velocity);
		VectorScale (velocity, sv_maxspeed.value, velocity);
	}
}

/*
===================
SV_AirMove

===================
*/
static void SV_AirMove (void)
{
	int			i;
	vec3_t		wishvel;
	float		fmove, smove;

	AngleVectors (sv_player->v.angles, forward, right, up);

	fmove = cmd.forwardmove;
	smove = cmd.sidemove;

	// hack to not let you back into teleporter
	if (sv.time < sv_player->v.teleport_time && fmove < 0)
		fmove = 0;

	for (i = 0; i < 3; i++)
		wishvel[i] = forward[i]*fmove + right[i]*smove;

	if ( (int)sv_player->v.movetype != MOVETYPE_WALK)
		wishvel[2] = cmd.upmove;
	else
		wishvel[2] = 0;

	VectorCopy (wishvel, wishdir);
	wishspeed = VectorNormalize(wishdir);
	if (wishspeed > sv_maxspeed.value)
	{
		VectorScale (wishvel, sv_maxspeed.value/wishspeed, wishvel);
		wishspeed = sv_maxspeed.value;
	}

	if ( sv_player->v.movetype == MOVETYPE_NOCLIP)
	{	// noclip
		VectorCopy (wishvel, velocity);
	}
	else if (onground)
	{
		SV_UserFriction ();
		SV_Accelerate ();
	}
	else
	{	// not on ground, so little effect on velocity
		SV_AirAccelerate (wishvel);
	}
}


/*
===============
V_CalcRoll
===============
*/
#if defined(SERVERONLY)
static float V_CalcRoll (vec3_t Angles, vec3_t Velocity)
{
	vec3_t	Fwd, Right, Up;
	float	sign, side, value;

	AngleVectors (Angles, Fwd, Right, Up);
	side = DotProduct (Velocity, Right);
	sign = side < 0 ? -1 : 1;
	side = fabs(side);

	value = cl_rollangle.value;

	if (side < cl_rollspeed.value)
		side = side * value / cl_rollspeed.value;
	else
		side = value;

	return side*sign;
}
#endif

/*
===================
SV_ClientThink

the move fields specify an intended velocity in pix/sec
the angle fields specify an exact angular motion in degrees
===================
*/
void SV_ClientThink (void)
{
	vec3_t		v_angle;

	if (sv_player->v.movetype == MOVETYPE_NONE)
		return;

	onground = (int)sv_player->v.flags & FL_ONGROUND;

	origin = sv_player->v.origin;
	velocity = sv_player->v.velocity;

	DropPunchAngle ();

//
// if dead, behave differently
//
	if (sv_player->v.health <= 0)
		return;

//
// angles
// show 1/3 the pitch angle and all the roll angle
	cmd = host_client->cmd;
	angles = sv_player->v.angles;

	VectorAdd (sv_player->v.v_angle, sv_player->v.punchangle, v_angle);
	angles[ROLL] = V_CalcRoll(sv_player->v.angles, sv_player->v.velocity) * 4;
	if (!sv_player->v.fixangle)
	{
		angles[PITCH] = -v_angle[PITCH]/3;
		angles[YAW] = v_angle[YAW];
	}

	if ( (int)sv_player->v.flags & FL_WATERJUMP )
	{
		SV_WaterJump ();
		return;
	}
//
// walk
//
	if (sv_player->v.movetype == MOVETYPE_NOCLIP && sv_altnoclip.integer)
	{
		SV_NoclipMove ();	// alternate quake2-style noclip
		return;
	}
	if ( (sv_player->v.waterlevel >= 2)
		&& (sv_player->v.movetype != MOVETYPE_NOCLIP) )
	{
		SV_WaterMove ();
		return;
	}
	else if (sv_player->v.movetype == MOVETYPE_FLY)
	{
		SV_FlightMove ();
		return;
	}

	SV_AirMove ();
}


/*
===================
SV_ReadClientMove
===================
*/
static void SV_ReadClientMove (usercmd_t *move)
{
	int		i;
	vec3_t	angle;
	int		bits;

	// read ping time
	host_client->ping_times[host_client->num_pings%NUM_PING_TIMES]
					= sv.time - MSG_ReadFloat ();
	host_client->num_pings++;

	// read current angles
	for (i = 0; i < 3; i++)
		angle[i] = MSG_ReadAngle ();

	VectorCopy (angle, host_client->edict->v.v_angle);

	// read movement
	move->forwardmove = MSG_ReadShort ();
	move->sidemove = MSG_ReadShort ();
	move->upmove = MSG_ReadShort ();

	// read buttons
	bits = MSG_ReadByte ();
	host_client->edict->v.button0 = bits & 1;
	host_client->edict->v.button2 = (bits & 2)>>1;

	if (bits & 4)	// crouched?
		host_client->edict->v.flags2 = ((int)host_client->edict->v.flags2) | FL2_CROUCHED;
	else
		host_client->edict->v.flags2 = ((int)host_client->edict->v.flags2) & (~FL2_CROUCHED);

	i = MSG_ReadByte ();
	if (i)
		host_client->edict->v.impulse = i;

	// read light level
	host_client->edict->v.light_level = MSG_ReadByte ();
}


/*
===================
SV_ReadClientMessage

Returns false if the client should be killed
===================
*/
static qboolean SV_ReadClientMessage (void)
{
	int		ret;
	int		ccmd;
	const char	*s;

	do
	{
nextmsg:
		ret = NET_GetMessage (host_client->netconnection);
		if (ret == -1)
		{
			Sys_Printf ("%s: NET_GetMessage failed\n", __thisfunc__);
			return false;
		}
		if (!ret)
			return true;

		MSG_BeginReading ();

		while (1)
		{
			if (!host_client->active)
				return false;	// a command caused an error

			if (msg_badread)
			{
				Sys_Printf ("%s: badread\n", __thisfunc__);
				return false;
			}

			ccmd = MSG_ReadChar ();

			switch (ccmd)
			{
			case -1:
				goto nextmsg;	// end of message

			default:
				Sys_Printf ("%s: unknown command char\n", __thisfunc__);
				return false;

			case clc_nop:
			//	Sys_Printf ("clc_nop\n");
				break;

			case clc_stringcmd:
				s = MSG_ReadString ();
				ret = 0;
				if (q_strncasecmp(s, "status", 6) == 0)
					ret = 1;
				else if (q_strncasecmp(s, "god", 3) == 0)
					ret = 1;
				else if (q_strncasecmp(s, "notarget", 8) == 0)
					ret = 1;
				else if (q_strncasecmp(s, "fly", 3) == 0)
					ret = 1;
				else if (q_strncasecmp(s, "name", 4) == 0)
					ret = 1;
				else if (q_strncasecmp(s, "playerclass", 11) == 0)
					ret = 1;
				else if (q_strncasecmp(s, "noclip", 6) == 0)
					ret = 1;
				else if (q_strncasecmp(s, "say", 3) == 0)
					ret = 1;
				else if (q_strncasecmp(s, "say_team", 8) == 0)
					ret = 1;
				else if (q_strncasecmp(s, "tell", 4) == 0)
					ret = 1;
				else if (q_strncasecmp(s, "color", 5) == 0)
					ret = 1;
				else if (q_strncasecmp(s, "kill", 4) == 0)
					ret = 1;
				else if (q_strncasecmp(s, "pause", 5) == 0)
					ret = 1;
				else if (q_strncasecmp(s, "spawn", 5) == 0)
					ret = 1;
				else if (q_strncasecmp(s, "begin", 5) == 0)
					ret = 1;
				else if (q_strncasecmp(s, "prespawn", 8) == 0)
					ret = 1;
				else if (q_strncasecmp(s, "kick", 4) == 0)
					ret = 1;
				else if (q_strncasecmp(s, "ping", 4) == 0)
					ret = 1;
				else if (q_strncasecmp(s, "give", 4) == 0)
					ret = 1;
				else if (q_strncasecmp(s, "ban", 3) == 0)
					ret = 1;

				if (ret == 1)
					Cmd_ExecuteString (s, src_client);
				else
					Con_DPrintf("%s tried to %s\n", host_client->name, s);
				break;

			case clc_disconnect:
			//	Sys_Printf ("%s: client disconnected\n", __thisfunc__);
				return false;

			case clc_move:
				SV_ReadClientMove (&host_client->cmd);
				break;

			case clc_inv_select:
				host_client->edict->v.inventory = MSG_ReadByte();
				break;

			case clc_frame:
				host_client->last_frame = MSG_ReadByte();
				host_client->last_sequence = MSG_ReadByte();
				break;
			}
		}
	} while (ret == 1);

	return true;
}


/*
==================
SV_RunClients
==================
*/
void SV_RunClients (void)
{
	int				i;

	for (i = 0, host_client = svs.clients; i < svs.maxclients; i++, host_client++)
	{
		if (!host_client->active)
			continue;

		sv_player = host_client->edict;

		if (!SV_ReadClientMessage ())
		{
			SV_DropClient (false);	// client misbehaved...
			continue;
		}

		if (!host_client->spawned)
		{
		// clear client movement until a new packet is received
			memset (&host_client->cmd, 0, sizeof(host_client->cmd));
			continue;
		}

		// always pause in single player if in console or menus
#if defined(SERVERONLY)
		if (!sv.paused)
#else
		if (!sv.paused && (svs.maxclients > 1 || Key_GetDest() == key_game))
#endif
			SV_ClientThink ();
	}
}


/*
==============
SV_UserInit
==============
*/
void SV_UserInit (void)
{
#if !defined(SERVERONLY)
	if (!isDedicated)
		return;
#endif	/* SERVERONLY */
	Cvar_RegisterVariable (&cl_rollspeed);
	Cvar_RegisterVariable (&cl_rollangle);
}

