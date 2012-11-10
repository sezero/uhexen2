/*
 * sv_main.c -- server main program
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

server_t	sv;
server_static_t	svs;

static char	localmodels[MAX_MODELS][8];	// inline model names for precache

static	cvar_t	sv_sound_distance	= {"sv_sound_distance", "800", CVAR_NONE};
						/* doesn't seem functional, but the hcode calls it */

static	cvar_t	sv_update_player	= {"sv_update_player", "1", CVAR_ARCHIVE};
static	cvar_t	sv_update_monsters	= {"sv_update_monsters", "1", CVAR_ARCHIVE};
static	cvar_t	sv_update_missiles	= {"sv_update_missiles", "1", CVAR_ARCHIVE};
static	cvar_t	sv_update_misc		= {"sv_update_misc", "1", CVAR_ARCHIVE};

cvar_t	sv_ce_scale		= {"sv_ce_scale", "0", CVAR_ARCHIVE};
cvar_t	sv_ce_max_size		= {"sv_ce_max_size", "0", CVAR_ARCHIVE};

extern	cvar_t	sv_maxvelocity;
extern	cvar_t	sv_gravity;
extern	cvar_t	sv_nostep;
extern	cvar_t	sv_friction;
extern	cvar_t	sv_edgefriction;
extern	cvar_t	sv_stopspeed;
extern	cvar_t	sv_maxspeed;
extern	cvar_t	sv_accelerate;
extern	cvar_t	sv_altnoclip;
extern	cvar_t	sv_idealpitchscale;
extern	cvar_t	sv_idealrollscale;
extern	cvar_t	sv_aim;
extern	cvar_t	sv_walkpitch;
extern	cvar_t	sv_flypitch;

int		current_skill;
int		sv_protocol = PROTOCOL_VERSION;	/* protocol version to use */
int		sv_kingofhill;		/* mission pack, king of the hill. */
unsigned int	info_mask, info_mask2;	/* mission pack, objectives */

extern float	scr_centertime_off;

//============================================================================


static void Sv_Edicts_f(void);

/*
===============
SV_Init
===============
*/
void SV_Init (void)
{
	int		i;
	const char	*p;

	Cvar_RegisterVariable (&sv_maxvelocity);
	Cvar_RegisterVariable (&sv_gravity);
	Cvar_RegisterVariable (&sv_friction);
	Cvar_SetCallback (&sv_gravity, Host_Callback_Notify);
	Cvar_SetCallback (&sv_friction, Host_Callback_Notify);
	Cvar_RegisterVariable (&sv_edgefriction);
	Cvar_RegisterVariable (&sv_stopspeed);
	Cvar_RegisterVariable (&sv_maxspeed);
	Cvar_SetCallback (&sv_maxspeed, Host_Callback_Notify);
	Cvar_RegisterVariable (&sv_accelerate);
	Cvar_RegisterVariable (&sv_altnoclip);
	Cvar_RegisterVariable (&sv_idealpitchscale);
	Cvar_RegisterVariable (&sv_idealrollscale);
	Cvar_RegisterVariable (&sv_aim);
	Cvar_RegisterVariable (&sv_nostep);
	Cvar_RegisterVariable (&sv_walkpitch);
	Cvar_RegisterVariable (&sv_flypitch);
	Cvar_RegisterVariable (&sv_sound_distance);
	Cvar_RegisterVariable (&sv_update_player);
	Cvar_RegisterVariable (&sv_update_monsters);
	Cvar_RegisterVariable (&sv_update_missiles);
	Cvar_RegisterVariable (&sv_update_misc);
	Cvar_RegisterVariable (&sv_ce_scale);
	Cvar_RegisterVariable (&sv_ce_max_size);

	SV_UserInit ();

	Cmd_AddCommand ("sv_edicts", Sv_Edicts_f);	

	for (i = 0; i < MAX_MODELS; i++)
		sprintf (localmodels[i], "*%i", i);

	// initialize King of Hill to world
	sv_kingofhill = 0;

	i = COM_CheckParm ("-protocol");
	if (i && i < com_argc - 1)
		sv_protocol = atoi (com_argv[i + 1]);
	switch (sv_protocol)
	{
	case PROTOCOL_RAVEN_111:
		p = "Raven/H2/1.11";
		break;
	case PROTOCOL_RAVEN_112:
		p = "Raven/MP/1.12";
		break;
	case PROTOCOL_UQE_113:
		p = "UQE/1.13";
		break;
	default:
		Sys_Error ("Bad protocol version request %i. Accepted values: %i, %i, %i.",
				sv_protocol, PROTOCOL_RAVEN_111, PROTOCOL_RAVEN_112, PROTOCOL_UQE_113);
		p = "Unknown";
	}
	Sys_Printf ("Server using protocol %i (%s)\n", sv_protocol, p);
}

void SV_Edicts (const char *Name)
{
	FILE	*FH;
	int		i;
	edict_t	*e;

	FH = fopen(FS_MakePath(FS_USERDIR,NULL,Name), "w");
	if (!FH)
	{
		Con_Printf("Could not open %s\n", Name);
		return;
	}

	fprintf(FH, "Number of Edicts: %d\n", sv.num_edicts);
	fprintf(FH, "Server Time: %f\n", sv.time);
	fprintf(FH, "\n");
	fprintf(FH, "Num.     Time Class Name                     Model                          Think                                    Touch                                    Use\n");
	fprintf(FH, "---- -------- ------------------------------ ------------------------------ ---------------------------------------- ---------------------------------------- ----------------------------------------\n");

	for (i = 1; i < sv.num_edicts; i++)
	{
		e = EDICT_NUM(i);
		fprintf(FH, "%3d. %8.2f %-30s %-30s %-40s %-40s %-40s\n",
			i, e->v.nextthink, PR_GetString(e->v.classname), PR_GetString(e->v.model),
			PR_GetString(pr_functions[e->v.think].s_name), PR_GetString(pr_functions[e->v.touch].s_name),
			PR_GetString(pr_functions[e->v.use].s_name));
	}
	fclose(FH);
}

static void Sv_Edicts_f (void)
{
	const char	*Name;

	if (!sv.active)
	{
		Con_Printf("This command can only be executed on a server running a map\n");
		return;
	}

	if (Cmd_Argc() < 2)
	{
		Name = "edicts.txt";
	}
	else
	{
		Name = Cmd_Argv(1);
	}

	SV_Edicts(Name);
}

/*
=============================================================================

EVENT MESSAGES

=============================================================================
*/

/*
==================
SV_StartParticle

Make sure the event gets sent to all clients
==================
*/
void SV_StartParticle (vec3_t org, vec3_t dir, int color, int count)
{
	int		i, v;

	if (sv.datagram.cursize > MAX_DATAGRAM-16)
		return;
	MSG_WriteByte (&sv.datagram, svc_particle);
	MSG_WriteCoord (&sv.datagram, org[0]);
	MSG_WriteCoord (&sv.datagram, org[1]);
	MSG_WriteCoord (&sv.datagram, org[2]);
	for (i = 0; i < 3; i++)
	{
		v = dir[i] * 16;
		if (v > 127)
			v = 127;
		else if (v < -128)
			v = -128;
		MSG_WriteChar (&sv.datagram, v);
	}
	MSG_WriteByte (&sv.datagram, count);
	MSG_WriteByte (&sv.datagram, color);
}

/*
==================
SV_StartParticle2

Make sure the event gets sent to all clients
==================
*/
void SV_StartParticle2 (vec3_t org, vec3_t dmin, vec3_t dmax, int color, int effect, int count)
{
	if (sv.datagram.cursize > MAX_DATAGRAM-36)
		return;
	MSG_WriteByte (&sv.datagram, svc_particle2);
	MSG_WriteCoord (&sv.datagram, org[0]);
	MSG_WriteCoord (&sv.datagram, org[1]);
	MSG_WriteCoord (&sv.datagram, org[2]);
	MSG_WriteFloat (&sv.datagram, dmin[0]);
	MSG_WriteFloat (&sv.datagram, dmin[1]);
	MSG_WriteFloat (&sv.datagram, dmin[2]);
	MSG_WriteFloat (&sv.datagram, dmax[0]);
	MSG_WriteFloat (&sv.datagram, dmax[1]);
	MSG_WriteFloat (&sv.datagram, dmax[2]);

	MSG_WriteShort (&sv.datagram, color);
	MSG_WriteByte (&sv.datagram, count);
	MSG_WriteByte (&sv.datagram, effect);
}

/*
==================
SV_StartParticle3

Make sure the event gets sent to all clients
==================
*/
void SV_StartParticle3 (vec3_t org, vec3_t box, int color, int effect, int count)
{
	if (sv.datagram.cursize > MAX_DATAGRAM-15)
		return;
	MSG_WriteByte (&sv.datagram, svc_particle3);
	MSG_WriteCoord (&sv.datagram, org[0]);
	MSG_WriteCoord (&sv.datagram, org[1]);
	MSG_WriteCoord (&sv.datagram, org[2]);
	MSG_WriteByte (&sv.datagram, box[0]);
	MSG_WriteByte (&sv.datagram, box[1]);
	MSG_WriteByte (&sv.datagram, box[2]);

	MSG_WriteShort (&sv.datagram, color);
	MSG_WriteByte (&sv.datagram, count);
	MSG_WriteByte (&sv.datagram, effect);
}

