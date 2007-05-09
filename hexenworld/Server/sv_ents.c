/*
	sv_ents.c
	server entities handling

	$Id: sv_ents.c,v 1.9 2007-05-09 18:11:37 sezero Exp $
*/

#include "quakedef.h"

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

//=============================================================================
/*
// because there can be a lot of nails, there is a special
// network protocol for them

	RDM: changed to use packed missiles, this code left here in case we
	decide to pack missiles which require a velocity as well

#define	MAX_NAILS	32
edict_t	*nails[MAX_NAILS];
int		numnails;

extern	int	sv_nailmodel, sv_supernailmodel, sv_playermodel[MAX_PLAYER_CLASS];

qboolean SV_AddNailUpdate (edict_t *ent)
{
	if (ent->v.modelindex != sv_nailmodel
		&& ent->v.modelindex != sv_supernailmodel)
		return false;
	if (numnails == MAX_NAILS)
		return true;
	nails[numnails] = ent;
	numnails++;
	return true;
}

void SV_EmitNailUpdate (sizebuf_t *msg)
{
	byte	bits[6];	// [48 bits] xyzpy 12 12 12 4 8 
	int		n, i;
	edict_t	*ent;
	int		x, y, z, p, yaw;

	if (!numnails)
		return;

	MSG_WriteByte (msg, svc_nails);
	MSG_WriteByte (msg, numnails);

	for (n = 0; n < numnails; n++)
	{
		ent = nails[n];
		x = (int)(ent->v.origin[0] + 4096) >> 1;
		y = (int)(ent->v.origin[1] + 4096) >> 1;
		z = (int)(ent->v.origin[2] + 4096) >> 1;
		p = (int)(16 * ent->v.angles[0] / 360) & 15;
		yaw = (int)(256 * ent->v.angles[1] / 360) & 255;

		bits[0] = x;
		bits[1] = (x>>8) | (y<<4);
		bits[2] = (y>>4);
		bits[3] = z;
		bits[4] = (z>>8) | (p<<4);
		bits[5] = yaw;

		for (i = 0; i < 6; i++)
			MSG_WriteByte (msg, bits[i]);
	}
}
*/

#define	MAX_MISSILES	32
static	edict_t	*missiles[MAX_MISSILES];
static	edict_t	*ravens[MAX_MISSILES];
static	edict_t	*raven2s[MAX_MISSILES];
static	int	nummissiles, numravens, numraven2s;
extern	int	sv_magicmissmodel, sv_playermodel[MAX_PLAYER_CLASS], sv_ravenmodel, sv_raven2model;

static qboolean SV_AddMissileUpdate (edict_t *ent)
{
	if (ent->v.modelindex == sv_magicmissmodel)
	{
		if (nummissiles == MAX_MISSILES)
			return true;
		missiles[nummissiles] = ent;
		nummissiles++;
		return true;
	}
	if (ent->v.modelindex == sv_ravenmodel)
	{
		if (numravens == MAX_MISSILES)
			return true;
		ravens[numravens] = ent;
		numravens++;
		return true;
	}
	if (ent->v.modelindex == sv_raven2model)
	{
		if (numraven2s == MAX_MISSILES)
			return true;
		raven2s[numraven2s] = ent;
		numraven2s++;
		return true;
	}
	return false;
}

static void SV_EmitMissileUpdate (sizebuf_t *msg)
{
	byte	bits[5];	// [40 bits] xyz type 12 12 12 4
	int		n, i;
	edict_t	*ent;
	int		x, y, z, type;

	if (!nummissiles)
		return;

	MSG_WriteByte (msg, svc_packmissile);
	MSG_WriteByte (msg, nummissiles);

	for (n = 0; n < nummissiles; n++)
	{
		ent = missiles[n];
		x = (int)(ent->v.origin[0] + 4096) >> 1;
		y = (int)(ent->v.origin[1] + 4096) >> 1;
		z = (int)(ent->v.origin[2] + 4096) >> 1;
		if (fabs(ent->v.scale - 0.1) < 0.05)
			type = 1;	//assume ice mace
		else
			type = 2;	//assume magic missile

		bits[0] = x;
		bits[1] = (x>>8) | (y<<4);
		bits[2] = (y>>4);
		bits[3] = z;
		bits[4] = (z>>8) | (type<<4);

		for (i = 0; i < 5; i++)
			MSG_WriteByte (msg, bits[i]);
	}
}

static void SV_EmitRavenUpdate (sizebuf_t *msg)
{
	byte	bits[6];	// [48 bits] xyzpy 12 12 12 4 8
	int		n, i;
	edict_t	*ent;
	int		x, y, z, p, yaw, frame;

	if ((!numravens) && (!numraven2s))
		return;

	MSG_WriteByte (msg, svc_nails);	//svc nails overloaded for ravens
	MSG_WriteByte (msg, numravens);

	for (n = 0; n < numravens; n++)
	{
		ent = ravens[n];
		x = (int)(ent->v.origin[0] + 4096) >> 1;
		y = (int)(ent->v.origin[1] + 4096) >> 1;
		z = (int)(ent->v.origin[2] + 4096) >> 1;
		p = (int)(16 * ent->v.angles[0] / 360) & 15;
		frame = (int)(ent->v.frame) & 7;
		yaw = (int)(32 * ent->v.angles[1] / 360) & 31;

		bits[0] = x;
		bits[1] = (x>>8) | (y<<4);
		bits[2] = (y>>4);
		bits[3] = z;
		bits[4] = (z>>8) | (p<<4);
		bits[5] = yaw | (frame<<5);

		for (i = 0; i < 6; i++)
			MSG_WriteByte (msg, bits[i]);
	}
	MSG_WriteByte (msg, numraven2s);

	for (n = 0; n < numraven2s; n++)
	{
		ent = raven2s[n];
		x = (int)(ent->v.origin[0] + 4096) >> 1;
		y = (int)(ent->v.origin[1] + 4096) >> 1;
		z = (int)(ent->v.origin[2] + 4096) >> 1;
		p = (int)(16 * ent->v.angles[0] / 360) & 15;
		yaw = (int)(256 * ent->v.angles[1] / 360) & 255;

		bits[0] = x;
		bits[1] = (x>>8) | (y<<4);
		bits[2] = (y>>4);
		bits[3] = z;
		bits[4] = (z>>8) | (p<<4);
		bits[5] = yaw;

		for (i = 0; i < 6; i++)
			MSG_WriteByte (msg, bits[i]);
	}
}

