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
#define	RETURN_STRING(s) (((int *)pr_globals)[OFS_RETURN] = PR_SetEngineString(s))

#define	MSG_BROADCAST	0		// unreliable to all
#define	MSG_ONE		1		// reliable to one (msg_entity)
#define	MSG_ALL		2		// reliable to all
#define	MSG_INIT	3		// write to the init string
#define	MSG_MULTICAST	4		// for multicast()


#if defined(SERVERONLY)
static int	d_lightstylevalue[256];
#else
extern int	d_lightstylevalue[256];
#endif
static sizebuf_t *WriteDest (void);


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
	int	level;

	level = G_FLOAT(OFS_PARM0);

	s = PF_VarString(1);

	if (spartanPrint.integer && level < 2)
		return;

	SV_BroadcastPrintf (level, "%s", s);
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
	int	level;

	entnum = G_EDICTNUM(OFS_PARM0);
	level = G_FLOAT(OFS_PARM1);

	s = PF_VarString(2);

	if (entnum < 1 || entnum > MAX_CLIENTS)
	{
		Con_Printf ("tried to sprint to a non-client\n");
		return;
	}

	client = &svs.clients[entnum-1];

	if (spartanPrint.integer && level < 2)
		return;

	SV_ClientPrintf (client, level, "%s", s);
}

/*
=================
PF_name_print

print player's name

name_print(to, level, who)
=================
*/
static void PF_name_print (void)
{
	int idx, style;

	idx = (int) G_EDICTNUM(OFS_PARM2);
	style = (int) G_FLOAT(OFS_PARM1);

	if (spartanPrint.integer && style < 2)
		return;

	if (idx < 1 || idx > MAX_CLIENTS)
		PR_RunError ("%s: unexpected index %d", __thisfunc__, idx);

	if ((int)G_FLOAT(OFS_PARM0) == MSG_BROADCAST)
	{/* broadcast message--send like bprint, print it out on server too. */
		client_t	*cl;
		int			i;

		Sys_Printf("%s", svs.clients[idx - 1].name);

		for (i = 0, cl = svs.clients; i < MAX_CLIENTS; i++, cl++)
		{
			if (style < cl->messagelevel)
				continue;
			if (cl->state != cs_spawned)
			{//should i be checking cs_connected too?
				if (cl->state)
				{//not fully in so won't know name yet, explicitly say the name
					MSG_WriteByte (&cl->netchan.message, svc_print);
					MSG_WriteByte (&cl->netchan.message, style);
					MSG_WriteString (&cl->netchan.message, svs.clients[idx - 1].name);
				}
				continue;
			}
			MSG_WriteByte (&cl->netchan.message, svc_name_print);
			MSG_WriteByte (&cl->netchan.message, style);
			//knows the name, send the index.
			MSG_WriteByte (&cl->netchan.message, idx - 1);
		}
		return;
	}

	MSG_WriteByte (WriteDest(), svc_name_print);
	MSG_WriteByte (WriteDest(), style);
	MSG_WriteByte (WriteDest(), idx - 1);//heh, don't need a short here.
}


/*
=================
PF_print_indexed

print string from strings.txt

print_indexed(to, level, index)
=================
*/
static void PF_print_indexed (void)
{
	int idx, style;

	idx = (int) G_FLOAT(OFS_PARM2);
	style = (int) G_FLOAT(OFS_PARM1);

	if (spartanPrint.integer && style < 2)
		return;

	if (idx < 1 || idx > host_string_count)
	{
		PR_RunError ("%s: unexpected index %d (host_string_count: %d)",
					__thisfunc__, idx, host_string_count);
	}

	if ((int)G_FLOAT(OFS_PARM0) == MSG_BROADCAST)
	{/* broadcast message--send like bprint, print it out on server too. */
		client_t	*cl;
		int			i;

		Sys_Printf("%s", Host_GetString(idx - 1));

		for (i = 0, cl = svs.clients; i < MAX_CLIENTS; i++, cl++)
		{
			if (style < cl->messagelevel)
				continue;
			if (!cl->state)
				continue;
			MSG_WriteByte (&cl->netchan.message, svc_indexed_print);
			MSG_WriteByte (&cl->netchan.message, style);
			MSG_WriteShort (&cl->netchan.message, idx);
		}
		return;
	}

	MSG_WriteByte (WriteDest(), svc_indexed_print);
	MSG_WriteByte (WriteDest(), style);
	MSG_WriteShort (WriteDest(), idx);
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

	if (entnum < 1 || entnum > MAX_CLIENTS)
	{
		Con_Printf ("tried to sprint to a non-client\n");
		return;
	}

	client = &svs.clients[entnum-1];

	MSG_WriteChar (&client->netchan.message,svc_centerprint);
	MSG_WriteString (&client->netchan.message, s);
}