/*
==================
SV_StartParticle4

Make sure the event gets sent to all clients
==================
*/
void SV_StartParticle4 (vec3_t org, float radius, int color, int effect, int count)
{
	if (sv.datagram.cursize > MAX_DATAGRAM-13)
		return;
	MSG_WriteByte (&sv.datagram, svc_particle4);
	MSG_WriteCoord (&sv.datagram, org[0]);
	MSG_WriteCoord (&sv.datagram, org[1]);
	MSG_WriteCoord (&sv.datagram, org[2]);
	MSG_WriteByte (&sv.datagram, radius);

	MSG_WriteShort (&sv.datagram, color);
	MSG_WriteByte (&sv.datagram, count);
	MSG_WriteByte (&sv.datagram, effect);
}

/*
==================
SV_StopSound
==================
*/
void SV_StopSound (edict_t *entity, int channel)
{
	int			ent;

	if (sv.datagram.cursize > MAX_DATAGRAM-4)
		return;

	ent = NUM_FOR_EDICT(entity);
	channel = (ent<<3) | channel;

	MSG_WriteByte (&sv.datagram, svc_stopsound);
	MSG_WriteShort (&sv.datagram, channel);
}

/*
==================
SV_UpdateSoundPos
==================
*/
void SV_UpdateSoundPos (edict_t *entity, int channel)
{
	int			ent;
	int			i;

	if (sv.datagram.cursize > MAX_DATAGRAM-4)
		return;

	ent = NUM_FOR_EDICT(entity);
	channel = (ent<<3) | channel;

	MSG_WriteByte (&sv.datagram, svc_sound_update_pos);
	MSG_WriteShort (&sv.datagram, channel);
	for (i = 0; i < 3; i++)
		MSG_WriteCoord (&sv.datagram, entity->v.origin[i] + 0.5*(entity->v.mins[i]+entity->v.maxs[i]));
}

/*
==================
SV_StartSound

Each entity can have eight independant sound sources, like voice,
weapon, feet, etc.

Channel 0 is an auto-allocate channel, the others override anything
already running on that entity/channel pair.

An attenuation of 0 will play full volume everywhere in the level.
Larger attenuations will drop off.  (max 4 attenuation)
==================
*/
void SV_StartSound (edict_t *entity, int channel, const char *sample, int volume, float attenuation)
{
	int			sound_num, ent;
	int			i, field_mask;

	if (q_strcasecmp(sample,"misc/null.wav") == 0)
	{
		SV_StopSound(entity,channel);
		return;
	}

	if (volume < 0 || volume > 255)
		Host_Error ("%s: volume = %i", __thisfunc__, volume);

	if (attenuation < 0 || attenuation > 4)
		Host_Error ("%s: attenuation = %f", __thisfunc__, attenuation);

	if (channel < 0 || channel > 7)
		Host_Error ("%s: channel = %i", __thisfunc__, channel);

	if (sv.datagram.cursize > MAX_DATAGRAM-16)
		return;

// find precache number for sound
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

	ent = NUM_FOR_EDICT(entity);

	channel = (ent<<3) | channel;

	field_mask = 0;
	if (volume != DEFAULT_SOUND_PACKET_VOLUME)
		field_mask |= SND_VOLUME;
	if (attenuation != DEFAULT_SOUND_PACKET_ATTENUATION)
		field_mask |= SND_ATTENUATION;
	if (sound_num >= MAX_SOUNDS_OLD)
	{
		if (sv_protocol == PROTOCOL_RAVEN_111)
		{
			Con_DPrintf("%s: protocol 18 violation: %s sound_num == %i >= %i\n",
					__thisfunc__, sample, sound_num, MAX_SOUNDS_OLD);
			return;
		}
		field_mask |= SND_OVERFLOW;
		sound_num -= MAX_SOUNDS_OLD;
	}

// directed messages go only to the entity the are targeted on
	MSG_WriteByte (&sv.datagram, svc_sound);
	MSG_WriteByte (&sv.datagram, field_mask);
	if (field_mask & SND_VOLUME)
		MSG_WriteByte (&sv.datagram, volume);
	if (field_mask & SND_ATTENUATION)
		MSG_WriteByte (&sv.datagram, attenuation*64);
	MSG_WriteShort (&sv.datagram, channel);
	MSG_WriteByte (&sv.datagram, sound_num);
	for (i = 0; i < 3; i++)
		MSG_WriteCoord (&sv.datagram, entity->v.origin[i] + 0.5*(entity->v.mins[i]+entity->v.maxs[i]));
}

/*
==============================================================================

CLIENT SPAWNING

==============================================================================
*/

/*
================
SV_SendServerinfo

Sends the first message from the server to a connected client.
This will be sent on the initial connection and upon each server load.
================
*/
static void SV_SendServerinfo (client_t *client)
{
	int			i;
	const char		**s;
	char			message[2048];

	MSG_WriteByte (&client->message, svc_print);
	sprintf (message, "%c\nVERSION %4.2f SERVER (%i CRC)", 2, ENGINE_VERSION, pr_crc);
	MSG_WriteString (&client->message,message);

	MSG_WriteByte (&client->message, svc_serverinfo);
	MSG_WriteLong (&client->message, sv_protocol);
	MSG_WriteByte (&client->message, svs.maxclients);

	if (!coop.integer && deathmatch.integer)
	{
		MSG_WriteByte (&client->message, GAME_DEATHMATCH);
		if (sv_protocol > PROTOCOL_RAVEN_111)
			MSG_WriteShort (&client->message, sv_kingofhill);
	}
	else
		MSG_WriteByte (&client->message, GAME_COOP);

// send full levelname
	MSG_WriteString(&client->message, SV_GetLevelname ());

	for (i = 1, s = sv.model_precache + 1; i < MAX_MODELS && *s; s++)
		MSG_WriteString (&client->message, *s);
	MSG_WriteByte (&client->message, 0);

	for (i = 1, s = sv.sound_precache + 1; i < MAX_SOUNDS && *s; s++)
		MSG_WriteString (&client->message, *s);
	MSG_WriteByte (&client->message, 0);

// send music
	MSG_WriteByte (&client->message, svc_cdtrack);
	MSG_WriteByte (&client->message, sv.cd_track);
	MSG_WriteByte (&client->message, sv.cd_track);

	MSG_WriteByte (&client->message, svc_midi_name);
	MSG_WriteString (&client->message, sv.midi_name);

	if (sv_protocol >= PROTOCOL_UQE_113)
	{
		MSG_WriteByte (&client->message, svc_mod_name);
		MSG_WriteString (&client->message, "");	/* uqe-hexen2 sends sv.mod_name */
		MSG_WriteByte (&client->message, svc_skybox);
		MSG_WriteString (&client->message, "");	/* uqe-hexen2 sends "sv.skybox" */
	}

// set view
	MSG_WriteByte (&client->message, svc_setview);
	MSG_WriteShort (&client->message, NUM_FOR_EDICT(client->edict));

	MSG_WriteByte (&client->message, svc_signonnum);
	MSG_WriteByte (&client->message, 1);

	client->sendsignon = true;
	client->spawned = false;	// need prespawn, spawn, etc
}

/*
================
SV_ConnectClient

Initializes a client_t for a new net connection.  This will only be called
once for a player each game, not once for each level change.
================
*/
static void SV_ConnectClient (int clientnum)
{
	edict_t			*ent;
	client_t		*client;
	int				edictnum;
	struct qsocket_s *netconnection;
//	int			i;
	float			spawn_parms[NUM_SPAWN_PARMS];

	client = svs.clients + clientnum;

	Con_DPrintf ("Client %s connected\n", NET_QSocketGetAddressString(client->netconnection));

	edictnum = clientnum+1;

	ent = EDICT_NUM(edictnum);

// set up the client_t
	netconnection = client->netconnection;

	if (sv.loadgame)
		memcpy (spawn_parms, client->spawn_parms, sizeof(spawn_parms));
	memset (client, 0, sizeof(*client));
	client->send_all_v = true;
	client->netconnection = netconnection;

	strcpy (client->name, "unconnected");
	client->active = true;
	client->spawned = false;
	client->edict = ent;

	SZ_Init (&client->message, client->msgbuf, sizeof(client->msgbuf));
	client->message.allowoverflow = true;	// we can catch it
	SZ_Init (&client->datagram, client->datagram_buf, sizeof(client->datagram_buf));

	memset(&sv.states[clientnum], 0, sizeof(client_state2_t ));

	if (sv.loadgame)
		memcpy (client->spawn_parms, spawn_parms, sizeof(spawn_parms));
//	else
//	{
	// call the progs to get default spawn parms for the new client
	//	PR_ExecuteProgram (*sv_globals.SetNewParms);
	//	for (i = 0; i < NUM_SPAWN_PARMS; i++)
	//		client->spawn_parms[i] = sv_globals.parm[i];
//	}

	SV_SendServerinfo (client);
}


