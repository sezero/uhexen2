/*
 * sv_effect.c -- Client side effects.
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

// HEADER FILES ------------------------------------------------------------

#include "quakedef.h"

// MACROS ------------------------------------------------------------------

// TYPES -------------------------------------------------------------------

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

// PUBLIC FUNCTION PROTOTYPES ----------------------------------------------

// PRIVATE FUNCTION PROTOTYPES ---------------------------------------------

// EXTERNAL DATA DECLARATIONS ----------------------------------------------

extern cvar_t	sv_ce_scale;
extern cvar_t	sv_ce_max_size;

// PUBLIC DATA DEFINITIONS -------------------------------------------------

// PRIVATE DATA DEFINITIONS ------------------------------------------------

// CODE --------------------------------------------------------------------


static void SV_ClearEffects (void)
{
	memset(sv.Effects, 0, sizeof(sv.Effects));
}

// All changes need to be in SV_SendEffect(), SV_ParseEffect(),
// SV_SaveEffects(), SV_LoadEffects(), CL_ParseEffect()
void SV_SendEffect (sizebuf_t *sb, int idx)
{
	qboolean	DoTest;
	vec3_t		TestO1;
//	int		TestDistance; /* not used. (cf. hexen2 version of the function.) */
	int		i;

	if (sv_ce_scale.value > 0)
		DoTest = true;
	else
		DoTest = false;

	VectorClear(TestO1);

	switch (sv.Effects[idx].type)
	{
	case CE_HWSHEEPINATOR:
	case CE_HWXBOWSHOOT:
		VectorCopy(sv.Effects[idx].ef.Xbow.origin[5], TestO1);
	//	TestDistance = 900;
		break;

	case CE_SCARABCHAIN:
		VectorCopy(sv.Effects[idx].ef.Chain.origin, TestO1);
	//	TestDistance = 900;
		break;

	case CE_TRIPMINE:
		VectorCopy(sv.Effects[idx].ef.Chain.origin, TestO1);
	//	DoTest = false;
		break;

	//ACHTUNG!!!!!!! setting DoTest to false here does not insure
	//		 that effect will be sent to everyone!
	case CE_TRIPMINESTILL:
	//	TestDistance = 10000;
		DoTest = false;
		break;

	case CE_RAIN:
	//	TestDistance = 10000;
		DoTest = false;
		break;

	case CE_FOUNTAIN:
	//	TestDistance = 10000;
		DoTest = false;
		break;

	case CE_QUAKE:
		VectorCopy(sv.Effects[idx].ef.Quake.origin, TestO1);
	//	TestDistance = 700;
		break;

	case CE_WHITE_SMOKE:
	case CE_GREEN_SMOKE:
	case CE_GREY_SMOKE:
	case CE_RED_SMOKE:
	case CE_SLOW_WHITE_SMOKE:
	case CE_TELESMK1:
	case CE_TELESMK2:
	case CE_GHOST:
	case CE_REDCLOUD:
	case CE_FLAMESTREAM:
	case CE_ACID_MUZZFL:
	case CE_FLAMEWALL:
	case CE_FLAMEWALL2:
	case CE_ONFIRE:
	case CE_RIPPLE:
		VectorCopy(sv.Effects[idx].ef.Smoke.origin, TestO1);
	//	TestDistance = 250;
		break;

	case CE_SM_WHITE_FLASH:
	case CE_YELLOWRED_FLASH:
	case CE_BLUESPARK:
	case CE_YELLOWSPARK:
	case CE_SM_CIRCLE_EXP:
	case CE_BG_CIRCLE_EXP:
	case CE_SM_EXPLOSION:
	case CE_SM_EXPLOSION2:
	case CE_LG_EXPLOSION:
	case CE_FLOOR_EXPLOSION:
	case CE_BLUE_EXPLOSION:
	case CE_REDSPARK:
	case CE_GREENSPARK:
	case CE_ICEHIT:
	case CE_MEDUSA_HIT:
	case CE_MEZZO_REFLECT:
	case CE_FLOOR_EXPLOSION2:
	case CE_XBOW_EXPLOSION:
	case CE_NEW_EXPLOSION:
	case CE_MAGIC_MISSILE_EXPLOSION:
	case CE_BONE_EXPLOSION:
	case CE_BLDRN_EXPL:
	case CE_ACID_HIT:
	case CE_LBALL_EXPL:
	case CE_FIREWALL_SMALL:
	case CE_FIREWALL_MEDIUM:
	case CE_FIREWALL_LARGE:
	case CE_ACID_SPLAT:
	case CE_ACID_EXPL:
	case CE_FBOOM:
	case CE_BRN_BOUNCE:
	case CE_LSHOCK:
	case CE_BOMB:
	case CE_FLOOR_EXPLOSION3:
		VectorCopy(sv.Effects[idx].ef.Smoke.origin, TestO1);
	//	TestDistance = 250;
		break;

	case CE_WHITE_FLASH:
	case CE_BLUE_FLASH:
	case CE_SM_BLUE_FLASH:
	case CE_HWSPLITFLASH:
	case CE_RED_FLASH:
		VectorCopy(sv.Effects[idx].ef.Smoke.origin, TestO1);
	//	TestDistance = 250;
		break;

	case CE_RIDER_DEATH:
		DoTest = false;
		break;

	case CE_TELEPORTERPUFFS:
		VectorCopy(sv.Effects[idx].ef.Teleporter.origin, TestO1);
	//	TestDistance = 350;
		break;

	case CE_TELEPORTERBODY:
		VectorCopy(sv.Effects[idx].ef.Teleporter.origin, TestO1);
	//	TestDistance = 350;
		break;

	case CE_DEATHBUBBLES:
		if (sv.Effects[idx].ef.Bubble.owner < 0 || sv.Effects[idx].ef.Bubble.owner >= sv.num_edicts)
		{
			return;
		}
		VectorCopy(PROG_TO_EDICT(sv.Effects[idx].ef.Bubble.owner)->v.origin, TestO1);
	//	TestDistance = 400;
		break;

	case CE_HWDRILLA:
	case CE_BONESHARD:
	case CE_BONESHRAPNEL:
	case CE_HWBONEBALL:
	case CE_HWRAVENSTAFF:
	case CE_HWRAVENPOWER:
		VectorCopy(sv.Effects[idx].ef.Missile.origin, TestO1);
	//	TestDistance = 900;
		break;

	case CE_HWMISSILESTAR:
	case CE_HWEIDOLONSTAR:
		VectorCopy(sv.Effects[idx].ef.Missile.origin, TestO1);
	//	TestDistance = 600;
		break;

	default:
		PR_RunError ("%s: bad type", __thisfunc__);
		break;
	}

	MSG_WriteByte (&sv.multicast, svc_start_effect);
	MSG_WriteByte (&sv.multicast, idx);
	MSG_WriteByte (&sv.multicast, sv.Effects[idx].type);

	switch (sv.Effects[idx].type)
	{
	case CE_RAIN:
		MSG_WriteCoord(&sv.multicast, sv.Effects[idx].ef.Rain.min_org[0]);
		MSG_WriteCoord(&sv.multicast, sv.Effects[idx].ef.Rain.min_org[1]);
		MSG_WriteCoord(&sv.multicast, sv.Effects[idx].ef.Rain.min_org[2]);
		MSG_WriteCoord(&sv.multicast, sv.Effects[idx].ef.Rain.max_org[0]);
		MSG_WriteCoord(&sv.multicast, sv.Effects[idx].ef.Rain.max_org[1]);
		MSG_WriteCoord(&sv.multicast, sv.Effects[idx].ef.Rain.max_org[2]);
		MSG_WriteCoord(&sv.multicast, sv.Effects[idx].ef.Rain.e_size[0]);
		MSG_WriteCoord(&sv.multicast, sv.Effects[idx].ef.Rain.e_size[1]);
		MSG_WriteCoord(&sv.multicast, sv.Effects[idx].ef.Rain.e_size[2]);
		MSG_WriteCoord(&sv.multicast, sv.Effects[idx].ef.Rain.dir[0]);
		MSG_WriteCoord(&sv.multicast, sv.Effects[idx].ef.Rain.dir[1]);
		MSG_WriteCoord(&sv.multicast, sv.Effects[idx].ef.Rain.dir[2]);
		MSG_WriteShort(&sv.multicast, sv.Effects[idx].ef.Rain.color);
		MSG_WriteShort(&sv.multicast, sv.Effects[idx].ef.Rain.count);
		MSG_WriteFloat(&sv.multicast, sv.Effects[idx].ef.Rain.wait);
		break;

	case CE_FOUNTAIN:
		MSG_WriteCoord(&sv.multicast, sv.Effects[idx].ef.Fountain.pos[0]);
		MSG_WriteCoord(&sv.multicast, sv.Effects[idx].ef.Fountain.pos[1]);
		MSG_WriteCoord(&sv.multicast, sv.Effects[idx].ef.Fountain.pos[2]);
		MSG_WriteAngle(&sv.multicast, sv.Effects[idx].ef.Fountain.angle[0]);
		MSG_WriteAngle(&sv.multicast, sv.Effects[idx].ef.Fountain.angle[1]);
		MSG_WriteAngle(&sv.multicast, sv.Effects[idx].ef.Fountain.angle[2]);
		MSG_WriteCoord(&sv.multicast, sv.Effects[idx].ef.Fountain.movedir[0]);
		MSG_WriteCoord(&sv.multicast, sv.Effects[idx].ef.Fountain.movedir[1]);
		MSG_WriteCoord(&sv.multicast, sv.Effects[idx].ef.Fountain.movedir[2]);
		MSG_WriteShort(&sv.multicast, sv.Effects[idx].ef.Fountain.color);
		MSG_WriteByte(&sv.multicast, sv.Effects[idx].ef.Fountain.cnt);
		break;

	case CE_QUAKE:
		MSG_WriteCoord(&sv.multicast, sv.Effects[idx].ef.Quake.origin[0]);
		MSG_WriteCoord(&sv.multicast, sv.Effects[idx].ef.Quake.origin[1]);
		MSG_WriteCoord(&sv.multicast, sv.Effects[idx].ef.Quake.origin[2]);
		MSG_WriteFloat(&sv.multicast, sv.Effects[idx].ef.Quake.radius);
		break;

	case CE_WHITE_SMOKE:
	case CE_GREEN_SMOKE:
	case CE_GREY_SMOKE:
	case CE_RED_SMOKE:
	case CE_SLOW_WHITE_SMOKE:
	case CE_TELESMK1:
	case CE_TELESMK2:
	case CE_GHOST:
	case CE_REDCLOUD:
	case CE_FLAMESTREAM:
	case CE_ACID_MUZZFL:
	case CE_FLAMEWALL:
	case CE_FLAMEWALL2:
	case CE_ONFIRE:
	case CE_RIPPLE:
		MSG_WriteCoord(&sv.multicast, sv.Effects[idx].ef.Smoke.origin[0]);
		MSG_WriteCoord(&sv.multicast, sv.Effects[idx].ef.Smoke.origin[1]);
		MSG_WriteCoord(&sv.multicast, sv.Effects[idx].ef.Smoke.origin[2]);
		MSG_WriteFloat(&sv.multicast, sv.Effects[idx].ef.Smoke.velocity[0]);
		MSG_WriteFloat(&sv.multicast, sv.Effects[idx].ef.Smoke.velocity[1]);
		MSG_WriteFloat(&sv.multicast, sv.Effects[idx].ef.Smoke.velocity[2]);
		MSG_WriteFloat(&sv.multicast, sv.Effects[idx].ef.Smoke.framelength);
		break;

	case CE_SM_WHITE_FLASH:
	case CE_YELLOWRED_FLASH:
	case CE_BLUESPARK:
	case CE_YELLOWSPARK:
	case CE_SM_CIRCLE_EXP:
	case CE_BG_CIRCLE_EXP:
	case CE_SM_EXPLOSION:
	case CE_SM_EXPLOSION2:
	case CE_LG_EXPLOSION:
	case CE_FLOOR_EXPLOSION:
	case CE_BLUE_EXPLOSION:
	case CE_REDSPARK:
	case CE_GREENSPARK:
	case CE_ICEHIT:
	case CE_MEDUSA_HIT:
	case CE_MEZZO_REFLECT:
	case CE_FLOOR_EXPLOSION2:
	case CE_XBOW_EXPLOSION:
	case CE_NEW_EXPLOSION:
	case CE_MAGIC_MISSILE_EXPLOSION:
	case CE_BONE_EXPLOSION:
	case CE_BLDRN_EXPL:
	case CE_ACID_HIT:
	case CE_ACID_SPLAT:
	case CE_ACID_EXPL:
	case CE_LBALL_EXPL:
	case CE_FIREWALL_SMALL:
	case CE_FIREWALL_MEDIUM:
	case CE_FIREWALL_LARGE:
	case CE_FBOOM:
	case CE_BOMB:
	case CE_BRN_BOUNCE:
	case CE_LSHOCK:
		MSG_WriteCoord(&sv.multicast, sv.Effects[idx].ef.Smoke.origin[0]);
		MSG_WriteCoord(&sv.multicast, sv.Effects[idx].ef.Smoke.origin[1]);
		MSG_WriteCoord(&sv.multicast, sv.Effects[idx].ef.Smoke.origin[2]);
		break;

	case CE_WHITE_FLASH:
	case CE_BLUE_FLASH:
	case CE_SM_BLUE_FLASH:
	case CE_HWSPLITFLASH:
	case CE_RED_FLASH:
		MSG_WriteCoord(&sv.multicast, sv.Effects[idx].ef.Smoke.origin[0]);
		MSG_WriteCoord(&sv.multicast, sv.Effects[idx].ef.Smoke.origin[1]);
		MSG_WriteCoord(&sv.multicast, sv.Effects[idx].ef.Smoke.origin[2]);
		break;

	case CE_RIDER_DEATH:
		MSG_WriteCoord(&sv.multicast, sv.Effects[idx].ef.RD.origin[0]);
		MSG_WriteCoord(&sv.multicast, sv.Effects[idx].ef.RD.origin[1]);
		MSG_WriteCoord(&sv.multicast, sv.Effects[idx].ef.RD.origin[2]);
		break;

	case CE_TELEPORTERPUFFS:
		MSG_WriteCoord(&sv.multicast, sv.Effects[idx].ef.Teleporter.origin[0]);
		MSG_WriteCoord(&sv.multicast, sv.Effects[idx].ef.Teleporter.origin[1]);
		MSG_WriteCoord(&sv.multicast, sv.Effects[idx].ef.Teleporter.origin[2]);
		break;

	case CE_TELEPORTERBODY:
		MSG_WriteCoord(&sv.multicast, sv.Effects[idx].ef.Teleporter.origin[0]);
		MSG_WriteCoord(&sv.multicast, sv.Effects[idx].ef.Teleporter.origin[1]);
		MSG_WriteCoord(&sv.multicast, sv.Effects[idx].ef.Teleporter.origin[2]);
		MSG_WriteFloat(&sv.multicast, sv.Effects[idx].ef.Teleporter.velocity[0][0]);
		MSG_WriteFloat(&sv.multicast, sv.Effects[idx].ef.Teleporter.velocity[0][1]);
		MSG_WriteFloat(&sv.multicast, sv.Effects[idx].ef.Teleporter.velocity[0][2]);
		MSG_WriteFloat(&sv.multicast, sv.Effects[idx].ef.Teleporter.skinnum);
		break;

	case CE_BONESHRAPNEL:
	case CE_HWBONEBALL:
		MSG_WriteCoord(&sv.multicast, sv.Effects[idx].ef.Missile.origin[0]);
		MSG_WriteCoord(&sv.multicast, sv.Effects[idx].ef.Missile.origin[1]);
		MSG_WriteCoord(&sv.multicast, sv.Effects[idx].ef.Missile.origin[2]);
		MSG_WriteFloat(&sv.multicast, sv.Effects[idx].ef.Missile.velocity[0]);
		MSG_WriteFloat(&sv.multicast, sv.Effects[idx].ef.Missile.velocity[1]);
		MSG_WriteFloat(&sv.multicast, sv.Effects[idx].ef.Missile.velocity[2]);
		MSG_WriteFloat(&sv.multicast, sv.Effects[idx].ef.Missile.angle[0]);
		MSG_WriteFloat(&sv.multicast, sv.Effects[idx].ef.Missile.angle[1]);
		MSG_WriteFloat(&sv.multicast, sv.Effects[idx].ef.Missile.angle[2]);
		MSG_WriteFloat(&sv.multicast, sv.Effects[idx].ef.Missile.avelocity[0]);
		MSG_WriteFloat(&sv.multicast, sv.Effects[idx].ef.Missile.avelocity[1]);
		MSG_WriteFloat(&sv.multicast, sv.Effects[idx].ef.Missile.avelocity[2]);
		break;

	case CE_BONESHARD:
	case CE_HWRAVENSTAFF:
	case CE_HWMISSILESTAR:
	case CE_HWEIDOLONSTAR:
	case CE_HWRAVENPOWER:
		MSG_WriteCoord(&sv.multicast, sv.Effects[idx].ef.Missile.origin[0]);
		MSG_WriteCoord(&sv.multicast, sv.Effects[idx].ef.Missile.origin[1]);
		MSG_WriteCoord(&sv.multicast, sv.Effects[idx].ef.Missile.origin[2]);
		MSG_WriteFloat(&sv.multicast, sv.Effects[idx].ef.Missile.velocity[0]);
		MSG_WriteFloat(&sv.multicast, sv.Effects[idx].ef.Missile.velocity[1]);
		MSG_WriteFloat(&sv.multicast, sv.Effects[idx].ef.Missile.velocity[2]);
		break;

	case CE_HWDRILLA:
		MSG_WriteCoord(&sv.multicast, sv.Effects[idx].ef.Missile.origin[0]);
		MSG_WriteCoord(&sv.multicast, sv.Effects[idx].ef.Missile.origin[1]);
		MSG_WriteCoord(&sv.multicast, sv.Effects[idx].ef.Missile.origin[2]);
		MSG_WriteAngle(&sv.multicast, sv.Effects[idx].ef.Missile.angle[0]);
		MSG_WriteAngle(&sv.multicast, sv.Effects[idx].ef.Missile.angle[1]);
		MSG_WriteShort(&sv.multicast, sv.Effects[idx].ef.Missile.speed);
		break;

	case CE_DEATHBUBBLES:
		MSG_WriteShort(&sv.multicast, sv.Effects[idx].ef.Bubble.owner);
		MSG_WriteByte(&sv.multicast, sv.Effects[idx].ef.Bubble.offset[0]);
		MSG_WriteByte(&sv.multicast, sv.Effects[idx].ef.Bubble.offset[1]);
		MSG_WriteByte(&sv.multicast, sv.Effects[idx].ef.Bubble.offset[2]);
		MSG_WriteByte(&sv.multicast, sv.Effects[idx].ef.Bubble.count);
		break;

	case CE_SCARABCHAIN:
		MSG_WriteCoord(&sv.multicast, sv.Effects[idx].ef.Chain.origin[0]);
		MSG_WriteCoord(&sv.multicast, sv.Effects[idx].ef.Chain.origin[1]);
		MSG_WriteCoord(&sv.multicast, sv.Effects[idx].ef.Chain.origin[2]);
		MSG_WriteShort(&sv.multicast, sv.Effects[idx].ef.Chain.owner + sv.Effects[idx].ef.Chain.material);
		MSG_WriteByte(&sv.multicast, sv.Effects[idx].ef.Chain.tag);
		break;

	case CE_TRIPMINESTILL:
	case CE_TRIPMINE:
		MSG_WriteCoord(&sv.multicast, sv.Effects[idx].ef.Chain.origin[0]);
		MSG_WriteCoord(&sv.multicast, sv.Effects[idx].ef.Chain.origin[1]);
		MSG_WriteCoord(&sv.multicast, sv.Effects[idx].ef.Chain.origin[2]);
		MSG_WriteFloat(&sv.multicast, sv.Effects[idx].ef.Chain.velocity[0]);
		MSG_WriteFloat(&sv.multicast, sv.Effects[idx].ef.Chain.velocity[1]);
		MSG_WriteFloat(&sv.multicast, sv.Effects[idx].ef.Chain.velocity[2]);
		break;

	case CE_HWSHEEPINATOR:
		MSG_WriteCoord(&sv.multicast, sv.Effects[idx].ef.Xbow.origin[5][0]);
		MSG_WriteCoord(&sv.multicast, sv.Effects[idx].ef.Xbow.origin[5][1]);
		MSG_WriteCoord(&sv.multicast, sv.Effects[idx].ef.Xbow.origin[5][2]);
		MSG_WriteAngle(&sv.multicast, sv.Effects[idx].ef.Xbow.angle[0]);
		MSG_WriteAngle(&sv.multicast, sv.Effects[idx].ef.Xbow.angle[1]);
		/* now send the guys that have turned */
		MSG_WriteByte(&sv.multicast, sv.Effects[idx].ef.Xbow.turnedbolts);
		MSG_WriteByte(&sv.multicast, sv.Effects[idx].ef.Xbow.activebolts);
		for (i = 0 ; i < 5 ; i++)
		{
			if ((1<<i) & sv.Effects[idx].ef.Xbow.turnedbolts)
			{
				MSG_WriteCoord(&sv.multicast, sv.Effects[idx].ef.Xbow.origin[i][0]);
				MSG_WriteCoord(&sv.multicast, sv.Effects[idx].ef.Xbow.origin[i][1]);
				MSG_WriteCoord(&sv.multicast, sv.Effects[idx].ef.Xbow.origin[i][2]);
				MSG_WriteAngle(&sv.multicast, sv.Effects[idx].ef.Xbow.vel[i][0]);
				MSG_WriteAngle(&sv.multicast, sv.Effects[idx].ef.Xbow.vel[i][1]);
			}
		}
		break;

	case CE_HWXBOWSHOOT:
		MSG_WriteCoord(&sv.multicast, sv.Effects[idx].ef.Xbow.origin[5][0]);
		MSG_WriteCoord(&sv.multicast, sv.Effects[idx].ef.Xbow.origin[5][1]);
		MSG_WriteCoord(&sv.multicast, sv.Effects[idx].ef.Xbow.origin[5][2]);
		MSG_WriteAngle(&sv.multicast, sv.Effects[idx].ef.Xbow.angle[0]);
		MSG_WriteAngle(&sv.multicast, sv.Effects[idx].ef.Xbow.angle[1]);
	//	MSG_WriteFloat(&sv.multicast, sv.Effects[idx].Xbow.angle[2]);
		MSG_WriteByte(&sv.multicast, sv.Effects[idx].ef.Xbow.bolts);
		MSG_WriteByte(&sv.multicast, sv.Effects[idx].ef.Xbow.randseed);
		/* now send the guys that have turned */
		MSG_WriteByte(&sv.multicast, sv.Effects[idx].ef.Xbow.turnedbolts);
		MSG_WriteByte(&sv.multicast, sv.Effects[idx].ef.Xbow.activebolts);
		for (i = 0 ; i < 5 ; i++)
		{
			if ((1<<i) & sv.Effects[idx].ef.Xbow.turnedbolts)
			{
				MSG_WriteCoord(&sv.multicast, sv.Effects[idx].ef.Xbow.origin[i][0]);
				MSG_WriteCoord(&sv.multicast, sv.Effects[idx].ef.Xbow.origin[i][1]);
				MSG_WriteCoord(&sv.multicast, sv.Effects[idx].ef.Xbow.origin[i][2]);
				MSG_WriteAngle(&sv.multicast, sv.Effects[idx].ef.Xbow.vel[i][0]);
				MSG_WriteAngle(&sv.multicast, sv.Effects[idx].ef.Xbow.vel[i][1]);
			}
		}
		break;

	default:
		PR_RunError ("%s: bad type", __thisfunc__);
		break;
	}

	if (sb)
	{
		SZ_Write (sb, sv.multicast.data, sv.multicast.cursize);
		SZ_Clear (&sv.multicast);
	}
	else
	{
		if (DoTest)
		{
			SV_Multicast (TestO1, MULTICAST_PVS_R);
		}
		else
		{
			SV_Multicast (TestO1, MULTICAST_ALL_R);
		}
		sv.Effects[idx].client_list = clients_multicast;
	}
}

