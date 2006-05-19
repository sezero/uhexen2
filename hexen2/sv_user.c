/*
	sv_user.c
	server code for moving users

	$Header: /home/ozzie/Download/0000/uhexen2/hexen2/sv_user.c,v 1.10 2006-05-19 11:32:54 sezero Exp $
*/

#include "quakedef.h"

edict_t	*sv_player = NULL;

extern	cvar_t	sv_friction;
cvar_t	sv_edgefriction = {"edgefriction", "2", CVAR_NONE};
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

	if (sv_player->v.movetype==MOVETYPE_FLY)
		return;

	angleval = sv_player->v.angles[YAW] * M_PI*2 / 360;
	sinval = sin(angleval);
	cosval = cos(angleval);

	save_hull = sv_player->v.hull;
	sv_player->v.hull = 0;

	for (i=0 ; i<MAX_FORWARD ; i++)
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
	for (j=1 ; j<i ; j++)
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

//
// Hexen II v1.11 progs doesn't initialize the friction properly (always 0).
// In fact, applying a patch to the progs can easily solve this issue, but
// in that case several unpatched mods would be left broken. Note: Compared
// to the AoT solution, this makes pure-hexen2 to feel slightly more slippery.
//
// COMPILE TIME OPTION: USE_AOT_FRICTION
// If you want to use the Anvil of Thyrion solution for the original hexen2
// friction emulation, define USE_AOT_FRICTION as 1, otherwise 0.
//
#define	USE_AOT_FRICTION	0

#if USE_AOT_FRICTION
	if (progs->crc == PROGS_V111_CRC)
		friction = 6;
	else
	{
		if (trace.fraction == 1.0)
			friction = sv_friction.value*sv_edgefriction.value*sv_player->v.friction;
		else
			friction = sv_friction.value*sv_player->v.friction;
	}
#else	// not using AoT friction
	if (progs->crc == PROGS_V111_CRC)
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

	for (i=0 ; i<3 ; i++)
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

	for (i=0 ; i<3 ; i++)
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

	for (i=0 ; i<3 ; i++)
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

	cl.nodrift = false;
	cl.driftmove = 0;

//
// user intentions
//
	AngleVectors (sv_player->v.v_angle, forward, right, up);

	for (i=0 ; i<3 ; i++)
		wishvel[i] = forward[i]*cmd.forwardmove + right[i]*cmd.sidemove + up[i]* cmd.upmove;

	wishspeed = Length(wishvel);
	if (wishspeed > sv_maxspeed.value)
	{
		VectorScale (wishvel, sv_maxspeed.value/wishspeed, wishvel);
		wishspeed = sv_maxspeed.value;
	}

//
// water friction
//
	speed = Length (velocity);
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

	for (i=0 ; i<3 ; i++)
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

	for (i=0 ; i<3 ; i++)
		wishvel[i] = forward[i]*cmd.forwardmove + right[i]*cmd.sidemove;

	if (!cmd.forwardmove && !cmd.sidemove && !cmd.upmove)
		wishvel[2] -= 60;	// drift towards bottom
	else
		wishvel[2] += cmd.upmove;

	wishspeed = Length(wishvel);
	if (wishspeed > sv_maxspeed.value)
	{
		VectorScale (wishvel, sv_maxspeed.value/wishspeed, wishvel);
		wishspeed = sv_maxspeed.value;
	}

	if (sv_player->v.playerclass==CLASS_DEMON)	// Paladin Special Ability #1 - unrestricted movement in water
		wishspeed *= 0.5;
	else if (sv_player->v.playerclass!=CLASS_PALADIN) // Paladin Special Ability #1 - unrestricted movement in water
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
	else
		wishspeed = wishspeed;