/*
===================
SV_CheckForNewClients

===================
*/
void SV_CheckForNewClients (void)
{
	struct qsocket_s	*ret;
	int				i;

//
// check for new connections
//
	while (1)
	{
		ret = NET_CheckNewConnections ();
		if (!ret)
			break;

	//
	// init a new client structure
	//
		for (i = 0; i < svs.maxclients; i++)
		{
			if (!svs.clients[i].active)
				break;
		}

		if (i == svs.maxclients)
			Sys_Error ("%s: no free clients", __thisfunc__);

		svs.clients[i].netconnection = ret;
		SV_ConnectClient (i);

		net_activeconnections++;
	}
}


/*
===============================================================================

FRAME UPDATES

===============================================================================
*/

/*
==================
SV_ClearDatagram

==================
*/
void SV_ClearDatagram (void)
{
	SZ_Clear (&sv.datagram);
}

/*
=============================================================================

The PVS must include a small area around the client to allow head bobbing
or other small motion on the client side.  Otherwise, a bob might cause an
entity that should be visible to not show up, especially when the bob
crosses a waterline.

=============================================================================
*/

static int	fatbytes;
static byte	fatpvs[MAX_MAP_LEAFS/8];

static void SV_AddToFatPVS (vec3_t org, mnode_t *node)
{
	int		i;
	byte	*pvs;
	mplane_t	*plane;
	float	d;

	while (1)
	{
	// if this is a leaf, accumulate the pvs bits
		if (node->contents < 0)
		{
			if (node->contents != CONTENTS_SOLID)
			{
				pvs = Mod_LeafPVS ( (mleaf_t *)node, sv.worldmodel);
				for (i = 0; i < fatbytes; i++)
					fatpvs[i] |= pvs[i];
			}
			return;
		}

		plane = node->plane;
		d = DotProduct (org, plane->normal) - plane->dist;
		if (d > 8)
			node = node->children[0];
		else if (d < -8)
			node = node->children[1];
		else
		{	// go down both
			SV_AddToFatPVS (org, node->children[0]);
			node = node->children[1];
		}
	}
}

/*
=============
SV_FatPVS

Calculates a PVS that is the inclusive or of all leafs within 8 pixels of the
given point.
=============
*/
static byte *SV_FatPVS (vec3_t org)
{
	fatbytes = (sv.worldmodel->numleafs+31)>>3;
	memset (fatpvs, 0, fatbytes);
	SV_AddToFatPVS (org, sv.worldmodel->nodes);
	return fatpvs;
}

#define CLIENT_FRAME_INIT	255
#define CLIENT_FRAME_RESET	254