// All changes need to be in SV_SendEffect(), SV_ParseEffect(),
// SV_SaveEffects(), SV_LoadEffects(), CL_ParseEffect()
void SV_ParseEffect (sizebuf_t *sb)
{
	int		idx;
	byte		effect;

	effect = G_FLOAT(OFS_PARM0);

	for (idx = 0 ; idx < MAX_EFFECTS ; idx++)
	{
		if (!sv.Effects[idx].type || 
				(sv.Effects[idx].expire_time && sv.Effects[idx].expire_time <= sv.time))
			break;
	}

	if (idx >= MAX_EFFECTS)
	{
		PR_RunError ("MAX_EFFECTS reached");
		return;
	}

//	Con_Printf("Effect #%d\n", idx);

	memset(&sv.Effects[idx], 0, sizeof(struct EffectT));

	sv.Effects[idx].type = effect;
	G_FLOAT(OFS_RETURN) = idx;

	switch (effect)
	{
	case CE_RAIN:
		VectorCopy(G_VECTOR(OFS_PARM1), sv.Effects[idx].ef.Rain.min_org);
		VectorCopy(G_VECTOR(OFS_PARM2), sv.Effects[idx].ef.Rain.max_org);
		VectorCopy(G_VECTOR(OFS_PARM3), sv.Effects[idx].ef.Rain.e_size);
		VectorCopy(G_VECTOR(OFS_PARM4), sv.Effects[idx].ef.Rain.dir);
		sv.Effects[idx].ef.Rain.color = G_FLOAT(OFS_PARM5);
		sv.Effects[idx].ef.Rain.count = G_FLOAT(OFS_PARM6);
		sv.Effects[idx].ef.Rain.wait = G_FLOAT(OFS_PARM7);
		sv.Effects[idx].ef.Rain.next_time = 0;
		break;

	case CE_FOUNTAIN:
		VectorCopy(G_VECTOR(OFS_PARM1), sv.Effects[idx].ef.Fountain.pos);
		VectorCopy(G_VECTOR(OFS_PARM2), sv.Effects[idx].ef.Fountain.angle);
		VectorCopy(G_VECTOR(OFS_PARM3), sv.Effects[idx].ef.Fountain.movedir);
		sv.Effects[idx].ef.Fountain.color = G_FLOAT(OFS_PARM4);
		sv.Effects[idx].ef.Fountain.cnt = G_FLOAT(OFS_PARM5);
		break;

	case CE_QUAKE:
		VectorCopy(G_VECTOR(OFS_PARM1), sv.Effects[idx].ef.Quake.origin);
		sv.Effects[idx].ef.Quake.radius = G_FLOAT(OFS_PARM2);
		break;

	case CE_WHITE_SMOKE:
	case CE_GREEN_SMOKE:
	case CE_GREY_SMOKE:
	case CE_RED_SMOKE:
	case CE_SLOW_WHITE_SMOKE:
	case CE_TELESMK1:
	case CE_TELESMK2:
	case CE_GHOST:
	case CE_REDCLOUD:
	case CE_RIPPLE:
		VectorCopy(G_VECTOR(OFS_PARM1), sv.Effects[idx].ef.Smoke.origin);
		VectorCopy(G_VECTOR(OFS_PARM2), sv.Effects[idx].ef.Smoke.velocity);
		sv.Effects[idx].ef.Smoke.framelength = G_FLOAT(OFS_PARM3);
		sv.Effects[idx].expire_time = sv.time + 1;
		break;

	case CE_ACID_MUZZFL:
	case CE_FLAMESTREAM:
	case CE_FLAMEWALL:
	case CE_FLAMEWALL2:
	case CE_ONFIRE:
		VectorCopy(G_VECTOR(OFS_PARM1), sv.Effects[idx].ef.Smoke.origin);
		VectorCopy(G_VECTOR(OFS_PARM2), sv.Effects[idx].ef.Smoke.velocity);
		sv.Effects[idx].ef.Smoke.framelength = 0.05;
		sv.Effects[idx].ef.Smoke.frame = G_FLOAT(OFS_PARM3);
		sv.Effects[idx].expire_time = sv.time + 1;
		break;

	case CE_SM_WHITE_FLASH:
	case CE_YELLOWRED_FLASH:
	case CE_BLUESPARK:
	case CE_YELLOWSPARK:
	case CE_SM_CIRCLE_EXP:
	case CE_BG_CIRCLE_EXP:
	case CE_SM_EXPLOSION:
	case CE_SM_EXPLOSION2:
	case CE_LG_EXPLOSION:
	case CE_FLOOR_EXPLOSION:
	case CE_BLUE_EXPLOSION:
	case CE_REDSPARK:
	case CE_GREENSPARK:
	case CE_ICEHIT:
	case CE_MEDUSA_HIT:
	case CE_MEZZO_REFLECT:
	case CE_FLOOR_EXPLOSION2:
	case CE_XBOW_EXPLOSION:
	case CE_NEW_EXPLOSION:
	case CE_MAGIC_MISSILE_EXPLOSION:
	case CE_BONE_EXPLOSION:
	case CE_BLDRN_EXPL:
	case CE_ACID_HIT:
	case CE_ACID_SPLAT:
	case CE_ACID_EXPL:
	case CE_LBALL_EXPL:
	case CE_FIREWALL_SMALL:
	case CE_FIREWALL_MEDIUM:
	case CE_FIREWALL_LARGE:
	case CE_FBOOM:
	case CE_BOMB:
	case CE_BRN_BOUNCE:
	case CE_LSHOCK:
		VectorCopy(G_VECTOR(OFS_PARM1), sv.Effects[idx].ef.Smoke.origin);
		sv.Effects[idx].expire_time = sv.time + 1;
		break;

	case CE_WHITE_FLASH:
	case CE_BLUE_FLASH:
	case CE_SM_BLUE_FLASH:
	case CE_HWSPLITFLASH:
	case CE_RED_FLASH:
		VectorCopy(G_VECTOR(OFS_PARM1), sv.Effects[idx].ef.Flash.origin);
		sv.Effects[idx].expire_time = sv.time + 1;
		break;

	case CE_RIDER_DEATH:
		VectorCopy(G_VECTOR(OFS_PARM1), sv.Effects[idx].ef.RD.origin);
		break;

	case CE_TELEPORTERPUFFS:
		VectorCopy(G_VECTOR(OFS_PARM1), sv.Effects[idx].ef.Teleporter.origin);
		sv.Effects[idx].expire_time = sv.time + 1;
		break;

	case CE_TELEPORTERBODY:
		VectorCopy(G_VECTOR(OFS_PARM1), sv.Effects[idx].ef.Teleporter.origin);
		VectorCopy(G_VECTOR(OFS_PARM2), sv.Effects[idx].ef.Teleporter.velocity[0]);
		sv.Effects[idx].ef.Teleporter.skinnum = G_FLOAT(OFS_PARM3);
		sv.Effects[idx].expire_time = sv.time + 1;
		break;

	case CE_BONESHRAPNEL:
	case CE_HWBONEBALL:
		VectorCopy(G_VECTOR(OFS_PARM1), sv.Effects[idx].ef.Missile.origin);
		VectorCopy(G_VECTOR(OFS_PARM2), sv.Effects[idx].ef.Missile.velocity);
		VectorCopy(G_VECTOR(OFS_PARM3), sv.Effects[idx].ef.Missile.angle);
		VectorCopy(G_VECTOR(OFS_PARM2), sv.Effects[idx].ef.Missile.avelocity);
		sv.Effects[idx].expire_time = sv.time + 10;
		break;

	case CE_BONESHARD:
	case CE_HWRAVENSTAFF:
	case CE_HWMISSILESTAR:
	case CE_HWEIDOLONSTAR:
	case CE_HWRAVENPOWER:
		VectorCopy(G_VECTOR(OFS_PARM1), sv.Effects[idx].ef.Missile.origin);
		VectorCopy(G_VECTOR(OFS_PARM2), sv.Effects[idx].ef.Missile.velocity);
		sv.Effects[idx].expire_time = sv.time + 10;
		break;

	case CE_DEATHBUBBLES:
		VectorCopy(G_VECTOR(OFS_PARM2), sv.Effects[idx].ef.Bubble.offset);
		sv.Effects[idx].ef.Bubble.owner = G_EDICTNUM(OFS_PARM1);
		sv.Effects[idx].ef.Bubble.count = G_FLOAT(OFS_PARM3);
		sv.Effects[idx].expire_time = sv.time + 30;
		break;

	case CE_HWDRILLA:
		VectorCopy(G_VECTOR(OFS_PARM1), sv.Effects[idx].ef.Missile.origin);
		VectorCopy(G_VECTOR(OFS_PARM2), sv.Effects[idx].ef.Missile.angle);
		sv.Effects[idx].ef.Missile.speed = G_FLOAT(OFS_PARM3);
		sv.Effects[idx].expire_time = sv.time + 10;
		break;

	case CE_TRIPMINESTILL:
		VectorCopy(G_VECTOR(OFS_PARM1), sv.Effects[idx].ef.Chain.origin);
		VectorCopy(G_VECTOR(OFS_PARM2), sv.Effects[idx].ef.Chain.velocity);
		sv.Effects[idx].expire_time = sv.time + 70;
		break;

	case CE_TRIPMINE:
		VectorCopy(G_VECTOR(OFS_PARM1), sv.Effects[idx].ef.Chain.origin);
		VectorCopy(G_VECTOR(OFS_PARM2), sv.Effects[idx].ef.Chain.velocity);
		sv.Effects[idx].expire_time = sv.time + 10;
		break;

	case CE_SCARABCHAIN:
		VectorCopy(G_VECTOR(OFS_PARM1), sv.Effects[idx].ef.Chain.origin);
		sv.Effects[idx].ef.Chain.owner = G_EDICTNUM(OFS_PARM2);
		sv.Effects[idx].ef.Chain.material = G_INT(OFS_PARM3);
		sv.Effects[idx].ef.Chain.tag = G_INT(OFS_PARM4);
		sv.Effects[idx].ef.Chain.state = 0;
		sv.Effects[idx].expire_time = sv.time + 15;
		break;

	case CE_HWSHEEPINATOR:
		VectorCopy(G_VECTOR(OFS_PARM1), sv.Effects[idx].ef.Xbow.origin[0]);
		VectorCopy(G_VECTOR(OFS_PARM1), sv.Effects[idx].ef.Xbow.origin[1]);
		VectorCopy(G_VECTOR(OFS_PARM1), sv.Effects[idx].ef.Xbow.origin[2]);
		VectorCopy(G_VECTOR(OFS_PARM1), sv.Effects[idx].ef.Xbow.origin[3]);
		VectorCopy(G_VECTOR(OFS_PARM1), sv.Effects[idx].ef.Xbow.origin[4]);
		VectorCopy(G_VECTOR(OFS_PARM1), sv.Effects[idx].ef.Xbow.origin[5]);
		VectorCopy(G_VECTOR(OFS_PARM2), sv.Effects[idx].ef.Xbow.angle);
		sv.Effects[idx].ef.Xbow.bolts = 5;
		sv.Effects[idx].ef.Xbow.activebolts = 31;
		sv.Effects[idx].ef.Xbow.randseed = 0;
		sv.Effects[idx].ef.Xbow.turnedbolts = 0;
		sv.Effects[idx].expire_time = sv.time + 7;
		break;

	case CE_HWXBOWSHOOT:
		VectorCopy(G_VECTOR(OFS_PARM1), sv.Effects[idx].ef.Xbow.origin[0]);
		VectorCopy(G_VECTOR(OFS_PARM1), sv.Effects[idx].ef.Xbow.origin[1]);
		VectorCopy(G_VECTOR(OFS_PARM1), sv.Effects[idx].ef.Xbow.origin[2]);
		VectorCopy(G_VECTOR(OFS_PARM1), sv.Effects[idx].ef.Xbow.origin[3]);
		VectorCopy(G_VECTOR(OFS_PARM1), sv.Effects[idx].ef.Xbow.origin[4]);
		VectorCopy(G_VECTOR(OFS_PARM1), sv.Effects[idx].ef.Xbow.origin[5]);
		VectorCopy(G_VECTOR(OFS_PARM2), sv.Effects[idx].ef.Xbow.angle);
		sv.Effects[idx].ef.Xbow.bolts = G_FLOAT(OFS_PARM3);
		sv.Effects[idx].ef.Xbow.randseed = G_FLOAT(OFS_PARM4);
		sv.Effects[idx].ef.Xbow.turnedbolts = 0;
		if (sv.Effects[idx].ef.Xbow.bolts == 3)
		{
			sv.Effects[idx].ef.Xbow.activebolts = 7;
		}
		else
		{
			sv.Effects[idx].ef.Xbow.activebolts = 31;
		}
		sv.Effects[idx].expire_time = sv.time + 15;
		break;

	default:
		PR_RunError ("%s: bad type", __thisfunc__);
	}

	SV_SendEffect(sb, idx);
}


