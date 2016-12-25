/*
 * sv_send.c -- server communication module
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

unsigned int	clients_multicast;

#define	CHAN_AUTO	0
#define	CHAN_WEAPON	1
#define	CHAN_VOICE	2
#define	CHAN_ITEM	3
#define	CHAN_BODY	4
#define	PHS_OVERRIDE_R	8


/*
=============================================================================

Con_Printf redirection

=============================================================================
*/

static char	outputbuf[8000];

redirect_t	sv_redirected;

extern	cvar_t	sv_phs;
extern	cvar_t	sv_namedistance;

extern	int	devlog;


/*
==================
SV_FlushRedirect
==================
*/
static void SV_FlushRedirect (void)
{
	if (sv_redirected == RD_PACKET)
	{
		unsigned char	senddata[8000 + 6];
		size_t siz = strlen(outputbuf) + 1;
		senddata[0] = 0xff;
		senddata[1] = 0xff;
		senddata[2] = 0xff;
		senddata[3] = 0xff;
		senddata[4] = A2C_PRINT;
		memcpy (&senddata[5], outputbuf, siz);

		NET_SendPacket (siz + 5, senddata, &net_from);
	}
	else if (sv_redirected == RD_CLIENT)
	{
		MSG_WriteByte (&host_client->netchan.message, svc_print);
		MSG_WriteByte (&host_client->netchan.message, PRINT_HIGH);
		MSG_WriteString (&host_client->netchan.message, outputbuf);
	}

	// clear it
	outputbuf[0] = 0;
}

/*
==================
SV_BeginRedirect

Send Con_Printf data to the remote client
instead of the console
==================
*/
void SV_BeginRedirect (redirect_t rd)
{
	sv_redirected = rd;
	outputbuf[0] = 0;
}

void SV_EndRedirect (void)
{
	SV_FlushRedirect ();
	sv_redirected = RD_NONE;
}

/*
================
CON_Printf
Prints either to the console, or, if redirection
is in effect, to the relevant client.
================
*/
void CON_Printf (unsigned int flags, const char *fmt, ...)
{
	va_list		argptr;
	char		msg[MAX_PRINTMSG];

	if (flags & _PRINT_DEVEL && !developer.integer)
	{
		if (devlog && sv_logfile)	/* full logging */
		{
			va_start (argptr, fmt);
			q_vsnprintf (msg, sizeof(msg), fmt, argptr);
			va_end (argptr);
			fprintf (sv_logfile, "%s", msg);
			fflush (sv_logfile);
		}
		return;
	}

	va_start (argptr, fmt);
	q_vsnprintf (msg, sizeof(msg), fmt, argptr);
	va_end (argptr);

	if (flags & _PRINT_TERMONLY)
		goto _end;

	// add to redirected message
	if (sv_redirected)
	{
		if (strlen(msg) + strlen(outputbuf) > sizeof(outputbuf) - 1)
			SV_FlushRedirect ();
		q_strlcat (outputbuf, msg, sizeof(outputbuf));
		return;
	}

_end:
	Sys_PrintTerm (msg);	// echo to the terminal
	if (sv_logfile)
	{
		fprintf (sv_logfile, "%s", msg);
		fflush (sv_logfile);
	}
}


/*
=============================================================================

EVENT MESSAGES

=============================================================================
*/

/*
=================
SV_ClientPrintf

Sends text across to be displayed if the level passes
=================
*/
void SV_ClientPrintf (client_t *cl, int level, const char *fmt, ...)
{
	va_list		argptr;
	char		string[1024];

	if (level < cl->messagelevel)
		return;

	va_start (argptr, fmt);
	q_vsnprintf (string, sizeof (string), fmt, argptr);
	va_end (argptr);

	MSG_WriteByte (&cl->netchan.message, svc_print);
	MSG_WriteByte (&cl->netchan.message, level);
	MSG_WriteString (&cl->netchan.message, string);
}