static void SV_PrepareClientEntities (client_t *client, edict_t	*clent, sizebuf_t *msg)
{
	int		e, i;
	int		bits;
	byte	*pvs;
	vec3_t	org;
	float	miss;
	edict_t	*ent;
	int		temp_index;
	char	NewName[MAX_QPATH];
	long	flagtest;
	int			position = 0;
	int			client_num;
	client_frames_t	*reference, *build;
	client_state2_t	*state;
	entity_state2_t	*ref_ent, *set_ent, build_ent;
	qboolean		FoundInList,DoRemove,DoPlayer,DoMonsters,DoMissiles,DoMisc,IgnoreEnt;
	short			RemoveList[MAX_CLIENT_STATES],NumToRemove;

	client_num = client-svs.clients;
	state = &sv.states[client_num];
	reference = &state->frames[0];

	if (client->last_sequence != client->current_sequence)
	{	// Old sequence
	//	Con_Printf("SV: Old sequence SV(%d,%d) CL(%d,%d)\n",client->current_sequence, client->current_frame, client->last_sequence, client->last_frame);
		client->current_frame++;
		if (client->current_frame > MAX_FRAMES+1)
			client->current_frame = MAX_FRAMES+1;
	}
	else if (client->last_frame == CLIENT_FRAME_INIT ||
			 client->last_frame == 0 ||
			 client->last_frame == MAX_FRAMES+1)
	{	// Reference expired in current sequence
	//	Con_Printf("SV: Expired SV(%d,%d) CL(%d,%d)\n",client->current_sequence, client->current_frame, client->last_sequence, client->last_frame);
		client->current_frame = 1;
		client->current_sequence++;
	}
	else if (client->last_frame >= 1 && client->last_frame <= client->current_frame)
	{	// Got a valid frame
	//	Con_Printf("SV: Valid SV(%d,%d) CL(%d,%d)\n",client->current_sequence, client->current_frame, client->last_sequence, client->last_frame);
		*reference = state->frames[client->last_frame];

		for (i = 0; i < reference->count; i++)
		{
			if (reference->states[i].flags & ENT_CLEARED)
			{
				e = reference->states[i].index;
				ent = EDICT_NUM(e);
				if (ent->baseline.ClearCount[client_num] < CLEAR_LIMIT)
				{
					ent->baseline.ClearCount[client_num]++;
				}
				else if (ent->baseline.ClearCount[client_num] == CLEAR_LIMIT)
				{
					ent->baseline.ClearCount[client_num] = 3;
					reference->states[i].flags &= ~ENT_CLEARED;
				}
			}
		}
		client->current_frame = 1;
		client->current_sequence++;
	}
	else
	{	// Normal frame advance
	//	Con_Printf("SV: Normal SV(%d,%d) CL(%d,%d)\n",client->current_sequence, client->current_frame, client->last_sequence, client->last_frame);
		client->current_frame++;
		if (client->current_frame > MAX_FRAMES+1)
			client->current_frame = MAX_FRAMES+1;
	}

	DoPlayer = DoMonsters = DoMissiles = DoMisc = false;

	if (sv_update_player.integer)
		DoPlayer = (client->current_sequence % sv_update_player.integer) == 0;
	if (sv_update_monsters.integer)
		DoMonsters = (client->current_sequence % sv_update_monsters.integer) == 0;
	if (sv_update_missiles.integer)
		DoMissiles = (client->current_sequence % sv_update_missiles.integer) == 0;
	if (sv_update_misc.integer)
		DoMisc = (client->current_sequence % sv_update_misc.integer) == 0;

	build = &state->frames[client->current_frame];
	memset(build, 0, sizeof(*build));
	client->last_frame = CLIENT_FRAME_RESET;

	NumToRemove = 0;
	MSG_WriteByte (msg, svc_reference);
	MSG_WriteByte (msg, client->current_frame);
	MSG_WriteByte (msg, client->current_sequence);

	// find the client's PVS
	if (clent->v.cameramode)
	{
		ent = PROG_TO_EDICT(clent->v.cameramode);
		VectorCopy(ent->v.origin, org);
	}
	else
		VectorAdd (clent->v.origin, clent->v.view_ofs, org);

	pvs = SV_FatPVS (org);

	// send over all entities (except the client) that touch the pvs
	ent = NEXT_EDICT(sv.edicts);
	for (e = 1; e < sv.num_edicts; e++, ent = NEXT_EDICT(ent))
	{
		DoRemove = false;
		// don't send if flagged for NODRAW and there are no lighting effects
		if (ent->v.effects == EF_NODRAW)
		{
			DoRemove = true;
			goto skipA;
		}

		// ignore if not touching a PV leaf
		if (ent != clent)	// clent is ALWAYS sent
		{	// ignore ents without visible models
			if (!ent->v.modelindex || !*PR_GetString(ent->v.model))
			{
				DoRemove = true;
				goto skipA;
			}

			for (i = 0; i < ent->num_leafs; i++)
			{
				if (pvs[ent->leafnums[i] >> 3] & (1 << (ent->leafnums[i] & 7)) )
					break;
			}

			if (i == ent->num_leafs)
			{
				DoRemove = true;
				goto skipA;
			}
		}

skipA:
		IgnoreEnt = false;
		flagtest = (long)ent->v.flags;
		if (!DoRemove)
		{
			if (flagtest & FL_CLIENT)
			{
				if (!DoPlayer)
					IgnoreEnt = true;
			}
			else if (flagtest & FL_MONSTER)
			{
				if (!DoMonsters)
					IgnoreEnt = true;
			}
			else if (ent->v.movetype == MOVETYPE_FLYMISSILE ||
					 ent->v.movetype == MOVETYPE_BOUNCEMISSILE ||
					 ent->v.movetype == MOVETYPE_BOUNCE)
			{
				if (!DoMissiles)
					IgnoreEnt = true;
			}
			else
			{
				if (!DoMisc)
					IgnoreEnt = true;
			}
		}

		bits = 0;

		while (position < reference->count && 
			   e > reference->states[position].index)
			position++;

		if (position < reference->count && reference->states[position].index == e)
		{
			FoundInList = true;
			if (DoRemove)
			{
				RemoveList[NumToRemove] = e;
				NumToRemove++;
				continue;
			}
			else
			{
				ref_ent = &reference->states[position];
			}
		}
		else
		{
			if (DoRemove || IgnoreEnt)
				continue;

			ref_ent = &build_ent;

			build_ent.index = e;
			build_ent.origin[0] = ent->baseline.origin[0];
			build_ent.origin[1] = ent->baseline.origin[1];
			build_ent.origin[2] = ent->baseline.origin[2];
			build_ent.angles[0] = ent->baseline.angles[0];
			build_ent.angles[1] = ent->baseline.angles[1];
			build_ent.angles[2] = ent->baseline.angles[2];
			build_ent.modelindex = ent->baseline.modelindex;
			build_ent.frame = ent->baseline.frame;
			build_ent.colormap = ent->baseline.colormap;
			build_ent.skin = ent->baseline.skin;
			build_ent.effects = ent->baseline.effects;
			build_ent.scale = ent->baseline.scale;
			build_ent.drawflags = ent->baseline.drawflags;
			build_ent.abslight = ent->baseline.abslight;
			build_ent.flags = 0;

			FoundInList = false;
		}

		set_ent = &build->states[build->count];
		build->count++;
		if (ent->baseline.ClearCount[client_num] < CLEAR_LIMIT)
		{
			memset(ref_ent, 0, sizeof(*ref_ent));
			ref_ent->index = e;
		}
		*set_ent = *ref_ent;

		if (IgnoreEnt)
			continue;

		// send an update
		for (i = 0; i < 3; i++)
		{
			miss = ent->v.origin[i] - ref_ent->origin[i];
			if ( miss < -0.1 || miss > 0.1 )
			{
				bits |= U_ORIGIN1<<i;
				set_ent->origin[i] = ent->v.origin[i];
			}
		}

		if ( ent->v.angles[0] != ref_ent->angles[0] )
		{
			bits |= U_ANGLE1;
			set_ent->angles[0] = ent->v.angles[0];
		}

		if ( ent->v.angles[1] != ref_ent->angles[1] )
		{
			bits |= U_ANGLE2;
			set_ent->angles[1] = ent->v.angles[1];
		}

		if ( ent->v.angles[2] != ref_ent->angles[2] )
		{
			bits |= U_ANGLE3;
			set_ent->angles[2] = ent->v.angles[2];
		}

		if (ent->v.movetype == MOVETYPE_STEP)
			bits |= U_NOLERP;	// don't mess up the step animation

		if (ref_ent->colormap != ent->v.colormap)
		{
			bits |= U_COLORMAP;
			set_ent->colormap = ent->v.colormap;
		}

		if (ref_ent->skin != ent->v.skin
			|| ref_ent->drawflags != ent->v.drawflags)
		{
			bits |= U_SKIN;
			set_ent->skin = ent->v.skin;
			set_ent->drawflags = ent->v.drawflags;
		}

		if (ref_ent->frame != ent->v.frame)
		{
			bits |= U_FRAME;
			set_ent->frame = ent->v.frame;
		}

		if (ref_ent->effects != ent->v.effects)
		{
			bits |= U_EFFECTS;
			set_ent->effects = ent->v.effects;
		}

	//	flagtest = (long)ent->v.flags;
		if (flagtest & 0xff000000)
		{
			Host_Error("Invalid flags setting for class %s", PR_GetString(ent->v.classname));
			return;
		}

		temp_index = ent->v.modelindex;
		if (((int)ent->v.flags & FL_CLASS_DEPENDENT) && ent->v.model)
		{
			strcpy (NewName, PR_GetString(ent->v.model));
			NewName[strlen(NewName)-5] = client->playerclass + 48;
			temp_index = SV_ModelIndex (NewName);
		}

		if (ref_ent->modelindex != temp_index)
		{
			bits |= U_MODEL;
			set_ent->modelindex = temp_index;
		}

		if ( ref_ent->scale != ((int)(ent->v.scale * 100.0) & 255)
			|| ref_ent->abslight != ((int)(ent->v.abslight * 255.0) & 255) )
		{
			bits |= U_SCALE;
			set_ent->scale = ((int)(ent->v.scale * 100.0) & 255);
			set_ent->abslight = (int)(ent->v.abslight * 255.0) & 255;
		}

		if (ent->baseline.ClearCount[client_num] < CLEAR_LIMIT)
		{
			bits |= U_CLEAR_ENT;
			set_ent->flags |= ENT_CLEARED;
		}

		if (!bits && FoundInList)
		{
			if (build->count >= MAX_CLIENT_STATES)
				break;

			continue;
		}

		if (e >= 256)
			bits |= U_LONGENTITY;

		if (bits >= 256)
			bits |= U_MOREBITS;

		if (bits >= 65536)
			bits |= U_MOREBITS2;

	//
	// write the message
	//
		MSG_WriteByte (msg,bits | U_SIGNAL);

		if (bits & U_MOREBITS)
			MSG_WriteByte (msg, bits >> 8);
		if (bits & U_MOREBITS2)
			MSG_WriteByte (msg, bits >> 16);

		if (bits & U_LONGENTITY)
			MSG_WriteShort (msg, e);
		else
			MSG_WriteByte (msg, e);

		if (bits & U_MODEL)
			MSG_WriteShort (msg, temp_index);
		if (bits & U_FRAME)
			MSG_WriteByte (msg, ent->v.frame);
		if (bits & U_COLORMAP)
			MSG_WriteByte (msg, ent->v.colormap);
		if (bits & U_SKIN)
		{ // Used for skin and drawflags
			MSG_WriteByte(msg, ent->v.skin);
			MSG_WriteByte(msg, ent->v.drawflags);
		}
		if (bits & U_EFFECTS)
			MSG_WriteByte (msg, ent->v.effects);
		if (bits & U_ORIGIN1)
			MSG_WriteCoord (msg, ent->v.origin[0]);
		if (bits & U_ANGLE1)
			MSG_WriteAngle (msg, ent->v.angles[0]);
		if (bits & U_ORIGIN2)
			MSG_WriteCoord (msg, ent->v.origin[1]);
		if (bits & U_ANGLE2)
			MSG_WriteAngle (msg, ent->v.angles[1]);
		if (bits & U_ORIGIN3)
			MSG_WriteCoord (msg, ent->v.origin[2]);
		if (bits & U_ANGLE3)
			MSG_WriteAngle (msg, ent->v.angles[2]);
		if (bits & U_SCALE)
		{ // Used for scale and abslight
			MSG_WriteByte (msg, (int)(ent->v.scale * 100.0) & 255);
			MSG_WriteByte (msg, (int)(ent->v.abslight * 255.0) & 255);
		}

		if (build->count >= MAX_CLIENT_STATES)
			break;
	}

	MSG_WriteByte (msg, svc_clear_edicts);
	MSG_WriteByte (msg, NumToRemove);
	for (i = 0; i < NumToRemove; i++)
		MSG_WriteShort (msg, RemoveList[i]);
}

/*
=============
SV_CleanupEnts

=============
*/
static void SV_CleanupEnts (void)
{
	int		e;
	edict_t	*ent;

	ent = NEXT_EDICT(sv.edicts);
	for (e = 1; e < sv.num_edicts; e++, ent = NEXT_EDICT(ent))
	{
		ent->v.effects = (int)ent->v.effects & ~EF_MUZZLEFLASH;
	}
}