/* this random generator can have its effects duplicated on the client
 * side by passing the randomseed over the network, as opposed to sending
 * all the generated values
 */
static unsigned int randomseed;

void SV_setseed (int seed)
{
	randomseed = seed;
}

float SV_seedrand (void)
{
	randomseed = (randomseed * 877 + 573) % 9968;
	return (float)randomseed / 9968;
}


/* this will create several effects and store the ids in the array */
static float MultiEffectIds[10];
static int MultiEffectIdCount;

void SV_ParseMultiEffect (sizebuf_t *sb)
{
	int	idx, count;
	byte	effect;
	vec3_t	orig, vel;

	MultiEffectIdCount = 0;
	effect = G_FLOAT(OFS_PARM0);
	switch (effect)
	{
	case CE_HWRAVENPOWER:
		/* need to set aside 3 effect ids */
		MSG_WriteByte (sb, svc_multieffect);
		MSG_WriteByte (sb, effect);

		VectorCopy(G_VECTOR(OFS_PARM1), orig);
		MSG_WriteCoord(sb, orig[0]);
		MSG_WriteCoord(sb, orig[1]);
		MSG_WriteCoord(sb, orig[2]);
		VectorCopy(G_VECTOR(OFS_PARM2), vel);
		MSG_WriteCoord(sb, vel[0]);
		MSG_WriteCoord(sb, vel[1]);
		MSG_WriteCoord(sb, vel[2]);
		for (count = 0 ; count < 3 ; count++)
		{
			for (idx = 0 ; idx < MAX_EFFECTS ; idx++)
			{
				if (!sv.Effects[idx].type || 
						(sv.Effects[idx].expire_time && sv.Effects[idx].expire_time <= sv.time))
					break;
			}
			if (idx >= MAX_EFFECTS)
			{
				PR_RunError ("MAX_EFFECTS reached");
				return;
			}
			MSG_WriteByte(sb, idx);
			sv.Effects[idx].type = CE_HWRAVENPOWER;
			VectorCopy(orig, sv.Effects[idx].ef.Missile.origin);
			VectorCopy(vel, sv.Effects[idx].ef.Missile.velocity);
			sv.Effects[idx].expire_time = sv.time + 10;
			MultiEffectIds[count] = idx;
		}
		break;

	default:
		PR_RunError ("%s: bad type", __thisfunc__);
	}
}

