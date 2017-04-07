/*
 * pr_cmds.c -- prog commands
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
#ifdef PLATFORM_WINDOWS
#include <windows.h>
#endif
#ifdef PLATFORM_UNIX
#include <signal.h>
#include <unistd.h>
#endif


#define	STRINGTEMP_BUFFERS		16
#define	STRINGTEMP_LENGTH		1024
static	char	pr_string_temp[STRINGTEMP_BUFFERS][STRINGTEMP_LENGTH];
static	byte	pr_string_tempindex = 0;

static char *PR_GetTempString (void)
{
	return pr_string_temp[(STRINGTEMP_BUFFERS-1) & ++pr_string_tempindex];
}

#define	RETURN_EDICT(e) (((int *)pr_globals)[OFS_RETURN] = EDICT_TO_PROG(e))

#define	MSG_BROADCAST	0		// unreliable to all
#define	MSG_ONE		1		// reliable to one (msg_entity)
#define	MSG_ALL		2		// reliable to all
#define	MSG_INIT	3		// write to the init string


#if defined(SERVERONLY)
static int	d_lightstylevalue[256];
#else
extern int	d_lightstylevalue[256];
#endif


/*
===============================================================================

	BUILT-IN FUNCTIONS

===============================================================================
*/

static char *PF_VarString (int	first)
{
	int		i;
	static char out[256];

	out[0] = 0;
	for (i = first; i < pr_argc; i++)
	{
		if ( q_strlcat(out, G_STRING((OFS_PARM0+i*3)), sizeof(out)) >= sizeof(out) )
		{
			Con_Printf("%s: overflow (string truncated)\n", __thisfunc__);
			break;
		}
	}
	return out;
}


/*
=================
PF_error

This is a TERMINAL error, which will kill off the entire server.
Dumps self.

error(value)
=================
*/
static void PF_error (void)
{
	char	*s;
	edict_t	*ed;

	s = PF_VarString(0);
	Con_Printf ("======SERVER ERROR in %s:\n%s\n",
			PR_GetString(pr_xfunction->s_name), s);
	ed = PROG_TO_EDICT(*sv_globals.self);
	ED_Print (ed);

	Host_Error ("Program error");
}

/*
=================
PF_objerror

Dumps out self, then an error message.  The program is aborted and self is
removed, but the level can continue.

objerror(value)
=================
*/
static void PF_objerror (void)
{
	char	*s;
	edict_t	*ed;

	s = PF_VarString(0);
	Con_Printf ("======OBJECT ERROR in %s:\n%s\n",
			PR_GetString(pr_xfunction->s_name), s);
	ed = PROG_TO_EDICT(*sv_globals.self);
	ED_Print (ed);
	ED_Free (ed);

	Host_Error ("Program error");
}


/*
==============
PF_makevectors

Writes new values for v_forward, v_up, and v_right based on angles
makevectors(vector)
==============
*/
static void PF_makevectors (void)
{
	AngleVectors (G_VECTOR(OFS_PARM0), *sv_globals.v_forward, *sv_globals.v_right, *sv_globals.v_up);
}

/*
=================
PF_setorigin

This is the only valid way to move an object without using the physics
of the world (setting velocity and waiting).  Directly changing origin
will not set internal links correctly, so clipping would be messed up.

This should be called when an object is spawned, and then only if it is
teleported.

setorigin (entity, origin)
=================
*/
static void PF_setorigin (void)
{
	edict_t	*e;
	float	*org;

	e = G_EDICT(OFS_PARM0);
	org = G_VECTOR(OFS_PARM1);
	VectorCopy (org, e->v.origin);
	SV_LinkEdict (e, false);
}


static void SetMinMaxSize (edict_t *e, float *minvec, float *maxvec, qboolean rotate)
{
	float	*angles;
	vec3_t	rmin, rmax;
	float	bounds[2][3];
	float	xvector[2], yvector[2];
	float	a;
	vec3_t	base, transformed;
	int		i, j, k, l;

	for (i = 0; i < 3; i++)
	{
		if (minvec[i] > maxvec[i])
			PR_RunError ("backwards mins/maxs");
	}

	rotate = false;		// FIXME: implement rotation properly again

	if (!rotate)
	{
		VectorCopy (minvec, rmin);
		VectorCopy (maxvec, rmax);
	}
	else
	{
	// find min / max for rotations
		angles = e->v.angles;

		a = angles[1]/180 * M_PI;

		xvector[0] = cos(a);
		xvector[1] = sin(a);
		yvector[0] = -sin(a);
		yvector[1] = cos(a);

		VectorCopy (minvec, bounds[0]);
		VectorCopy (maxvec, bounds[1]);

		rmin[0] = rmin[1] = rmin[2] = 9999;
		rmax[0] = rmax[1] = rmax[2] = -9999;

		for (i = 0; i <= 1; i++)
		{
			base[0] = bounds[i][0];
			for (j = 0; j <= 1; j++)
			{
				base[1] = bounds[j][1];
				for (k = 0; k <= 1; k++)
				{
					base[2] = bounds[k][2];

				// transform the point
					transformed[0] = xvector[0]*base[0] + yvector[0]*base[1];
					transformed[1] = xvector[1]*base[0] + yvector[1]*base[1];
					transformed[2] = base[2];

					for (l = 0; l < 3; l++)
					{
						if (transformed[l] < rmin[l])
							rmin[l] = transformed[l];
						if (transformed[l] > rmax[l])
							rmax[l] = transformed[l];
					}
				}
			}
		}
	}

// set derived values
	VectorCopy (rmin, e->v.mins);
	VectorCopy (rmax, e->v.maxs);
	VectorSubtract (maxvec, minvec, e->v.size);

	SV_LinkEdict (e, false);
}

/*
=================
PF_setsize

the size box is rotated by the current angle

setsize (entity, minvector, maxvector)
=================
*/
static void PF_setsize (void)
{
	edict_t	*e;
	float	*minvec, *maxvec;

	e = G_EDICT(OFS_PARM0);
	minvec = G_VECTOR(OFS_PARM1);
	maxvec = G_VECTOR(OFS_PARM2);
	SetMinMaxSize (e, minvec, maxvec, false);
}


/*
=================
PF_setmodel

setmodel(entity, model)
=================
*/
static void PF_setmodel (void)
{
	int		i;
	const char	*m, **check;
	qmodel_t	*mod;
	edict_t		*e;

	e = G_EDICT(OFS_PARM0);
	m = G_STRING(OFS_PARM1);

// check to see if model was properly precached
	for (i = 0, check = sv.model_precache;
	     i < MAX_MODELS && *check; i++, check++)
	{
		if (!strcmp(*check, m))
			break;
	}

	if (i >= MAX_MODELS || !*check)
	{
		PR_RunError ("no precache: %s", m);
	}
	else
	{
		e->v.model = PR_SetEngineString(*check);
		e->v.modelindex = i; //SV_ModelIndex (m);

		mod = sv.models[ (int)e->v.modelindex];	// Mod_ForName (m, true);

		if (mod)
			SetMinMaxSize (e, mod->mins, mod->maxs, true);
		else
			SetMinMaxSize (e, vec3_origin, vec3_origin, true);
	}
}

static void PF_setpuzzlemodel (void)
{
	int		i;
	const char	*m, **check;
	qmodel_t	*mod;
	edict_t		*e;

	e = G_EDICT(OFS_PARM0);
	m = G_STRING(OFS_PARM1);

	m = va ("models/puzzle/%s.mdl", m);
// check to see if model was properly precached
	for (i = 0, check = sv.model_precache;
	     i < MAX_MODELS && *check; i++, check++)
	{
		if (!strcmp(*check, m))
			break;
	}

	if (i >= MAX_MODELS)
	{
		PR_RunError ("%s: overflow", __thisfunc__);
	}
	else
	{
		if (!*check)
		{
			Con_Printf("NO PRECACHE FOR PUZZLE PIECE: %s\n", m);
			m = (const char *)Hunk_Strdup(m, "puzzlemodel");
			sv.model_precache[i] = m;
			e->v.model = PR_SetEngineString(m);
#if !defined(SERVERONLY)
			sv.models[i] = Mod_ForName (m, true);
#endif	/* SERVERONLY */
		}
		else
		{
			e->v.model = PR_SetEngineString(*check);
		}

		e->v.modelindex = i;	//SV_ModelIndex (m);

		mod = sv.models[ (int)e->v.modelindex];	// Mod_ForName (m, true);

		if (mod)
			SetMinMaxSize (e, mod->mins, mod->maxs, true);
		else
			SetMinMaxSize (e, vec3_origin, vec3_origin, true);
	}
}

/*
=================
PF_bprint

broadcast print to everyone on server

bprint(value)
=================
*/
static void PF_bprint (void)
{
	char		*s;

	s = PF_VarString(0);
	SV_BroadcastPrintf ("%s", s);
}

/*
=================
PF_sprint

single print to a specific client

sprint(clientent, value)
=================
*/
static void PF_sprint (void)
{
	char		*s;
	client_t	*client;
	int	entnum;

	entnum = G_EDICTNUM(OFS_PARM0);
	s = PF_VarString(1);

	if (entnum < 1 || entnum > svs.maxclients)
	{
		Con_Printf ("tried to sprint to a non-client\n");
		return;
	}

	client = &svs.clients[entnum-1];

	MSG_WriteChar (&client->message,svc_print);
	MSG_WriteString (&client->message, s );
}


/*
=================
PF_centerprint

single print to a specific client

centerprint(clientent, value)
=================
*/
static void PF_centerprint (void)
{
	char		*s;
	client_t	*client;
	int	entnum;

	entnum = G_EDICTNUM(OFS_PARM0);
	s = PF_VarString(1);

	if (entnum < 1 || entnum > svs.maxclients)
	{
		Con_Printf ("tried to sprint to a non-client\n");
		return;
	}

	client = &svs.clients[entnum-1];

	MSG_WriteChar (&client->message,svc_centerprint);
	MSG_WriteString (&client->message, s);
}


/*
=================
PF_normalize

vector normalize(vector)
=================
*/
static void PF_normalize (void)
{
	float	*value1;
	vec3_t	newvalue;
	float	new_temp;

	value1 = G_VECTOR(OFS_PARM0);

	new_temp = value1[0] * value1[0] + value1[1] * value1[1] + value1[2]*value1[2];
	new_temp = sqrt (new_temp);

	if (new_temp == 0)
		newvalue[0] = newvalue[1] = newvalue[2] = 0;
	else
	{
		new_temp = 1 / new_temp;
		newvalue[0] = value1[0] * new_temp;
		newvalue[1] = value1[1] * new_temp;
		newvalue[2] = value1[2] * new_temp;
	}

	VectorCopy (newvalue, G_VECTOR(OFS_RETURN));
}

/*
=================
PF_vlen

scalar vlen(vector)
=================
*/
static void PF_vlen (void)
{
	float	*value1;
	float	new_temp;

	value1 = G_VECTOR(OFS_PARM0);

	new_temp = value1[0] * value1[0] + value1[1] * value1[1] + value1[2]*value1[2];
	new_temp = sqrt(new_temp);

	G_FLOAT(OFS_RETURN) = new_temp;
}