/*
==================
SV_WriteClientdataToMessage

==================
*/
void SV_WriteClientdataToMessage (client_t *client, edict_t *ent, sizebuf_t *msg)
{
	int	bits,sc1,sc2;
	byte	test;
	int	i;
	edict_t	*other;
	static	int next_update = 0;
	static	int next_count = 0;

//
// send a damage message
//
	if (ent->v.dmg_take || ent->v.dmg_save)
	{
		other = PROG_TO_EDICT(ent->v.dmg_inflictor);
		MSG_WriteByte (msg, svc_damage);
		MSG_WriteByte (msg, ent->v.dmg_save);
		MSG_WriteByte (msg, ent->v.dmg_take);
		for (i = 0; i < 3; i++)
			MSG_WriteCoord (msg, other->v.origin[i] + 0.5*(other->v.mins[i] + other->v.maxs[i]));

		ent->v.dmg_take = 0;
		ent->v.dmg_save = 0;
	}

//
// send the current viewpos offset from the view entity
//
	SV_SetIdealPitch ();	// how much to look up / down ideally

// a fixangle might get lost in a dropped packet.  Oh well.
	if (ent->v.fixangle)
	{
		MSG_WriteByte (msg, svc_setangle);
		for (i = 0; i < 3; i++)
			MSG_WriteAngle (msg, ent->v.angles[i] );
		ent->v.fixangle = 0;
	}

	bits = 0;

	if (client->send_all_v)
	{
		bits = SU_VIEWHEIGHT | SU_IDEALPITCH | SU_IDEALROLL | 
			SU_VELOCITY1 | (SU_VELOCITY1<<1) | (SU_VELOCITY1<<2) | 
			SU_PUNCH1 | (SU_PUNCH1<<1) | (SU_PUNCH1<<2) | SU_WEAPONFRAME | 
			SU_ARMOR | SU_WEAPON;
	}
	else
	{
		if (ent->v.view_ofs[2] != client->old_v.view_ofs[2])
			bits |= SU_VIEWHEIGHT;

		if (ent->v.idealpitch != client->old_v.idealpitch)
			bits |= SU_IDEALPITCH;

		if (ent->v.idealroll != client->old_v.idealroll)
			bits |= SU_IDEALROLL;

		for (i = 0; i < 3; i++)
		{
			if (ent->v.punchangle[i] != client->old_v.punchangle[i])
				bits |= (SU_PUNCH1<<i);
			if (ent->v.velocity[i] != client->old_v.velocity[i])
				bits |= (SU_VELOCITY1<<i);
		}

		if (ent->v.weaponframe != client->old_v.weaponframe)
			bits |= SU_WEAPONFRAME;

		if (ent->v.armorvalue != client->old_v.armorvalue)
			bits |= SU_ARMOR;

		if (ent->v.weaponmodel != client->old_v.weaponmodel)
			bits |= SU_WEAPON;
	}

// send the data

	//fjm: this wasn't in here b4, and the centerview command requires it.
	if ( (int)ent->v.flags & FL_ONGROUND)
		bits |= SU_ONGROUND;

	next_count++;
	if (next_count >= 3)
	{
		next_count = 0;
		next_update++;
		if (next_update > 11)
			next_update = 0;

		switch (next_update)
		{
			case 0:
				bits |= SU_VIEWHEIGHT;
				break;
			case 1:
				bits |= SU_IDEALPITCH;
				break;
			case 2:
				bits |= SU_IDEALROLL;
				break;
			case 3:
				bits |= SU_VELOCITY1;
				break;
			case 4:
				bits |= (SU_VELOCITY1<<1);
				break;
			case 5:
				bits |= (SU_VELOCITY1<<2);
				break;
			case 6:
				bits |= SU_PUNCH1;
				break;
			case 7:
				bits |= (SU_PUNCH1<<1);
				break;
			case 8:
				bits |= (SU_PUNCH1<<2);
				break;
			case 9:
				bits |= SU_WEAPONFRAME;
				break;
			case 10:
				bits |= SU_ARMOR;
				break;
			case 11:
				bits |= SU_WEAPON;
				break;
		}
	}

	MSG_WriteByte (msg, svc_clientdata);
	MSG_WriteShort (msg, bits);

	if (bits & SU_VIEWHEIGHT)
		MSG_WriteChar (msg, ent->v.view_ofs[2]);

	if (bits & SU_IDEALPITCH)
		MSG_WriteChar (msg, ent->v.idealpitch);

	if (bits & SU_IDEALROLL)
		MSG_WriteChar (msg, ent->v.idealroll);

	for (i = 0; i < 3; i++)
	{
		if (bits & (SU_PUNCH1<<i))
			MSG_WriteChar (msg, ent->v.punchangle[i]);
		if (bits & (SU_VELOCITY1<<i))
			MSG_WriteChar (msg, ent->v.velocity[i]/16);
	}

	if (bits & SU_WEAPONFRAME)
		MSG_WriteByte (msg, ent->v.weaponframe);
	if (bits & SU_ARMOR)
		MSG_WriteByte (msg, ent->v.armorvalue);
	if (bits & SU_WEAPON)
		MSG_WriteShort (msg, SV_ModelIndex(PR_GetString(ent->v.weaponmodel)));

	if (host_client->send_all_v)
	{
		sc1 = sc2 = 0xffffffff;
		host_client->send_all_v = false;
	}
	else
	{
		sc1 = sc2 = 0;

		if (ent->v.health != host_client->old_v.health)
			sc1 |= SC1_HEALTH;
		if (ent->v.level != host_client->old_v.level)
			sc1 |= SC1_LEVEL;
		if (ent->v.intelligence != host_client->old_v.intelligence)
			sc1 |= SC1_INTELLIGENCE;
		if (ent->v.wisdom != host_client->old_v.wisdom)
			sc1 |= SC1_WISDOM;
		if (ent->v.strength != host_client->old_v.strength)
			sc1 |= SC1_STRENGTH;
		if (ent->v.dexterity != host_client->old_v.dexterity)
			sc1 |= SC1_DEXTERITY;
		if (ent->v.weapon != host_client->old_v.weapon)
			sc1 |= SC1_WEAPON;
		if (ent->v.bluemana != host_client->old_v.bluemana)
			sc1 |= SC1_BLUEMANA;
		if (ent->v.greenmana != host_client->old_v.greenmana)
			sc1 |= SC1_GREENMANA;
		if (ent->v.experience != host_client->old_v.experience)
			sc1 |= SC1_EXPERIENCE;
		if (ent->v.cnt_torch != host_client->old_v.cnt_torch)
			sc1 |= SC1_CNT_TORCH;
		if (ent->v.cnt_h_boost != host_client->old_v.cnt_h_boost)
			sc1 |= SC1_CNT_H_BOOST;
		if (ent->v.cnt_sh_boost != host_client->old_v.cnt_sh_boost)
			sc1 |= SC1_CNT_SH_BOOST;
		if (ent->v.cnt_mana_boost != host_client->old_v.cnt_mana_boost)
			sc1 |= SC1_CNT_MANA_BOOST;
		if (ent->v.cnt_teleport != host_client->old_v.cnt_teleport)
			sc1 |= SC1_CNT_TELEPORT;
		if (ent->v.cnt_tome != host_client->old_v.cnt_tome)
			sc1 |= SC1_CNT_TOME;
		if (ent->v.cnt_summon != host_client->old_v.cnt_summon)
			sc1 |= SC1_CNT_SUMMON;
		if (ent->v.cnt_invisibility != host_client->old_v.cnt_invisibility)
			sc1 |= SC1_CNT_INVISIBILITY;
		if (ent->v.cnt_glyph != host_client->old_v.cnt_glyph)
			sc1 |= SC1_CNT_GLYPH;
		if (ent->v.cnt_haste != host_client->old_v.cnt_haste)
			sc1 |= SC1_CNT_HASTE;
		if (ent->v.cnt_blast != host_client->old_v.cnt_blast)
			sc1 |= SC1_CNT_BLAST;
		if (ent->v.cnt_polymorph != host_client->old_v.cnt_polymorph)
			sc1 |= SC1_CNT_POLYMORPH;
		if (ent->v.cnt_flight != host_client->old_v.cnt_flight)
			sc1 |= SC1_CNT_FLIGHT;
		if (ent->v.cnt_cubeofforce != host_client->old_v.cnt_cubeofforce)
			sc1 |= SC1_CNT_CUBEOFFORCE;
		if (ent->v.cnt_invincibility != host_client->old_v.cnt_invincibility)
			sc1 |= SC1_CNT_INVINCIBILITY;
		if (ent->v.artifact_active != host_client->old_v.artifact_active)
			sc1 |= SC1_ARTIFACT_ACTIVE;
		if (ent->v.artifact_low != host_client->old_v.artifact_low)
			sc1 |= SC1_ARTIFACT_LOW;
		if (ent->v.movetype != host_client->old_v.movetype)
			sc1 |= SC1_MOVETYPE;
		if (ent->v.cameramode != host_client->old_v.cameramode)
			sc1 |= SC1_CAMERAMODE;
		if (ent->v.hasted != host_client->old_v.hasted)
			sc1 |= SC1_HASTED;
		if (ent->v.inventory != host_client->old_v.inventory)
			sc1 |= SC1_INVENTORY;
		if (ent->v.rings_active != host_client->old_v.rings_active)
			sc1 |= SC1_RINGS_ACTIVE;

		if (ent->v.rings_low != host_client->old_v.rings_low)
			sc2 |= SC2_RINGS_LOW;
		if (ent->v.armor_amulet != host_client->old_v.armor_amulet)
			sc2 |= SC2_AMULET;
		if (ent->v.armor_bracer != host_client->old_v.armor_bracer)
			sc2 |= SC2_BRACER;
		if (ent->v.armor_breastplate != host_client->old_v.armor_breastplate)
			sc2 |= SC2_BREASTPLATE;
		if (ent->v.armor_helmet != host_client->old_v.armor_helmet)
			sc2 |= SC2_HELMET;
		if (ent->v.ring_flight != host_client->old_v.ring_flight)
			sc2 |= SC2_FLIGHT_T;
		if (ent->v.ring_water != host_client->old_v.ring_water)
			sc2 |= SC2_WATER_T;
		if (ent->v.ring_turning != host_client->old_v.ring_turning)
			sc2 |= SC2_TURNING_T;
		if (ent->v.ring_regeneration != host_client->old_v.ring_regeneration)
			sc2 |= SC2_REGEN_T;
		if (ent->v.haste_time != host_client->old_v.haste_time)
			sc2 |= SC2_HASTE_T;
		if (ent->v.tome_time != host_client->old_v.tome_time)
			sc2 |= SC2_TOME_T;
		if (ent->v.puzzle_inv1 != host_client->old_v.puzzle_inv1)
			sc2 |= SC2_PUZZLE1;
		if (ent->v.puzzle_inv2 != host_client->old_v.puzzle_inv2)
			sc2 |= SC2_PUZZLE2;
		if (ent->v.puzzle_inv3 != host_client->old_v.puzzle_inv3)
			sc2 |= SC2_PUZZLE3;
		if (ent->v.puzzle_inv4 != host_client->old_v.puzzle_inv4)
			sc2 |= SC2_PUZZLE4;
		if (ent->v.puzzle_inv5 != host_client->old_v.puzzle_inv5)
			sc2 |= SC2_PUZZLE5;
		if (ent->v.puzzle_inv6 != host_client->old_v.puzzle_inv6)
			sc2 |= SC2_PUZZLE6;
		if (ent->v.puzzle_inv7 != host_client->old_v.puzzle_inv7)
			sc2 |= SC2_PUZZLE7;
		if (ent->v.puzzle_inv8 != host_client->old_v.puzzle_inv8)
			sc2 |= SC2_PUZZLE8;
		if (ent->v.max_health != host_client->old_v.max_health)
			sc2 |= SC2_MAXHEALTH;
		if (ent->v.max_mana != host_client->old_v.max_mana)
			sc2 |= SC2_MAXMANA;
		if (ent->v.flags != host_client->old_v.flags)
			sc2 |= SC2_FLAGS;

		// mission pack, objectives
		if (sv_protocol > PROTOCOL_RAVEN_111)
		{
			if (info_mask != client->info_mask)
				sc2 |= SC2_OBJ;
			if (info_mask2 != client->info_mask2)
				sc2 |= SC2_OBJ2;
		}
	}

	if (!sc1 && !sc2)
		goto end;

	MSG_WriteByte (&host_client->message, svc_update_inv);
	test = 0;
	if (sc1 & 0x000000ff)
		test |= 1;
	if (sc1 & 0x0000ff00)
		test |= 2;
	if (sc1 & 0x00ff0000)
		test |= 4;
	if (sc1 & 0xff000000)
		test |= 8;
	if (sc2 & 0x000000ff)
		test |= 16;
	if (sc2 & 0x0000ff00)
		test |= 32;
	if (sc2 & 0x00ff0000)
		test |= 64;
	if (sc2 & 0xff000000)
		test |= 128;

	MSG_WriteByte (&host_client->message, test);

	if (test & 1)
		MSG_WriteByte (&host_client->message, sc1 & 0xff);
	if (test & 2)
		MSG_WriteByte (&host_client->message, (sc1 >> 8) & 0xff);
	if (test & 4)
		MSG_WriteByte (&host_client->message, (sc1 >> 16) & 0xff);
	if (test & 8)
		MSG_WriteByte (&host_client->message, (sc1 >> 24) & 0xff);
	if (test & 16)
		MSG_WriteByte (&host_client->message, sc2 & 0xff);
	if (test & 32)
		MSG_WriteByte (&host_client->message, (sc2 >> 8) & 0xff);
	if (test & 64)
		MSG_WriteByte (&host_client->message, (sc2 >> 16) & 0xff);
	if (test & 128)
		MSG_WriteByte (&host_client->message, (sc2 >> 24) & 0xff);

	if (sc1 & SC1_HEALTH)
		MSG_WriteShort (&host_client->message, ent->v.health);
	if (sc1 & SC1_LEVEL)
		MSG_WriteByte (&host_client->message, ent->v.level);
	if (sc1 & SC1_INTELLIGENCE)
		MSG_WriteByte (&host_client->message, ent->v.intelligence);
	if (sc1 & SC1_WISDOM)
		MSG_WriteByte (&host_client->message, ent->v.wisdom);
	if (sc1 & SC1_STRENGTH)
		MSG_WriteByte (&host_client->message, ent->v.strength);
	if (sc1 & SC1_DEXTERITY)
		MSG_WriteByte (&host_client->message, ent->v.dexterity);
	if (sc1 & SC1_WEAPON)
		MSG_WriteByte (&host_client->message, ent->v.weapon);
	if (sc1 & SC1_BLUEMANA)
		MSG_WriteByte (&host_client->message, ent->v.bluemana);
	if (sc1 & SC1_GREENMANA)
		MSG_WriteByte (&host_client->message, ent->v.greenmana);
	if (sc1 & SC1_EXPERIENCE)
		MSG_WriteLong (&host_client->message, ent->v.experience);
	if (sc1 & SC1_CNT_TORCH)
		MSG_WriteByte (&host_client->message, ent->v.cnt_torch);
	if (sc1 & SC1_CNT_H_BOOST)
		MSG_WriteByte (&host_client->message, ent->v.cnt_h_boost);
	if (sc1 & SC1_CNT_SH_BOOST)
		MSG_WriteByte (&host_client->message, ent->v.cnt_sh_boost);
	if (sc1 & SC1_CNT_MANA_BOOST)
		MSG_WriteByte (&host_client->message, ent->v.cnt_mana_boost);
	if (sc1 & SC1_CNT_TELEPORT)
		MSG_WriteByte (&host_client->message, ent->v.cnt_teleport);
	if (sc1 & SC1_CNT_TOME)
		MSG_WriteByte (&host_client->message, ent->v.cnt_tome);
	if (sc1 & SC1_CNT_SUMMON)
		MSG_WriteByte (&host_client->message, ent->v.cnt_summon);
	if (sc1 & SC1_CNT_INVISIBILITY)
		MSG_WriteByte (&host_client->message, ent->v.cnt_invisibility);
	if (sc1 & SC1_CNT_GLYPH)
		MSG_WriteByte (&host_client->message, ent->v.cnt_glyph);
	if (sc1 & SC1_CNT_HASTE)
		MSG_WriteByte (&host_client->message, ent->v.cnt_haste);
	if (sc1 & SC1_CNT_BLAST)
		MSG_WriteByte (&host_client->message, ent->v.cnt_blast);
	if (sc1 & SC1_CNT_POLYMORPH)
		MSG_WriteByte (&host_client->message, ent->v.cnt_polymorph);
	if (sc1 & SC1_CNT_FLIGHT)
		MSG_WriteByte (&host_client->message, ent->v.cnt_flight);
	if (sc1 & SC1_CNT_CUBEOFFORCE)
		MSG_WriteByte (&host_client->message, ent->v.cnt_cubeofforce);
	if (sc1 & SC1_CNT_INVINCIBILITY)
		MSG_WriteByte (&host_client->message, ent->v.cnt_invincibility);
	if (sc1 & SC1_ARTIFACT_ACTIVE)
		MSG_WriteFloat (&host_client->message, ent->v.artifact_active);
	if (sc1 & SC1_ARTIFACT_LOW)
		MSG_WriteFloat (&host_client->message, ent->v.artifact_low);
	if (sc1 & SC1_MOVETYPE)
		MSG_WriteByte (&host_client->message, ent->v.movetype);
	if (sc1 & SC1_CAMERAMODE)
		MSG_WriteByte (&host_client->message, ent->v.cameramode);
	if (sc1 & SC1_HASTED)
		MSG_WriteFloat (&host_client->message, ent->v.hasted);
	if (sc1 & SC1_INVENTORY)
		MSG_WriteByte (&host_client->message, ent->v.inventory);
	if (sc1 & SC1_RINGS_ACTIVE)
		MSG_WriteFloat (&host_client->message, ent->v.rings_active);

	if (sc2 & SC2_RINGS_LOW)
		MSG_WriteFloat (&host_client->message, ent->v.rings_low);
	if (sc2 & SC2_AMULET)
		MSG_WriteByte (&host_client->message, ent->v.armor_amulet);
	if (sc2 & SC2_BRACER)
		MSG_WriteByte (&host_client->message, ent->v.armor_bracer);
	if (sc2 & SC2_BREASTPLATE)
		MSG_WriteByte (&host_client->message, ent->v.armor_breastplate);
	if (sc2 & SC2_HELMET)
		MSG_WriteByte (&host_client->message, ent->v.armor_helmet);
	if (sc2 & SC2_FLIGHT_T)
		MSG_WriteByte (&host_client->message, ent->v.ring_flight);
	if (sc2 & SC2_WATER_T)
		MSG_WriteByte (&host_client->message, ent->v.ring_water);
	if (sc2 & SC2_TURNING_T)
		MSG_WriteByte (&host_client->message, ent->v.ring_turning);
	if (sc2 & SC2_REGEN_T)
		MSG_WriteByte (&host_client->message, ent->v.ring_regeneration);
	if (sc2 & SC2_HASTE_T)
		MSG_WriteFloat (&host_client->message, ent->v.haste_time);
	if (sc2 & SC2_TOME_T)
		MSG_WriteFloat (&host_client->message, ent->v.tome_time);
	if (sc2 & SC2_PUZZLE1)
		MSG_WriteString (&host_client->message, PR_GetString(ent->v.puzzle_inv1));
	if (sc2 & SC2_PUZZLE2)
		MSG_WriteString (&host_client->message, PR_GetString(ent->v.puzzle_inv2));
	if (sc2 & SC2_PUZZLE3)
		MSG_WriteString (&host_client->message, PR_GetString(ent->v.puzzle_inv3));
	if (sc2 & SC2_PUZZLE4)
		MSG_WriteString (&host_client->message, PR_GetString(ent->v.puzzle_inv4));
	if (sc2 & SC2_PUZZLE5)
		MSG_WriteString (&host_client->message, PR_GetString(ent->v.puzzle_inv5));
	if (sc2 & SC2_PUZZLE6)
		MSG_WriteString (&host_client->message, PR_GetString(ent->v.puzzle_inv6));
	if (sc2 & SC2_PUZZLE7)
		MSG_WriteString (&host_client->message, PR_GetString(ent->v.puzzle_inv7));
	if (sc2 & SC2_PUZZLE8)
		MSG_WriteString (&host_client->message, PR_GetString(ent->v.puzzle_inv8));
	if (sc2 & SC2_MAXHEALTH)
		MSG_WriteShort (&host_client->message, ent->v.max_health);
	if (sc2 & SC2_MAXMANA)
		MSG_WriteByte (&host_client->message, ent->v.max_mana);
	if (sc2 & SC2_FLAGS)
		MSG_WriteFloat (&host_client->message, ent->v.flags);

// mission pack, objectives
	if (sv_protocol > PROTOCOL_RAVEN_111)
	{
		if (sc2 & SC2_OBJ)
		{
			MSG_WriteLong (&host_client->message, info_mask);
			client->info_mask = info_mask;
		}
		if (sc2 & SC2_OBJ2)
		{
			MSG_WriteLong (&host_client->message, info_mask2);
			client->info_mask2 = info_mask2;
		}
	}

end:
	memcpy (&client->old_v, &ent->v, sizeof(client->old_v));
}