/*
=================
SV_BroadcastPrintf

Sends text to all active clients
=================
*/
void SV_BroadcastPrintf (int level, const char *fmt, ...)
{
	va_list		argptr;
	char		string[1024];
	client_t	*cl;
	int			i;

	va_start (argptr, fmt);
	q_vsnprintf (string, sizeof(string), fmt, argptr);
	va_end (argptr);

	Sys_Printf ("%s", string);	// print to the console

	for (i = 0, cl = svs.clients; i < MAX_CLIENTS; i++, cl++)
	{
		if (level < cl->messagelevel)
			continue;
		if (!cl->state)
			continue;
		MSG_WriteByte (&cl->netchan.message, svc_print);
		MSG_WriteByte (&cl->netchan.message, level);
		MSG_WriteString (&cl->netchan.message, string);
	}
}

/*
=================
SV_BroadcastCommand

Sends text to all active clients
=================
*/
void SV_BroadcastCommand (const char *fmt, ...)
{
	va_list		argptr;
	char		string[1024];

	if (!sv.state)
		return;
	va_start (argptr, fmt);
	q_vsnprintf (string, sizeof (string), fmt, argptr);
	va_end (argptr);

	MSG_WriteByte (&sv.reliable_datagram, svc_stufftext);
	MSG_WriteString (&sv.reliable_datagram, string);
}

/*
=================
SV_Multicast

Sends the contents of sv.multicast to a subset of the clients,
then clears sv.multicast.

MULTICAST_ALL	same as broadcast
MULTICAST_PVS	send to clients potentially visible from org
MULTICAST_PHS	send to clients potentially hearable from org
=================
*/
void SV_Multicast (vec3_t origin, int to)
{
	client_t	*client;
	byte		*mask;
	mleaf_t		*leaf;
	int			leafnum;
	int			j;
	qboolean	reliable;
	vec3_t		adjust_origin;

	clients_multicast = 0;

	leaf = Mod_PointInLeaf (origin, sv.worldmodel);
	if (!leaf)
		leafnum = 0;
	else
		leafnum = leaf - sv.worldmodel->leafs;

	reliable = false;

	switch (to)
	{
	case MULTICAST_ALL_R:
		reliable = true;	// intentional fallthrough
	case MULTICAST_ALL:
		mask = sv.pvs;		// leaf 0 is everything;
		break;

	case MULTICAST_PHS_R:
		reliable = true;	// intentional fallthrough
	case MULTICAST_PHS:
		mask = sv.phs + leafnum * 4 * ((sv.worldmodel->numleafs + 31) >> 5);
		break;

	case MULTICAST_PVS_R:
		reliable = true;	// intentional fallthrough
	case MULTICAST_PVS:
		mask = sv.pvs + leafnum * 4 * ((sv.worldmodel->numleafs + 31) >> 5);
		break;

	default:
		mask = NULL;
		SV_Error ("%s: bad to: %i", __thisfunc__, to);
	}

	// send the data to all relevent clients
	for (j = 0, client = svs.clients; j < MAX_CLIENTS; j++, client++)
	{
		if (client->state != cs_spawned)
			continue;

		VectorCopy(client->edict->v.origin, adjust_origin);
		adjust_origin[2] += 16;
		leaf = Mod_PointInLeaf (adjust_origin, sv.worldmodel);
		if (leaf)// && leaf != sv.worldmodel->leafs)
		{
			// -1 is because pvs rows are 1 based, not 0 based like leafs
			leafnum = leaf - sv.worldmodel->leafs - 1;
			if ( !(mask[leafnum>>3] & (1 << (leafnum & 7)) ))
			{
			//	Con_Printf ("supressed multicast\n");
				if (mask == sv.pvs)
					Sys_Printf("supressed multicast to all!!!\n");
				continue;
			}
		}

		clients_multicast |= 1l << j;

		if (reliable)
			SZ_Write (&client->netchan.message, sv.multicast.data, sv.multicast.cursize);
		else
			SZ_Write (&client->datagram, sv.multicast.data, sv.multicast.cursize);
	}

	SZ_Clear (&sv.multicast);
}