/*
=================
PF_vhlen

scalar vhlen(vector)
=================
*/
static void PF_vhlen (void)
{
	float	*value1;
	float	new_temp;

	value1 = G_VECTOR(OFS_PARM0);

	new_temp = value1[0] * value1[0] + value1[1] * value1[1];
	new_temp = sqrt(new_temp);

	G_FLOAT(OFS_RETURN) = new_temp;
}

/*
=================
PF_vectoyaw

float vectoyaw(vector)
=================
*/
static void PF_vectoyaw (void)
{
	float	*value1;
	float	yaw;

	value1 = G_VECTOR(OFS_PARM0);

	if (value1[1] == 0 && value1[0] == 0)
		yaw = 0;
	else
	{
		yaw = (int) (atan2(value1[1], value1[0]) * 180 / M_PI);
		if (yaw < 0)
			yaw += 360;
	}

	G_FLOAT(OFS_RETURN) = yaw;
}


/*
=================
PF_vectoangles

vector vectoangles(vector)
=================
*/
static void PF_vectoangles (void)
{
	float	*value1;
	float	forward;
	float	yaw, pitch;

	value1 = G_VECTOR(OFS_PARM0);

	if (value1[1] == 0 && value1[0] == 0)
	{
		yaw = 0;
		if (value1[2] > 0)
			pitch = 90;
		else
			pitch = 270;
	}
	else
	{
		yaw = (int) (atan2(value1[1], value1[0]) * 180 / M_PI);
		if (yaw < 0)
			yaw += 360;

		forward = sqrt (value1[0]*value1[0] + value1[1]*value1[1]);
		pitch = (int) (atan2(value1[2], forward) * 180 / M_PI);
		if (pitch < 0)
			pitch += 360;
	}

	G_FLOAT(OFS_RETURN+0) = pitch;
	G_FLOAT(OFS_RETURN+1) = yaw;
	G_FLOAT(OFS_RETURN+2) = 0;
}

/*
=================
PF_Random

Returns a number from 0 <= num < 1

random()
=================
*/
static void PF_random (void)
{
	float		num;

//	num = (rand() & 0x7fff) / ((float)0x7fff);
	num = rand() * (1.0 / RAND_MAX);

	G_FLOAT(OFS_RETURN) = num;
}

/*
=================
PF_particle

particle(origin, color, count)
=================
*/
static void PF_particle (void)
{
	float		*org, *dir;
	float		color;
	float		count;

	org = G_VECTOR(OFS_PARM0);
	dir = G_VECTOR(OFS_PARM1);
	color = G_FLOAT(OFS_PARM2);
	count = G_FLOAT(OFS_PARM3);
	SV_StartParticle (org, dir, color, count);
}


/*
=================
PF_particle2

particle(origin, dmin, dmax, color, effect, count)
=================
*/
static void PF_particle2 (void)
{
	float		*org, *dmin, *dmax;
	float		color;
	float		count;
	float		effect;

	org = G_VECTOR(OFS_PARM0);
	dmin = G_VECTOR(OFS_PARM1);
	dmax = G_VECTOR(OFS_PARM2);
	color = G_FLOAT(OFS_PARM3);
	effect = G_FLOAT(OFS_PARM4);
	count = G_FLOAT(OFS_PARM5);
	SV_StartParticle2 (org, dmin, dmax, color, effect, count);
}


/*
=================
PF_particle3

particle(origin, box, color, effect, count)
=================
*/
static void PF_particle3 (void)
{
	float		*org, *box;
	float		color;
	float		count;
	float		effect;

	org = G_VECTOR(OFS_PARM0);
	box = G_VECTOR(OFS_PARM1);
	color = G_FLOAT(OFS_PARM2);
	effect = G_FLOAT(OFS_PARM3);
	count = G_FLOAT(OFS_PARM4);
	SV_StartParticle3 (org, box, color, effect, count);
}

/*
=================
PF_particle4

particle(origin, radius, color, effect, count)
=================
*/
static void PF_particle4 (void)
{
	float		*org;
	float		radius;
	float		color;
	float		count;
	float		effect;

	org = G_VECTOR(OFS_PARM0);
	radius = G_FLOAT(OFS_PARM1);
	color = G_FLOAT(OFS_PARM2);
	effect = G_FLOAT(OFS_PARM3);
	count = G_FLOAT(OFS_PARM4);
	SV_StartParticle4 (org, radius, color, effect, count);
}


/*
=================
PF_ambientsound

=================
*/
static void PF_ambientsound (void)
{
	const char	*samp, **check;
	float		*pos;
	float		vol, attenuation;
	int		i, soundnum;
	int		SOUNDS_MAX; /* just to be on the safe side */

	pos = G_VECTOR (OFS_PARM0);
	samp = G_STRING(OFS_PARM1);
	vol = G_FLOAT(OFS_PARM2);
	attenuation = G_FLOAT(OFS_PARM3);

// check to see if samp was properly precached
	SOUNDS_MAX = (sv_protocol == PROTOCOL_RAVEN_111) ? MAX_SOUNDS_OLD : MAX_SOUNDS;
	for (soundnum = 0, check = sv.sound_precache;
	     soundnum < SOUNDS_MAX && *check; soundnum++, check++)
	{
		if (!strcmp(*check, samp))
			break;
	}

	if (soundnum == SOUNDS_MAX || !*check)
	{
		Con_Printf ("no precache: %s\n", samp);
		return;
	}

// add an svc_spawnambient command to the level signon packet
	MSG_WriteByte (&sv.signon,svc_spawnstaticsound);
	for (i = 0; i < 3; i++)
		MSG_WriteCoord(&sv.signon, pos[i]);

	if (sv_protocol == PROTOCOL_RAVEN_111)
		MSG_WriteByte (&sv.signon, soundnum);
	else
		MSG_WriteShort(&sv.signon, soundnum);

	MSG_WriteByte (&sv.signon, vol*255);
	MSG_WriteByte (&sv.signon, attenuation*64);
}

/*
=================
PF_StopSound
stop ent's sound on this chan
=================
*/
static void PF_StopSound(void)
{
	int		channel;
	edict_t		*entity;

	entity = G_EDICT(OFS_PARM0);
	channel = G_FLOAT(OFS_PARM1);

	if (channel < 0 || channel > 7)
		Host_Error ("%s: channel = %i", __thisfunc__, channel);

	SV_StopSound (entity, channel);
}

/*
=================
PF_UpdateSoundPos
sends cur pos to client to update this ent/chan pair
=================
*/
static void PF_UpdateSoundPos(void)
{
	int		channel;
	edict_t		*entity;

	entity = G_EDICT(OFS_PARM0);
	channel = G_FLOAT(OFS_PARM1);

	if (channel < 0 || channel > 7)
		Host_Error ("%s: channel = %i", __thisfunc__, channel);

	SV_UpdateSoundPos (entity, channel);
}

/*
=================
PF_sound

Each entity can have eight independant sound sources, like voice,
weapon, feet, etc.

Channel 0 is an auto-allocate channel, the others override anything
already running on that entity/channel pair.

An attenuation of 0 will play full volume everywhere in the level.
Larger attenuations will drop off.

=================
*/
static void PF_sound (void)
{
	const char	*sample;
	int		channel;
	edict_t		*entity;
	int		volume;
	float	attenuation;

	entity = G_EDICT(OFS_PARM0);
	channel = G_FLOAT(OFS_PARM1);
	sample = G_STRING(OFS_PARM2);
	volume = G_FLOAT(OFS_PARM3) * 255;
	attenuation = G_FLOAT(OFS_PARM4);

	if (volume < 0 || volume > 255)
		Host_Error ("%s: volume = %i", __thisfunc__, volume);

	if (attenuation < 0 || attenuation > 4)
		Host_Error ("%s: attenuation = %f", __thisfunc__, attenuation);

	if (channel < 0 || channel > 7)
		Host_Error ("%s: channel = %i", __thisfunc__, channel);

	SV_StartSound (entity, channel, sample, volume, attenuation);
}

/*
=================
PF_break

break()
=================
*/
static void PF_break (void)
{
	static qboolean done = false;

	if (!done)
	{
		done = true;

		Con_Printf ("break statement\n");
#if defined(PLATFORM_WINDOWS)
		DebugBreak();
#elif defined(PLATFORM_UNIX)
		kill(getpid(), SIGKILL);
#else
		*(int *)-4 = 0;	// dump to debugger
#endif
	}
//	PR_RunError ("break statement");
}

static void PR_SetTrace (trace_t trace)
{
	*sv_globals.trace_allsolid = trace.allsolid;
	*sv_globals.trace_startsolid = trace.startsolid;
	*sv_globals.trace_fraction = trace.fraction;
	*sv_globals.trace_inwater = trace.inwater;
	*sv_globals.trace_inopen = trace.inopen;
	VectorCopy (trace.endpos, *sv_globals.trace_endpos);
	VectorCopy (trace.plane.normal, *sv_globals.trace_plane_normal);
	*sv_globals.trace_plane_dist =  trace.plane.dist;
	if (trace.ent)
		*sv_globals.trace_ent = EDICT_TO_PROG(trace.ent);
	else
		*sv_globals.trace_ent = EDICT_TO_PROG(sv.edicts);
}

/*
=================
PF_traceline

Used for use tracing and shot targeting
Traces are blocked by bbox and exact bsp entityes, and also slide box entities
if the tryents flag is set.

traceline (vector1, vector2, tryents)
=================
*/
static void PF_traceline (void)
{
	float	*v1, *v2;
	trace_t	trace;
	int	nomonsters;
	edict_t	*ent;
	float	save_hull;

	v1 = G_VECTOR(OFS_PARM0);
	v2 = G_VECTOR(OFS_PARM1);
	nomonsters = G_FLOAT(OFS_PARM2);
	ent = G_EDICT(OFS_PARM3);

	save_hull = ent->v.hull;
	ent->v.hull = 0;
	trace = SV_Move (v1, vec3_origin, vec3_origin, v2, nomonsters, ent);
	ent->v.hull = save_hull;

	PR_SetTrace (trace);

}

#ifdef QUAKE2
extern trace_t SV_Trace_Toss (edict_t *ent, edict_t *ignore);

static void PF_TraceToss (void)
{
	trace_t	trace;
	edict_t	*ent;
	edict_t	*ignore;

	ent = G_EDICT(OFS_PARM0);
	ignore = G_EDICT(OFS_PARM1);

	trace = SV_Trace_Toss (ent, ignore);

	PR_SetTrace (trace);
}
#endif	/* QUAKE2 */

/*
=================
PF_tracearea

Used for use tracing and shot targeting
Traces are blocked by bbox and exact bsp entityes, and also slide box entities
if the tryents flag is set.

tracearea (vector1, vector2, mins, maxs, tryents)
=================
*/
static void PF_tracearea (void)
{
	float	*v1, *v2, *mins, *maxs;
	trace_t	trace;
	int	nomonsters;
	edict_t	*ent;
	float	save_hull;

	v1 = G_VECTOR(OFS_PARM0);
	v2 = G_VECTOR(OFS_PARM1);
	mins = G_VECTOR(OFS_PARM2);
	maxs = G_VECTOR(OFS_PARM3);
	nomonsters = G_FLOAT(OFS_PARM4);
	ent = G_EDICT(OFS_PARM5);

	save_hull = ent->v.hull;
	ent->v.hull = 0;
	trace = SV_Move (v1, mins, maxs, v2, nomonsters, ent);
	ent->v.hull = save_hull;

	PR_SetTrace (trace);
}