static void SV_EmitPackedEntities(sizebuf_t *msg)
{
	SV_EmitMissileUpdate(msg);
	SV_EmitRavenUpdate(msg);
}


//=============================================================================

/*
==================
SV_WriteDelta

Writes part of a packetentities message.
Can delta from either a baseline or a previous packet_entity
==================
*/
static void SV_WriteDelta (entity_state_t *from, entity_state_t *to, sizebuf_t *msg, qboolean force, edict_t *ent, client_t *client)
{
	int		bits;
	int		i;
	float	miss;
	int		temp_index;
	char	NewName[MAX_QPATH];

// send an update
	bits = 0;

	for (i = 0; i < 3; i++)
	{
		miss = to->origin[i] - from->origin[i];
		if ( miss < -0.1 || miss > 0.1 )
			bits |= U_ORIGIN1<<i;
	}

	if ( to->angles[0] != from->angles[0] )
		bits |= U_ANGLE1;

	if ( to->angles[1] != from->angles[1] )
		bits |= U_ANGLE2;

	if ( to->angles[2] != from->angles[2] )
		bits |= U_ANGLE3;

	if ( to->colormap != from->colormap )
		bits |= U_COLORMAP;

	if ( to->skinnum != from->skinnum)
	{
		bits |= U_SKIN;
	}

	if (to->drawflags != from->drawflags)
		bits |= U_DRAWFLAGS;

	if ( to->frame != from->frame )
		bits |= U_FRAME;

	if ( to->effects != from->effects )
		bits |= U_EFFECTS;

	temp_index = to->modelindex;
	if (((int)ent->v.flags & FL_CLASS_DEPENDENT) && ent->v.model)
	{
		strcpy (NewName, PR_GetString(ent->v.model));
		if (client->playerclass <= 0 || client->playerclass > MAX_PLAYER_CLASS)
		{
			NewName[strlen(NewName)-5] = '1';
		}
		else
		{
			NewName[strlen(NewName)-5] = client->playerclass + 48;
		}
		temp_index = SV_ModelIndex (NewName);
	}

	if (temp_index != from->modelindex )
	{
		bits |= U_MODEL;
		if (temp_index > 255)
		{
			bits |= U_MODEL16;
		}
	}

	if (to->scale != from->scale)
	{
		bits |= U_SCALE;
	}

	if (to->abslight != from->abslight)
	{
		bits |= U_ABSLIGHT;
	}

	if (to->wpn_sound)
	{	//not delta'ed, sound gets cleared after send
		bits |= U_SOUND;
	}

	if (bits & 0xff0000)
		bits |= U_MOREBITS2;

	if (bits & 511)
		bits |= U_MOREBITS;

	//
	// write the message
	//
	if (!to->number)
		SV_Error ("Unset entity number");
	if (to->number >= 512)
		SV_Error ("Entity number >= 512");

	if (!bits && !force)
		return;		// nothing to send!
	i = to->number | (bits & ~511);
	if (i & U_REMOVE)
		Sys_Error ("U_REMOVE");
	MSG_WriteShort (msg, i & 0xffff);

	if (bits & U_MOREBITS)
		MSG_WriteByte (msg, bits & 255);
	if (bits & U_MOREBITS2)
		MSG_WriteByte (msg, (bits >> 16) & 0xff);
	if (bits & U_MODEL)
	{
		if (bits & U_MODEL16)
		{
			MSG_WriteShort (msg, temp_index);
		}
		else
		{
			MSG_WriteByte (msg, temp_index);
		}
	}
	if (bits & U_FRAME)
		MSG_WriteByte (msg, to->frame);
	if (bits & U_COLORMAP)
		MSG_WriteByte (msg, to->colormap);
	if (bits & U_SKIN)
		MSG_WriteByte (msg, to->skinnum);
	if (bits & U_DRAWFLAGS)
		MSG_WriteByte (msg, to->drawflags);
	if (bits & U_EFFECTS)
		MSG_WriteLong (msg, to->effects);
	if (bits & U_ORIGIN1)
		MSG_WriteCoord (msg, to->origin[0]);
	if (bits & U_ANGLE1)
		MSG_WriteAngle(msg, to->angles[0]);
	if (bits & U_ORIGIN2)
		MSG_WriteCoord (msg, to->origin[1]);
	if (bits & U_ANGLE2)
		MSG_WriteAngle(msg, to->angles[1]);
	if (bits & U_ORIGIN3)
		MSG_WriteCoord (msg, to->origin[2]);
	if (bits & U_ANGLE3)
		MSG_WriteAngle(msg, to->angles[2]);
	if (bits & U_SCALE)
		MSG_WriteByte (msg, to->scale);
	if (bits & U_ABSLIGHT)
		MSG_WriteByte (msg, to->abslight);
	if (bits & U_SOUND)
		MSG_WriteShort (msg, to->wpn_sound);
}

/*
=============
SV_EmitPacketEntities

Writes a delta update of a packet_entities_t to the message.
=============
*/
static void SV_EmitPacketEntities (client_t *client, packet_entities_t *to, sizebuf_t *msg)
{
	edict_t	*ent;
	client_frame_t	*fromframe;
	packet_entities_t *from;
	int		oldindex, newindex;
	int		oldnum, newnum;
	int		oldmax;

	// this is the frame that we are going to delta update from
	if (client->delta_sequence != -1)
	{
		fromframe = &client->frames[client->delta_sequence & UPDATE_MASK];
		from = &fromframe->entities;
		oldmax = from->num_entities;

		MSG_WriteByte (msg, svc_deltapacketentities);
		MSG_WriteByte (msg, client->delta_sequence);
	}
	else
	{
		oldmax = 0;	// no delta update
		from = NULL;

		MSG_WriteByte (msg, svc_packetentities);
	}

	newindex = 0;
	oldindex = 0;
//	Con_Printf ("---%i to %i ----\n", client->delta_sequence & UPDATE_MASK,
//				client->netchan.outgoing_sequence & UPDATE_MASK);
	while (newindex < to->num_entities || oldindex < oldmax)
	{
		newnum = newindex >= to->num_entities ? 9999 : to->entities[newindex].number;
		oldnum = oldindex >= oldmax ? 9999 : from->entities[oldindex].number;

		if (newnum == oldnum)
		{	// delta update from old position
		//	Con_Printf ("delta %i\n", newnum);
			SV_WriteDelta (&from->entities[oldindex], &to->entities[newindex], msg, false, EDICT_NUM(newnum), client);
			oldindex++;
			newindex++;
			continue;
		}

		if (newnum < oldnum)
		{	// this is a new entity, send it from the baseline
			ent = EDICT_NUM(newnum);
		//	Con_Printf ("baseline %i\n", newnum);
			SV_WriteDelta (&ent->baseline, &to->entities[newindex], msg, true, ent, client);
			newindex++;
			continue;
		}

		if (newnum > oldnum)
		{	// the old entity isn't present in the new message
		//	Con_Printf ("remove %i\n", oldnum);
			MSG_WriteShort (msg, oldnum | U_REMOVE);
			oldindex++;
			continue;
		}
	}

	MSG_WriteShort (msg, 0);	// end of packetentities
}