/*
=================
SV_MulticastSpecific

Sends the contents of sv.multicast to a subset of the clients,
then clears sv.multicast.
=================
*/
void SV_MulticastSpecific (unsigned int clients, qboolean reliable)
{
	client_t	*client;
	int			j;

	clients_multicast = 0;

	// send the data to all relevent clients
	for (j = 0, client = svs.clients; j < MAX_CLIENTS; j++, client++)
	{
		if (client->state != cs_spawned)
			continue;

		if ((1l << j) & clients)
		{
			clients_multicast |= 1l << j;

			if (reliable)
				SZ_Write (&client->netchan.message, sv.multicast.data, sv.multicast.cursize);
			else
				SZ_Write (&client->datagram, sv.multicast.data, sv.multicast.cursize);
		}
	}

	SZ_Clear (&sv.multicast);
}


/*
==================
SV_StopSound
==================
*/
void SV_StopSound (edict_t *entity, int channel)
{
	int			ent, i;
	vec3_t		origin;

	ent = NUM_FOR_EDICT(entity);
	channel = (ent<<3) | channel;

	// use the entity origin unless it is a bmodel
	if (entity->v.solid == SOLID_BSP)
	{
		for (i = 0; i < 3; i++) //FIXME: This may not work- should be using (absmin + absmax)*0.5?
			origin[i] = entity->v.origin[i] + 0.5 * (entity->v.mins[i] + entity->v.maxs[i]);
	}
	else
	{
		VectorCopy (entity->v.origin, origin);
	}

	MSG_WriteByte (&sv.multicast, svc_stopsound);
	MSG_WriteShort (&sv.multicast, channel);
	SV_Multicast (origin, MULTICAST_ALL_R);
}


/*
==================
SV_UpdateSoundPos
==================
*/
void SV_UpdateSoundPos (edict_t *entity, int channel)
{
	int			ent, i;
	vec3_t		origin;

	ent = NUM_FOR_EDICT(entity);
	channel = (ent<<3) | channel;

	// use the entity origin unless it is a bmodel
	if (entity->v.solid == SOLID_BSP)
	{
		for (i = 0; i < 3; i++) //FIXME: This may not work- should be using (absmin + absmax)*0.5?
			origin[i] = entity->v.origin[i] + 0.5 * (entity->v.mins[i] + entity->v.maxs[i]);
	}
	else
	{
		VectorCopy (entity->v.origin, origin);
	}

	MSG_WriteByte (&sv.multicast, svc_sound_update_pos);
	MSG_WriteShort (&sv.multicast, channel);
	for (i = 0; i < 3; i++)
		MSG_WriteCoord (&sv.multicast, entity->v.origin[i] + 0.5 * (entity->v.mins[i] + entity->v.maxs[i]));
	SV_Multicast (origin, MULTICAST_PHS);
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
	int			i;
	vec3_t		origin;
	qboolean	use_phs;
	qboolean	reliable = false;

	if (volume < 0 || volume > 255)
		SV_Error ("%s: volume = %i", __thisfunc__, volume);

	if (attenuation < 0 || attenuation > 4)
		SV_Error ("%s: attenuation = %f", __thisfunc__, attenuation);

	if (channel < 0 || channel > 15)
		SV_Error ("%s: channel = %i", __thisfunc__, channel);

// find precache number for sound
	for (sound_num = 0; sound_num < MAX_SOUNDS && sv.sound_precache[sound_num]; sound_num++)
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

	if ((channel & PHS_OVERRIDE_R) || !sv_phs.integer)	// no PHS flag
	{
		if (channel & PHS_OVERRIDE_R)	//PHS_OVERRIDE_R = 8
			reliable = true;	// sounds that break the phs are reliable
		use_phs = false;
		channel &= 7;	//clear out the PHS_OVERRIDE_R flag
	}
	else
		use_phs = true;

//	if (channel == CHAN_BODY || channel == CHAN_VOICE)
//		reliable = true;

	channel = (ent<<3) | channel;

	if (volume != DEFAULT_SOUND_PACKET_VOLUME)
		channel |= SND_VOLUME;
	if (attenuation != DEFAULT_SOUND_PACKET_ATTENUATION)
		channel |= SND_ATTENUATION;

	// use the entity origin unless it is a bmodel
	if (entity->v.solid == SOLID_BSP)
	{
		for (i = 0; i < 3; i++)	//FIXME: This may not work- should be using (absmin + absmax)*0.5?
			origin[i] = entity->v.origin[i]+0.5*(entity->v.mins[i]+entity->v.maxs[i]);
	}
	else
	{
		VectorCopy (entity->v.origin, origin);
	}

	MSG_WriteByte (&sv.multicast, svc_sound);
	MSG_WriteShort (&sv.multicast, channel);
	if (channel & SND_VOLUME)
		MSG_WriteByte (&sv.multicast, volume);
	if (channel & SND_ATTENUATION)
		MSG_WriteByte (&sv.multicast, attenuation*32);
	MSG_WriteByte (&sv.multicast, sound_num);
	for (i = 0; i < 3; i++)
		MSG_WriteCoord (&sv.multicast, origin[i]);

	if (use_phs)
		SV_Multicast (origin, reliable ? MULTICAST_PHS_R : MULTICAST_PHS);
	else
		SV_Multicast (origin, reliable ? MULTICAST_ALL_R : MULTICAST_ALL);
}