#if 0	/* not used */

struct PointInfo_t
{
	char	Found, NumFound, MarkedWhen;
	struct PointInfo_t	*FromPos, *Next;
};

#define MAX_POINT_X	21
#define MAX_POINT_Y	21
#define MAX_POINT_Z	11
#define MAX_POINT	(MAX_POINT_X * MAX_POINT_Y * MAX_POINT_Z)

struct PointInfo_t PI[MAX_POINT];

extern particle_t	*active_particles, *free_particles;


static int ZOffset, YOffset;

#define POINT_POS(x,y,z)	((z*ZOffset)+(y*YOffset)+(x))

#define POINT_X_SIZE	160
#define POINT_Y_SIZE	160
#define POINT_Z_SIZE	50

#define POINT_MAX_DEPTH	5

static void AddParticle (float *Org, float color)
{
	particle_t	*p;

	p = free_particles;
	free_particles = p->next;
	p->next = active_particles;
	active_particles = p;

	p->die = 99999;
	p->color = color;
	p->type = pt_static;
	VectorClear (p->vel);
	VectorCopy (Org, p->org);
}

static void FindPath (float *StartV, float *EndV, float *Mins, float *Maxs, int NoMonsters, edict_t *Ent)
{
	vec3_t	NewStartV, NewEndV;
	int	XSize, YSize, ZSize;
	int	x, y, z, c, c2, xs, ys, zs, nx, ny, nz, XPos, YPos, ZPos;
	int	StartX, StartY, StartZ, DiffX, DiffY, DiffZ, Diff, OrigDiff;
	int	NumTraces, NumTracesEach, NumMarks;
	float	TracePercent, PercentEach;
	struct PointInfo_t *Pos,*EndPos;//,*ToDo[POINT_MAX_DEPTH];
	int	test;
	trace_t	trace;

	XSize = 11;
	YSize = 11;
	ZSize = 11;

	NumTraces = 0;
	NumMarks = 0;
	ZOffset = XSize*YSize;
	YOffset = XSize;

//	for (c = 0; c < POINT_MAX_DEPTH; c++)
//		ToDo[c] = NULL;

	for (c = 0, Pos = PI; c < MAX_POINT; c++, Pos++)
	{
		Pos->Found = Pos->NumFound = Pos->MarkedWhen = 0;
		Pos->FromPos = Pos->Next = NULL;
	}

	StartX = (XSize) / 2;
	StartY = (YSize) / 2;
	StartZ = (ZSize) / 2;

//	ToDo[0] = &PI[POINT_POS(StartX,StartY,StartZ)];
//	ToDo[0]->Found = 1;

	PI[POINT_POS(StartX,StartY,StartZ)].Found = 1;

	test = 0;
	for (c = 1; c <= 5; c++)
	{
		NumMarks = NumTracesEach = 0;

		for (z = 0, Pos = PI; z < ZSize; z++)
		{
		    for (y = 0; y < YSize; y++)
		    {
			for (x = 0; x < XSize; x++, Pos++)
			{
			    if (Pos->Found == c)
			    {
				for (zs = -1; zs <= 1; zs++)
				{
				    switch (zs)
				    {
				    case -1:
					ZPos = 0;
					if (z == ZPos)
						continue;
					break;
				    case 0:
					ZPos = z;
					break;
				    case 1:
					ZPos = ZSize-1;
					if (z == ZPos)
						continue;
					break;
				    }

				    for (ys = -1; ys <= 1; ys++)
				    {
					switch (ys)
					{
					case -1:
						YPos = 0;
						if (y == YPos)
							continue;
						break;
					case 0:
						YPos = y;
						break;
					case 1:
						YPos = YSize-1;
						if (y == YPos)
							continue;
						break;
					}

					for (xs = -1; xs <= 1; xs++)
					{
					    if (zs || ys || xs)
					    {
						switch (xs)
						{
						case -1:
							XPos = 0;
							if (x == XPos)
								continue;
							break;
						case 0:
							XPos = x;
							break;
						case 1:
							XPos = XSize-1;
							if (x == XPos)
								continue;
							break;
						}

						if (XPos == x && YPos == y && ZPos == z)
							continue;

						test++;
						DiffX = abs(x - XPos);
						DiffY = abs(y - YPos);
						DiffZ = abs(z - ZPos);

						Diff = 999;
						if (DiffX && DiffX < Diff)
							Diff = DiffX;
						if (DiffY && DiffY < Diff)
							Diff = DiffY;
						if (DiffZ && DiffZ < Diff)
							Diff = DiffZ;
						if (Diff == 999)
							continue;

						OrigDiff = Diff;

						nx = x;
						ny = y;
						nz = z;
						Diff = 0;

						do
						{
							Diff++;
							nx += xs;
							ny += ys;
							nz += zs;

							EndPos = &PI[POINT_POS(nx,ny,nz)];

						//	if (EndPos < PI || EndPos >= &PI[MAX_POINT])
						//	{
						//		Diff = 0;
						//		Con_Printf("ERROR2\n");
						//		break;
						//	}
						} while (!EndPos->Found && Diff != OrigDiff);

						if (Diff != OrigDiff || EndPos->Found)
						{
							nx -= xs;
							ny -= ys;
							nz -= zs;
							Diff--;
						}

						if (!Diff)
						{
							continue;
						}

						DiffX = x - StartX;
						DiffY = y - StartY;
						DiffZ = z - StartZ;

						NewStartV[0] = StartV[0] + (DiffX * POINT_X_SIZE);
						NewStartV[1] = StartV[1] + (DiffY * POINT_Y_SIZE);
						NewStartV[2] = StartV[2] + (DiffZ * POINT_Z_SIZE);

						DiffX = nx - x;
						DiffY = ny - y;
						DiffZ = nz - z;

						NewEndV[0] = NewStartV[0] + (DiffX * POINT_X_SIZE);
						NewEndV[1] = NewStartV[1] + (DiffY * POINT_Y_SIZE);
						NewEndV[2] = NewStartV[2] + (DiffZ * POINT_Z_SIZE);

						NumTraces++;
						NumTracesEach++;
						trace = SV_Move (NewStartV, Mins, Maxs, NewEndV, NoMonsters, Ent);
					//	trace = SV_Move (NewStartV,vec3_origin, vec3_origin, NewEndV, NoMonsters, Ent);

						TracePercent = trace.fraction;
						PercentEach = 1 / (float)Diff;

					//	OrigStartV[0] = NewStartV[0];
					//	OrigStartV[1] = NewStartV[1];
					//	OrigStartV[2] = NewStartV[2];

						nz = z;
						ny = y;
						nx = x;
						c2 = Pos->Found;

						while (TracePercent >= PercentEach)
						{
							nz += zs;
							ny += ys;
							nx += xs;

						//	NewStartV[0] += xs*POINT_X_SIZE;
						//	NewStartV[1] += ys*POINT_Y_SIZE;
						//	NewStartV[2] += zs*POINT_Z_SIZE;
						//	AddParticle(NewStartV,(test == 2062 ? 255 : 252));

							c2++;

							EndPos = &PI[POINT_POS(nx,ny,nz)];

						//	if (EndPos < PI || EndPos >= &PI[MAX_POINT])
						//	{
						//		Con_Printf("ERROR %d %d\n",OrigDiff,test);
						//		break;
						//	}

							if (EndPos->Found && EndPos->Found <= c2+1)
								break;

							EndPos->Found = c2;
							EndPos->MarkedWhen = Pos->Found;
							EndPos->FromPos = Pos;
							NumMarks++;

							TracePercent -= PercentEach;
						}
					    }
					}
				    }
				}
			    }
			}
		    }
		}

//		Con_Printf("NumMarks: %d  NumTraces: %d\n",NumMarks,NumTracesEach);

		if (!NumMarks)
			break;
	}

//	Con_Printf("\n\n%d traces\n",NumTraces);
}

static void PF_FindPath(void)
{
	float	*v1, *v2, *mins, *maxs;
	int	nomonsters;
	edict_t	*ent;
	double b;

	v1 = G_VECTOR(OFS_PARM0);
	v2 = G_VECTOR(OFS_PARM1);
	mins = G_VECTOR(OFS_PARM2);
	maxs = G_VECTOR(OFS_PARM3);
	nomonsters = G_FLOAT(OFS_PARM4);
	ent = G_EDICT(OFS_PARM5);

	b = Sys_DoubleTime ();
	FindPath(v1, v2, mins, maxs, nomonsters,ent);
	Con_Printf("Time is %10.4f\n", Sys_DoubleTime() - b);
}

#endif	/* not used */



/*
=================
PF_checkpos

Returns true if the given entity can move to the given position from it's
current position by walking or rolling.
FIXME: make work...
scalar checkpos (entity, vector)
=================
*/
#if 0
static void PF_checkpos (void)
{
}
#endif

//============================================================================

static byte	checkpvs[MAX_MAP_LEAFS/8];

static int PF_newcheckclient (int check)
{
	int		i;
	byte	*pvs;
	edict_t	*ent;
	mleaf_t	*leaf;
	vec3_t	org;

// cycle to the next one

	if (check < 1)
		check = 1;
	if (check > svs.maxclients)
		check = svs.maxclients;

	if (check == svs.maxclients)
		i = 1;
	else
		i = check + 1;

	for ( ;  ; i++)
	{
		if (i == svs.maxclients+1)
			i = 1;

		ent = EDICT_NUM(i);

		if (i == check)
			break;	// didn't find anything else

		if (ent->free)
			continue;
		if (ent->v.health <= 0)
			continue;
		if ((int)ent->v.flags & FL_NOTARGET)
			continue;

	// anything that is a client, or has a client as an enemy
		break;
	}

// get the PVS for the entity
	VectorAdd (ent->v.origin, ent->v.view_ofs, org);
	leaf = Mod_PointInLeaf (org, sv.worldmodel);
	pvs = Mod_LeafPVS (leaf, sv.worldmodel);
	memcpy (checkpvs, pvs, (sv.worldmodel->numleafs+7)>>3 );

	return i;
}