void SV_WriteInventory (client_t *host_cl, edict_t *ent, sizebuf_t *msg)
{
	int		sc1, sc2;
	byte	test;

	if (host_cl->send_all_v)
	{
		sc1 = sc2 = 0xffffffff;
		host_cl->send_all_v = false;
	}
	else
	{
		sc1 = sc2 = 0;

		if (ent->v.health != host_cl->old_v.health)
			sc1 |= SC1_HEALTH;
		if (ent->v.level != host_cl->old_v.level)
			sc1 |= SC1_LEVEL;
		if (ent->v.intelligence != host_cl->old_v.intelligence)
			sc1 |= SC1_INTELLIGENCE;
		if (ent->v.wisdom != host_cl->old_v.wisdom)
			sc1 |= SC1_WISDOM;
		if (ent->v.strength != host_cl->old_v.strength)
			sc1 |= SC1_STRENGTH;
		if (ent->v.dexterity != host_cl->old_v.dexterity)
			sc1 |= SC1_DEXTERITY;
		if (ent->v.teleport_time > sv.time)
		{
		//	Con_Printf ("Teleport_time>time, sending bit\n");
			sc1 |= SC1_TELEPORT_TIME;
		//	ent->v.teleport_time = 0;
		}

//		if (ent->v.weapon != host_cl->old_v.weapon)
//			sc1 |= SC1_WEAPON;
		if (ent->v.bluemana != host_cl->old_v.bluemana)
			sc1 |= SC1_BLUEMANA;
		if (ent->v.greenmana != host_cl->old_v.greenmana)
			sc1 |= SC1_GREENMANA;
		if (ent->v.experience != host_cl->old_v.experience)
			sc1 |= SC1_EXPERIENCE;
		if (ent->v.cnt_torch != host_cl->old_v.cnt_torch)
			sc1 |= SC1_CNT_TORCH;
		if (ent->v.cnt_h_boost != host_cl->old_v.cnt_h_boost)
			sc1 |= SC1_CNT_H_BOOST;
		if (ent->v.cnt_sh_boost != host_cl->old_v.cnt_sh_boost)
			sc1 |= SC1_CNT_SH_BOOST;
		if (ent->v.cnt_mana_boost != host_cl->old_v.cnt_mana_boost)
			sc1 |= SC1_CNT_MANA_BOOST;
		if (ent->v.cnt_teleport != host_cl->old_v.cnt_teleport)
			sc1 |= SC1_CNT_TELEPORT;
		if (ent->v.cnt_tome != host_cl->old_v.cnt_tome)
			sc1 |= SC1_CNT_TOME;
		if (ent->v.cnt_summon != host_cl->old_v.cnt_summon)
			sc1 |= SC1_CNT_SUMMON;
		if (ent->v.cnt_invisibility != host_cl->old_v.cnt_invisibility)
			sc1 |= SC1_CNT_INVISIBILITY;
		if (ent->v.cnt_glyph != host_cl->old_v.cnt_glyph)
			sc1 |= SC1_CNT_GLYPH;
		if (ent->v.cnt_haste != host_cl->old_v.cnt_haste)
			sc1 |= SC1_CNT_HASTE;
		if (ent->v.cnt_blast != host_cl->old_v.cnt_blast)
			sc1 |= SC1_CNT_BLAST;
		if (ent->v.cnt_polymorph != host_cl->old_v.cnt_polymorph)
			sc1 |= SC1_CNT_POLYMORPH;
		if (ent->v.cnt_flight != host_cl->old_v.cnt_flight)
			sc1 |= SC1_CNT_FLIGHT;
		if (ent->v.cnt_cubeofforce != host_cl->old_v.cnt_cubeofforce)
			sc1 |= SC1_CNT_CUBEOFFORCE;
		if (ent->v.cnt_invincibility != host_cl->old_v.cnt_invincibility)
			sc1 |= SC1_CNT_INVINCIBILITY;
		if (ent->v.artifact_active != host_cl->old_v.artifact_active)
			sc1 |= SC1_ARTIFACT_ACTIVE;
		if (ent->v.artifact_low != host_cl->old_v.artifact_low)
			sc1 |= SC1_ARTIFACT_LOW;
		if (ent->v.movetype != host_cl->old_v.movetype)
			sc1 |= SC1_MOVETYPE;
		if (ent->v.cameramode != host_cl->old_v.cameramode)
			sc1 |= SC1_CAMERAMODE;
		if (ent->v.hasted != host_cl->old_v.hasted)
			sc1 |= SC1_HASTED;
		if (ent->v.inventory != host_cl->old_v.inventory)
			sc1 |= SC1_INVENTORY;
		if (ent->v.rings_active != host_cl->old_v.rings_active)
			sc1 |= SC1_RINGS_ACTIVE;

		if (ent->v.rings_low != host_cl->old_v.rings_low)
			sc2 |= SC2_RINGS_LOW;
		if (ent->v.armor_amulet != host_cl->old_v.armor_amulet)
			sc2 |= SC2_AMULET;
		if (ent->v.armor_bracer != host_cl->old_v.armor_bracer)
			sc2 |= SC2_BRACER;
		if (ent->v.armor_breastplate != host_cl->old_v.armor_breastplate)
			sc2 |= SC2_BREASTPLATE;
		if (ent->v.armor_helmet != host_cl->old_v.armor_helmet)
			sc2 |= SC2_HELMET;
		if (ent->v.ring_flight != host_cl->old_v.ring_flight)
			sc2 |= SC2_FLIGHT_T;
		if (ent->v.ring_water != host_cl->old_v.ring_water)
			sc2 |= SC2_WATER_T;
		if (ent->v.ring_turning != host_cl->old_v.ring_turning)
			sc2 |= SC2_TURNING_T;
		if (ent->v.ring_regeneration != host_cl->old_v.ring_regeneration)
			sc2 |= SC2_REGEN_T;
//		if (ent->v.haste_time != host_cl->old_v.haste_time)
//			sc2 |= SC2_HASTE_T;
//		if (ent->v.tome_time != host_cl->old_v.tome_time)
//			sc2 |= SC2_TOME_T;
		if (ent->v.puzzle_inv1 != host_cl->old_v.puzzle_inv1)
			sc2 |= SC2_PUZZLE1;
		if (ent->v.puzzle_inv2 != host_cl->old_v.puzzle_inv2)
			sc2 |= SC2_PUZZLE2;
		if (ent->v.puzzle_inv3 != host_cl->old_v.puzzle_inv3)
			sc2 |= SC2_PUZZLE3;
		if (ent->v.puzzle_inv4 != host_cl->old_v.puzzle_inv4)
			sc2 |= SC2_PUZZLE4;
		if (ent->v.puzzle_inv5 != host_cl->old_v.puzzle_inv5)
			sc2 |= SC2_PUZZLE5;
		if (ent->v.puzzle_inv6 != host_cl->old_v.puzzle_inv6)
			sc2 |= SC2_PUZZLE6;
		if (ent->v.puzzle_inv7 != host_cl->old_v.puzzle_inv7)
			sc2 |= SC2_PUZZLE7;
		if (ent->v.puzzle_inv8 != host_cl->old_v.puzzle_inv8)
			sc2 |= SC2_PUZZLE8;
		if (ent->v.max_health != host_cl->old_v.max_health)
			sc2 |= SC2_MAXHEALTH;
		if (ent->v.max_mana != host_cl->old_v.max_mana)
			sc2 |= SC2_MAXMANA;
		if (ent->v.flags != host_cl->old_v.flags)
			sc2 |= SC2_FLAGS;
	}

	if (!sc1 && !sc2)
		goto end;

	MSG_WriteByte (msg, svc_update_inv);
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

	MSG_WriteByte (msg, test);

	if (test & 1)
		MSG_WriteByte (msg, sc1 & 0xff);
	if (test & 2)
		MSG_WriteByte (msg, (sc1 >> 8) & 0xff);
	if (test & 4)
		MSG_WriteByte (msg, (sc1 >> 16) & 0xff);
	if (test & 8)
		MSG_WriteByte (msg, (sc1 >> 24) & 0xff);
	if (test & 16)
		MSG_WriteByte (msg, sc2 & 0xff);
	if (test & 32)
		MSG_WriteByte (msg, (sc2 >> 8) & 0xff);
	if (test & 64)
		MSG_WriteByte (msg, (sc2 >> 16) & 0xff);
	if (test & 128)
		MSG_WriteByte (msg, (sc2 >> 24) & 0xff);

	if (sc1 & SC1_HEALTH)
		MSG_WriteShort (msg, ent->v.health);
	if (sc1 & SC1_LEVEL)
		MSG_WriteByte(msg, ent->v.level);
	if (sc1 & SC1_INTELLIGENCE)
		MSG_WriteByte(msg, ent->v.intelligence);
	if (sc1 & SC1_WISDOM)
		MSG_WriteByte(msg, ent->v.wisdom);
	if (sc1 & SC1_STRENGTH)
		MSG_WriteByte(msg, ent->v.strength);
	if (sc1 & SC1_DEXTERITY)
		MSG_WriteByte(msg, ent->v.dexterity);
//	if (sc1 & SC1_WEAPON)
//		MSG_WriteByte (msg, ent->v.weapon);
	if (sc1 & SC1_BLUEMANA)
		MSG_WriteByte (msg, ent->v.bluemana);
	if (sc1 & SC1_GREENMANA)
		MSG_WriteByte (msg, ent->v.greenmana);
	if (sc1 & SC1_EXPERIENCE)
		MSG_WriteLong (msg, ent->v.experience);
	if (sc1 & SC1_CNT_TORCH)
		MSG_WriteByte (msg, ent->v.cnt_torch);
	if (sc1 & SC1_CNT_H_BOOST)
		MSG_WriteByte (msg, ent->v.cnt_h_boost);
	if (sc1 & SC1_CNT_SH_BOOST)
		MSG_WriteByte (msg, ent->v.cnt_sh_boost);
	if (sc1 & SC1_CNT_MANA_BOOST)
		MSG_WriteByte (msg, ent->v.cnt_mana_boost);
	if (sc1 & SC1_CNT_TELEPORT)
		MSG_WriteByte (msg, ent->v.cnt_teleport);
	if (sc1 & SC1_CNT_TOME)
		MSG_WriteByte (msg, ent->v.cnt_tome);
	if (sc1 & SC1_CNT_SUMMON)
		MSG_WriteByte (msg, ent->v.cnt_summon);
	if (sc1 & SC1_CNT_INVISIBILITY)
		MSG_WriteByte (msg, ent->v.cnt_invisibility);
	if (sc1 & SC1_CNT_GLYPH)
		MSG_WriteByte (msg, ent->v.cnt_glyph);
	if (sc1 & SC1_CNT_HASTE)
		MSG_WriteByte (msg, ent->v.cnt_haste);
	if (sc1 & SC1_CNT_BLAST)
		MSG_WriteByte (msg, ent->v.cnt_blast);
	if (sc1 & SC1_CNT_POLYMORPH)
		MSG_WriteByte (msg, ent->v.cnt_polymorph);
	if (sc1 & SC1_CNT_FLIGHT)
		MSG_WriteByte (msg, ent->v.cnt_flight);
	if (sc1 & SC1_CNT_CUBEOFFORCE)
		MSG_WriteByte (msg, ent->v.cnt_cubeofforce);
	if (sc1 & SC1_CNT_INVINCIBILITY)
		MSG_WriteByte (msg, ent->v.cnt_invincibility);
	if (sc1 & SC1_ARTIFACT_ACTIVE)
		MSG_WriteByte (msg, ent->v.artifact_active);
	if (sc1 & SC1_ARTIFACT_LOW)
		MSG_WriteByte (msg, ent->v.artifact_low);
	if (sc1 & SC1_MOVETYPE)
		MSG_WriteByte (msg, ent->v.movetype);
	if (sc1 & SC1_CAMERAMODE)
		MSG_WriteByte (msg, ent->v.cameramode);
	if (sc1 & SC1_HASTED)
		MSG_WriteFloat (msg, ent->v.hasted);
	if (sc1 & SC1_INVENTORY)
		MSG_WriteByte (msg, ent->v.inventory);
	if (sc1 & SC1_RINGS_ACTIVE)
		MSG_WriteByte (msg, ent->v.rings_active);

	if (sc2 & SC2_RINGS_LOW)
		MSG_WriteByte (msg, ent->v.rings_low);
	if (sc2 & SC2_AMULET)
		MSG_WriteByte(msg, ent->v.armor_amulet);
	if (sc2 & SC2_BRACER)
		MSG_WriteByte(msg, ent->v.armor_bracer);
	if (sc2 & SC2_BREASTPLATE)
		MSG_WriteByte(msg, ent->v.armor_breastplate);
	if (sc2 & SC2_HELMET)
		MSG_WriteByte(msg, ent->v.armor_helmet);
	if (sc2 & SC2_FLIGHT_T)
		MSG_WriteByte(msg, ent->v.ring_flight);
	if (sc2 & SC2_WATER_T)
		MSG_WriteByte(msg, ent->v.ring_water);
	if (sc2 & SC2_TURNING_T)
		MSG_WriteByte(msg, ent->v.ring_turning);
	if (sc2 & SC2_REGEN_T)
		MSG_WriteByte(msg, ent->v.ring_regeneration);
//	if (sc2 & SC2_HASTE_T)
//		MSG_WriteFloat(msg, ent->v.haste_time);
//	if (sc2 & SC2_TOME_T)
//		MSG_WriteFloat(msg, ent->v.tome_time);
	if (sc2 & SC2_PUZZLE1)
		MSG_WriteString(msg, PR_GetString(ent->v.puzzle_inv1));
	if (sc2 & SC2_PUZZLE2)
		MSG_WriteString(msg, PR_GetString(ent->v.puzzle_inv2));
	if (sc2 & SC2_PUZZLE3)
		MSG_WriteString(msg, PR_GetString(ent->v.puzzle_inv3));
	if (sc2 & SC2_PUZZLE4)
		MSG_WriteString(msg, PR_GetString(ent->v.puzzle_inv4));
	if (sc2 & SC2_PUZZLE5)
		MSG_WriteString(msg, PR_GetString(ent->v.puzzle_inv5));
	if (sc2 & SC2_PUZZLE6)
		MSG_WriteString(msg, PR_GetString(ent->v.puzzle_inv6));
	if (sc2 & SC2_PUZZLE7)
		MSG_WriteString(msg, PR_GetString(ent->v.puzzle_inv7));
	if (sc2 & SC2_PUZZLE8)
		MSG_WriteString(msg, PR_GetString(ent->v.puzzle_inv8));
	if (sc2 & SC2_MAXHEALTH)
		MSG_WriteShort(msg, ent->v.max_health);
	if (sc2 & SC2_MAXMANA)
		MSG_WriteByte(msg, ent->v.max_mana);
	if (sc2 & SC2_FLAGS)
		MSG_WriteFloat(msg, ent->v.flags);

end:
	memcpy (&host_cl->old_v, &ent->v, sizeof(host_cl->old_v));
}


