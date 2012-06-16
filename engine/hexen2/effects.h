/*
 * effects.h -- effect types and defs for Hexen II
 * $Id: effects.h,v 1.1 2009-01-10 22:08:50 sezero Exp $
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

#ifndef __EFFECTS_H
#define __EFFECTS_H

#define MAX_EFFECTS		256

/* Types for various chunks */
#define THINGTYPE_GREYSTONE	1
#define THINGTYPE_WOOD		2
#define THINGTYPE_METAL		3
#define THINGTYPE_FLESH		4
#define THINGTYPE_FIRE		5
#define THINGTYPE_CLAY		6
#define THINGTYPE_LEAVES	7
#define THINGTYPE_HAY		8
#define THINGTYPE_BROWNSTONE	9
#define THINGTYPE_CLOTH		10
#define THINGTYPE_WOOD_LEAF	11
#define THINGTYPE_WOOD_METAL	12
#define THINGTYPE_WOOD_STONE	13
#define THINGTYPE_METAL_STONE	14
#define THINGTYPE_METAL_CLOTH	15
#define THINGTYPE_WEBS		16
#define THINGTYPE_GLASS		17
#define THINGTYPE_ICE		18
#define THINGTYPE_CLEARGLASS	19
#define THINGTYPE_REDGLASS	20
#define THINGTYPE_ACID		21
#define THINGTYPE_METEOR	22
#define THINGTYPE_GREENFLESH	23
#define THINGTYPE_BONE		24

#define CE_NONE			0
#define CE_RAIN			1
#define CE_FOUNTAIN		2
#define CE_QUAKE		3
#define CE_WHITE_SMOKE		4	/* whtsmk1.spr */
#define CE_BLUESPARK		5	/* bspark.spr */
#define CE_YELLOWSPARK		6	/* spark.spr */
#define CE_SM_CIRCLE_EXP	7	/* fcircle.spr */
#define CE_BG_CIRCLE_EXP	8	/* fcircle.spr */
#define CE_SM_WHITE_FLASH	9	/* sm_white.spr */
#define CE_WHITE_FLASH		10	/* gryspt.spr */
#define CE_YELLOWRED_FLASH	11	/* yr_flash.spr */
#define CE_BLUE_FLASH		12	/* bluflash.spr */
#define CE_SM_BLUE_FLASH	13	/* bluflash.spr */
#define CE_RED_FLASH		14	/* redspt.spr */
#define CE_SM_EXPLOSION		15	/* sm_expld.spr */
#define CE_LG_EXPLOSION		16	/* bg_expld.spr */
#define CE_FLOOR_EXPLOSION	17	/* fl_expld.spr */
#define CE_RIDER_DEATH		18
#define CE_BLUE_EXPLOSION	19	/* xpspblue.spr */
#define CE_GREEN_SMOKE		20	/* grnsmk1.spr */
#define CE_GREY_SMOKE		21	/* grysmk1.spr */
#define CE_RED_SMOKE		22	/* redsmk1.spr */
#define CE_SLOW_WHITE_SMOKE	23	/* whtsmk1.spr */
#define CE_REDSPARK		24	/* rspark.spr */
#define CE_GREENSPARK		25	/* gspark.spr */
#define CE_TELESMK1		26	/* telesmk1.spr */
#define CE_TELESMK2		27	/* telesmk2.spr */
#define CE_ICEHIT		28	/* icehit.spr */
#define CE_MEDUSA_HIT		29	/* medhit.spr */
#define CE_MEZZO_REFLECT	30	/* mezzoref.spr */
#define CE_FLOOR_EXPLOSION2	31	/* flrexpl2.spr */
#define CE_XBOW_EXPLOSION	32	/* xbowexpl.spr */
#define CE_NEW_EXPLOSION	33	/* gen_expl.spr */
#define CE_MAGIC_MISSILE_EXPLOSION 34	/* mm_expld.spr */
#define CE_GHOST		35	/* ghost.spr */
#define CE_BONE_EXPLOSION	36
#define CE_REDCLOUD		37
#define CE_TELEPORTERPUFFS	38
#define CE_TELEPORTERBODY	39
#define CE_BONESHARD		40
#define CE_BONESHRAPNEL		41

/* New for Mission Pack... */
#define CE_FLAMESTREAM		42	/* Flamethrower */
#define CE_SNOW			43
#define CE_GRAVITYWELL		44
#define CE_BLDRN_EXPL		45
#define CE_ACID_MUZZFL		46
#define CE_ACID_HIT		47
#define CE_FIREWALL_SMALL	48
#define CE_FIREWALL_MEDIUM	49
#define CE_FIREWALL_LARGE	50
#define CE_LBALL_EXPL		51
#define CE_ACID_SPLAT		52
#define CE_ACID_EXPL		53
#define CE_FBOOM		54
#define CE_CHUNK		55
#define CE_BOMB			56
#define CE_BRN_BOUNCE		57
#define CE_LSHOCK		58
#define CE_FLAMEWALL		59
#define CE_FLAMEWALL2		60
#define CE_FLOOR_EXPLOSION3	61
#define CE_ONFIRE		62

struct EffectT
{
	int			type;
	float		expire_time;

	union
	{
		struct
		{
			vec3_t e_size, dir, min_org, max_org;
			float next_time, wait;
			int color, count, veer, flags;
		} Rain;
		struct
		{
			vec3_t pos, angle, movedir;
			vec3_t vforward, vup, vright;
			int color, cnt;
		} Fountain;
		struct
		{
			vec3_t origin;
			float radius;
		} Quake;
		struct
		{
			vec3_t origin;
			vec3_t velocity;
			int entity_index;
			float time_amount, framelength, frame;
		} Smoke;
		struct
		{
			vec3_t origin;
			int entity_index;
			float time_amount;
			int reverse;  /* Forward animation has been run,
					 now go backwards */
		} Flash;
		struct
		{
			vec3_t origin;
			int entity_index[13];
			float time_amount;
			int stage;
			int color;
			float lifetime;
		} RD; /* Rider Death */
		struct
		{
			int entity_index[16];
			vec3_t origin;
			vec3_t velocity[16];
			float time_amount, framelength;
			float skinnum;
		} Teleporter;
		struct
		{
			vec3_t angle;
			vec3_t origin;
			vec3_t avelocity;
			vec3_t velocity;
			int entity_index;
			float time_amount;
		} Missile;
		struct
		{
			int	entity_index[16];
			vec3_t	velocity[16];
			vec3_t	avel[3];
			vec3_t	origin;
			unsigned char type;
			vec3_t	srcVel;
			unsigned char numChunks;
			float	time_amount;
			float	aveScale;
		} Chunk;

	} ef;
};

#endif	/* __EFFECTS_H */