/*
=================
PF_checkclient

Returns a client (or object that has a client enemy) that would be a
valid target.

If there are more than one valid options, they are cycled each frame

If (self.origin + self.viewofs) is not in the PVS of the current target,
it is not returned at all.

name checkclient ()
=================
*/
#define	MAX_CHECK	16
static int c_invis, c_notvis;
static void PF_checkclient (void)
{
	edict_t	*ent, *self;
	mleaf_t	*leaf;
	int		l;
	vec3_t	view;

// find a new check if on a new frame
	if (sv.time - sv.lastchecktime >= 0.1)
	{
		sv.lastcheck = PF_newcheckclient (sv.lastcheck);
		sv.lastchecktime = sv.time;
	}

// return check if it might be visible
	ent = EDICT_NUM(sv.lastcheck);
	if (ent->free || ent->v.health <= 0)
	{
		RETURN_EDICT(sv.edicts);
		return;
	}

// if current entity can't possibly see the check entity, return 0
	self = PROG_TO_EDICT(*sv_globals.self);
	VectorAdd (self->v.origin, self->v.view_ofs, view);
	leaf = Mod_PointInLeaf (view, sv.worldmodel);
	l = (leaf - sv.worldmodel->leafs) - 1;
	if ( (l < 0) || !(checkpvs[l>>3] & (1 << (l & 7))) )
	{
		c_notvis++;
		RETURN_EDICT(sv.edicts);
		return;
	}

// might be able to see it
	c_invis++;
	RETURN_EDICT(ent);
}

//============================================================================


/*
=================
PF_stuffcmd

Sends text over to the client's execution buffer

stuffcmd (clientent, value)
=================
*/
static void PF_stuffcmd (void)
{
	int		entnum;
	const char	*str;
	client_t	*old;

	entnum = G_EDICTNUM(OFS_PARM0);
	if (entnum < 1 || entnum > svs.maxclients)
		PR_RunError ("Parm 0 not a client");
	str = G_STRING(OFS_PARM1);

	old = host_client;
	host_client = &svs.clients[entnum-1];
	Host_ClientCommands ("%s", str);
	host_client = old;
}

/*
=================
PF_localcmd

Sends text over to the client's execution buffer

localcmd (string)
=================
*/
static void PF_localcmd (void)
{
	const char	*str;

	str = G_STRING(OFS_PARM0);
	Cbuf_AddText (str);
}

/*
=================
PF_cvar

float cvar (string)
=================
*/
static void PF_cvar (void)
{
	const char	*str;

	str = G_STRING(OFS_PARM0);

	G_FLOAT(OFS_RETURN) = Cvar_VariableValue (str);
}

/*
=================
PF_cvar_set

float cvar (string)
=================
*/
static void PF_cvar_set (void)
{
	const char	*var, *val;

	var = G_STRING(OFS_PARM0);
	val = G_STRING(OFS_PARM1);

	Cvar_Set (var, val);
}

/*
=================
PF_findradius

Returns a chain of entities that have origins within a spherical area

findradius (origin, radius)
=================
*/
static void PF_findradius (void)
{
	edict_t	*ent, *chain;
	float	rad;
	float	*org;
	vec3_t	eorg;
	int	i, j;

	chain = (edict_t *)sv.edicts;

	org = G_VECTOR(OFS_PARM0);
	rad = G_FLOAT(OFS_PARM1);

	ent = NEXT_EDICT(sv.edicts);
	for (i = 1; i < sv.num_edicts; i++, ent = NEXT_EDICT(ent))
	{
		if (ent->free)
			continue;
		if (ent->v.solid == SOLID_NOT)
			continue;
		for (j = 0; j < 3; j++)
			eorg[j] = org[j] - (ent->v.origin[j] + (ent->v.mins[j] + ent->v.maxs[j]) * 0.5);
		if (VectorLength(eorg) > rad)
			continue;

		ent->v.chain = EDICT_TO_PROG(chain);
		chain = ent;
	}

	RETURN_EDICT(chain);
}


/*
=========
PF_dprint
=========
*/
static void PF_dprint (void)
{
	Con_DPrintf ("%s",PF_VarString(0));
}

static void PF_dprintf (void)
{
	char temp[256];
	float	v;

	v = G_FLOAT(OFS_PARM1);

	if (v == (int)v)
		sprintf (temp, "%d",(int)v);
	else
		sprintf (temp, "%5.1f",v);

	Con_DPrintf (G_STRING(OFS_PARM0),temp);
}

static void PF_dprintv (void)
{
	char temp[256];

	sprintf (temp, "'%5.1f %5.1f %5.1f'", G_VECTOR(OFS_PARM1)[0], G_VECTOR(OFS_PARM1)[1], G_VECTOR(OFS_PARM1)[2]);

	Con_DPrintf (G_STRING(OFS_PARM0),temp);
}

static void PF_ftos (void)
{
	float	v;
	char	*s;

	v = G_FLOAT(OFS_PARM0);
	s = PR_GetTempString();
	if (v == (int)v)
		sprintf (s, "%d",(int)v);
	else
		sprintf (s, "%5.1f",v);
	G_INT(OFS_RETURN) = PR_SetEngineString(s);
}

static void PF_fabs (void)
{
	float	v;
	v = G_FLOAT(OFS_PARM0);
	G_FLOAT(OFS_RETURN) = fabs(v);
}

static void PF_vtos (void)
{
	char	*s;

	s = PR_GetTempString();
	sprintf (s, "'%5.1f %5.1f %5.1f'", G_VECTOR(OFS_PARM0)[0], G_VECTOR(OFS_PARM0)[1], G_VECTOR(OFS_PARM0)[2]);
	G_INT(OFS_RETURN) = PR_SetEngineString(s);
}

#ifdef QUAKE2
static void PF_etos (void)
{
	char	*s;

	s = PR_GetTempString();
	sprintf (s, "entity %i", G_EDICTNUM(OFS_PARM0));
	G_INT(OFS_RETURN) = PR_SetEngineString(s);
}
#endif

static void PF_Spawn (void)
{
	edict_t	*ed;

	ed = ED_Alloc();

	RETURN_EDICT(ed);
}

static void PF_SpawnTemp (void)
{
	edict_t	*ed;

	ed = ED_Alloc_Temp();

	RETURN_EDICT(ed);
}

static void PF_Remove (void)
{
	edict_t	*ed;
	int i;

	ed = G_EDICT(OFS_PARM0);
	if (ed == sv.edicts)
	{
		Con_DPrintf("Tried to remove the world at %s in %s!\n",
				PR_GetString(pr_xfunction->s_name), PR_GetString(pr_xfunction->s_file));
		return;
	}

	i = NUM_FOR_EDICT(ed);
	if (i <= svs.maxclients)
	{
		Con_DPrintf("Tried to remove a client at %s in %s!\n",
				PR_GetString(pr_xfunction->s_name), PR_GetString(pr_xfunction->s_file));
		return;
	}
	ED_Free (ed);
}


// entity (entity start, .string field, string match) find = #5;
static void PF_Find (void)
#ifdef QUAKE2
{
	int		e;
	int		f;
	const char	*s, *t;
	edict_t	*ed;
	edict_t	*first;
	edict_t	*second;
	edict_t	*last;

	first = second = last = (edict_t *)sv.edicts;
	e = G_EDICTNUM(OFS_PARM0);
	f = G_INT(OFS_PARM1);
	s = G_STRING(OFS_PARM2);
	if (!s)
		PR_RunError ("%s: bad search string", __thisfunc__);

	for (e++ ; e < sv.num_edicts ; e++)
	{
		ed = EDICT_NUM(e);
		if (ed->free)
			continue;
		t = E_STRING(ed,f);
		if (!t)
			continue;
		if (!strcmp(t,s))
		{
			if (first == (edict_t *)sv.edicts)
				first = ed;
			else if (second == (edict_t *)sv.edicts)
				second = ed;
			ed->v.chain = EDICT_TO_PROG(last);
			last = ed;
		}
	}

	if (first != last)
	{
		if (last != second)
			first->v.chain = last->v.chain;
		else
			first->v.chain = EDICT_TO_PROG(last);
		last->v.chain = EDICT_TO_PROG((edict_t *)sv.edicts);
		if (second && second != last)
			second->v.chain = EDICT_TO_PROG(last);
	}
	RETURN_EDICT(first);
}
#else
{
	int		e;
	int		f;
	const char	*s, *t;
	edict_t	*ed;

	e = G_EDICTNUM(OFS_PARM0);
	f = G_INT(OFS_PARM1);
	s = G_STRING(OFS_PARM2);
	if (!s)
		PR_RunError ("%s: bad search string", __thisfunc__);

	for (e++ ; e < sv.num_edicts ; e++)
	{
		ed = EDICT_NUM(e);
		if (ed->free)
			continue;
		t = E_STRING(ed,f);
		if (!t)
			continue;
		if (!strcmp(t,s))
		{
			RETURN_EDICT(ed);
			return;
		}
	}

	RETURN_EDICT(sv.edicts);
}
#endif	/* QUAKE2 */

#if 0
static void PF_FindFloat (void)
{
	int		e;
	int		f;
	float	s, t;
	edict_t	*ed;

	e = G_EDICTNUM(OFS_PARM0);
	f = G_INT(OFS_PARM1);
	s = G_FLOAT(OFS_PARM2);
	if (!s)
		PR_RunError ("%s: bad search string", __thisfunc__);

	for (e++ ; e < sv.num_edicts ; e++)
	{
		ed = EDICT_NUM(e);
		if (ed->free)
			continue;
		t = E_FLOAT(ed,f);
		if (t == s)
		{
			RETURN_EDICT(ed);
			return;
		}
	}

	RETURN_EDICT(sv.edicts);
}
#endif	/* #if 0 */

static void PR_CheckEmptyString (const char *s)
{
	if (s[0] <= ' ')
		PR_RunError ("Bad string");
}

static void PF_precache_file (void)
{	// precache_file is only used to copy files with qcc, it does nothing
	G_INT(OFS_RETURN) = G_INT(OFS_PARM0);
}

static void PF_precache_sound (void)
{
	const char	*s;
	int		SOUNDS_MAX; /* just to be on the safe side */
	int		i;

	if (sv.state != ss_loading && !ignore_precache)
		PR_RunError ("%s: Precache can only be done in spawn functions", __thisfunc__);

	s = G_STRING(OFS_PARM0);
	G_INT(OFS_RETURN) = G_INT(OFS_PARM0);
	PR_CheckEmptyString (s);

	SOUNDS_MAX = (sv_protocol == PROTOCOL_RAVEN_111) ? MAX_SOUNDS_OLD : MAX_SOUNDS;
	for (i = 0; i < SOUNDS_MAX; i++)
	{
		if (!sv.sound_precache[i])
		{
			sv.sound_precache[i] = s;
			return;
		}
		if (!strcmp(sv.sound_precache[i], s))
			return;
	}
	PR_RunError ("%s: overflow", __thisfunc__);
}

static void PF_precache_sound2 (void)
{
	if (!registered.integer)
		return;

	PF_precache_sound();
}

static void PF_precache_sound3 (void)
{
	if (!registered.integer && !oem.integer)
		return;

	PF_precache_sound();
}

static void PF_precache_sound4 (void)
{//mission pack only
	if (!registered.integer)
		return;

	PF_precache_sound();
}

static void PF_precache_model (void)
{
	const char	*s;
	int		i;

	if (sv.state != ss_loading && !ignore_precache)
		PR_RunError ("%s: Precache can only be done in spawn functions", __thisfunc__);

	s = G_STRING(OFS_PARM0);
	G_INT(OFS_RETURN) = G_INT(OFS_PARM0);
	PR_CheckEmptyString (s);

	for (i = 0; i < MAX_MODELS; i++)
	{
		if (!sv.model_precache[i])
		{
			sv.model_precache[i] = s;
#if !defined(SERVERONLY)
			sv.models[i] = Mod_ForName (s, true);
#endif	/* SERVERONLY */
			return;
		}
		if (!strcmp(sv.model_precache[i], s))
			return;	/* duplicate precache */
	}
	PR_RunError ("%s: overflow", __thisfunc__);
}