/*
==================
SV_StartParticle

Make sure the event gets sent to all clients
==================
*/
void SV_StartParticle (vec3_t org, vec3_t dir, int color, int count)
{
	int		i, v;

	MSG_WriteByte (&sv.multicast, svc_particle);
	MSG_WriteCoord (&sv.multicast, org[0]);
	MSG_WriteCoord (&sv.multicast, org[1]);
	MSG_WriteCoord (&sv.multicast, org[2]);
	for (i = 0; i < 3; i++)
	{
		v = dir[i] * 16;
		if (v > 127)
			v = 127;
		else if (v < -128)
			v = -128;
		MSG_WriteChar (&sv.multicast, v);
	}
	MSG_WriteByte (&sv.multicast, count);
	MSG_WriteByte (&sv.multicast, color);

	SV_Multicast (org, MULTICAST_PVS);
}


/*
==================
SV_StartParticle2

Make sure the event gets sent to all clients
==================
*/
void SV_StartParticle2 (vec3_t org, vec3_t dmin, vec3_t dmax, int color, int effect, int count)
{
	MSG_WriteByte (&sv.multicast, svc_particle2);
	MSG_WriteCoord (&sv.multicast, org[0]);
	MSG_WriteCoord (&sv.multicast, org[1]);
	MSG_WriteCoord (&sv.multicast, org[2]);
	MSG_WriteFloat (&sv.multicast, dmin[0]);
	MSG_WriteFloat (&sv.multicast, dmin[1]);
	MSG_WriteFloat (&sv.multicast, dmin[2]);
	MSG_WriteFloat (&sv.multicast, dmax[0]);
	MSG_WriteFloat (&sv.multicast, dmax[1]);
	MSG_WriteFloat (&sv.multicast, dmax[2]);

	MSG_WriteShort (&sv.multicast, color);
	MSG_WriteByte (&sv.multicast, count);
	MSG_WriteByte (&sv.multicast, effect);

	SV_Multicast (org, MULTICAST_PVS);
}


/*
==================
SV_StartParticle3

Make sure the event gets sent to all clients
==================
*/
void SV_StartParticle3 (vec3_t org, vec3_t box, int color, int effect, int count)
{
	MSG_WriteByte (&sv.multicast, svc_particle3);
	MSG_WriteCoord (&sv.multicast, org[0]);
	MSG_WriteCoord (&sv.multicast, org[1]);
	MSG_WriteCoord (&sv.multicast, org[2]);
	MSG_WriteByte (&sv.multicast, box[0]);
	MSG_WriteByte (&sv.multicast, box[1]);
	MSG_WriteByte (&sv.multicast, box[2]);

	MSG_WriteShort (&sv.multicast, color);
	MSG_WriteByte (&sv.multicast, count);
	MSG_WriteByte (&sv.multicast, effect);

	SV_Multicast (org, MULTICAST_PVS);
}