/*
=======================
SV_SendClientDatagram
=======================
*/
static qboolean SV_SendClientDatagram (client_t *client)
{
	byte		buf[NET_MAXMESSAGE];
	sizebuf_t	msg;

	SZ_Init (&msg, buf, sizeof(buf));

	MSG_WriteByte (&msg, svc_time);
	MSG_WriteFloat (&msg, sv.time);

// add the client specific data to the datagram
	SV_WriteClientdataToMessage (client, client->edict, &msg);

	SV_PrepareClientEntities (client, client->edict, &msg);

/*	if ((rand() & 0xff) < 200)
	{
		return true;
	}
*/

// copy the server datagram if there is space
	if (msg.cursize + sv.datagram.cursize < msg.maxsize)
		SZ_Write (&msg, sv.datagram.data, sv.datagram.cursize);

	if (msg.cursize + client->datagram.cursize < msg.maxsize)
		SZ_Write (&msg, client->datagram.data, client->datagram.cursize);

	SZ_Clear(&client->datagram);

/*	if (msg.cursize > 300)
	{
		Con_DPrintf("WARNING: packet size is %i\n",msg.cursize);
	}
*/

// send the datagram
	if (NET_SendUnreliableMessage (client->netconnection, &msg) == -1)
	{
		SV_DropClient (true);// if the message couldn't send, kick off
		return false;
	}

	return true;
}