static void PF_precache_model2 (void)
{
	if (!registered.integer)
		return;

	PF_precache_model();
}

static void PF_precache_model3 (void)
{
	if (!registered.integer && !oem.integer)
		return;

	PF_precache_model();
}

static void PF_precache_model4 (void)
{
	if (!registered.integer)
		return;
	PF_precache_model();
}

static void PF_precache_puzzle_model (void)
{
	int		i;
	const char	*s, *temp;

	if (sv.state != ss_loading && !ignore_precache)
		PR_RunError ("%s: Precache can only be done in spawn functions", __thisfunc__);

	s = G_STRING(OFS_PARM0);
	G_INT(OFS_RETURN) = G_INT(OFS_PARM0);

	PR_CheckEmptyString (s);
	temp = va ("models/puzzle/%s.mdl", s);

	for (i = 0; i < MAX_MODELS; i++)
	{
		if (!sv.model_precache[i])
		{
			s = (const char *)Hunk_Strdup(temp, "puzzlemodel");
			sv.model_precache[i] = s;
#if !defined(SERVERONLY)
			sv.models[i] = Mod_ForName (s, true);
#endif	/* SERVERONLY */
			return;
		}
		if (!strcmp(sv.model_precache[i], temp))
			return;
	}
	PR_RunError ("%s: overflow", __thisfunc__);
}


static void PF_coredump (void)
{
	ED_PrintEdicts ();
}

static void PF_traceon (void)
{
	pr_trace = true;
}

static void PF_traceoff (void)
{
	pr_trace = false;
}

static void PF_eprint (void)
{
	ED_PrintNum (G_EDICTNUM(OFS_PARM0));
}

/*
===============
PF_walkmove

float(float yaw, float dist) walkmove
===============
*/
static void PF_walkmove (void)
{
	edict_t	*ent;
	float	yaw, dist;
	vec3_t	move;
	dfunction_t	*oldf;
	int	oldself;
	qboolean set_trace;

	ent = PROG_TO_EDICT(*sv_globals.self);
	yaw = G_FLOAT(OFS_PARM0);
	dist = G_FLOAT(OFS_PARM1);
	set_trace = G_FLOAT(OFS_PARM2);

	if ( !( (int)ent->v.flags & (FL_ONGROUND|FL_FLY|FL_SWIM) ) )
	{
		G_FLOAT(OFS_RETURN) = 0;
		return;
	}

	yaw = yaw * M_PI * 2 / 360;

	move[0] = cos(yaw) * dist;
	move[1] = sin(yaw) * dist;
	move[2] = 0;

// save program state, because SV_movestep may call other progs
	oldf = pr_xfunction;
	oldself = *sv_globals.self;

	G_FLOAT(OFS_RETURN) = SV_movestep(ent, move, true, true, set_trace);

// restore program state
	pr_xfunction = oldf;
	*sv_globals.self = oldself;
}

/*
===============
PF_droptofloor

void() droptofloor
===============
*/
static void PF_droptofloor (void)
{
	edict_t		*ent;
	vec3_t		end;
	trace_t		trace;

	ent = PROG_TO_EDICT(*sv_globals.self);

	VectorCopy (ent->v.origin, end);
	end[2] -= 256;

	trace = SV_Move (ent->v.origin, ent->v.mins, ent->v.maxs, end, false, ent);

	if (trace.fraction == 1 || trace.allsolid)
		G_FLOAT(OFS_RETURN) = 0;
	else
	{
		VectorCopy (trace.endpos, ent->v.origin);
		SV_LinkEdict (ent, false);
		ent->v.flags = (int)ent->v.flags | FL_ONGROUND;
		ent->v.groundentity = EDICT_TO_PROG(trace.ent);
		G_FLOAT(OFS_RETURN) = 1;
	}
}

/*
===============
PF_lightstyle

void(float style, string value) lightstyle
===============
*/
static void PF_lightstyle (void)
{
	int		style;
	const char	*val;
	client_t	*client;
	int	j;

	style = G_FLOAT(OFS_PARM0);
	val = G_STRING(OFS_PARM1);

// bounds check to avoid clobbering sv struct
	if (style < 0 || style >= MAX_LIGHTSTYLES)
		Host_Error("%s: style = %d", __thisfunc__, style);

// change the string in sv
	sv.lightstyles[style] = val;

// send message to all clients on this server
	if (sv.state != ss_active)
		return;

	for (j = 0, client = svs.clients; j < svs.maxclients; j++, client++)
	{
		if (client->active || client->spawned)
		{
			MSG_WriteChar (&client->message, svc_lightstyle);
			MSG_WriteChar (&client->message, style);
			MSG_WriteString (&client->message, val);
		}
	}
}

//==========================================================================
//
// PF_lightstylevalue
//
// void lightstylevalue(float style);
//
//==========================================================================

static void PF_lightstylevalue(void)
{
	int style;

	style = G_FLOAT(OFS_PARM0);
	if (style < 0 || style >= MAX_LIGHTSTYLES)
	{
		G_FLOAT(OFS_RETURN) = 0;
		return;
	}

	G_FLOAT(OFS_RETURN) = (int)((float)d_lightstylevalue[style]/22.0);
}

//==========================================================================
//
// PF_lightstylestatic
//
// void lightstylestatic(float style, float value);
//
//==========================================================================

static const char *styleDefs[] =
{
	"a", "b", "c", "d", "e", "f", "g",
	"h", "i", "j", "k", "l", "m", "n",
	"o", "p", "q", "r", "s", "t", "u",
	"v", "w", "x", "y", "z"
};

static void PF_lightstylestatic(void)
{
	int	i, value, styleNumber;
	const char	*styleString;
	client_t	*client;

	styleNumber = G_FLOAT(OFS_PARM0);
	value = G_FLOAT(OFS_PARM1);
	if (value < 0)
	{
		value = 0;
	}
	else if (value > 'z'-'a')
	{
		value = 'z'-'a';
	}
	styleString = styleDefs[value];

	// Change the string in sv
	sv.lightstyles[styleNumber] = styleString;
#if defined(SERVERONLY)
	d_lightstylevalue[styleNumber] = value;
#endif	/* SERVERONLY */

	if (sv.state != ss_active)
	{
		return;
	}

	// Send message to all clients on this server
	for (i = 0, client = svs.clients; i < svs.maxclients; i++, client++)
	{
		if (client->active || client->spawned)
		{
			MSG_WriteChar(&client->message, svc_lightstyle);
			MSG_WriteChar(&client->message, styleNumber);
			MSG_WriteString(&client->message, styleString);
		}
	}
}

static void PF_rint (void)
{
	float	f;

	f = G_FLOAT(OFS_PARM0);
	if (f > 0)
		G_FLOAT(OFS_RETURN) = (int)(f + 0.1);
	else
		G_FLOAT(OFS_RETURN) = (int)(f - 0.1);
}

static void PF_floor (void)
{
	G_FLOAT(OFS_RETURN) = floor(G_FLOAT(OFS_PARM0));
}

static void PF_ceil (void)
{
	G_FLOAT(OFS_RETURN) = ceil(G_FLOAT(OFS_PARM0));
}


/*
=============
PF_checkbottom
=============
*/
static void PF_checkbottom (void)
{
	edict_t	*ent;

	ent = G_EDICT(OFS_PARM0);

	G_FLOAT(OFS_RETURN) = SV_CheckBottom (ent);
}

/*
=============
PF_pointcontents
=============
*/
static void PF_pointcontents (void)
{
	float	*v;

	v = G_VECTOR(OFS_PARM0);

	G_FLOAT(OFS_RETURN) = SV_PointContents (v);
}

/*
=============
PF_nextent

entity nextent(entity)
=============
*/
static void PF_nextent (void)
{
	int		i;
	edict_t	*ent;

	i = G_EDICTNUM(OFS_PARM0);
	while (1)
	{
		i++;
		if (i == sv.num_edicts)
		{
			RETURN_EDICT(sv.edicts);
			return;
		}
		ent = EDICT_NUM(i);
		if (!ent->free)
		{
			RETURN_EDICT(ent);
			return;
		}
	}
}

/*
=============
PF_aim

Pick a vector for the player to shoot along
vector aim(entity, missilespeed)
=============
*/
cvar_t	sv_aim = {"sv_aim", "0.93", CVAR_NONE};
static void PF_aim (void)
{
	edict_t	*ent, *check, *bestent;
	vec3_t	start, dir, end, bestdir,hold_org;
	int		i, j;
	trace_t	tr;
	float	dist, bestdist;
	float	speed;
	float	*shot_org;
	float	save_hull;

	ent = G_EDICT(OFS_PARM0);
	shot_org = G_VECTOR(OFS_PARM1);
	speed = G_FLOAT(OFS_PARM2);
	(void) speed; /* variable set but not used */

//	VectorCopy (ent->v.origin, start);
	VectorCopy (shot_org, start);
	start[2] += 20;

// try sending a trace straight
	VectorCopy (*sv_globals.v_forward, dir);
	VectorMA (start, 2048, dir, end);

	save_hull = ent->v.hull;
	ent->v.hull = 0;
	tr = SV_Move (start, vec3_origin, vec3_origin, end, false, ent);
	ent->v.hull = save_hull;

	if (tr.ent && tr.ent->v.takedamage == DAMAGE_YES
		&& (!teamplay.integer || ent->v.team <= 0 || ent->v.team != tr.ent->v.team) )
	{
		VectorCopy (*sv_globals.v_forward, G_VECTOR(OFS_RETURN));
		return;
	}

// try all possible entities
	VectorCopy (dir, bestdir);
	bestdist = sv_aim.value;
	bestent = NULL;

	check = NEXT_EDICT(sv.edicts);
	for (i = 1; i < sv.num_edicts; i++, check = NEXT_EDICT(check) )
	{
		if (check->v.takedamage != DAMAGE_YES)
			continue;
		if (check == ent)
			continue;
		if (teamplay.integer && ent->v.team > 0 && ent->v.team == check->v.team)
			continue;	// don't aim at teammate
		for (j = 0; j < 3; j++)
			end[j] = check->v.origin[j] + 0.5 * (check->v.mins[j] + check->v.maxs[j]);
		VectorSubtract (end, start, dir);
		VectorNormalize (dir);
		dist = DotProduct (dir, *sv_globals.v_forward);
		if (dist < bestdist)
			continue;	// to far to turn
		save_hull = ent->v.hull;
		ent->v.hull = 0;
		tr = SV_Move (start, vec3_origin, vec3_origin, end, false, ent);
		ent->v.hull = save_hull;
		if (tr.ent == check)
		{	// can shoot at this one
			bestdist = dist;
			bestent = check;
		}
	}

	if (bestent)
	{	// Since all origins are at the base, move the point to the middle of the victim model
		hold_org[0] =bestent->v.origin[0];
		hold_org[1] =bestent->v.origin[1];
		hold_org[2] =bestent->v.origin[2] + (0.5 * bestent->v.maxs[2]);

		VectorSubtract (hold_org,shot_org,dir);
		dist = DotProduct (dir, *sv_globals.v_forward);
		VectorScale (*sv_globals.v_forward, dist, end);
		end[2] = dir[2];
		VectorNormalize (end);
		VectorCopy (end, G_VECTOR(OFS_RETURN));
	}
	else
	{
		VectorCopy (bestdir, G_VECTOR(OFS_RETURN));
	}
}