#ifdef MGNET
/*
=============
float cardioid_rating (edict_t *targ , edict_t *self)

Determines how important a visclient is- based on offset from
forward angle and distance.  Resultant pattern is a somewhat
extended 3-dimensional cleaved cardioid with each point on
the surface being equal in priority(0) and increasing linearly
towards equal priority(1) along a straight line to the center.
=============
*/
static float cardioid_rating (edict_t *targ , edict_t *self)
{
	vec3_t	vec, spot1, spot2;
	vec3_t	forward, right, up;
	float	dot, dist;

	AngleVectors (self->v.v_angle,forward,right,up);

	VectorAdd(self->v.origin,self->v.view_ofs,spot1);
	VectorSubtract(targ->v.absmax,targ->v.absmin,spot2);
	VectorMA(targ->v.absmin,0.5,spot2,spot2);

	VectorSubtract(spot2,spot1,vec);
	dist = VectorNormalize(vec);
	dot = DotProduct(vec,forward);	//from 1 to -1

	if (dot < -0.3)	//see only from -125 to 125 degrees
		return false;

	if (dot > 0)	//to front of perpendicular plane to forward
		dot *= 31;//much more distance leniency in front, max dist = 2048 directly in front
	dot = (dot + 1) * 64;//64 = base distance if along the perpendicular plane, max is 2048 straight ahead
	if (dist >= dot)//too far away for that angle to be important
		return false;

	//from 0.000000? to almost 1
	return 1 - (dist/dot);//The higher this number is, the more important it is to send this ent
}