/*
==================
SV_StartParticle4

Make sure the event gets sent to all clients
==================
*/
void SV_StartParticle4 (vec3_t org, float radius, int color, int effect, int count)
{
	MSG_WriteByte (&sv.multicast, svc_particle4);
	MSG_WriteCoord (&sv.multicast, org[0]);
	MSG_WriteCoord (&sv.multicast, org[1]);
	MSG_WriteCoord (&sv.multicast, org[2]);
	MSG_WriteByte (&sv.multicast, radius);

	MSG_WriteShort (&sv.multicast, color);
	MSG_WriteByte (&sv.multicast, count);
	MSG_WriteByte (&sv.multicast, effect);

	SV_Multicast (org, MULTICAST_PVS);
}


void SV_StartRainEffect (vec3_t org, vec3_t e_size, int x_dir, int y_dir, int color, int count)
{
	MSG_WriteByte (&sv.multicast, svc_raineffect);
	MSG_WriteCoord (&sv.multicast, org[0]);
	MSG_WriteCoord (&sv.multicast, org[1]);
	MSG_WriteCoord (&sv.multicast, org[2]);
	MSG_WriteCoord (&sv.multicast, e_size[0]);
	MSG_WriteCoord (&sv.multicast, e_size[1]);
	MSG_WriteCoord (&sv.multicast, e_size[2]);
	MSG_WriteAngle (&sv.multicast, x_dir);
	MSG_WriteAngle (&sv.multicast, y_dir);
	MSG_WriteShort (&sv.multicast, color);
	MSG_WriteShort (&sv.multicast, count);

	SV_Multicast (org, MULTICAST_PVS);
}


/*
===============================================================================

FRAME UPDATES

===============================================================================
*/

//int		sv_nailmodel, sv_supernailmodel, sv_playermodel[MAX_PLAYER_CLASS];
int		sv_magicmissmodel, sv_playermodel[MAX_PLAYER_CLASS];
int		sv_ravenmodel, sv_raven2model;

void SV_FindModelNumbers (void)
{
	int		i;

//	sv_nailmodel = -1;
//	sv_supernailmodel = -1;
	sv_playermodel[0] = -1;
	sv_playermodel[1] = -1;
	sv_playermodel[2] = -1;
	sv_playermodel[3] = -1;
	sv_playermodel[4] = -1;
	sv_magicmissmodel = -1;
	sv_ravenmodel = -1;
	sv_raven2model = -1;

	for (i = 0; i < MAX_MODELS; i++)
	{
		if (!sv.model_precache[i])
			break;
//		if (!strcmp(sv.model_precache[i],"progs/spike.mdl"))
//			sv_nailmodel = i;
//		if (!strcmp(sv.model_precache[i],"progs/s_spike.mdl"))
//			sv_supernailmodel = i;
		if (!strcmp(sv.model_precache[i],"models/paladin.mdl"))
			sv_playermodel[0] = i;
		if (!strcmp(sv.model_precache[i],"models/crusader.mdl"))
			sv_playermodel[1] = i;
		if (!strcmp(sv.model_precache[i],"models/necro.mdl"))
			sv_playermodel[2] = i;
		if (!strcmp(sv.model_precache[i],"models/assassin.mdl"))
			sv_playermodel[3] = i;
		if (!strcmp(sv.model_precache[i],"models/succubus.mdl"))
			sv_playermodel[4] = i;
		if (!strcmp(sv.model_precache[i],"models/ball.mdl"))
			sv_magicmissmodel = i;
		if (!strcmp(sv.model_precache[i],"models/ravproj.mdl"))
			sv_ravenmodel = i;
		if (!strcmp(sv.model_precache[i],"models/vindsht1.mdl"))
			sv_raven2model = i;
	}
}