/*
==============
PF_changeyaw

This was a major timewaster in progs, so it was converted to C
==============
*/
void PF_changeyaw (void)
{
	edict_t		*ent;
	float		ideal, current, move, speed;

	ent = PROG_TO_EDICT(*sv_globals.self);
	current = anglemod( ent->v.angles[1] );
	ideal = ent->v.ideal_yaw;
	speed = ent->v.yaw_speed;

	if (current == ideal)
	{
		G_FLOAT(OFS_RETURN) = 0;
		return;
	}
	move = ideal - current;

	if (ideal > current)
	{
		if (move >= 180)
			move = move - 360;
	}
	else
	{
		if (move <= -180)
			move = move + 360;
	}

	G_FLOAT(OFS_RETURN) = move;

	if (move > 0)
	{
		if (move > speed)
			move = speed;
	}
	else
	{
		if (move < -speed)
			move = -speed;
	}

	ent->v.angles[1] = anglemod (current + move);
}

#ifdef QUAKE2
/*
==============
PF_changepitch
==============
*/
static void PF_changepitch (void)
{
	edict_t		*ent;
	float		ideal, current, move, speed;

	ent = G_EDICT(OFS_PARM0);
	current = anglemod( ent->v.angles[0] );
	ideal = ent->v.idealpitch;
	speed = ent->v.pitch_speed;

	if (current == ideal)
		return;
	move = ideal - current;
	if (ideal > current)
	{
		if (move >= 180)
			move = move - 360;
	}
	else
	{
		if (move <= -180)
			move = move + 360;
	}
	if (move > 0)
	{
		if (move > speed)
			move = speed;
	}
	else
	{
		if (move < -speed)
			move = -speed;
	}

	ent->v.angles[0] = anglemod (current + move);
}
#endif	/* QUAKE2 */

/*
===============================================================================

MESSAGE WRITING

===============================================================================
*/

static sizebuf_t *WriteDest (void)
{
	int		entnum;
	int		dest;
	edict_t	*ent;

	dest = G_FLOAT(OFS_PARM0);
	switch (dest)
	{
	case MSG_BROADCAST:
		return &sv.datagram;

	case MSG_ONE:
		ent = PROG_TO_EDICT(*sv_globals.msg_entity);
		entnum = NUM_FOR_EDICT(ent);
		if (entnum < 1 || entnum > svs.maxclients)
			PR_RunError ("%s: not a client", __thisfunc__);
		return &svs.clients[entnum-1].message;

	case MSG_ALL:
		return &sv.reliable_datagram;

	case MSG_INIT:
		return &sv.signon;

	default:
		PR_RunError ("%s: bad destination", __thisfunc__);
		break;
	}

	return NULL;
}

static void PF_WriteByte (void)
{
	MSG_WriteByte (WriteDest(), G_FLOAT(OFS_PARM1));
}

static void PF_WriteChar (void)
{
	MSG_WriteChar (WriteDest(), G_FLOAT(OFS_PARM1));
}

static void PF_WriteShort (void)
{
	MSG_WriteShort (WriteDest(), G_FLOAT(OFS_PARM1));
}

static void PF_WriteLong (void)
{
	MSG_WriteLong (WriteDest(), G_FLOAT(OFS_PARM1));
}

static void PF_WriteAngle (void)
{
	MSG_WriteAngle (WriteDest(), G_FLOAT(OFS_PARM1));
}

static void PF_WriteCoord (void)
{
	MSG_WriteCoord (WriteDest(), G_FLOAT(OFS_PARM1));
}

static void PF_WriteString (void)
{
	MSG_WriteString (WriteDest(), G_STRING(OFS_PARM1));
}

static void PF_WriteEntity (void)
{
	MSG_WriteShort (WriteDest(), G_EDICTNUM(OFS_PARM1));
}

//=============================================================================

static void PF_makestatic (void)
{
	edict_t	*ent;
	int		i;

	ent = G_EDICT(OFS_PARM0);

	MSG_WriteByte (&sv.signon,svc_spawnstatic);

	MSG_WriteShort(&sv.signon, SV_ModelIndex(PR_GetString(ent->v.model)));

	MSG_WriteByte (&sv.signon, ent->v.frame);
	MSG_WriteByte (&sv.signon, ent->v.colormap);
	MSG_WriteByte (&sv.signon, ent->v.skin);
	MSG_WriteByte (&sv.signon, (int)(ent->v.scale*100.0)&255);
	MSG_WriteByte (&sv.signon, ent->v.drawflags);
	MSG_WriteByte (&sv.signon, (int)(ent->v.abslight*255.0)&255);

	for (i = 0; i < 3; i++)
	{
		MSG_WriteCoord(&sv.signon, ent->v.origin[i]);
		MSG_WriteAngle(&sv.signon, ent->v.angles[i]);
	}

// throw the entity away now
	ED_Free (ent);
}

//=============================================================================

/*
==============
PF_setspawnparms
==============
*/
static void PF_setspawnparms (void)
{
	edict_t	*ent;
	int		i;
	client_t	*client;

	ent = G_EDICT(OFS_PARM0);
	i = NUM_FOR_EDICT(ent);
	if (i < 1 || i > svs.maxclients)
		PR_RunError ("Entity is not a client");

	// copy spawn parms out of the client_t
	client = svs.clients + (i-1);
	for (i = 0; i < NUM_SPAWN_PARMS; i++)
		sv_globals.parm[i] = client->spawn_parms[i];
}

/*
==============
PF_changelevel
==============
*/
static void PF_changelevel (void)
{
	const char	*s1, *s2;

	if (svs.changelevel_issued)
		return;
	svs.changelevel_issued = true;

	s1 = G_STRING(OFS_PARM0);
	s2 = G_STRING(OFS_PARM1);

	if ((int)*sv_globals.serverflags & (SFL_NEW_UNIT | SFL_NEW_EPISODE))
		Cbuf_AddText (va("changelevel %s %s\n",s1, s2));
	else
		Cbuf_AddText (va("changelevel2 %s %s\n",s1, s2));
}

#ifdef QUAKE2

#define	CONTENT_WATER	-3
#define CONTENT_SLIME	-4
#define CONTENT_LAVA	-5

#define FL_IMMUNE_WATER	131072
#define	FL_IMMUNE_SLIME	262144
#define FL_IMMUNE_LAVA	524288

#define	CHAN_VOICE	2
#define	CHAN_BODY	4

#define	ATTN_NORM	1

static void PF_WaterMove (void)
{
	edict_t		*self;
	int			flags;
	int			waterlevel;
	int			watertype;
	float		drownlevel;
	float		damage = 0.0;

	self = PROG_TO_EDICT(*sv_globals.self);

	if (self->v.movetype == MOVETYPE_NOCLIP)
	{
		self->v.air_finished = sv.time + 12;
		G_FLOAT(OFS_RETURN) = damage;
		return;
	}

	if (self->v.health < 0)
	{
		G_FLOAT(OFS_RETURN) = damage;
		return;
	}

	if (self->v.deadflag == DEAD_NO)
		drownlevel = 3;
	else
		drownlevel = 1;

	flags = (int)self->v.flags;
	waterlevel = (int)self->v.waterlevel;
	watertype = (int)self->v.watertype;

	if (!(flags & (FL_IMMUNE_WATER + FL_GODMODE)))
	{
		if (((flags & FL_SWIM) && (waterlevel < drownlevel)) || (waterlevel >= drownlevel))
		{
			if (self->v.air_finished < sv.time)
			{
				if (self->v.pain_finished < sv.time)
				{
					self->v.dmg = self->v.dmg + 2;
					if (self->v.dmg > 15)
						self->v.dmg = 10;
//					T_Damage (self, world, world, self.dmg, 0, FALSE);
					damage = self->v.dmg;
					self->v.pain_finished = sv.time + 1.0;
				}
			}
		}
		else
		{
			if (self->v.air_finished < sv.time)
				SV_StartSound (self, CHAN_VOICE, "raven/gasp2.wav", 255, ATTN_NORM);
			else if (self->v.air_finished < sv.time + 9)
				SV_StartSound (self, CHAN_VOICE, "raven/gasp1.wav", 255, ATTN_NORM);
			self->v.air_finished = sv.time + 12.0;
			self->v.dmg = 2;
		}
	}

	if (!waterlevel)
	{
		if (flags & FL_INWATER)
		{
			// play leave water sound
			SV_StartSound (self, CHAN_BODY, "raven/outwater.wav", 255, ATTN_NORM);
			self->v.flags = (float)(flags &~FL_INWATER);
		}
		self->v.air_finished = sv.time + 12.0;
		G_FLOAT(OFS_RETURN) = damage;
		return;
	}

	if (watertype == CONTENT_LAVA)
	{	// do damage
		if (!(flags & (FL_IMMUNE_LAVA + FL_GODMODE)))
		{
			if (self->v.dmgtime < sv.time)
			{
				if (self->v.radsuit_finished < sv.time)
					self->v.dmgtime = sv.time + 0.2;
				else
					self->v.dmgtime = sv.time + 1.0;
//				T_Damage (self, world, world, 10*self.waterlevel, 0, TRUE);
				damage = (float)(10*waterlevel);
			}
		}
	}
	else if (watertype == CONTENT_SLIME)
	{	// do damage
		if (!(flags & (FL_IMMUNE_SLIME + FL_GODMODE)))
		{
			if (self->v.dmgtime < sv.time && self->v.radsuit_finished < sv.time)
			{
				self->v.dmgtime = sv.time + 1.0;
//				T_Damage (self, world, world, 4*self.waterlevel, 0, TRUE);
				damage = (float)(4*waterlevel);
			}
		}
	}

	if ( !(flags & FL_INWATER) )
	{
// player enter water sound
		if (watertype == CONTENT_LAVA)
			SV_StartSound (self, CHAN_BODY, "raven/inlava.wav", 255, ATTN_NORM);
		else if (watertype == CONTENT_WATER)
			SV_StartSound (self, CHAN_BODY, "raven/inh2o.wav", 255, ATTN_NORM);
		else if (watertype == CONTENT_SLIME)
			SV_StartSound (self, CHAN_BODY, "player/slimbrn1.wav", 255, ATTN_NORM);

		self->v.flags = (float)(flags | FL_INWATER);
		self->v.dmgtime = 0;
	}

	if (! (flags & FL_WATERJUMP) )
	{
//		self.velocity = self.velocity - 0.8*self.waterlevel*frametime*self.velocity;
		VectorMA (self->v.velocity, -0.8 * self->v.waterlevel * host_frametime, self->v.velocity, self->v.velocity);
	}

	G_FLOAT(OFS_RETURN) = damage;
}


