/*
 * quakedef.h -- common definitions for client and server.
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

#ifndef __QUAKEDEFS_H
#define __QUAKEDEFS_H

#define __STRINGIFY(x) #x
#define STRINGIFY(x) __STRINGIFY(x)

#define	HOT_VERSION_MAJ		1
#define	HOT_VERSION_MID		5
#define	HOT_VERSION_MIN		6
#define	HOT_VERSION_REL_DATE	"2013-02-28"
#define	HOT_VERSION_STR		STRINGIFY(HOT_VERSION_MAJ) "." STRINGIFY(HOT_VERSION_MID) "." STRINGIFY(HOT_VERSION_MIN)
#define	GLQUAKE_VERSION		1.00
#define	ENGINE_VERSION		1.27
#define	ENGINE_NAME		"Hexen2"

#define	MAX_QPATH	64	// max length of a quake game pathname
#define	MAX_OSPATH	256	// max length of a filesystem pathname

#define	QUAKE_GAME		// as opposed to utilities

// !!! if this is changed, it must be changed in d_ifacea.h too !!!
#define CACHE_SIZE	32	// used to align key data structures

#define UNUSED(x)	(x = x)	// for pesky compiler / lint warnings

#define	MINIMUM_MEMORY	0x550000
#define	MINIMUM_MEMORY_LEVELPAK	(MINIMUM_MEMORY + 0x100000)

#define MAX_NUM_ARGVS	50

// up / down
#define	PITCH		0

// left / right
#define	YAW		1

// fall over
#define	ROLL		2

//
// Timing macros
//
#define HX_FRAME_TIME	0.05
#define HX_FPS		20

//#ifdef DEMOBUILD
//#define	MAX_CLIENTS	8
//#endif
#define	MAX_CLIENTS	16

#define	ON_EPSILON	0.1		// point on plane side epsilon

#define	DIST_EPSILON	(0.03125)	// 1/32 epsilon to keep floating point happy (moved from world.h)

//#define	MAX_MSGLEN	8000		// max length of a reliable message
//#define	MAX_MSGLEN	16000		// max length of a reliable message
#define	MAX_MSGLEN	20000		// for mission pack tibet2

//#define	MAX_DATAGRAM	2048		// max length of unreliable message  TEMP: This only for E3
#define	MAX_DATAGRAM	1024		// max length of unreliable message

#define MAX_PRINTMSG	4096		// maximum allowed print message length

//
// per-level limits
//
#define	MAX_EDICTS	600		// FIXME: ouch! ouch! ouch!
#define	MAX_LIGHTSTYLES	64

#define	MAX_MODELS	512		/* Sent over the net as a word */
#define	MAX_SOUNDS_OLD	256		/* Hexen2 v1.11 (protocol 18) and older: sent as a byte	*/
#define	MAX_SOUNDS_H2MP	512		/* Mission Pack (protocol 19), messy thing:		*/
					/* SV_StartSound sends it as a byte, but PF_ambientsound
					   sends it as a word.					*/
#define	MAX_SOUNDS	(MAX_SOUNDS_H2MP)

#define	MAX_STYLESTRING		64

//
// stats are integers communicated to the client by the server
//
#define	MAX_CL_STATS		32
//#define	STAT_HEALTH		0
#define	STAT_FRAGS		1
#define	STAT_WEAPON		2
//#define	STAT_AMMO		3
#define	STAT_ARMOR		4
#define	STAT_WEAPONFRAME	5
//#define	STAT_SHELLS		6
//#define	STAT_NAILS		7
//#define	STAT_ROCKETS		8
//#define	STAT_CELLS		9
//#define	STAT_ACTIVEWEAPON	10
#define	STAT_TOTALSECRETS	11
#define	STAT_TOTALMONSTERS	12
#define	STAT_SECRETS		13	// bumped on client side by svc_foundsecret
#define	STAT_MONSTERS		14	// bumped by svc_killedmonster
//#define	STAT_BLUEMANA		15
//#define	STAT_GREENMANA		16
//#define	STAT_EXPERIENCE		17

#define	MAX_INVENTORY		15	// Max inventory array size
/* the number of cnt_<artifact_name> members in the entvars_t struct:
   from cnt_torch to cnt_invincibility: 15 total (see in progdefs.h). */

#define	SAVEGAME_VERSION	5
#define	SAVEGAME_COMMENT_LENGTH	39	// 0-19: level name, 21-rest: save time
#define	MAX_SAVEGAMES		12	// Max number of savegames for the menu listing