/*
==================
SV_WriteClientdataToMessage

==================
*/
void SV_WriteClientdataToMessage (client_t *client, sizebuf_t *msg)
{
	int		i;
	edict_t	*other;
	edict_t	*ent;

	ent = client->edict;

	// send the chokecount for r_netgraph
	if (client->chokecount)
	{
		MSG_WriteByte (msg, svc_chokecount);
		MSG_WriteByte (msg, client->chokecount);
		client->chokecount = 0;
	}

	// send a damage message if the player got hit this frame
	if (ent->v.dmg_take || ent->v.dmg_save)
	{
		other = PROG_TO_EDICT(ent->v.dmg_inflictor);
		MSG_WriteByte (msg, svc_damage);
		MSG_WriteByte (msg, ent->v.dmg_save);
		MSG_WriteByte (msg, ent->v.dmg_take);
		for (i = 0; i < 3; i++)
			MSG_WriteCoord (msg, other->v.origin[i] + 0.5 * (other->v.mins[i] + other->v.maxs[i]));

		ent->v.dmg_take = 0;
		ent->v.dmg_save = 0;
	}

	// a fixangle might get lost in a dropped packet.  Oh well.
	if ( ent->v.fixangle )
	{
		MSG_WriteByte (msg, svc_setangle);
		for (i = 0; i < 3; i++)
			MSG_WriteAngle (msg, ent->v.angles[i]);
		ent->v.fixangle = 0;
	}

	// if the player has a target, send its info...
	if (ent->v.targDist)
	{
		MSG_WriteByte(msg, svc_targetupdate);
		MSG_WriteByte(msg, ent->v.targAng);
		MSG_WriteByte(msg, ent->v.targPitch);
		MSG_WriteByte(msg, (ent->v.targDist < 255.0) ? (int)(ent->v.targDist) : 255);
	}
}


/*
=======================
SV_UpdateClientStats

Performs a delta update of the stats array.  This should only be performed
when a reliable message can be delivered this frame.
=======================
*/
static void SV_UpdateClientStats (client_t *client)
{
	edict_t	*ent;
	int		stats[MAX_CL_STATS];
	int		i;

	ent = client->edict;
	memset (stats, 0, sizeof(stats));

	// if we are a spectator and we are tracking a player, we get his stats
	// so our status bar reflects his
	if (client->spectator && client->spec_track > 0)
		ent = svs.clients[client->spec_track - 1].edict;

	stats[STAT_HEALTH] = 0;	//ent->v.health;
	stats[STAT_WEAPON] = SV_ModelIndex(PR_GetString(ent->v.weaponmodel));
	stats[STAT_AMMO] = 0;	//ent->v.currentammo;
	stats[STAT_ARMOR] = 0;	//ent->v.armorvalue;
	stats[STAT_SHELLS] = 0;	//ent->v.ammo_shells;
	stats[STAT_NAILS] = 0;	//ent->v.ammo_nails;
	stats[STAT_ROCKETS] = 0;//ent->v.ammo_rockets;
	stats[STAT_CELLS] = 0;	//ent->v.ammo_cells;
	if (!client->spectator)
		stats[STAT_ACTIVEWEAPON] = 0;	//ent->v.weapon;
	// stuff the sigil bits into the high bits of items for sbar
	stats[STAT_ITEMS] = 0;	//(int)ent->v.items | ((int)*sv_globals.serverflags << 28);

	for (i = 0; i < MAX_CL_STATS; i++)
	{
		if (stats[i] != client->stats[i])
		{
			client->stats[i] = stats[i];
			if (stats[i] >=0 && stats[i] <= 255)
			{
				MSG_WriteByte (&client->netchan.message, svc_updatestat);
				MSG_WriteByte (&client->netchan.message, i);
				MSG_WriteByte (&client->netchan.message, stats[i]);
			}
			else
			{
				MSG_WriteByte (&client->netchan.message, svc_updatestatlong);
				MSG_WriteByte (&client->netchan.message, i);
				MSG_WriteLong (&client->netchan.message, stats[i]);
			}
		}
	}
}


/*
=======================
SV_SendClientDatagram
=======================
*/
static qboolean SV_SendClientDatagram (client_t *client)
{
	byte		buf[MAX_DATAGRAM];
	sizebuf_t	msg;

	SZ_Init (&msg, buf, sizeof(buf));
	msg.allowoverflow = true;

	// add the client specific data to the datagram
	SV_WriteClientdataToMessage (client, &msg);

	// send over all the objects that are in the PVS
	// this will include clients, a packetentities, and
	// possibly a nails update
	SV_WriteEntitiesToClient (client, &msg);

	// copy the accumulated multicast datagram
	// for this client out to the message
	if (client->datagram.overflowed)
		Con_Printf ("WARNING: datagram overflowed for %s\n", client->name);
	else
		SZ_Write (&msg, client->datagram.data, client->datagram.cursize);
	SZ_Clear (&client->datagram);

	// send deltas over reliable stream
	if (Netchan_CanReliable (&client->netchan))
		SV_UpdateClientStats (client);

	if (msg.overflowed)
	{
		Con_Printf ("WARNING: msg overflowed for %s\n", client->name);
		SZ_Clear (&msg);
	}

	// send the datagram
	Netchan_Transmit (&client->netchan, msg.cursize, buf);

	return true;
}