static void PF_sin (void)
{
	G_FLOAT(OFS_RETURN) = sin(G_FLOAT(OFS_PARM0));
}

static void PF_cos (void)
{
	G_FLOAT(OFS_RETURN) = cos(G_FLOAT(OFS_PARM0));
}
#endif	/* QUAKE2 */

static void PF_sqrt (void)
{
	G_FLOAT(OFS_RETURN) = sqrt(G_FLOAT(OFS_PARM0));
}

static void PF_Fixme (void)
{
	PR_RunError ("unimplemented builtin");
}


static void PF_plaque_draw (void)
{
	int idx = (int) G_FLOAT(OFS_PARM1);

	/* 0 means "clear the plaquemessage", hence
	 * the check for idx < 0 and NOT for idx < 1 */
	if (idx < 0 || idx > host_string_count)
	{
		PR_RunError ("%s: unexpected index %d (host_string_count: %d)",
					__thisfunc__, idx, host_string_count);
	}

	MSG_WriteByte (WriteDest(), svc_plaque);
	MSG_WriteShort (WriteDest(), idx);
}

static void PF_rain_go (void)
{
	float	*min_org, *max_org, *e_size;
	float	*dir;
	vec3_t	org;
	int	color, count, x_dir, y_dir;

	min_org = G_VECTOR (OFS_PARM0);
	max_org = G_VECTOR (OFS_PARM1);
	e_size  = G_VECTOR (OFS_PARM2);
	dir	= G_VECTOR (OFS_PARM3);
	color	= G_FLOAT (OFS_PARM4);
	count	= G_FLOAT (OFS_PARM5);

	org[0] = min_org[0];
	org[1] = min_org[1];
	org[2] = max_org[2];

	x_dir = dir[0];
	y_dir = dir[1];

	MSG_WriteByte (&sv.datagram, svc_raineffect);
	MSG_WriteCoord (&sv.datagram, org[0]);
	MSG_WriteCoord (&sv.datagram, org[1]);
	MSG_WriteCoord (&sv.datagram, org[2]);
	MSG_WriteCoord (&sv.datagram, e_size[0]);
	MSG_WriteCoord (&sv.datagram, e_size[1]);
	MSG_WriteCoord (&sv.datagram, e_size[2]);
	MSG_WriteAngle (&sv.datagram, x_dir);
	MSG_WriteAngle (&sv.datagram, y_dir);
	MSG_WriteShort (&sv.datagram, color);
	MSG_WriteShort (&sv.datagram, count);

//	SV_Multicast (org, MULTICAST_PVS);
}

static void PF_particleexplosion (void)
{
	float *org;
	int color, radius, counter;

	org = G_VECTOR(OFS_PARM0);
	color = G_FLOAT(OFS_PARM1);
	radius = G_FLOAT(OFS_PARM2);
	counter = G_FLOAT(OFS_PARM3);

	MSG_WriteByte(&sv.datagram, svc_particle_explosion);
	MSG_WriteCoord(&sv.datagram, org[0]);
	MSG_WriteCoord(&sv.datagram, org[1]);
	MSG_WriteCoord(&sv.datagram, org[2]);
	MSG_WriteShort(&sv.datagram, color);
	MSG_WriteShort(&sv.datagram, radius);
	MSG_WriteShort(&sv.datagram, counter);
}

static void PF_movestep (void)
{
	vec3_t v;
	edict_t	*ent;
	dfunction_t	*oldf;
	int	oldself;
	qboolean set_trace;

	ent = PROG_TO_EDICT(*sv_globals.self);

	v[0] = G_FLOAT(OFS_PARM0);
	v[1] = G_FLOAT(OFS_PARM1);
	v[2] = G_FLOAT(OFS_PARM2);
	set_trace = G_FLOAT(OFS_PARM3);

// save program state, because SV_movestep may call other progs
	oldf = pr_xfunction;
	oldself = *sv_globals.self;

	G_INT(OFS_RETURN) = SV_movestep (ent, v, false, true, set_trace);

// restore program state
	pr_xfunction = oldf;
	*sv_globals.self = oldself;
}

static void PF_Cos (void)
{
	float angle;

	angle = G_FLOAT(OFS_PARM0);

	angle = angle * M_PI * 2 / 360;

	G_FLOAT(OFS_RETURN) = cos(angle);
}

static void PF_Sin (void)
{
	float angle;

	angle = G_FLOAT(OFS_PARM0);

	angle = angle * M_PI * 2 / 360;

	G_FLOAT(OFS_RETURN) = sin(angle);
}

static void PF_AdvanceFrame (void)
{
	edict_t *ent;
	float start, end, result;

	ent = PROG_TO_EDICT(*sv_globals.self);
	start = G_FLOAT(OFS_PARM0);
	end = G_FLOAT(OFS_PARM1);

	if (ent->v.frame < start || ent->v.frame > end)
	{ // Didn't start in the range
		ent->v.frame = start;
		result = 0;
	}
	else if (ent->v.frame == end)
	{  // Wrapping
		ent->v.frame = start;
		result = 1;
	}
	else
	{  // Regular Advance
		ent->v.frame++;
		if (ent->v.frame == end)
			result = 2;
		else
			result = 0;
	}

	G_FLOAT(OFS_RETURN) = result;
}

static void PF_RewindFrame (void)
{
	edict_t *ent;
	float start, end, result;

	ent = PROG_TO_EDICT(*sv_globals.self);
	start = G_FLOAT(OFS_PARM0);
	end = G_FLOAT(OFS_PARM1);

	if (ent->v.frame > start || ent->v.frame < end)
	{ // Didn't start in the range
		ent->v.frame = start;
		result = 0;
	}
	else if (ent->v.frame == end)
	{  // Wrapping
		ent->v.frame = start;
		result = 1;
	}
	else
	{  // Regular Advance
		ent->v.frame--;
		if (ent->v.frame == end)
			result = 2;
		else
			result = 0;
	}

	G_FLOAT(OFS_RETURN) = result;
}

#define WF_NORMAL_ADVANCE	0
#define WF_CYCLE_STARTED	1
#define WF_CYCLE_WRAPPED	2
#define WF_LAST_FRAME		3

static void PF_advanceweaponframe (void)
{
	edict_t *ent;
	float startframe, endframe;
	float state;

	ent = PROG_TO_EDICT(*sv_globals.self);
	startframe = G_FLOAT(OFS_PARM0);
	endframe = G_FLOAT(OFS_PARM1);

	if ( (endframe > startframe && (ent->v.weaponframe > endframe || ent->v.weaponframe < startframe)) ||
		(endframe < startframe && (ent->v.weaponframe < endframe || ent->v.weaponframe > startframe)) )
	{
		ent->v.weaponframe=startframe;
		state = WF_CYCLE_STARTED;
	}
	else if (ent->v.weaponframe==endframe)
	{
		ent->v.weaponframe=startframe;
		state = WF_CYCLE_WRAPPED;
	}
	else
	{
		if (startframe > endframe)
			ent->v.weaponframe = ent->v.weaponframe - 1;
		else if (startframe < endframe)
			ent->v.weaponframe = ent->v.weaponframe + 1;

		if (ent->v.weaponframe==endframe)
			state = WF_LAST_FRAME;
		else
			state = WF_NORMAL_ADVANCE;
	}

	G_FLOAT(OFS_RETURN) = state;
}

static void PF_setclass (void)
{
	float		newclass;
	int			entnum;
	edict_t	*e;
	client_t	*client, *old;

	entnum = G_EDICTNUM(OFS_PARM0);
	e = G_EDICT(OFS_PARM0);
	newclass = G_FLOAT(OFS_PARM1);

	if (entnum < 1 || entnum > svs.maxclients)
	{
		Con_Printf ("tried to sprint to a non-client\n");
		return;
	}

	client = &svs.clients[entnum-1];

	old = host_client;
	host_client = client;
	Host_ClientCommands ("playerclass %i\n", (int)newclass);
	host_client = old;

	// These will get set again after the message has filtered its way
	// but it wouldn't take affect right away
	e->v.playerclass = newclass;
	client->playerclass = newclass;
}

static void PF_starteffect (void)
{
	SV_ParseEffect(&sv.reliable_datagram);
}

static void PF_endeffect (void)
{
	int idx;

	idx = G_FLOAT(OFS_PARM0);
	idx = G_FLOAT(OFS_PARM1);

	if (!sv.Effects[idx].type)
		return;

	sv.Effects[idx].type = 0;
	MSG_WriteByte (&sv.reliable_datagram, svc_end_effect);
	MSG_WriteByte (&sv.reliable_datagram, idx);
}

#if 0	/* not used */
static void PF_randomrange(void)
{
	float num, minv, maxv;

	minv = G_FLOAT(OFS_PARM0);
	maxv = G_FLOAT(OFS_PARM1);

//	num = (rand() & 0x7fff) / ((float)0x7fff);
	num = rand() * (1.0 / RAND_MAX);

	G_FLOAT(OFS_RETURN) = ((maxv-minv) * num) + minv;
}

static void PF_randomvalue(void)
{
	float num,range;

	range = G_FLOAT(OFS_PARM0);

//	num = (rand() & 0x7fff) / ((float)0x7fff);
	num = rand() * (1.0 / RAND_MAX);

	G_FLOAT(OFS_RETURN) = range * num;
}

static void PF_randomvrange(void)
{
	float num, *minv, *maxv;
	vec3_t result;

	minv = G_VECTOR(OFS_PARM0);
	maxv = G_VECTOR(OFS_PARM1);

//	num = (rand() & 0x7fff) / ((float)0x7fff);
	num = rand() * (1.0 / RAND_MAX);
	result[0] = ((maxv[0] - minv[0]) * num) + minv[0];
//	num = (rand() & 0x7fff) / ((float)0x7fff);
	num = rand() * (1.0 / RAND_MAX);
	result[1] = ((maxv[1] - minv[1]) * num) + minv[1];
//	num = (rand() & 0x7fff) / ((float)0x7fff);
	num = rand() * (1.0 / RAND_MAX);
	result[2] = ((maxv[2] - minv[2]) * num) + minv[2];

	VectorCopy (result, G_VECTOR(OFS_RETURN));
}

static void PF_randomvvalue(void)
{
	float num, *range;
	vec3_t result;

	range = G_VECTOR(OFS_PARM0);

//	num = (rand() & 0x7fff) / ((float)0x7fff);
	num = rand() * (1.0 / RAND_MAX);
	result[0] = range[0] * num;
//	num = (rand() & 0x7fff) / ((float)0x7fff);
	num = rand() * (1.0 / RAND_MAX);
	result[1] = range[1] * num;
//	num = (rand() & 0x7fff) / ((float)0x7fff);
	num = rand() * (1.0 / RAND_MAX);
	result[2] = range[2] * num;

	VectorCopy (result, G_VECTOR(OFS_RETURN));
}
#endif	/* not used */