float SV_GetMultiEffectId (void)
{
	MultiEffectIdCount++;
	return MultiEffectIds[MultiEffectIdCount-1];
}


/* saving and loading games not supported in hexenworld yet. */

// All changes need to be in SV_SendEffect(), SV_ParseEffect(),
// SV_SaveEffects(), SV_LoadEffects(), CL_ParseEffect()
void SV_SaveEffects (FILE *FH)
{
	int	idx, count;

	for (idx = count = 0 ; idx < MAX_EFFECTS ; idx++)
	{
		if (sv.Effects[idx].type)
			count++;
	}

	fprintf(FH, "Effects: %d\n", count);

	for (idx = count = 0 ; idx < MAX_EFFECTS ; idx++)
	{
		if (!sv.Effects[idx].type)
			continue;

		fprintf(FH, "Effect: %d %d %f: ", idx, sv.Effects[idx].type, sv.Effects[idx].expire_time);

		switch (sv.Effects[idx].type)
		{
		case CE_RAIN:
			fprintf(FH, "%f ", sv.Effects[idx].ef.Rain.min_org[0]);
			fprintf(FH, "%f ", sv.Effects[idx].ef.Rain.min_org[1]);
			fprintf(FH, "%f ", sv.Effects[idx].ef.Rain.min_org[2]);
			fprintf(FH, "%f ", sv.Effects[idx].ef.Rain.max_org[0]);
			fprintf(FH, "%f ", sv.Effects[idx].ef.Rain.max_org[1]);
			fprintf(FH, "%f ", sv.Effects[idx].ef.Rain.max_org[2]);
			fprintf(FH, "%f ", sv.Effects[idx].ef.Rain.e_size[0]);
			fprintf(FH, "%f ", sv.Effects[idx].ef.Rain.e_size[1]);
			fprintf(FH, "%f ", sv.Effects[idx].ef.Rain.e_size[2]);
			fprintf(FH, "%f ", sv.Effects[idx].ef.Rain.dir[0]);
			fprintf(FH, "%f ", sv.Effects[idx].ef.Rain.dir[1]);
			fprintf(FH, "%f ", sv.Effects[idx].ef.Rain.dir[2]);
			fprintf(FH, "%d ", sv.Effects[idx].ef.Rain.color);
			fprintf(FH, "%d ", sv.Effects[idx].ef.Rain.count);
			fprintf(FH, "%f\n", sv.Effects[idx].ef.Rain.wait);
			break;

		case CE_FOUNTAIN:
			fprintf(FH, "%f ", sv.Effects[idx].ef.Fountain.pos[0]);
			fprintf(FH, "%f ", sv.Effects[idx].ef.Fountain.pos[1]);
			fprintf(FH, "%f ", sv.Effects[idx].ef.Fountain.pos[2]);
			fprintf(FH, "%f ", sv.Effects[idx].ef.Fountain.angle[0]);
			fprintf(FH, "%f ", sv.Effects[idx].ef.Fountain.angle[1]);
			fprintf(FH, "%f ", sv.Effects[idx].ef.Fountain.angle[2]);
			fprintf(FH, "%f ", sv.Effects[idx].ef.Fountain.movedir[0]);
			fprintf(FH, "%f ", sv.Effects[idx].ef.Fountain.movedir[1]);
			fprintf(FH, "%f ", sv.Effects[idx].ef.Fountain.movedir[2]);
			fprintf(FH, "%d ", sv.Effects[idx].ef.Fountain.color);
			fprintf(FH, "%d\n", sv.Effects[idx].ef.Fountain.cnt);
			break;

		case CE_QUAKE:
			fprintf(FH, "%f ", sv.Effects[idx].ef.Quake.origin[0]);
			fprintf(FH, "%f ", sv.Effects[idx].ef.Quake.origin[1]);
			fprintf(FH, "%f ", sv.Effects[idx].ef.Quake.origin[2]);
			fprintf(FH, "%f\n", sv.Effects[idx].ef.Quake.radius);
			break;

		case CE_WHITE_SMOKE:
		case CE_GREEN_SMOKE:
		case CE_GREY_SMOKE:
		case CE_RED_SMOKE:
		case CE_SLOW_WHITE_SMOKE:
		case CE_TELESMK1:
		case CE_TELESMK2:
		case CE_GHOST:
		case CE_REDCLOUD:
		case CE_ACID_MUZZFL:
		case CE_FLAMESTREAM:
		case CE_FLAMEWALL:
		case CE_FLAMEWALL2:
		case CE_ONFIRE:
		case CE_RIPPLE:
			fprintf(FH, "%f ", sv.Effects[idx].ef.Smoke.origin[0]);
			fprintf(FH, "%f ", sv.Effects[idx].ef.Smoke.origin[1]);
			fprintf(FH, "%f ", sv.Effects[idx].ef.Smoke.origin[2]);
			fprintf(FH, "%f ", sv.Effects[idx].ef.Smoke.velocity[0]);
			fprintf(FH, "%f ", sv.Effects[idx].ef.Smoke.velocity[1]);
			fprintf(FH, "%f ", sv.Effects[idx].ef.Smoke.velocity[2]);
			fprintf(FH, "%f\n", sv.Effects[idx].ef.Smoke.framelength);
			break;

		case CE_SM_WHITE_FLASH:
		case CE_YELLOWRED_FLASH:
		case CE_BLUESPARK:
		case CE_YELLOWSPARK:
		case CE_SM_CIRCLE_EXP:
		case CE_BG_CIRCLE_EXP:
		case CE_SM_EXPLOSION:
		case CE_SM_EXPLOSION2:
		case CE_LG_EXPLOSION:
		case CE_FLOOR_EXPLOSION:
		case CE_BLUE_EXPLOSION:
		case CE_REDSPARK:
		case CE_GREENSPARK:
		case CE_ICEHIT:
		case CE_MEDUSA_HIT:
		case CE_MEZZO_REFLECT:
		case CE_FLOOR_EXPLOSION2:
		case CE_XBOW_EXPLOSION:
		case CE_NEW_EXPLOSION:
		case CE_MAGIC_MISSILE_EXPLOSION:
		case CE_BONE_EXPLOSION:
		case CE_BLDRN_EXPL:
		case CE_BRN_BOUNCE:
		case CE_LSHOCK:
		case CE_ACID_HIT:
		case CE_ACID_SPLAT:
		case CE_ACID_EXPL:
		case CE_LBALL_EXPL:
		case CE_FIREWALL_SMALL:
		case CE_FIREWALL_MEDIUM:
		case CE_FIREWALL_LARGE:
		case CE_FBOOM:
		case CE_BOMB:
			fprintf(FH, "%f ", sv.Effects[idx].ef.Smoke.origin[0]);
			fprintf(FH, "%f ", sv.Effects[idx].ef.Smoke.origin[1]);
			fprintf(FH, "%f\n", sv.Effects[idx].ef.Smoke.origin[2]);
			break;

		case CE_WHITE_FLASH:
		case CE_BLUE_FLASH:
		case CE_SM_BLUE_FLASH:
		case CE_HWSPLITFLASH:
		case CE_RED_FLASH:
			fprintf(FH, "%f ", sv.Effects[idx].ef.Flash.origin[0]);
			fprintf(FH, "%f ", sv.Effects[idx].ef.Flash.origin[1]);
			fprintf(FH, "%f\n", sv.Effects[idx].ef.Flash.origin[2]);
			break;

		case CE_RIDER_DEATH:
			fprintf(FH, "%f ", sv.Effects[idx].ef.RD.origin[0]);
			fprintf(FH, "%f ", sv.Effects[idx].ef.RD.origin[1]);
			fprintf(FH, "%f\n", sv.Effects[idx].ef.RD.origin[2]);
			break;

		case CE_TELEPORTERPUFFS:
			fprintf(FH, "%f ", sv.Effects[idx].ef.Teleporter.origin[0]);
			fprintf(FH, "%f ", sv.Effects[idx].ef.Teleporter.origin[1]);
			fprintf(FH, "%f\n", sv.Effects[idx].ef.Teleporter.origin[2]);
			break;

		case CE_TELEPORTERBODY:
			fprintf(FH, "%f ", sv.Effects[idx].ef.Teleporter.origin[0]);
			fprintf(FH, "%f ", sv.Effects[idx].ef.Teleporter.origin[1]);
			fprintf(FH, "%f\n", sv.Effects[idx].ef.Teleporter.origin[2]);
			break;

		case CE_BONESHRAPNEL:
		case CE_HWBONEBALL:
			fprintf(FH, "%f ", sv.Effects[idx].ef.Missile.origin[0]);
			fprintf(FH, "%f ", sv.Effects[idx].ef.Missile.origin[1]);
			fprintf(FH, "%f ", sv.Effects[idx].ef.Missile.origin[2]);
			fprintf(FH, "%f ", sv.Effects[idx].ef.Missile.velocity[0]);
			fprintf(FH, "%f ", sv.Effects[idx].ef.Missile.velocity[1]);
			fprintf(FH, "%f ", sv.Effects[idx].ef.Missile.velocity[2]);
			fprintf(FH, "%f ", sv.Effects[idx].ef.Missile.angle[0]);
			fprintf(FH, "%f ", sv.Effects[idx].ef.Missile.angle[1]);
			fprintf(FH, "%f ", sv.Effects[idx].ef.Missile.angle[2]);
			fprintf(FH, "%f ", sv.Effects[idx].ef.Missile.avelocity[0]);
			fprintf(FH, "%f ", sv.Effects[idx].ef.Missile.avelocity[1]);
			fprintf(FH, "%f ", sv.Effects[idx].ef.Missile.avelocity[2]);
			break;

		case CE_BONESHARD:
		case CE_HWRAVENSTAFF:
		case CE_HWRAVENPOWER:
		case CE_HWMISSILESTAR:
		case CE_HWEIDOLONSTAR:
			fprintf(FH, "%f ", sv.Effects[idx].ef.Missile.origin[0]);
			fprintf(FH, "%f ", sv.Effects[idx].ef.Missile.origin[1]);
			fprintf(FH, "%f ", sv.Effects[idx].ef.Missile.origin[2]);
			fprintf(FH, "%f ", sv.Effects[idx].ef.Missile.velocity[0]);
			fprintf(FH, "%f ", sv.Effects[idx].ef.Missile.velocity[1]);
			fprintf(FH, "%f ", sv.Effects[idx].ef.Missile.velocity[2]);
			break;

		case CE_DEATHBUBBLES:
			fprintf(FH, "%f ", sv.Effects[idx].ef.Bubble.offset[0]);
			fprintf(FH, "%f ", sv.Effects[idx].ef.Bubble.offset[1]);
			fprintf(FH, "%f ", sv.Effects[idx].ef.Bubble.offset[2]);
			fprintf(FH, "%d ", sv.Effects[idx].ef.Bubble.owner);
			fprintf(FH, "%d ", sv.Effects[idx].ef.Bubble.count);
			break;

		case CE_HWDRILLA:
			fprintf(FH, "%f ", sv.Effects[idx].ef.Missile.origin[0]);
			fprintf(FH, "%f ", sv.Effects[idx].ef.Missile.origin[1]);
			fprintf(FH, "%f ", sv.Effects[idx].ef.Missile.origin[2]);
			fprintf(FH, "%f ", sv.Effects[idx].ef.Missile.angle[0]);
			fprintf(FH, "%f ", sv.Effects[idx].ef.Missile.angle[1]);
			fprintf(FH, "%f ", sv.Effects[idx].ef.Missile.angle[2]);
			fprintf(FH, "%f ", sv.Effects[idx].ef.Missile.speed);
			break;

		case CE_SCARABCHAIN:
			fprintf(FH, "%f ", sv.Effects[idx].ef.Chain.origin[0]);
			fprintf(FH, "%f ", sv.Effects[idx].ef.Chain.origin[1]);
			fprintf(FH, "%f ", sv.Effects[idx].ef.Chain.origin[2]);
			fprintf(FH, "%d ", sv.Effects[idx].ef.Chain.owner);
			fprintf(FH, "%d ", sv.Effects[idx].ef.Chain.material);
			fprintf(FH, "%d ", sv.Effects[idx].ef.Chain.tag);
			break;

		case CE_HWSHEEPINATOR:
		case CE_HWXBOWSHOOT:
			fprintf(FH, "%f ", sv.Effects[idx].ef.Xbow.origin[5][0]);
			fprintf(FH, "%f ", sv.Effects[idx].ef.Xbow.origin[5][1]);
			fprintf(FH, "%f ", sv.Effects[idx].ef.Xbow.origin[5][2]);
			fprintf(FH, "%f ", sv.Effects[idx].ef.Xbow.angle[0]);
			fprintf(FH, "%f ", sv.Effects[idx].ef.Xbow.angle[1]);
			fprintf(FH, "%f ", sv.Effects[idx].ef.Xbow.angle[2]);
			fprintf(FH, "%d ", sv.Effects[idx].ef.Xbow.bolts);
			fprintf(FH, "%d ", sv.Effects[idx].ef.Xbow.activebolts);
			fprintf(FH, "%d ", sv.Effects[idx].ef.Xbow.turnedbolts);
			break;

		case CE_TRIPMINESTILL:
		case CE_TRIPMINE:
			fprintf(FH, "%f ", sv.Effects[idx].ef.Chain.origin[0]);
			fprintf(FH, "%f ", sv.Effects[idx].ef.Chain.origin[1]);
			fprintf(FH, "%f ", sv.Effects[idx].ef.Chain.origin[2]);
			fprintf(FH, "%f ", sv.Effects[idx].ef.Chain.velocity[0]);
			fprintf(FH, "%f ", sv.Effects[idx].ef.Chain.velocity[1]);
			fprintf(FH, "%f ", sv.Effects[idx].ef.Chain.velocity[2]);
			break;

		default:
			Host_Error ("%s: bad type", __thisfunc__);
			break;
		}
	}
}