static qboolean ValidToShowName(edict_t *edict)
{
	if (edict->v.deadflag)
		return false;
	if ((int)edict->v.effects & EF_NODRAW)
		return false;
	return true;
}


static void UpdatePIV(void)
{
	int			i, j;
	client_t	*client;
	trace_t		trace;
	vec3_t		adjust_org1, adjust_org2, distvec;
	float		save_hull, dist;

	for (i = 0, host_client = svs.clients; i < MAX_CLIENTS; i++, host_client++)
	{
		host_client->PIV = 0;
	}

	for (i = 0, host_client = svs.clients; i < MAX_CLIENTS; i++, host_client++)
	{
		if (host_client->state != cs_spawned || host_client->spectator)
			continue;

		VectorCopy(host_client->edict->v.origin, adjust_org1);
		adjust_org1[2] += 24;

		save_hull = host_client->edict->v.hull;
		host_client->edict->v.hull = 0;

		for (j = i+1, client = host_client+1; j < MAX_CLIENTS; j++, client++)
		{
			if (client->state != cs_spawned || client->spectator)
				continue;

			VectorSubtract(client->edict->v.origin, host_client->edict->v.origin, distvec);
			dist = VectorNormalize(distvec);
			if (dist > sv_namedistance.value)
			{
			//	Con_Printf("dist %f\n", dist);
				continue;
			}

			VectorCopy(client->edict->v.origin, adjust_org2);
			adjust_org2[2] += 24;

			trace = SV_Move (adjust_org1, vec3_origin, vec3_origin, adjust_org2, false, host_client->edict);
			if (trace.ent == client->edict)
			{	//can see each other, check for invisible, dead
				if (ValidToShowName(client->edict))
					host_client->PIV |= 1<<j;
				if (ValidToShowName(host_client->edict))
					client->PIV |= 1<<i;
			}
		}
		host_client->edict->v.hull = save_hull;
	}
}