//
// quake item flags
//
#define	IT_SHOTGUN		(1 << 0 )
#define	IT_SUPER_SHOTGUN	(1 << 1 )
#define	IT_NAILGUN		(1 << 2 )
#define	IT_SUPER_NAILGUN	(1 << 3 )
#define	IT_GRENADE_LAUNCHER	(1 << 4 )
#define	IT_ROCKET_LAUNCHER	(1 << 5 )
#define	IT_LIGHTNING		(1 << 6 )
#define	IT_SUPER_LIGHTNING	(1 << 7 )
#define	IT_SHELLS		(1 << 8 )
#define	IT_NAILS		(1 << 9 )
#define	IT_ROCKETS		(1 << 10)
#define	IT_CELLS		(1 << 11)
#define	IT_AXE			(1 << 12)
#define	IT_ARMOR1		(1 << 13)
#define	IT_ARMOR2		(1 << 14)
#define	IT_ARMOR3		(1 << 15)
#define	IT_SUPERHEALTH		(1 << 16)
#define	IT_KEY1			(1 << 17)
#define	IT_KEY2			(1 << 18)
#define	IT_INVISIBILITY		(1 << 19)
#define	IT_INVULNERABILITY	(1 << 20)
#define	IT_SUIT			(1 << 21)
#define	IT_QUAD			(1 << 22)
#define	IT_SIGIL1		(1 << 28)
#define	IT_SIGIL2		(1 << 29)
#define	IT_SIGIL3		(1 << 30)
#define	IT_SIGIL4		(1 << 31)
//
// hexen2 artifact flags
//
#define	ART_HASTE			(1 << 0)
#define	ART_INVINCIBILITY		(1 << 1)
#define	ART_TOMEOFPOWER			(1 << 2)
#define	ART_INVISIBILITY		(1 << 3)
/* hexen2 and hexenworld versions of these
   flags are different !!!		*/
#define	ARTFLAG_FROZEN			(1 << 7)
#define	ARTFLAG_STONED			(1 << 8)
#define	ARTFLAG_DIVINE_INTERVENTION	(1 << 9)

//
// edict->drawflags
//
#define MLS_MASKIN			7	// MLS: Model Light Style
#define MLS_MASKOUT			248
#define MLS_NONE			0
#define MLS_FULLBRIGHT			1
#define MLS_POWERMODE			2
#define MLS_TORCH			3
#define MLS_TOTALDARK			4
#define MLS_ABSLIGHT			7
#define SCALE_TYPE_MASKIN		24
#define SCALE_TYPE_MASKOUT		231
#define SCALE_TYPE_UNIFORM		0	// Scale X, Y, and Z
#define SCALE_TYPE_XYONLY		8	// Scale X and Y
#define SCALE_TYPE_ZONLY		16	// Scale Z
#define SCALE_ORIGIN_MASKIN		96
#define SCALE_ORIGIN_MASKOUT		159
#define SCALE_ORIGIN_CENTER		0	// Scaling origin at object center
#define SCALE_ORIGIN_BOTTOM		32	// Scaling origin at object bottom
#define SCALE_ORIGIN_TOP		64	// Scaling origin at object top
#define DRF_TRANSLUCENT			128
#define DRF_ANIMATEONCE			256


//===========================================

//
// Player Classes
//
#define MAX_PLAYER_CLASS	5	/* total number of available player classes			*/
#define PORTALS_EXTRA_CLASSES	1	/* number of player classes only available in the mission pack	*/
#define ABILITIES_STR_INDEX	400	/* starting number of class ability lines in strings.txt - 1	*/

#define CLASS_PALADIN		1
#define CLASS_CLERIC		2
#define CLASS_CRUSADER	CLASS_CLERIC	/* alias, the progs actually use this one */
#define CLASS_NECROMANCER	3
#define CLASS_THEIF		4
#define CLASS_THIEF	CLASS_THEIF	/* for those who type correctly ;) */
#define CLASS_ASSASSIN	CLASS_THEIF	/* another alias, progs actually use this one */
#define CLASS_DEMON		5
#define CLASS_SUCCUBUS	CLASS_DEMON	/* alias, the h2w progs actually use this one */

//#define BASE_ENT_ON		1
//#define BASE_ENT_SENT	2


/* include our common header file	*/
/* FIXME: kill this in the future and make each C
   file include only the necessary headers.	*/
#include "quakeinc.h"

#endif	/* __QUAKEDEFS_H */