//
// water friction
//
	speed = Length (velocity);
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

	for (i=0 ; i<3 ; i++)
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

	for (i=0 ; i<3 ; i++)
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
	angles[ROLL] = V_CalcRoll (sv_player->v.angles, sv_player->v.velocity)*4;
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
	for (i=0 ; i<3 ; i++)
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
	char		*s;

	do
	{
nextmsg:
		ret = NET_GetMessage (host_client->netconnection);
		if (ret == -1)
		{
			Sys_Printf ("SV_ReadClientMessage: NET_GetMessage failed\n");
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
				Sys_Printf ("SV_ReadClientMessage: badread\n");
				return false;
			}

			ccmd = MSG_ReadChar ();

			switch (ccmd)
			{
			case -1:
				goto nextmsg;	// end of message

			default:
				Sys_Printf ("SV_ReadClientMessage: unknown command char\n");
				return false;

			case clc_nop:
			//	Sys_Printf ("clc_nop\n");
				break;

			case clc_stringcmd:
				s = MSG_ReadString ();
				ret = 0;
				if (Q_strncasecmp(s, "status", 6) == 0)
					ret = 1;
				else if (Q_strncasecmp(s, "god", 3) == 0)
					ret = 1;
				else if (Q_strncasecmp(s, "notarget", 8) == 0)
					ret = 1;
				else if (Q_strncasecmp(s, "fly", 3) == 0)
					ret = 1;
				else if (Q_strncasecmp(s, "name", 4) == 0)
					ret = 1;
				else if (Q_strncasecmp(s, "playerclass", 11) == 0)
					ret = 1;
				else if (Q_strncasecmp(s, "noclip", 6) == 0)
					ret = 1;
				else if (Q_strncasecmp(s, "say", 3) == 0)
					ret = 1;
				else if (Q_strncasecmp(s, "say_team", 8) == 0)
					ret = 1;
				else if (Q_strncasecmp(s, "tell", 4) == 0)
					ret = 1;
				else if (Q_strncasecmp(s, "color", 5) == 0)
					ret = 1;
				else if (Q_strncasecmp(s, "kill", 4) == 0)
					ret = 1;
				else if (Q_strncasecmp(s, "pause", 5) == 0)
					ret = 1;
				else if (Q_strncasecmp(s, "spawn", 5) == 0)
					ret = 1;
				else if (Q_strncasecmp(s, "begin", 5) == 0)
					ret = 1;
				else if (Q_strncasecmp(s, "prespawn", 8) == 0)
					ret = 1;
				else if (Q_strncasecmp(s, "kick", 4) == 0)
					ret = 1;
				else if (Q_strncasecmp(s, "ping", 4) == 0)
					ret = 1;
				else if (Q_strncasecmp(s, "give", 4) == 0)
					ret = 1;
				else if (Q_strncasecmp(s, "ban", 3) == 0)
					ret = 1;

				if (ret == 1)
					Cmd_ExecuteString (s, src_client);
				else
					Con_DPrintf("%s tried to %s\n", host_client->name, s);
				break;

			case clc_disconnect:
			//	Sys_Printf ("SV_ReadClientMessage: client disconnected\n");
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

	for (i=0, host_client = svs.clients ; i<svs.maxclients ; i++, host_client++)
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
		if (!sv.paused && (svs.maxclients > 1 || key_dest == key_game) )
			SV_ClientThink ();
	}
}

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.9  2006/03/24 15:05:39  sezero
 * killed the archive, server and info members of the cvar structure.
 * the new flags member is now employed for all those purposes. also
 * made all non-globally used cvars static.
 *
 * Revision 1.8  2006/03/21 22:20:59  sezero
 * Commented out Anvil of Thyrion solution for the original hexen2 friction
 * emulation which was hard-set at 6. Merged the solution found in pa3pyx's
 * tree.  v1.11 progs doesn't initialize the friction properly (always 0).
 * In fact, applying a patch to the progs can easily solve this issue, but
 * in that case several unpatched mods would be left broken. Note: Compared
 * to the AoT solution down below, this makes pure-hexen2 to feel slightly
 * more slippery.
 *
 * Revision 1.7  2006/02/26 12:28:16  sezero
 * continue making static functions and vars static. whitespace and coding style
 * cleanup. (part 31:  sv_send.c, sv_user.c).
 *
 * Revision 1.6  2005/10/02 15:43:08  sezero
 * killed -Wshadow warnings
 *
 * Revision 1.5  2005/09/10 12:34:59  sezero
 * cleaned-up h2/h2mp friction code
 *
 * Revision 1.4  2005/05/19 16:47:18  sezero
 * killed client->privileged (was only available to IDGODS)
 *
 * Revision 1.3  2005/05/19 16:41:50  sezero
 * removed all unused (never used) non-RJNET and non-QUAKE2RJ code
 *
 * Revision 1.2  2004/12/12 14:14:43  sezero
 * style changes to our liking
 *
 * Revision 1.1.1.1  2004/11/28 00:07:56  sezero
 * Initial import of AoT 1.2.0 code
 *
 * Revision 1.2  2001/11/13 21:12:07  theoddone33
 * Yay, Hexen 2 is playable apart from the mission pack now.
 *
 * Doubt it works with a real Hexen 2 client over the net, but that's what HW
 * is for, eh?
 *
 * 6     3/13/98 1:51p Mgummelt
 * Fixed friction_change entity to work,  made checkbottom use the hull
 * mins/maxs for it's checks, not the bounding box's.
 * 
 * 5     3/13/98 12:02p Jmonroe
 * more fixes for hullforent
 * 
 * 4     2/09/98 11:43a Jmonroe
 * 
 * 3     2/02/98 10:28a Mgummelt
 * 
 * 21    8/26/97 11:41a Rlove
 * 
 * 20    8/26/97 11:38a Rlove
 * 
 * 19    8/26/97 10:29a Rjohnson
 * Made flags2 be set when a player crouches
 * 
 * 18    8/26/97 8:17a Rjohnson
 * Just a few changes
 * 
 * 17    8/04/97 2:22p Rjohnson
 * Included light level for the player
 * 
 * 16    7/24/97 5:21p Rlove
 * 
 * 15    7/09/97 6:08a Rlove
 * 
 * 14    7/08/97 9:12a Rlove
 * 
 * 13    6/05/97 4:42p Rlove
 * Flight mode is network friendly now.
 * 
 * 12    5/31/97 10:01a Rlove
 * Had to up sv_maxspeed so haste would work
 * 
 * 11    4/04/97 3:07p Rjohnson
 * Networking updates and corrections
 * 
 * 10    3/31/97 7:24p Rjohnson
 * Added a playerclass field and made sure the server/clients handle it
 * properly
 * 
 * 9     3/15/97 3:08p Rlove
 * Added COMA console command
 * 
 * 8     3/07/97 2:19p Rjohnson
 * Id Updates
 * 
 * 7     3/03/97 5:00p Rjohnson
 * Added spawn flags and code to prevent items flagged from being spawned
 * 
 * 6     2/18/97 4:47p Rjohnson
 * Added headers
 */