/*
=======================
SV_UpdateToReliableMessages
=======================
*/
static void SV_UpdateToReliableMessages (void)
{
	int			i, j;
	client_t	*client;
	eval_t		*val;
	edict_t		*ent;
	qboolean	CheckPIV = false;

//	Con_Printf("%s\n", __thisfunc__);
	if (sv.time - sv.next_PIV_time >= 1)
	{
		sv.next_PIV_time = sv.time + 1;
		CheckPIV = true;
		UpdatePIV();
	}

// check for changes to be sent over the reliable streams to all clients
	for (i = 0, host_client = svs.clients; i < MAX_CLIENTS; i++, host_client++)
	{
		if (host_client->state != cs_spawned)
			continue;
		if (host_client->sendinfo)
		{
			host_client->sendinfo = false;
			SV_FullClientUpdate (host_client, &sv.reliable_datagram);
		}
		if (host_client->old_frags != host_client->edict->v.frags)
		{
			for (j = 0, client = svs.clients; j < MAX_CLIENTS; j++, client++)
			{
				if (client->state < cs_connected)
					continue;
			//	Sys_Printf("%s: Updated frags for client %d to %d\n", __thisfunc__, i, j);
				MSG_WriteByte (&client->netchan.message, svc_updatedminfo);
				MSG_WriteByte (&client->netchan.message, i);
				MSG_WriteShort (&client->netchan.message, host_client->edict->v.frags);
				MSG_WriteByte (&client->netchan.message, (host_client->playerclass<<5)|((int)host_client->edict->v.level&31));

				if (dmMode.integer == DM_SIEGE && SV_PROGS_HAVE_SIEGE)
				{
					MSG_WriteByte (&client->netchan.message, svc_updatesiegelosses);
					MSG_WriteByte (&client->netchan.message, *sv_globals.defLosses);
					MSG_WriteByte (&client->netchan.message, *sv_globals.attLosses);
				}
			}

			host_client->old_frags = host_client->edict->v.frags;
		}

		SV_WriteInventory(host_client, host_client->edict, &host_client->netchan.message);

		if (CheckPIV && host_client->PIV != host_client->LastPIV)
		{
			MSG_WriteByte (&host_client->netchan.message, svc_update_piv);
			MSG_WriteLong (&host_client->netchan.message, host_client->PIV);
			host_client->LastPIV = host_client->PIV;
		}

		// maxspeed/entgravity changes
		ent = host_client->edict;

		val = GetEdictFieldValue(ent, "gravity");
		if (val && host_client->entgravity != val->_float)
		{
			host_client->entgravity = val->_float;
			MSG_WriteByte(&host_client->netchan.message, svc_entgravity);
			MSG_WriteFloat(&host_client->netchan.message, host_client->entgravity);
		}
		val = GetEdictFieldValue(ent, "maxspeed");
		if (val && host_client->maxspeed != val->_float)
		{
			host_client->maxspeed = val->_float;
			MSG_WriteByte(&host_client->netchan.message, svc_maxspeed);
			MSG_WriteFloat(&host_client->netchan.message, host_client->maxspeed);
		}
	}

	if (sv.datagram.overflowed)
		SZ_Clear (&sv.datagram);

	// append the broadcast messages to each client messages
	for (j = 0, client = svs.clients; j < MAX_CLIENTS; j++, client++)
	{
		if (client->state < cs_connected)
			continue;	// reliables go to all connected or spawned
		SZ_Write (&client->netchan.message, sv.reliable_datagram.data, sv.reliable_datagram.cursize);

		if (client->state != cs_spawned)
			continue;	// datagrams only go to spawned
		SZ_Write (&client->datagram, sv.datagram.data, sv.datagram.cursize);
	}

	SZ_Clear (&sv.reliable_datagram);
	SZ_Clear (&sv.datagram);
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
		ent->v.wpn_sound = 0;
	}
}


/*
=======================
SV_SendClientMessages
=======================
*/
void SV_SendClientMessages (void)
{
	int			i;
	client_t	*c;

// update frags, names, etc
	SV_UpdateToReliableMessages ();

// build individual updates
	for (i = 0, c = svs.clients; i < MAX_CLIENTS; i++, c++)
	{
		if (!c->state)
			continue;
		// if the reliable message overflowed,
		// drop the client
		if (c->netchan.message.overflowed)
		{
			SZ_Clear (&c->netchan.message);
			SZ_Clear (&c->datagram);
			SV_BroadcastPrintf (PRINT_HIGH, "%s overflowed\n", c->name);
			Con_Printf ("WARNING: reliable overflow for %s\n", c->name);
			SV_DropClient (c);
			c->send_message = true;
			c->netchan.cleartime = 0;	// don't choke this message
		}

		// only send messages if the client has sent one
		// and the bandwidth is not choked
		if (!c->send_message)
			continue;
		c->send_message = false;	// try putting this after choke?
		if (!Netchan_CanPacket (&c->netchan))
		{
			c->chokecount++;
			continue;		// bandwidth choke
		}

		if (c->state == cs_spawned)
			SV_SendClientDatagram (c);
		else
			Netchan_Transmit (&c->netchan, 0, NULL);	// just update reliable
	}

	// clear muzzle flashes & wpn_sound
	SV_CleanupEnts ();
}


/*
=======================
SV_SendMessagesToAll

FIXME: is this sequence right?
=======================
*/
void SV_SendMessagesToAll (void)
{
	int			i;
	client_t	*c;

	for (i = 0, c = svs.clients; i < MAX_CLIENTS; i++, c++)
	{
		if (c->state)	// FIXME: should this only send to active?
			c->send_message = true;
	}

	SV_SendClientMessages ();
}