#define	MAX_VISCLIENTS	2
/*
=============
SV_WritePlayersToClient

=============
*/
static void SV_WritePlayersToClient (client_t *client, edict_t *clent, byte *pvs, sizebuf_t *msg)
{
	int			i, j;
	client_t	*cl;
	edict_t		*ent;
	int			msec;
	usercmd_t	cmd;
	int			pflags;
	int			invis_level;
	qboolean	playermodel = false;
	// vars for the cardioid_rating/MGNET code
	int			k, l;
	int			visclient[MAX_CLIENTS];
	int			forcevisclient[MAX_CLIENTS];
	int			cl_v_priority[MAX_CLIENTS];
	int			cl_v_psort[MAX_CLIENTS];
	int			numvc, forcevc, totalvc, num_eliminated;

	for (j = 0, cl = svs.clients, numvc = 0, forcevc = 0; j < MAX_CLIENTS; j++, cl++)
	{
		if (cl->state != cs_spawned)
			continue;

		ent = cl->edict;

		// ZOID visibility tracking
		invis_level = false;
		if (ent != clent &&
			!(client->spec_track && client->spec_track - 1 == j))
		{
			if ((int)ent->v.effects & EF_NODRAW)
			{
				if (dmMode.integer == DM_SIEGE && clent->v.playerclass == CLASS_DWARF)
					invis_level = false;
				else
					invis_level = true; //still can hear
			}
			//could be invisiblenow and still sent, cull out by other methods as well
			if (cl->spectator)
			{
				invis_level = 2; //no vis or weaponsound
			}
			else
			{
				// ignore if not touching a PV leaf
				for (i = 0; i < ent->num_leafs; i++)
				{
					if (pvs[ent->leafnums[i] >> 3] & (1 << (ent->leafnums[i] & 7)) )
						break;
				}

				if (i == ent->num_leafs)
					invis_level = 2; //no vis or weaponsound
			}
		}

		if (invis_level == true)
		{	//ok to send weaponsound
			if (ent->v.wpn_sound)
			{
				MSG_WriteByte (msg, svc_player_sound);
				MSG_WriteByte (msg, j);
				for (i = 0; i < 3; i++)
					MSG_WriteCoord (msg, ent->v.origin[i]);
				MSG_WriteShort (msg, ent->v.wpn_sound);
			}
		}
		if (invis_level > 0)
			continue;

		if (!cl->skipsend && ent != clent)
		{	//don't count self
			visclient[numvc]=j;
			numvc++;
		}
		else
		{	//Self, or Wasn't sent last time, must send this frame
			cl->skipsend = false;
			forcevisclient[forcevc] = j;
			forcevc++;
			continue;
		}
	}

	totalvc = numvc + forcevc;
	if (totalvc > MAX_VISCLIENTS)
	{
		// You have more than 5 clients in your view, cull some out
		// prioritize by
		//	line of sight (20%)
		//	distance (50%)
		//	dot off v_forward (30%)
		// put this in "priority" then sort by priority
		// and send the highest priority
		// number of highest priority sent depends on how
		// many are forced through because they were skipped
		// last send.  Ideally, no more than 5 are sent.
		for (j = 0; j < numvc && totalvc > MAX_VISCLIENTS; j++)
		{	//priority 1 - if behind, cull out
			for (k = 0, cl = svs.clients; k < visclient[j]; k++, cl++);
		//	cl = svs.clients + visclient[j];
			ent = cl->edict;
			cl_v_priority[j] = cardioid_rating(ent, clent);
			if (!cl_v_priority[j])
			{	//% they won't be sent, l represents how many
				// were forced through
				cl->skipsend = true;
				totalvc--;
			}
		}

		if (totalvc > MAX_VISCLIENTS)
		{
			//still more than 5 inside cardioid, sort by priority
			//and drop those after 5

			//CHECK this make sure it works

			for (i = 0; i < numvc; i++)
			{	//do this as many times as there are visclients
				for (j = 0; j < numvc-1-i; j++)
				{	//go through the list
					if (cl_v_priority[j] < cl_v_priority[j+1])
					{
						//store lower one
						k = cl_v_psort[j];
						//put next one in it's spot
						cl_v_psort[j] = cl_v_psort[j+1];
						//put lower one next
						cl_v_psort[j+1] = k;
					}
				}
			}

			num_eliminated = 0;

			while (totalvc > MAX_VISCLIENTS)
			{	//eliminate all over 5 unless not sent last time
				if (!cl->skipsend)
				{
					cl = svs.clients + cl_v_psort[numvc - num_eliminated];
					cl->skipsend = true;
					num_eliminated++;
					totalvc--;
				}
			}
		}

		// Alternate Possibilities: ...?
		// priority 2 - if too many numleafs away, cull out
		// priority 3 - don't send those farthest away, flag for re-send next time
		// priority 4 - flat percentage based on how many over 5
		/*	if (rand() % 10 < (numvc + l - 5))
			{//% they won't be sent, l represents how many were forced through
				cl->skipsend = true;
				numvc--;
			}
		*/
		// priority 5 - send less info on clients
	}

	for (j = 0, l = 0, k = 0, cl = svs.clients; j < MAX_CLIENTS; j++, cl++)
	{	//priority 1 - if behind, cull out
		if (forcevisclient[l] == j && l <= forcevc)
			l++;
		else if (visclient[k] == j && k <= numvc)
			k++;	//clent is always forced
		else
			continue;//not in PVS or forced

		if (cl->skipsend)
		{	//still 2 bytes, but what ya gonna do?
			MSG_WriteByte (msg, svc_playerskipped);
			MSG_WriteByte (msg, j);
			continue;
		}

		ent = cl->edict;

		pflags = PF_MSEC | PF_COMMAND;

		if (ent->v.modelindex != sv_playermodel[0] &&//paladin
		    ent->v.modelindex != sv_playermodel[1] &&//crusader
		    ent->v.modelindex != sv_playermodel[2] &&//necro
		    ent->v.modelindex != sv_playermodel[3] &&//assassin
		    ent->v.modelindex != sv_playermodel[4] &&//succ
		    ent->v.modelindex != sv_playermodel[5])//dwarf
			pflags |= PF_MODEL;
		else
			playermodel = true;

		for (i = 0; i < 3; i++)
		{
			if (ent->v.velocity[i])
				pflags |= PF_VELOCITY1<<i;
		}
		if (((long)ent->v.effects & 0xff))
			pflags |= PF_EFFECTS;
		if (((long)ent->v.effects & 0xff00))
			pflags |= PF_EFFECTS2;
		if (ent->v.skin)
		{
			if (dmMode.integer == DM_SIEGE && playermodel && ent->v.skin == 1);
			// in siege, don't send skin if 2nd skin and using
			// playermodel, it will know on other side- saves
			// us 1 byte per client per frame!
			else
				pflags |= PF_SKINNUM;
		}
		if (ent->v.health <= 0)
			pflags |= PF_DEAD;
		if (ent->v.hull == HULL_CROUCH)
			pflags |= PF_CROUCH;

		if (cl->spectator)
		{	// only sent origin and velocity to spectators
			pflags &= PF_VELOCITY1 | PF_VELOCITY2 | PF_VELOCITY3;
		}
		else if (ent == clent)
		{	// don't send a lot of data on personal entity
			pflags &= ~(PF_MSEC|PF_COMMAND);
			if (ent->v.weaponframe)
				pflags |= PF_WEAPONFRAME;
		}
		if (ent->v.drawflags)
		{
			pflags |= PF_DRAWFLAGS;
		}
		if (ent->v.scale != 0 && ent->v.scale != 1.0)
		{
			pflags |= PF_SCALE;
		}
		if (ent->v.abslight != 0)
		{
			pflags |= PF_ABSLIGHT;
		}
		if (ent->v.wpn_sound)
		{
			pflags |= PF_SOUND;
		}

		MSG_WriteByte (msg, svc_playerinfo);
		MSG_WriteByte (msg, j);
		MSG_WriteShort (msg, pflags);

		for (i = 0; i < 3; i++)
			MSG_WriteCoord (msg, ent->v.origin[i]);

		MSG_WriteByte (msg, ent->v.frame);

		if (pflags & PF_MSEC)
		{
			msec = 1000*(sv.time - cl->localtime);
			if (msec > 255)
				msec = 255;
			MSG_WriteByte (msg, msec);
		}

		if (pflags & PF_COMMAND)
		{
			cmd = cl->lastcmd;

			if (ent->v.health <= 0)
			{	// don't show the corpse looking around...
				cmd.angles[0] = 0;
				cmd.angles[1] = ent->v.angles[1];
				cmd.angles[0] = 0;
			}

			cmd.buttons = 0;	// never send buttons
			cmd.impulse = 0;	// never send impulses
			MSG_WriteUsercmd (msg, &cmd, false);
		}

		for (i = 0; i < 3; i++)
		{
			if (pflags & (PF_VELOCITY1<<i) )
				MSG_WriteShort (msg, ent->v.velocity[i]);
		}

		// rjr
		if (pflags & PF_MODEL)
			MSG_WriteShort (msg, ent->v.modelindex);

		if (pflags & PF_SKINNUM)
			MSG_WriteByte (msg, ent->v.skin);

		if (pflags & PF_EFFECTS)
			MSG_WriteByte (msg, ((long)ent->v.effects & 0xff));

		if (pflags & PF_EFFECTS2)
			MSG_WriteByte(msg, ((long)ent->v.effects & 0xff00) >> 8);

		if (pflags & PF_WEAPONFRAME)
			MSG_WriteByte (msg, ent->v.weaponframe);

		if (pflags & PF_DRAWFLAGS)
		{
			MSG_WriteByte (msg, ent->v.drawflags);
		}
		if (pflags & PF_SCALE)
		{
			MSG_WriteByte (msg, (int)(ent->v.scale * 100.0) & 255);
		}
		if (pflags & PF_ABSLIGHT)
		{
			MSG_WriteByte (msg, (int)(ent->v.abslight * 100.0) & 255);
		}
		if (pflags & PF_SOUND)
		{
			MSG_WriteShort (msg, ent->v.wpn_sound);
		}
	}
}