/*
=======================
SV_UpdateToReliableMessages
=======================
*/
static void SV_UpdateToReliableMessages (void)
{
	int		i, j;
	client_t	*client;
	edict_t		*ent;

// check for changes to be sent over the reliable streams
	for (i = 0, host_client = svs.clients; i < svs.maxclients; i++, host_client++)
	{
		ent = host_client->edict;
		if (host_client->old_frags != ent->v.frags)
		{
			for (j = 0, client = svs.clients; j < svs.maxclients; j++, client++)
			{
				if (!client->active)
					continue;

				MSG_WriteByte (&client->message, svc_updatefrags);
				MSG_WriteByte (&client->message, i);
				MSG_WriteShort (&client->message, host_client->edict->v.frags);
			}

			host_client->old_frags = ent->v.frags;
		}
	}

	for (j = 0, client = svs.clients; j < svs.maxclients; j++, client++)
	{
		if (!client->active)
			continue;
		SZ_Write (&client->message, sv.reliable_datagram.data, sv.reliable_datagram.cursize);
	}

	SZ_Clear (&sv.reliable_datagram);
}


/*
=======================
SV_SendNop

Send a nop message without trashing or sending the accumulated client
message buffer
=======================
*/
static void SV_SendNop (client_t *client)
{
	sizebuf_t	msg;
	byte		buf[4];

	SZ_Init (&msg, buf, sizeof(buf));

	MSG_WriteChar (&msg, svc_nop);

	if (NET_SendUnreliableMessage (client->netconnection, &msg) == -1)
		SV_DropClient (true);	// if the message couldn't send, kick off
	client->last_message = realtime;
}

/*
=======================
SV_SendClientMessages
=======================
*/
void SV_SendClientMessages (void)
{
	int			i;

// update frags, names, etc
	SV_UpdateToReliableMessages ();

// build individual updates
	for (i = 0, host_client = svs.clients; i < svs.maxclients; i++, host_client++)
	{
		if (!host_client->active)
			continue;

		if (host_client->spawned)
		{
			if (!SV_SendClientDatagram (host_client))
				continue;
		}
		else
		{
		// the player isn't totally in the game yet
		// send small keepalive messages if too much time has passed
		// send a full message when the next signon stage has been requested
		// some other message data (name changes, etc) may accumulate
		// between signon stages
			if (!host_client->sendsignon)
			{
				if (realtime - host_client->last_message > 5)
					SV_SendNop (host_client);
				continue;	// don't send out non-signon messages
			}
		}

		// check for an overflowed message.  Should only happen
		// on a very fucked up connection that backs up a lot, then
		// changes level
		if (host_client->message.overflowed)
		{
			SV_DropClient (true);
			host_client->message.overflowed = false;
			continue;
		}

		if (host_client->message.cursize || host_client->dropasap)
		{
			if (!NET_CanSendMessage (host_client->netconnection))
			{
			//	I_Printf ("can't write\n");
				continue;
			}

			if (host_client->dropasap)
				SV_DropClient (false);	// went to another level
			else
			{
				if (NET_SendMessage (host_client->netconnection,
						&host_client->message) == -1)
					SV_DropClient (true);	// if the message couldn't send, kick off
				SZ_Clear (&host_client->message);
				host_client->last_message = realtime;
				host_client->sendsignon = false;
			}
		}
	}

// clear muzzle flashes
	SV_CleanupEnts ();
}


/*
==============================================================================

SERVER SPAWNING

==============================================================================
*/

/*
================
SV_ModelIndex

================
*/
int SV_ModelIndex (const char *name)
{
	int		i;

	if (!name || !name[0])
		return 0;

	for (i = 0; i < MAX_MODELS && sv.model_precache[i]; i++)
	{
		if (!strcmp(sv.model_precache[i], name))
			return i;
	}

	if (i == MAX_MODELS || !sv.model_precache[i])
	{
		Con_Printf("%s: model %s not precached\n", __thisfunc__, name);
		return 0;
	}

	return i;
}

/*
================
SV_CreateBaseline

================
*/
static void SV_CreateBaseline (void)
{
	int			i;
	edict_t			*svent;
	int				entnum;

	for (entnum = 0; entnum < sv.num_edicts ; entnum++)
	{
	// get the current server version
		svent = EDICT_NUM(entnum);
		if (svent->free)
			continue;
		if (entnum > svs.maxclients && !svent->v.modelindex)
			continue;

	//
	// create entity baseline
	//
		VectorCopy (svent->v.origin, svent->baseline.origin);
		VectorCopy (svent->v.angles, svent->baseline.angles);
		svent->baseline.frame = svent->v.frame;
		svent->baseline.skin = svent->v.skin;
		svent->baseline.scale = (int)(svent->v.scale*100.0)&255;
		svent->baseline.drawflags = svent->v.drawflags;
		svent->baseline.abslight = (int)(svent->v.abslight*255.0)&255;
		if (entnum > 0	&& entnum <= svs.maxclients)
		{
			svent->baseline.colormap = entnum;
			svent->baseline.modelindex = 0;//SV_ModelIndex("models/paladin.mdl");
		}
		else
		{
			svent->baseline.colormap = 0;
			svent->baseline.modelindex =
				SV_ModelIndex(PR_GetString(svent->v.model));
		}
		memset (svent->baseline.ClearCount, 99, sizeof(svent->baseline.ClearCount));

	//
	// add to the message
	//
		MSG_WriteByte (&sv.signon,svc_spawnbaseline);
		MSG_WriteShort (&sv.signon,entnum);

		MSG_WriteShort (&sv.signon, svent->baseline.modelindex);
		MSG_WriteByte (&sv.signon, svent->baseline.frame);
		MSG_WriteByte (&sv.signon, svent->baseline.colormap);
		MSG_WriteByte (&sv.signon, svent->baseline.skin);
		MSG_WriteByte (&sv.signon, svent->baseline.scale);
		MSG_WriteByte (&sv.signon, svent->baseline.drawflags);
		MSG_WriteByte (&sv.signon, svent->baseline.abslight);
		for (i = 0; i < 3; i++)
		{
			MSG_WriteCoord (&sv.signon, svent->baseline.origin[i]);
			MSG_WriteAngle (&sv.signon, svent->baseline.angles[i]);
		}
	}
}