/*
=================
PF_bcenterprint2

single print to all

bcenterprint2(value, value)
=================
*/
static void PF_bcenterprint2 (void)
{
	char		*s;
	client_t	*cl;
	int	i;

	s = PF_VarString(0);

	for (i = 0, cl = svs.clients; i < MAX_CLIENTS; i++, cl++)
	{
		if (!cl->state)
			continue;
		MSG_WriteByte (&cl->netchan.message, svc_centerprint);
		MSG_WriteString (&cl->netchan.message, s);
	}
}

/*
=================
PF_centerprint2

single print to a specific client

centerprint(clientent, value, value)
=================
*/
static void PF_centerprint2 (void)
{
	char		*s;
	client_t	*client;
	int	entnum;

	entnum = G_EDICTNUM(OFS_PARM0);
	s = PF_VarString(1);

	if (entnum < 1 || entnum > MAX_CLIENTS)
	{
		Con_Printf ("tried to sprint to a non-client\n");
		return;
	}

	client = &svs.clients[entnum-1];

	MSG_WriteChar (&client->netchan.message,svc_centerprint);
	MSG_WriteString (&client->netchan.message, s);
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

	num = (rand() & 0x7fff) / ((float)0x7fff);

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

	pos = G_VECTOR (OFS_PARM0);
	samp = G_STRING(OFS_PARM1);
	vol = G_FLOAT(OFS_PARM2);
	attenuation = G_FLOAT(OFS_PARM3);

// check to see if samp was properly precached
	for (soundnum = 0, check = sv.sound_precache;
	     soundnum < MAX_SOUNDS && *check; soundnum++, check++)
	{
		if (!strcmp(*check, samp))
			break;
	}

	if (soundnum == MAX_SOUNDS || !*check)
	{
		Con_Printf ("no precache: %s\n", samp);
		return;
	}

// add an svc_spawnambient command to the level signon packet
	MSG_WriteByte (&sv.signon,svc_spawnstaticsound);
	for (i = 0; i < 3; i++)
		MSG_WriteCoord(&sv.signon, pos[i]);

	MSG_WriteByte (&sv.signon, soundnum);

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

static void PR_SetTrace (trace_t *trace)
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

	PR_SetTrace (&trace);

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

	PR_SetTrace (&trace);
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

	PR_SetTrace (&trace);
}

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
	if (check > MAX_CLIENTS)
		check = MAX_CLIENTS;

	if (check == MAX_CLIENTS)
		i = 1;
	else
		i = check + 1;

	for ( ;  ; i++)
	{
		if (i == MAX_CLIENTS+1)
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
	if (sv.time - sv.lastchecktime >= HX_FRAME_TIME)
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
	if (entnum < 1 || entnum > MAX_CLIENTS)
		PR_RunError ("Parm 0 not a client");
	str = G_STRING(OFS_PARM1);

	old = host_client;
	host_client = &svs.clients[entnum-1];

	MSG_WriteByte (&host_client->netchan.message, svc_stufftext);
	MSG_WriteString (&host_client->netchan.message, str);

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
	if (i <= MAX_CLIENTS)
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
	int		i;

	if (sv.state != ss_loading && !ignore_precache)
		PR_RunError ("%s: Precache can only be done in spawn functions", __thisfunc__);

	s = G_STRING(OFS_PARM0);
	G_INT(OFS_RETURN) = G_INT(OFS_PARM0);
	PR_CheckEmptyString (s);

	for (i = 0; i < MAX_SOUNDS; i++)
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

	for (j = 0, client = svs.clients; j < MAX_CLIENTS; j++, client++)
	{
		if (client->state == cs_spawned)
		{
			MSG_WriteChar (&client->netchan.message, svc_lightstyle);
			MSG_WriteChar (&client->netchan.message, style);
			MSG_WriteString (&client->netchan.message, val);
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

//	G_FLOAT(OFS_RETURN) = 0;
	G_FLOAT(OFS_RETURN) = (int)d_lightstylevalue[style];
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
	d_lightstylevalue[styleNumber] = value;

	if (sv.state != ss_active)
	{
		return;
	}

	// Send message to all clients on this server
	for (i = 0, client = svs.clients; i < MAX_CLIENTS; i++, client++)
	{
		if (client->state == cs_spawned)
		{
			MSG_WriteChar (&client->netchan.message, svc_lightstyle);
			MSG_WriteChar (&client->netchan.message, styleNumber);
			MSG_WriteString (&client->netchan.message, styleString);
		}
	}
}

static void PF_rint (void)
{
	float	f;

	f = G_FLOAT(OFS_PARM0);
	if (f > 0)
		G_FLOAT(OFS_RETURN) = (int)(f + 0.5);
	else
		G_FLOAT(OFS_RETURN) = (int)(f - 0.5);
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
		if (entnum < 1 || entnum > MAX_CLIENTS)
			PR_RunError ("%s: not a client", __thisfunc__);
		return &svs.clients[entnum-1].netchan.message;

	case MSG_ALL:
		return &sv.reliable_datagram;

	case MSG_INIT:
		if (sv.state != ss_loading)
			PR_RunError ("%s: MSG_INIT can only be written in spawn functions", __thisfunc__);
		return &sv.signon;

	case MSG_MULTICAST:
		return &sv.multicast;

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
	if (i < 1 || i > MAX_CLIENTS)
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
		Cbuf_AddText (va("map %s %s\n",s1, s2));
	else
		Cbuf_AddText (va("changelevel2 %s %s\n",s1, s2));
}


/*
==============
PF_logfrag

logfrag (killer, killee)
==============
*/
static void PF_logfrag (void)
{
	edict_t	*ent1, *ent2;
	int		e1, e2;
	char	*s;

	ent1 = G_EDICT(OFS_PARM0);
	ent2 = G_EDICT(OFS_PARM1);

	e1 = NUM_FOR_EDICT(ent1);
	e2 = NUM_FOR_EDICT(ent2);

	if (e1 < 1 || e1 > MAX_CLIENTS ||
	    e2 < 1 || e2 > MAX_CLIENTS)
		return;

	s = va("\\%s\\%s\\\n",svs.clients[e1-1].name, svs.clients[e2-1].name);

	SZ_Print (&svs.log[svs.logsequence&1], s);
	if (sv_fraglogfile)
	{
		fprintf (sv_fraglogfile, "%s", s);
		fflush (sv_fraglogfile);
	}
}


/*
==============
PF_infokey

string(entity e, string key) infokey
==============
*/
static void PF_infokey (void)
{
	edict_t		*e;
	int		e1;
	const char	*value;
	const char	*key;

	e = G_EDICT(OFS_PARM0);
	e1 = NUM_FOR_EDICT(e);
	key = G_STRING(OFS_PARM1);

	if (e1 == 0)
	{
		value = Info_ValueForKey (svs.info, key);
		if (!*value)
			value = Info_ValueForKey(localinfo, key);
	}
	else if (e1 <= MAX_CLIENTS)
		value = Info_ValueForKey (svs.clients[e1-1].userinfo, key);
	else
		value = "";

	RETURN_STRING(value);
}

/*
==============
PF_stof

float(string s) stof
==============
*/
static void PF_stof (void)
{
	const char	*s;

	s = G_STRING(OFS_PARM0);

	G_FLOAT(OFS_RETURN) = atof(s);
}


/*
==============
PF_multicast

void(vector where, float set) multicast
==============
*/
static void PF_multicast (void)
{
	float	*o;
	int		to;

	o = G_VECTOR(OFS_PARM0);
	to = G_FLOAT(OFS_PARM1);

	SV_Multicast (o, to);
}


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
	vec3_t	org, org2;
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

	org2[0] = e_size[0];
	org2[1] = e_size[1];
	org2[2] = e_size[2];

	x_dir = dir[0];
	y_dir = dir[1];

	SV_StartRainEffect (org,org2,x_dir,y_dir,color,count);
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

	if ( (start < end && (ent->v.frame < start || ent->v.frame > end)) ||
		(start > end && (ent->v.frame > start || ent->v.frame < end)) )
	{ // Didn't start in the range
		ent->v.frame = start;
		result = 0;
	}
	else if (ent->v.frame == end)
	{  // Wrapping
		ent->v.frame = start;
		result = 1;
	}
	else if (end > start)
	{  // Regular Advance
		ent->v.frame++;
		if (ent->v.frame == end)
			result = 2;
		else
			result = 0;
	}
	else if (end < start)
	{  // Reverse Advance
		ent->v.frame--;
		if (ent->v.frame == end)
			result = 2;
		else
			result = 0;
	}
	else
	{
		ent->v.frame = end;
		result = 1;
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
	char		temp[1024];

	entnum = G_EDICTNUM(OFS_PARM0);
	e = G_EDICT(OFS_PARM0);
	newclass = G_FLOAT(OFS_PARM1);

	if (entnum < 1 || entnum > MAX_CLIENTS)
	{
		Con_Printf ("tried to change class of a non-client\n");
		return;
	}

	client = &svs.clients[entnum-1];

	old = host_client;
	host_client = client;

	if (newclass > CLASS_DEMON && (dmMode.integer != DM_SIEGE || !SV_PROGS_HAVE_SIEGE))
		newclass = CLASS_PALADIN;

	e->v.playerclass = newclass;
	host_client->playerclass = newclass;

	sprintf(temp,"%d",(int)newclass);
	Info_SetValueForKey (host_client->userinfo, "playerclass", temp, MAX_INFO_STRING);
	q_strlcpy (host_client->name, Info_ValueForKey(host_client->userinfo, "name"), sizeof(host_client->name));
	host_client->sendinfo = true;

	// process any changed values
//	SV_ExtractFromUserinfo (host_client);

	//update everyone else about playerclass change
	MSG_WriteByte (&sv.reliable_datagram, svc_updatepclass);
	MSG_WriteByte (&sv.reliable_datagram, entnum - 1);
	MSG_WriteByte (&sv.reliable_datagram, ((host_client->playerclass<<5)|((int)e->v.level&31)));
	host_client = old;
}

static void PF_setsiegeteam (void)
{
	float		newteam;
	int		entnum;
	edict_t		*e;
	client_t	*client,*old;
//	char		temp[1024];

	if (! SV_PROGS_HAVE_SIEGE)	/* paranoia? */
		return;

	entnum = G_EDICTNUM(OFS_PARM0);
	e = G_EDICT(OFS_PARM0);
	newteam = G_FLOAT(OFS_PARM1);

	if (entnum < 1 || entnum > MAX_CLIENTS)
	{
		Con_Printf ("tried to change siege_team of a non-client\n");
		return;
	}

	client = &svs.clients[entnum-1];

	old = host_client;
	host_client = client;

	e->v.siege_team = newteam;
	host_client->siege_team = newteam;

//???
//	sprintf(temp,"%d",(int)newteam);
//	Info_SetValueForKey (host_client->userinfo, "playerclass", temp, MAX_INFO_STRING);
//	q_strlcpy (host_client->name, Info_ValueForKey(host_client->userinfo, "name"), sizeof(host_client->name));
//	host_client->sendinfo = true;

	//update everyone else about playerclass change
	MSG_WriteByte (&sv.reliable_datagram, svc_updatesiegeteam);
	MSG_WriteByte (&sv.reliable_datagram, entnum - 1);
	MSG_WriteByte (&sv.reliable_datagram, host_client->siege_team);
	host_client = old;
}

static void PF_updateSiegeInfo (void)
{
	int			j;
	client_t	*client;

	for (j = 0, client = svs.clients; j < MAX_CLIENTS; j++, client++)
	{
		if (client->state < cs_connected)
			continue;
		MSG_WriteByte (&client->netchan.message, svc_updatesiegeinfo);
		MSG_WriteByte (&client->netchan.message, (int)ceil(timelimit.value));
		MSG_WriteByte (&client->netchan.message, (int)ceil(fraglimit.value));
	}
}

static void PF_starteffect (void)
{
	SV_ParseEffect(NULL);
}

static void PF_endeffect (void)
{
	int idx;

	idx = G_FLOAT(OFS_PARM0);
	idx = G_FLOAT(OFS_PARM1);

	if (!sv.Effects[idx].type)
		return;

	sv.Effects[idx].type = 0;
	MSG_WriteByte (&sv.multicast, svc_end_effect);
	MSG_WriteByte (&sv.multicast, idx);
	SV_Multicast (vec3_origin, MULTICAST_ALL_R);
}

static void PF_turneffect (void)
{
	float *dir, *pos;
	int idx;

	idx = G_FLOAT(OFS_PARM0);
	pos = G_VECTOR(OFS_PARM1);
	dir = G_VECTOR(OFS_PARM2);

	if (!sv.Effects[idx].type)
		return;
	VectorCopy(pos, sv.Effects[idx].ef.Missile.origin);
	VectorCopy(dir, sv.Effects[idx].ef.Missile.velocity);

	MSG_WriteByte (&sv.multicast, svc_turn_effect);
	MSG_WriteByte (&sv.multicast, idx);
	MSG_WriteFloat(&sv.multicast, sv.time);
	MSG_WriteCoord(&sv.multicast, pos[0]);
	MSG_WriteCoord(&sv.multicast, pos[1]);
	MSG_WriteCoord(&sv.multicast, pos[2]);
	MSG_WriteCoord(&sv.multicast, dir[0]);
	MSG_WriteCoord(&sv.multicast, dir[1]);
	MSG_WriteCoord(&sv.multicast, dir[2]);

	SV_MulticastSpecific (sv.Effects[idx].client_list, true);
}

static void PF_updateeffect (void)
//type-specific what this will send
{
	int idx, type, cmd;
	vec3_t tvec;

	// the effect we're lookin to change is parm 0
	idx = G_FLOAT(OFS_PARM0);
	// the type of effect that it had better be is parm 1
	type = G_FLOAT(OFS_PARM1);

	if (!sv.Effects[idx].type)
		return;

	if (sv.Effects[idx].type != type)
		return;

	//common writing--PLEASE use sent type when determining
	// how much and what to read, so it's safe
	MSG_WriteByte (&sv.multicast, svc_update_effect);
	MSG_WriteByte (&sv.multicast, idx);
	//paranoia alert--make sure client reads the correct number of bytes
	MSG_WriteByte (&sv.multicast, type);

	switch (type)
	{
	case CE_SCARABCHAIN:
		//new ent to be attached to--pass in 0 for chain retract
		sv.Effects[idx].ef.Chain.owner = G_INT(OFS_PARM2) & 0x0fff;
		sv.Effects[idx].ef.Chain.material = G_INT(OFS_PARM2) >> 12;

		if (sv.Effects[idx].ef.Chain.owner)
			sv.Effects[idx].ef.Chain.state = 1;
		else
			sv.Effects[idx].ef.Chain.state = 2;

		MSG_WriteShort (&sv.multicast, G_EDICTNUM(OFS_PARM2));
		break;
	case CE_HWSHEEPINATOR:
	case CE_HWXBOWSHOOT:
		cmd = G_FLOAT(OFS_PARM2);
		MSG_WriteByte (&sv.multicast, cmd);
		if (cmd & 1)
		{
			sv.Effects[idx].ef.Xbow.activebolts &= ~(1 << ((cmd >> 4) & 7));
			MSG_WriteCoord (&sv.multicast, G_FLOAT(OFS_PARM3));
		}
		else
		{
			sv.Effects[idx].ef.Xbow.vel[(cmd >> 4) & 7][0] = G_FLOAT(OFS_PARM3);
			sv.Effects[idx].ef.Xbow.vel[(cmd >> 4) & 7][1] = G_FLOAT(OFS_PARM4);
			sv.Effects[idx].ef.Xbow.vel[(cmd >> 4) & 7][2] = 0;

			MSG_WriteAngle (&sv.multicast, G_FLOAT(OFS_PARM3));
			MSG_WriteAngle (&sv.multicast, G_FLOAT(OFS_PARM4));
			if (cmd & 128)//send origin too
			{
				sv.Effects[idx].ef.Xbow.turnedbolts |= 1 << ((cmd >> 4) & 7);
				VectorCopy(G_VECTOR(OFS_PARM5), sv.Effects[idx].ef.Xbow.origin[(cmd >> 4) & 7]);
				MSG_WriteCoord (&sv.multicast, sv.Effects[idx].ef.Xbow.origin[(cmd >> 4) & 7][0]);
				MSG_WriteCoord (&sv.multicast, sv.Effects[idx].ef.Xbow.origin[(cmd >> 4) & 7][1]);
				MSG_WriteCoord (&sv.multicast, sv.Effects[idx].ef.Xbow.origin[(cmd >> 4) & 7][2]);
			}
		}
		break;
	case CE_HWDRILLA:
		cmd = G_FLOAT(OFS_PARM2);
		MSG_WriteByte(&sv.multicast, cmd);
		if (cmd == 0)
		{
			VectorCopy(G_VECTOR(OFS_PARM3), tvec);
			MSG_WriteCoord (&sv.multicast, tvec[0]);
			MSG_WriteCoord (&sv.multicast, tvec[1]);
			MSG_WriteCoord (&sv.multicast, tvec[2]);
			MSG_WriteByte (&sv.multicast, G_FLOAT(OFS_PARM4));
		}
		else
		{
			sv.Effects[idx].ef.Missile.angle[0] = G_FLOAT(OFS_PARM3);
			MSG_WriteAngle (&sv.multicast, G_FLOAT(OFS_PARM3));
			sv.Effects[idx].ef.Missile.angle[1] = G_FLOAT(OFS_PARM4);
			MSG_WriteAngle (&sv.multicast, G_FLOAT(OFS_PARM4));

			VectorCopy(G_VECTOR(OFS_PARM5), sv.Effects[idx].ef.Missile.origin);
			MSG_WriteCoord (&sv.multicast, sv.Effects[idx].ef.Missile.origin[0]);
			MSG_WriteCoord (&sv.multicast, sv.Effects[idx].ef.Missile.origin[1]);
			MSG_WriteCoord (&sv.multicast, sv.Effects[idx].ef.Missile.origin[2]);
		}
		break;
	}

	SV_MulticastSpecific (sv.Effects[idx].client_list, true);
}

#if 0	/* not used */
static void PF_randomrange(void)
{
	float num, minv, maxv;

	minv = G_FLOAT(OFS_PARM0);
	maxv = G_FLOAT(OFS_PARM1);

	num = (rand() & 0x7fff) / ((float)0x7fff);

	G_FLOAT(OFS_RETURN) = ((maxv-minv) * num) + minv;
}

static void PF_randomvalue(void)
{
	float num,range;

	range = G_FLOAT(OFS_PARM0);

	num = (rand() & 0x7fff) / ((float)0x7fff);

	G_FLOAT(OFS_RETURN) = range * num;
}

static void PF_randomvrange(void)
{
	float num, *minv, *maxv;
	vec3_t result;

	minv = G_VECTOR(OFS_PARM0);
	maxv = G_VECTOR(OFS_PARM1);

	num = (rand() & 0x7fff) / ((float)0x7fff);
	result[0] = ((maxv[0] - minv[0]) * num) + minv[0];
	num = (rand() & 0x7fff) / ((float)0x7fff);
	result[1] = ((maxv[1] - minv[1]) * num) + minv[1];
	num = (rand() & 0x7fff) / ((float)0x7fff);
	result[2] = ((maxv[2] - minv[2]) * num) + minv[2];

	VectorCopy (result, G_VECTOR(OFS_RETURN));
}

static void PF_randomvvalue(void)
{
	float num, *range;
	vec3_t result;

	range = G_VECTOR(OFS_PARM0);

	num = (rand() & 0x7fff) / ((float)0x7fff);
	result[0] = range[0] * num;
	num = (rand() & 0x7fff) / ((float)0x7fff);
	result[1] = range[1] * num;
	num = (rand() & 0x7fff) / ((float)0x7fff);
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

	num = (rand() & 0x7fff) / ((float)0x7fff);
	result[0] = ((maxv[0] - minv[0]) * num) + minv[0];
	num = (rand() & 0x7fff) / ((float)0x7fff);
	result[1] = ((maxv[1] - minv[1]) * num) + minv[1];
	num = (rand() & 0x7fff) / ((float)0x7fff);
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


extern void SV_setseed(int seed);
extern float SV_seedrand(void);
extern float SV_GetMultiEffectId(void);
extern void SV_ParseMultiEffect(sizebuf_t *sb);

static void PF_setseed(void)
{
	SV_setseed(G_FLOAT(OFS_PARM0));
}

static void PF_seedrand(void)
{
	G_FLOAT(OFS_RETURN) = SV_seedrand();
}

static void PF_multieffect(void)
{
	SV_ParseMultiEffect(&sv.reliable_datagram);
}

static void PF_getmeid(void)
{
	G_FLOAT(OFS_RETURN) = SV_GetMultiEffectId();
}

static void PF_weapon_sound(void)
{
	edict_t	*entity;
	int	sound_num;
	const char	*sample;

	entity = G_EDICT(OFS_PARM0);
	sample = G_STRING(OFS_PARM1);

	for (sound_num = 1; sound_num < MAX_SOUNDS && sv.sound_precache[sound_num]; sound_num++)
	{
		if (!strcmp(sample, sv.sound_precache[sound_num]))
			break;
	}

	if (sound_num == MAX_SOUNDS || !sv.sound_precache[sound_num])
	{
		Con_Printf ("%s: %s not precached\n", __thisfunc__, sample);
		return;
	}
	entity->v.wpn_sound = sound_num;
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

	PF_logfrag,		// 99

	PF_infokey,		// 100
	PF_stof,		// 101
	PF_multicast,		// 102
	PF_turneffect,		// 103
	PF_updateeffect,	// 104
	PF_setseed,		// 105
	PF_seedrand,		// 106
	PF_multieffect,		// 107
	PF_getmeid,		// 108
	PF_weapon_sound,	// 109
	PF_bcenterprint2,	// 110
	PF_print_indexed,	// 111
	PF_centerprint2,	// 112
	PF_name_print,		// 113
	PF_StopSound,		// 114
	PF_UpdateSoundPos,	// 115

	PF_precache_sound,	// 116
	PF_precache_model,	// 117
	PF_precache_file,	// 118
	PF_setsiegeteam,	// 119
	PF_updateSiegeInfo,	// 120
};

builtin_t *pr_builtins = pr_builtin;
int pr_numbuiltins = sizeof(pr_builtin)/sizeof(pr_builtin[0]);