/*	End of MGNET code		*/

#else

/*
=============
SV_WritePlayersToClient

=============
*/
static void SV_WritePlayersToClient (client_t *client, edict_t *clent, byte *pvs, sizebuf_t *msg)
{
	int			i, j;
	client_t	*cl;
	edict_t		*ent;
	int			msec;
	usercmd_t	cmd;
	int			pflags;
	int			invis_level;
	qboolean	playermodel = false;

	for (j = 0, cl = svs.clients; j < MAX_CLIENTS; j++, cl++)
	{
		if (cl->state != cs_spawned)
			continue;

		ent = cl->edict;

		// ZOID visibility tracking
		invis_level = false;
		if (ent != clent &&
			!(client->spec_track && client->spec_track - 1 == j))
		{
			if ((int)ent->v.effects & EF_NODRAW)
			{
				if (dmMode.integer == DM_SIEGE && clent->v.playerclass == CLASS_DWARF)
					invis_level = false;
				else
					invis_level = true; //still can hear
			}
			else if (cl->spectator)
			{
				invis_level = 2; //no vis or weaponsound
			}
			else
			{
				// ignore if not touching a PV leaf
				for (i = 0; i < ent->num_leafs; i++)
				{
					if (pvs[ent->leafnums[i] >> 3] & (1 << (ent->leafnums[i] & 7)) )
						break;
				}

				if (i == ent->num_leafs)
					invis_level = 2; //no vis or weaponsound
			}
		}

		if (invis_level == true)
		{	//ok to send weaponsound
			if (ent->v.wpn_sound)
			{
				MSG_WriteByte (msg, svc_player_sound);
				MSG_WriteByte (msg, j);
				for (i = 0; i < 3; i++)
					MSG_WriteCoord (msg, ent->v.origin[i]);
				MSG_WriteShort (msg, ent->v.wpn_sound);
			}
		}
		if (invis_level > 0)
			continue;

		pflags = PF_MSEC | PF_COMMAND;

		if (ent->v.modelindex != sv_playermodel[0] &&//paladin
		    ent->v.modelindex != sv_playermodel[1] &&//crusader
		    ent->v.modelindex != sv_playermodel[2] &&//necro
		    ent->v.modelindex != sv_playermodel[3] &&//assassin
		    ent->v.modelindex != sv_playermodel[4] &&//succ
		    ent->v.modelindex != sv_playermodel[5])//dwarf
			pflags |= PF_MODEL;
		else
			playermodel = true;

		for (i = 0; i < 3; i++)
		{
			if (ent->v.velocity[i])
				pflags |= PF_VELOCITY1<<i;
		}
		if (((long)ent->v.effects & 0xff))
			pflags |= PF_EFFECTS;
		if (((long)ent->v.effects & 0xff00))
			pflags |= PF_EFFECTS2;
		if (ent->v.skin)
		{
			if (dmMode.integer == DM_SIEGE && playermodel && ent->v.skin == 1);
			// in siege, don't send skin if 2nd skin and using
			// playermodel, it will know on other side- saves
			// us 1 byte per client per frame!
			else
				pflags |= PF_SKINNUM;
		}
		if (ent->v.health <= 0)
			pflags |= PF_DEAD;
		if (ent->v.hull == HULL_CROUCH)
			pflags |= PF_CROUCH;

		if (cl->spectator)
		{	// only sent origin and velocity to spectators
			pflags &= PF_VELOCITY1 | PF_VELOCITY2 | PF_VELOCITY3;
		}
		else if (ent == clent)
		{	// don't send a lot of data on personal entity
			pflags &= ~(PF_MSEC|PF_COMMAND);
			if (ent->v.weaponframe)
				pflags |= PF_WEAPONFRAME;
		}
		if (ent->v.drawflags)
		{
			pflags |= PF_DRAWFLAGS;
		}
		if (ent->v.scale != 0 && ent->v.scale != 1.0)
		{
			pflags |= PF_SCALE;
		}
		if (ent->v.abslight != 0)
		{
			pflags |= PF_ABSLIGHT;
		}
		if (ent->v.wpn_sound)
		{
			pflags |= PF_SOUND;
		}

		MSG_WriteByte (msg, svc_playerinfo);
		MSG_WriteByte (msg, j);
		MSG_WriteShort (msg, pflags);

		for (i = 0; i < 3; i++)
			MSG_WriteCoord (msg, ent->v.origin[i]);

		MSG_WriteByte (msg, ent->v.frame);

		if (pflags & PF_MSEC)
		{
			msec = 1000*(sv.time - cl->localtime);
			if (msec > 255)
				msec = 255;
			MSG_WriteByte (msg, msec);
		}

		if (pflags & PF_COMMAND)
		{
			cmd = cl->lastcmd;

			if (ent->v.health <= 0)
			{	// don't show the corpse looking around...
				cmd.angles[0] = 0;
				cmd.angles[1] = ent->v.angles[1];
				cmd.angles[0] = 0;
			}

			cmd.buttons = 0;	// never send buttons
			cmd.impulse = 0;	// never send impulses
			MSG_WriteUsercmd (msg, &cmd, false);
		}

		for (i = 0; i < 3; i++)
		{
			if (pflags & (PF_VELOCITY1<<i) )
				MSG_WriteShort (msg, ent->v.velocity[i]);
		}

		// rjr
		if (pflags & PF_MODEL)
			MSG_WriteShort (msg, ent->v.modelindex);

		if (pflags & PF_SKINNUM)
			MSG_WriteByte (msg, ent->v.skin);

		if (pflags & PF_EFFECTS)
			MSG_WriteByte (msg, ((long)ent->v.effects & 0xff));

		if (pflags & PF_EFFECTS2)
			MSG_WriteByte(msg, ((long)ent->v.effects & 0xff00) >> 8);

		if (pflags & PF_WEAPONFRAME)
			MSG_WriteByte (msg, ent->v.weaponframe);

		if (pflags & PF_DRAWFLAGS)
		{
			MSG_WriteByte (msg, ent->v.drawflags);
		}
		if (pflags & PF_SCALE)
		{
			MSG_WriteByte (msg, (int)(ent->v.scale * 100.0) & 255);
		}
		if (pflags & PF_ABSLIGHT)
		{
			MSG_WriteByte (msg, (int)(ent->v.abslight * 100.0) & 255);
		}
		if (pflags & PF_SOUND)
		{
			MSG_WriteShort (msg, ent->v.wpn_sound);
		}
	}
}
#endif