/*
================
SV_SendReconnect

Tell all the clients that the server is changing levels
================
*/
static void SV_SendReconnect (void)
{
	byte	data[128];
	sizebuf_t	msg;

	SZ_Init (&msg, data, sizeof(data));

	MSG_WriteChar (&msg, svc_stufftext);
	MSG_WriteString (&msg, "reconnect\n");
	NET_SendToAll (&msg, 5.0);

#if !defined(SERVERONLY)
	if (!isDedicated)
#ifdef QUAKE2
		Cbuf_InsertText ("reconnect\n");
#else
		Cmd_ExecuteString ("reconnect\n", src_command);
#endif
#endif	/* ! SERVERONLY */
}


/*
================
SV_GetLevelname

Return the full levelname
================
*/
const char *SV_GetLevelname (void)
{
	int idx = (int)sv.edicts->v.message;
	if (idx > 0 && idx <= host_string_count)
		return Host_GetString(idx - 1);

/*	return "";*/
/* Use netname on map if there is one, so they don't have to edit strings.txt */
	return PR_GetString(sv.edicts->v.netname);
}


/*
================
SV_SaveSpawnparms

Grabs the current state of each client for saving across the
transition to another level
================
*/
void SV_SaveSpawnparms (void)
{
	int		i;
//	int		j;

	svs.serverflags = *sv_globals.serverflags;

	for (i = 0, host_client = svs.clients; i < svs.maxclients; i++, host_client++)
	{
		if (!host_client->active)
			continue;

	// call the progs to get default spawn parms for the new client
//		*sv_globals.self = EDICT_TO_PROG(host_client->edict);
//		PR_ExecuteProgram (*sv_globals.SetChangeParms);
//		for (j = 0; j < NUM_SPAWN_PARMS; j++)
//			host_client->spawn_parms[j] = sv_globals.parm[j];
	}
}


/*
================
SV_SpawnServer

This is called at the start of each level
================
*/
void SV_SpawnServer (const char *server, const char *startspot)
{
	static char	dummy[8] = { 0,0,0,0,0,0,0,0 };
	edict_t		*ent;
	int			i;

	// let's not have any servers with no name
	if (hostname.string[0] == 0)
		Cvar_Set ("hostname", "UNNAMED");
#if !defined(SERVERONLY)
	scr_centertime_off = 0;
#endif

	Con_DPrintf ("%s: %s\n", __thisfunc__, server);
	if (svs.changelevel_issued)
	{
		SaveGamestate(true);
	}

//
// tell all connected clients that we are going to a new level
//
	if (sv.active)
	{
		SV_SendReconnect ();
	}

/* if this is GL version, we need to tell D_FlushCaches() whether to flush
   OGL textures depending on mapname change. */
#ifdef GLQUAKE
	flush_textures = q_strncasecmp(server, sv.name, 64) ? true : false;
#endif

//
// make cvars consistant
//
	if (coop.integer)
		Cvar_Set ("deathmatch", "0");

	current_skill = skill.integer;
	if (current_skill < 0)
		current_skill = 0;
	if (current_skill > 4)
		current_skill = 4;

	Cvar_SetValue ("skill", current_skill);

//
// set up the new server
//
	//memset (&sv, 0, sizeof(sv));
	Host_ClearMemory ();

	q_strlcpy (sv.name, server, sizeof(sv.name));
	if (startspot)
		q_strlcpy(sv.startspot, startspot, sizeof(sv.startspot));

// load progs to get entity field count
#if !defined(SERVERONLY)
	total_loading_size = 100;
	current_loading_size = 0;
	loading_stage = 1;
	D_ShowLoadingSize();
#endif
	PR_LoadProgs ();
#if !defined(SERVERONLY)
	current_loading_size += 10;
	D_ShowLoadingSize();
#endif
	Host_LoadStrings();
#if !defined(SERVERONLY)
	current_loading_size += 5;
	D_ShowLoadingSize();
#endif

// allocate server memory
	/* Host_ClearMemory() called above already cleared the whole sv structure */
	sv.states = (client_state2_t *) Hunk_AllocName (svs.maxclients * sizeof(client_state2_t), "states");
	sv.edicts = (edict_t *) Hunk_AllocName (MAX_EDICTS*pr_edict_size, "edicts");

	SZ_Init (&sv.datagram, sv.datagram_buf, sizeof(sv.datagram_buf));
	SZ_Init (&sv.reliable_datagram, sv.reliable_datagram_buf, sizeof(sv.reliable_datagram_buf));
	SZ_Init (&sv.signon, sv.signon_buf, sizeof(sv.signon_buf));

// leave slots at start for clients only
	sv.num_edicts = svs.maxclients + 1 + max_temp_edicts.integer;
	for (i = 0; i < svs.maxclients; i++)
	{
		ent = EDICT_NUM(i+1);
		svs.clients[i].edict = ent;
		svs.clients[i].send_all_v = true;
	}

	for (i = 0; i < max_temp_edicts.integer; i++)
	{
		ent = EDICT_NUM(i + svs.maxclients + 1);
		ED_ClearEdict(ent);

		ent->free = true;
		ent->freetime = -999;
	}

	sv.state = ss_loading;
	sv.paused = false;

	sv.time = 1.0;

	q_strlcpy (sv.name, server, sizeof(sv.name));
	q_snprintf (sv.modelname, sizeof(sv.modelname), "maps/%s.bsp", server);

	sv.worldmodel = Mod_ForName (sv.modelname, false);
	if (!sv.worldmodel)
	{
		Con_Printf ("Couldn't spawn server %s\n", sv.modelname);
		sv.active = false;
#if !defined(SERVERONLY)
		total_loading_size = 0;
		loading_stage = 0;
#endif
		return;
	}
	sv.models[1] = sv.worldmodel;

//
// clear world interaction links
//
	SV_ClearWorld ();

	sv.sound_precache[0] = dummy;
	sv.model_precache[0] = dummy;
	sv.model_precache[1] = sv.modelname;
	for (i = 1; i < sv.worldmodel->numsubmodels; i++)
	{
		sv.model_precache[1+i] = localmodels[i];
		sv.models[i+1] = Mod_ForName (localmodels[i], false);
	}

//
// load the rest of the entities
//
	ent = EDICT_NUM(0);
	memset (&ent->v, 0, progs->entityfields * 4);
	ent->free = false;
	ent->v.model = PR_SetEngineString(sv.worldmodel->name);
	ent->v.modelindex = 1;		// world model
	ent->v.solid = SOLID_BSP;
	ent->v.movetype = MOVETYPE_PUSH;

	if (coop.integer)
		*sv_globals.coop = coop.value;
	else
		*sv_globals.deathmatch = deathmatch.value;
	if (sv_globals.randomclass)
		*sv_globals.randomclass = randomclass.value;
	*sv_globals.mapname = PR_SetEngineString(sv.name);
	*sv_globals.startspot = PR_SetEngineString(sv.startspot);
	// serverflags are for cross level information (sigils)
	*sv_globals.serverflags = svs.serverflags;

#if !defined(SERVERONLY)
	current_loading_size += 5;
	D_ShowLoadingSize();
#endif
	ED_LoadFromFile (sv.worldmodel->entities);

	sv.active = true;

// all setup is completed, any further precache statements are errors
	sv.state = ss_active;

// run two frames to allow everything to settle
	host_frametime = 0.1;
	SV_Physics ();
	SV_Physics ();

// create a baseline for more efficient communications
	SV_CreateBaseline ();

// send serverinfo to all connected clients
	for (i = 0, host_client = svs.clients; i < svs.maxclients; i++, host_client++)
	{
		if (host_client->active)
			SV_SendServerinfo (host_client);
	}

	svs.changelevel_issued = false;		// now safe to issue another

	Con_DPrintf ("Server spawned.\n");

#if !defined(SERVERONLY)
	total_loading_size = 0;
	loading_stage = 0;
#endif
}