// All changes need to be in SV_SendEffect(), SV_ParseEffect(),
// SV_SaveEffects(), SV_LoadEffects(), CL_ParseEffect()
void SV_LoadEffects (FILE *FH)
{
	int		idx, Total, count;

	Total = idx = -1;
	/* Since the map is freshly loaded, clear out any effects as a result of
	   the loading */
	SV_ClearEffects();

	fscanf(FH, "Effects: %d\n", &Total);
	if (Total < 0 || Total > MAX_EFFECTS)
		Host_Error ("%s: bad numeffects", __thisfunc__);

	for (count = 0 ; count < Total ; idx = -1, count++)
	{
		fscanf(FH, "Effect: %d ", &idx);
		if (idx < 0 || idx >= MAX_EFFECTS)
			Host_Error ("%s: bad index", __thisfunc__);
		fscanf(FH, "%d %f: ", &sv.Effects[idx].type, &sv.Effects[idx].expire_time);

		switch (sv.Effects[idx].type)
		{
		case CE_RAIN:
			fscanf(FH, "%f ", &sv.Effects[idx].ef.Rain.min_org[0]);
			fscanf(FH, "%f ", &sv.Effects[idx].ef.Rain.min_org[1]);
			fscanf(FH, "%f ", &sv.Effects[idx].ef.Rain.min_org[2]);
			fscanf(FH, "%f ", &sv.Effects[idx].ef.Rain.max_org[0]);
			fscanf(FH, "%f ", &sv.Effects[idx].ef.Rain.max_org[1]);
			fscanf(FH, "%f ", &sv.Effects[idx].ef.Rain.max_org[2]);
			fscanf(FH, "%f ", &sv.Effects[idx].ef.Rain.e_size[0]);
			fscanf(FH, "%f ", &sv.Effects[idx].ef.Rain.e_size[1]);
			fscanf(FH, "%f ", &sv.Effects[idx].ef.Rain.e_size[2]);
			fscanf(FH, "%f ", &sv.Effects[idx].ef.Rain.dir[0]);
			fscanf(FH, "%f ", &sv.Effects[idx].ef.Rain.dir[1]);
			fscanf(FH, "%f ", &sv.Effects[idx].ef.Rain.dir[2]);
			fscanf(FH, "%d ", &sv.Effects[idx].ef.Rain.color);
			fscanf(FH, "%d ", &sv.Effects[idx].ef.Rain.count);
			fscanf(FH, "%f\n", &sv.Effects[idx].ef.Rain.wait);
			break;

		case CE_FOUNTAIN:
			fscanf(FH, "%f ", &sv.Effects[idx].ef.Fountain.pos[0]);
			fscanf(FH, "%f ", &sv.Effects[idx].ef.Fountain.pos[1]);
			fscanf(FH, "%f ", &sv.Effects[idx].ef.Fountain.pos[2]);
			fscanf(FH, "%f ", &sv.Effects[idx].ef.Fountain.angle[0]);
			fscanf(FH, "%f ", &sv.Effects[idx].ef.Fountain.angle[1]);
			fscanf(FH, "%f ", &sv.Effects[idx].ef.Fountain.angle[2]);
			fscanf(FH, "%f ", &sv.Effects[idx].ef.Fountain.movedir[0]);
			fscanf(FH, "%f ", &sv.Effects[idx].ef.Fountain.movedir[1]);
			fscanf(FH, "%f ", &sv.Effects[idx].ef.Fountain.movedir[2]);
			fscanf(FH, "%d ", &sv.Effects[idx].ef.Fountain.color);
			fscanf(FH, "%d\n", &sv.Effects[idx].ef.Fountain.cnt);
			break;

		case CE_QUAKE:
			fscanf(FH, "%f ", &sv.Effects[idx].ef.Quake.origin[0]);
			fscanf(FH, "%f ", &sv.Effects[idx].ef.Quake.origin[1]);
			fscanf(FH, "%f ", &sv.Effects[idx].ef.Quake.origin[2]);
			fscanf(FH, "%f\n", &sv.Effects[idx].ef.Quake.radius);
			break;

		case CE_WHITE_SMOKE:
		case CE_GREEN_SMOKE:
		case CE_GREY_SMOKE:
		case CE_RED_SMOKE:
		case CE_SLOW_WHITE_SMOKE:
		case CE_TELESMK1:
		case CE_TELESMK2:
		case CE_GHOST:
		case CE_REDCLOUD:
		case CE_ACID_MUZZFL:
		case CE_FLAMESTREAM:
		case CE_FLAMEWALL:
		case CE_FLAMEWALL2:
		case CE_ONFIRE:
		case CE_RIPPLE:
			fscanf(FH, "%f ", &sv.Effects[idx].ef.Smoke.origin[0]);
			fscanf(FH, "%f ", &sv.Effects[idx].ef.Smoke.origin[1]);
			fscanf(FH, "%f ", &sv.Effects[idx].ef.Smoke.origin[2]);
			fscanf(FH, "%f ", &sv.Effects[idx].ef.Smoke.velocity[0]);
			fscanf(FH, "%f ", &sv.Effects[idx].ef.Smoke.velocity[1]);
			fscanf(FH, "%f ", &sv.Effects[idx].ef.Smoke.velocity[2]);
			fscanf(FH, "%f\n", &sv.Effects[idx].ef.Smoke.framelength);
			break;

		case CE_SM_WHITE_FLASH:
		case CE_YELLOWRED_FLASH:
		case CE_BLUESPARK:
		case CE_YELLOWSPARK:
		case CE_SM_CIRCLE_EXP:
		case CE_BG_CIRCLE_EXP:
		case CE_SM_EXPLOSION:
		case CE_SM_EXPLOSION2:
		case CE_LG_EXPLOSION:
		case CE_FLOOR_EXPLOSION:
		case CE_BLUE_EXPLOSION:
		case CE_REDSPARK:
		case CE_GREENSPARK:
		case CE_ICEHIT:
		case CE_MEDUSA_HIT:
		case CE_MEZZO_REFLECT:
		case CE_FLOOR_EXPLOSION2:
		case CE_XBOW_EXPLOSION:
		case CE_NEW_EXPLOSION:
		case CE_MAGIC_MISSILE_EXPLOSION:
		case CE_BONE_EXPLOSION:
		case CE_BLDRN_EXPL:
		case CE_BRN_BOUNCE:
		case CE_LSHOCK:
		case CE_ACID_HIT:
		case CE_ACID_SPLAT:
		case CE_ACID_EXPL:
		case CE_LBALL_EXPL:
		case CE_FBOOM:
		case CE_FIREWALL_SMALL:
		case CE_FIREWALL_MEDIUM:
		case CE_FIREWALL_LARGE:
		case CE_BOMB:
			fscanf(FH, "%f ", &sv.Effects[idx].ef.Smoke.origin[0]);
			fscanf(FH, "%f ", &sv.Effects[idx].ef.Smoke.origin[1]);
			fscanf(FH, "%f\n", &sv.Effects[idx].ef.Smoke.origin[2]);
			break;

		case CE_WHITE_FLASH:
		case CE_BLUE_FLASH:
		case CE_SM_BLUE_FLASH:
		case CE_HWSPLITFLASH:
		case CE_RED_FLASH:
			fscanf(FH, "%f ", &sv.Effects[idx].ef.Flash.origin[0]);
			fscanf(FH, "%f ", &sv.Effects[idx].ef.Flash.origin[1]);
			fscanf(FH, "%f\n", &sv.Effects[idx].ef.Flash.origin[2]);
			break;

		case CE_RIDER_DEATH:
			fscanf(FH, "%f ", &sv.Effects[idx].ef.RD.origin[0]);
			fscanf(FH, "%f ", &sv.Effects[idx].ef.RD.origin[1]);
			fscanf(FH, "%f\n", &sv.Effects[idx].ef.RD.origin[2]);
			break;

		case CE_TELEPORTERPUFFS:
			fscanf(FH, "%f ", &sv.Effects[idx].ef.Teleporter.origin[0]);
			fscanf(FH, "%f ", &sv.Effects[idx].ef.Teleporter.origin[1]);
			fscanf(FH, "%f\n", &sv.Effects[idx].ef.Teleporter.origin[2]);
			break;

		case CE_TELEPORTERBODY:
			fscanf(FH, "%f ", &sv.Effects[idx].ef.Teleporter.origin[0]);
			fscanf(FH, "%f ", &sv.Effects[idx].ef.Teleporter.origin[1]);
			fscanf(FH, "%f\n", &sv.Effects[idx].ef.Teleporter.origin[2]);
			break;

		case CE_BONESHRAPNEL:
		case CE_HWBONEBALL:
			fscanf(FH, "%f ", &sv.Effects[idx].ef.Missile.origin[0]);
			fscanf(FH, "%f ", &sv.Effects[idx].ef.Missile.origin[1]);
			fscanf(FH, "%f ", &sv.Effects[idx].ef.Missile.origin[2]);
			fscanf(FH, "%f ", &sv.Effects[idx].ef.Missile.velocity[0]);
			fscanf(FH, "%f ", &sv.Effects[idx].ef.Missile.velocity[1]);
			fscanf(FH, "%f ", &sv.Effects[idx].ef.Missile.velocity[2]);
			fscanf(FH, "%f ", &sv.Effects[idx].ef.Missile.angle[0]);
			fscanf(FH, "%f ", &sv.Effects[idx].ef.Missile.angle[1]);
			fscanf(FH, "%f ", &sv.Effects[idx].ef.Missile.angle[2]);
			fscanf(FH, "%f ", &sv.Effects[idx].ef.Missile.avelocity[0]);
			fscanf(FH, "%f ", &sv.Effects[idx].ef.Missile.avelocity[1]);
			fscanf(FH, "%f ", &sv.Effects[idx].ef.Missile.avelocity[2]);
			break;

		case CE_BONESHARD:
		case CE_HWRAVENSTAFF:
		case CE_HWRAVENPOWER:
		case CE_HWMISSILESTAR:
		case CE_HWEIDOLONSTAR:
			fscanf(FH, "%f ", &sv.Effects[idx].ef.Missile.origin[0]);
			fscanf(FH, "%f ", &sv.Effects[idx].ef.Missile.origin[1]);
			fscanf(FH, "%f ", &sv.Effects[idx].ef.Missile.origin[2]);
			fscanf(FH, "%f ", &sv.Effects[idx].ef.Missile.velocity[0]);
			fscanf(FH, "%f ", &sv.Effects[idx].ef.Missile.velocity[1]);
			fscanf(FH, "%f ", &sv.Effects[idx].ef.Missile.velocity[2]);
			break;

		case CE_DEATHBUBBLES:
			fscanf(FH, "%f ", &sv.Effects[idx].ef.Bubble.offset[0]);
			fscanf(FH, "%f ", &sv.Effects[idx].ef.Bubble.offset[1]);
			fscanf(FH, "%f ", &sv.Effects[idx].ef.Bubble.offset[2]);
			fscanf(FH, "%d ", &sv.Effects[idx].ef.Bubble.owner);
			fscanf(FH, "%d ", &sv.Effects[idx].ef.Bubble.count);
			break;

		case CE_HWDRILLA:
			fscanf(FH, "%f ", &sv.Effects[idx].ef.Missile.origin[0]);
			fscanf(FH, "%f ", &sv.Effects[idx].ef.Missile.origin[1]);
			fscanf(FH, "%f ", &sv.Effects[idx].ef.Missile.origin[2]);
			fscanf(FH, "%f ", &sv.Effects[idx].ef.Missile.angle[0]);
			fscanf(FH, "%f ", &sv.Effects[idx].ef.Missile.angle[1]);
			fscanf(FH, "%f ", &sv.Effects[idx].ef.Missile.angle[2]);
			fscanf(FH, "%f ", &sv.Effects[idx].ef.Missile.speed);
			break;

		case CE_SCARABCHAIN:
			fscanf(FH, "%f ", &sv.Effects[idx].ef.Chain.origin[0]);
			fscanf(FH, "%f ", &sv.Effects[idx].ef.Chain.origin[1]);
			fscanf(FH, "%f ", &sv.Effects[idx].ef.Chain.origin[2]);
			fscanf(FH, "%d ", &sv.Effects[idx].ef.Chain.owner);
			fscanf(FH, "%d ", &sv.Effects[idx].ef.Chain.material);
			fscanf(FH, "%d ", &sv.Effects[idx].ef.Chain.tag);
			break;

		case CE_HWSHEEPINATOR:
		case CE_HWXBOWSHOOT:
			fscanf(FH, "%f ", &sv.Effects[idx].ef.Xbow.origin[5][0]);
			fscanf(FH, "%f ", &sv.Effects[idx].ef.Xbow.origin[5][1]);
			fscanf(FH, "%f ", &sv.Effects[idx].ef.Xbow.origin[5][2]);
			fscanf(FH, "%f ", &sv.Effects[idx].ef.Xbow.angle[0]);
			fscanf(FH, "%f ", &sv.Effects[idx].ef.Xbow.angle[1]);
			fscanf(FH, "%f ", &sv.Effects[idx].ef.Xbow.angle[2]);
			fscanf(FH, "%d ", &sv.Effects[idx].ef.Xbow.bolts);
			fscanf(FH, "%d ", &sv.Effects[idx].ef.Xbow.activebolts);
			fscanf(FH, "%d ", &sv.Effects[idx].ef.Xbow.turnedbolts);
			break;

		case CE_TRIPMINESTILL:
		case CE_TRIPMINE:
			fscanf(FH, "%f ", &sv.Effects[idx].ef.Chain.origin[0]);
			fscanf(FH, "%f ", &sv.Effects[idx].ef.Chain.origin[1]);
			fscanf(FH, "%f ", &sv.Effects[idx].ef.Chain.origin[2]);
			fscanf(FH, "%f ", &sv.Effects[idx].ef.Chain.velocity[0]);
			fscanf(FH, "%f ", &sv.Effects[idx].ef.Chain.velocity[1]);
			fscanf(FH, "%f ", &sv.Effects[idx].ef.Chain.velocity[2]);
			break;

		default:
			Host_Error ("%s: bad type", __thisfunc__);
			break;
		}
	}
}