/*
=============
SV_WriteEntitiesToClient

Encodes the current state of the world as
a svc_packetentities messages and possibly
a svc_nails message and
svc_playerinfo messages
=============
*/
void SV_WriteEntitiesToClient (client_t *client, sizebuf_t *msg)
{
	int		e, i;
	byte	*pvs;
	vec3_t	org;
	edict_t	*ent;
	packet_entities_t	*pack;
	edict_t	*clent;
	client_frame_t	*frame;
	entity_state_t	*state;

	// this is the frame we are creating
	frame = &client->frames[client->netchan.incoming_sequence & UPDATE_MASK];

	// find the client's PVS
	clent = client->edict;
	VectorAdd (clent->v.origin, clent->v.view_ofs, org);
	pvs = SV_FatPVS (org);

	// send over the players in the PVS
	SV_WritePlayersToClient (client, clent, pvs, msg);

	// put other visible entities into either a packet_entities or a nails message
	pack = &frame->entities;
	pack->num_entities = 0;

//	numnails = 0;
	nummissiles = 0;
	numravens = 0;
	numraven2s = 0;

	for (e = MAX_CLIENTS+1, ent = EDICT_NUM(e); e < sv.num_edicts; e++, ent = NEXT_EDICT(ent))
	{
		// ignore ents without visible models
		if (!ent->v.modelindex || !PR_GetString(ent->v.model)[0])
			continue;

		if ((int)ent->v.effects & EF_NODRAW)
		{
			continue;
		}

		// ignore if not touching a PV leaf
		for (i = 0; i < ent->num_leafs; i++)
		{
			if (pvs[ent->leafnums[i] >> 3] & (1 << (ent->leafnums[i] & 7)) )
				break;
		}

		if (i == ent->num_leafs)
			continue;	// not visible

//		if (SV_AddNailUpdate (ent))
//			continue;
		if (SV_AddMissileUpdate (ent))
			continue;	// added to the special update list

		// add to the packetentities
		if (pack->num_entities == MAX_PACKET_ENTITIES)
			continue;	// all full

		state = &pack->entities[pack->num_entities];
		pack->num_entities++;

		state->number = e;
		state->flags = 0;
		VectorCopy (ent->v.origin, state->origin);
		VectorCopy (ent->v.angles, state->angles);
		state->modelindex = ent->v.modelindex;
		state->frame = ent->v.frame;
		state->colormap = ent->v.colormap;
		state->skinnum = ent->v.skin;
		state->effects = ent->v.effects;
		state->scale = (int)(ent->v.scale * 100.0) & 255;
		state->drawflags = ent->v.drawflags;
		state->abslight = (int)(ent->v.abslight * 255.0) & 255;
		//clear sound so it doesn't send twice
		state->wpn_sound = ent->v.wpn_sound;
	}

	// encode the packet entities as a delta from the
	// last packetentities acknowledged by the client

	SV_EmitPacketEntities (client, pack, msg);

	// now add the specialized nail update
//	SV_EmitNailUpdate (msg);
	SV_EmitPackedEntities (msg);
}