static void PF_concatv(void)
{
	float *in,*range;
	vec3_t result;

	in = G_VECTOR(OFS_PARM0);
	range = G_VECTOR(OFS_PARM1);

	VectorCopy (in, result);
	if (result[0] < -range[0])
		result[0] = -range[0];
	if (result[0] > range[0])
		result[0] = range[0];
	if (result[1] < -range[1])
		result[1] = -range[1];
	if (result[1] > range[1])
		result[1] = range[1];
	if (result[2] < -range[2])
		result[2] = -range[2];
	if (result[2] > range[2])
		result[2] = range[2];

	VectorCopy (result, G_VECTOR(OFS_RETURN));
}

static void PF_GetString(void)
{
	int idx = (int) G_FLOAT(OFS_PARM0);

	if (idx < 1 || idx > host_string_count)
	{
		PR_RunError ("%s: unexpected index %d (host_string_count: %d)",
					__thisfunc__, idx, host_string_count);
	}

	G_INT(OFS_RETURN) = PR_SetEngineString(Host_GetString(idx - 1));
}


static void PF_v_factor(void)
// returns (v_right * factor_x) + (v_forward * factor_y) + (v_up * factor_z)
{
	float *range;
	vec3_t result;

	range = G_VECTOR(OFS_PARM0);

	result[0] = ((*sv_globals.v_right)[0] * range[0]) +
		    ((*sv_globals.v_forward)[0] * range[1]) +
		    ((*sv_globals.v_up)[0] * range[2]);

	result[1] = ((*sv_globals.v_right)[1] * range[0]) +
		    ((*sv_globals.v_forward)[1] * range[1]) +
		    ((*sv_globals.v_up)[1] * range[2]);

	result[2] = ((*sv_globals.v_right)[2] * range[0]) +
		    ((*sv_globals.v_forward)[2] * range[1]) +
		    ((*sv_globals.v_up)[2] * range[2]);

	VectorCopy (result, G_VECTOR(OFS_RETURN));
}

static void PF_v_factorrange(void)
// returns (v_right * factor_x) + (v_forward * factor_y) + (v_up * factor_z)
{
	float num, *minv, *maxv;
	vec3_t result, r2;

	minv = G_VECTOR(OFS_PARM0);
	maxv = G_VECTOR(OFS_PARM1);

//	num = (rand() & 0x7fff) / ((float)0x7fff);
	num = rand() * (1.0 / RAND_MAX);
	result[0] = ((maxv[0] - minv[0]) * num) + minv[0];
//	num = (rand() & 0x7fff) / ((float)0x7fff);
	num = rand() * (1.0 / RAND_MAX);
	result[1] = ((maxv[1] - minv[1]) * num) + minv[1];
//	num = (rand() & 0x7fff) / ((float)0x7fff);
	num = rand() * (1.0 / RAND_MAX);
	result[2] = ((maxv[2] - minv[2]) * num) + minv[2];

	r2[0] = ((*sv_globals.v_right)[0] * result[0]) +
		((*sv_globals.v_forward)[0] * result[1]) +
		((*sv_globals.v_up)[0] * result[2]);

	r2[1] = ((*sv_globals.v_right)[1] * result[0]) +
		((*sv_globals.v_forward)[1] * result[1]) +
		((*sv_globals.v_up)[1] * result[2]);

	r2[2] = ((*sv_globals.v_right)[2] * result[0]) +
		((*sv_globals.v_forward)[2] * result[1]) +
		((*sv_globals.v_up)[2] * result[2]);

	VectorCopy (r2, G_VECTOR(OFS_RETURN));
}

static void PF_matchAngleToSlope(void)
{
	edict_t	*actor;
	vec3_t v_forward, old_forward, old_right, new_angles2 = { 0, 0, 0 };
	float pitch, mod, dot;

	// OFS_PARM0 is used by PF_vectoangles below
	actor = G_EDICT(OFS_PARM1);

	AngleVectors(actor->v.angles, old_forward, old_right, *sv_globals.v_up);

	PF_vectoangles();

	pitch = G_FLOAT(OFS_RETURN) - 90;

	new_angles2[1] = G_FLOAT(OFS_RETURN+1);

	AngleVectors(new_angles2, v_forward, *sv_globals.v_right, *sv_globals.v_up);

	mod = DotProduct(v_forward, old_right);

	if (mod < 0)
		mod = 1;
	else
		mod = -1;

	dot = DotProduct(v_forward, old_forward);

	actor->v.angles[0] = dot * pitch;
	actor->v.angles[2] = (1-fabs(dot)) * pitch * mod;
}

static void PF_updateInfoPlaque (void)
{
/* update the objectives in the mission pack :
 * see trigger_objective() and objective_use()  */
	unsigned int check;
	unsigned int idx, mode;
	unsigned int *use;

	idx = G_FLOAT(OFS_PARM0);
	mode = G_FLOAT(OFS_PARM1);

	/* index to infolist.txt must be >= 0 && < 64 */
	if (idx < 32)
	{
		use = &info_mask;
		check = 1U << idx;
	}
	else if (idx < 64)
	{
		use = &info_mask2;
		idx -= 32U;
		check = 1U << idx;
	}
	else
	{
		PR_RunError ("%s: bad objective index %u", __thisfunc__, idx);
	}

	if ( ((mode & 1 /* FORCE_ON */) &&  (*use & check)) ||
	     ((mode & 2 /* FORCE_OFF */) && !(*use & check)) )
		;
	else
	{
		*use ^= check;
	}
}

static void PF_doWhiteFlash(void)
{
/* called from mission pack's buddha.hc::buddha_die().  original code
 * used to do V_WhiteFlash_f() here affecting the local client only. */
	MSG_WriteByte (&sv.reliable_datagram, svc_stufftext);
	MSG_WriteString (&sv.reliable_datagram, "wf\n");
}


static builtin_t pr_builtin[] =
{
	PF_Fixme,

	PF_makevectors,		// void(entity e) makevectors		= #1
	PF_setorigin,		// void(entity e, vector o) setorigin	= #2
	PF_setmodel,		// void(entity e, string m) setmodel	= #3
	PF_setsize,		// void(entity e, vector min, vector max) setsize	= #4
	PF_lightstylestatic,	// 5

	PF_break,		// void() break				= #6
	PF_random,		// float() random			= #7
	PF_sound,		// void(entity e, float chan, string samp) sound	= #8
	PF_normalize,		// vector(vector v) normalize		= #9
	PF_error,		// void(string e) error			= #10
	PF_objerror,		// void(string e) objerror		= #11
	PF_vlen,		// float(vector v) vlen			= #12
	PF_vectoyaw,		// float(vector v) vectoyaw		= #13
	PF_Spawn,		// entity() spawn			= #14
	PF_Remove,		// void(entity e) remove		= #15
	PF_traceline,		// float(vector v1, vector v2, float tryents) traceline	= #16
	PF_checkclient,		// entity() clientlist			= #17
	PF_Find,		// entity(entity start, .string fld, string match) find	= #18
	PF_precache_sound,	// void(string s) precache_sound	= #19
	PF_precache_model,	// void(string s) precache_model	= #20
	PF_stuffcmd,		// void(entity client, string s)stuffcmd	= #21
	PF_findradius,		// entity(vector org, float rad) findradius	= #22
	PF_bprint,		// void(string s) bprint		= #23
	PF_sprint,		// void(entity client, string s) sprint	= #24
	PF_dprint,		// void(string s) dprint		= #25
	PF_ftos,		// void(string s) ftos			= #26
	PF_vtos,		// void(string s) vtos			= #27
	PF_coredump,		// PF_coredump	= #28
	PF_traceon,		// PF_traceon	= #29
	PF_traceoff,		// PF_traceoff	= #30
	PF_eprint,		// void(entity e) debug print an entire entity	= #31
	PF_walkmove,		// float(float yaw, float dist) walkmove	= #32
	PF_tracearea,		// float(vector v1, vector v2, vector mins, vector maxs, 
				//		float tryents) traceline	= #33
	PF_droptofloor,		// PF_droptofloor = #34
	PF_lightstyle,		// 35
	PF_rint,		// 36
	PF_floor,		// 37
	PF_ceil,		// 38
	PF_Fixme,
	PF_checkbottom,		// 40
	PF_pointcontents,	// 41
	PF_particle2,
	PF_fabs,		// 43
	PF_aim,			// 44
	PF_cvar,		// 45
	PF_localcmd,		// 46
	PF_nextent,		// 47
	PF_particle,		// 48
	PF_changeyaw,		// 49
	PF_vhlen,		// float(vector v) vhlen		= #50
	PF_vectoangles,		// 51

	PF_WriteByte,		// 52
	PF_WriteChar,		// 53
	PF_WriteShort,		// 54
	PF_WriteLong,		// 55
	PF_WriteCoord,		// 56
	PF_WriteAngle,		// 57
	PF_WriteString,		// 58
	PF_WriteEntity,		// 59

//	PF_FindPath,		// 60
	PF_dprintf,		// void(string s1, string s2) dprint	= #60
	PF_Cos,			// 61
	PF_Sin,			// 62
	PF_AdvanceFrame,	// 63
	PF_dprintv,		// void(string s1, string s2) dprint	= #64
	PF_RewindFrame,		// 65
	PF_setclass,

	SV_MoveToGoal,
	PF_precache_file,
	PF_makestatic,

	PF_changelevel,

	PF_lightstylevalue,	// 71

	PF_cvar_set,
	PF_centerprint,

	PF_ambientsound,

	PF_precache_model2,
	PF_precache_sound2,	// precache_sound2 is different only for qcc
	PF_precache_file,

	PF_setspawnparms,
	PF_plaque_draw,
	PF_rain_go,		// 80
	PF_particleexplosion,	// 81
	PF_movestep,
	PF_advanceweaponframe,
	PF_sqrt,

	PF_particle3,		// 85
	PF_particle4,		// 86
	PF_setpuzzlemodel,	// 87

	PF_starteffect,		// 88
	PF_endeffect,		// 89

	PF_precache_puzzle_model,	// 90
	PF_concatv,		// 91
	PF_GetString,		// 92
	PF_SpawnTemp,		// 93
	PF_v_factor,		// 94
	PF_v_factorrange,	// 95

	PF_precache_sound3,	// 96
	PF_precache_model3,	// 97
	PF_precache_file,	// 98
	PF_matchAngleToSlope,	// 99
	PF_updateInfoPlaque,	// 100

	PF_precache_sound4,	// 101
	PF_precache_model4,	// 102
	PF_precache_file,	// 103

	PF_doWhiteFlash,	// 104
	PF_UpdateSoundPos,	// 105
	PF_StopSound,		// 106

#ifdef QUAKE2
	PF_sin,
	PF_cos,
	PF_sqrt,
	PF_changepitch,
	PF_TraceToss,
	PF_etos,
	PF_WaterMove,
#else
	PF_Fixme,
	PF_Fixme,
	PF_Fixme,
	PF_Fixme,
	PF_Fixme,
	PF_Fixme,
	PF_Fixme,
#endif

};

builtin_t *pr_builtins = pr_builtin;
int pr_numbuiltins = sizeof(pr_builtin)/sizeof(pr_builtin[0]);

