/*
 * protocol.h -- communications protocols
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

#ifndef __H2W_PROTOCOL_H
#define __H2W_PROTOCOL_H

#define	OLD_PROTOCOL_VERSION	24
#define	PROTOCOL_VERSION	25
#define	PROTOCOL_VERSION_EXT	26

//=========================================

#define	PORT_CLIENT	26901		//27001
#define	PORT_MASTER	26900		//27000
#define	PORT_SERVER	26950		//27500

//=========================================

// out of band message id bytes

// M = master, S = server, C = client, A = any
// the second character will always be \n if the message isn't a single
// byte long (?? not true anymore?)

#define	A2S_ECHO		'g'	// echo back a message

#define	S2C_CHALLENGE		'c'

#define	S2C_CONNECTION		'j'
#define	A2A_PING		'k'	// respond with an A2A_ACK
#define	A2A_ACK			'l'	// general acknowledgement without info
#define	A2A_NACK		'm'	// [+ comment] general failure
#define	A2C_PRINT		'n'	// print a message on client

#define	S2M_HEARTBEAT		'a'	// + serverinfo + userlist + fraglist
#define	A2C_CLIENT_COMMAND	'B'	// + command line
#define	S2M_SHUTDOWN		'C'

#define	M2C_SERVERLST		'd'	// + \n + hw server port list


//==================
// note that constant.hc may mirror to some of these numbers
// also related to svc_strings[] in cl_parse
//==================

//
// server to client
//
#define	svc_bad			0
#define	svc_nop			1
#define	svc_disconnect		2
#define	svc_updatestat		3	// [byte] [byte]
//	svc_version		4	// [long] server version
#define	svc_setview		5	// [short] entity number
#define	svc_sound		6	// <see code>
#define	svc_time		7	// [float] server time
#define	svc_print		8	// [byte] id [string] null terminated string
#define	svc_stufftext		9	// [string] stuffed into client's console buffer
					// the string should be \n terminated

#define	svc_setangle		10	// [angle3] set the view angle to this absolute value

#define	svc_serverdata		11	// [long] protocol ...

#define	svc_lightstyle		12	// [byte] [string]
//	svc_updatename		13	// [byte] [string]
#define	svc_updatefrags		14	// [byte] [short]
//	svc_clientdata		15	// <shortbits + data>
#define	svc_stopsound		16	// <see code>
//	svc_updatecolors	17	// [byte] [byte] [byte]
#define	svc_particle		18	// [vec3] <variable>
#define	svc_damage		19
#define	svc_spawnstatic		20
//	svc_spawnbinary		21
#define	svc_spawnbaseline	22
#define	svc_temp_entity		23	// variable
//	svc_setpause		24	// [byte] on / off
//	svc_signonnum		25	// [byte]  used for the signon sequence
#define	svc_centerprint		26	// [string] to put in center of the screen
#define	svc_killedmonster	27
#define	svc_foundsecret		28
#define	svc_spawnstaticsound	29	// [coord3] [byte] samp [byte] vol [byte] aten
#define	svc_intermission	30	// [vec3_t] origin [vec3_t] angle
#define	svc_finale		31	// [string] text
#define	svc_cdtrack		32	// [byte] track
#define	svc_sellscreen		33
#define	svc_smallkick		34	// set client punchangle to 2
#define	svc_bigkick		35	// set client punchangle to 4
#define	svc_updateping		36	// [byte] [short]
#define	svc_updateentertime	37	// [byte] [float]
#define	svc_updatestatlong	38	// [byte] [long]
#define	svc_muzzleflash		39	// [short] entity
#define	svc_updateuserinfo	40	// [byte] slot [long] uid
					// [string] userinfo

#define	svc_download		41	// [short] size [size bytes]
#define	svc_playerinfo		42	// variable
#define	svc_nails		43	// [byte] num [48 bits] xyzpy 12 12 12 4 8 
#define	svc_chokecount		44	// [byte] packets choked
#define	svc_modellist		45	// [strings]
#define	svc_soundlist		46	// [strings]
#define	svc_packetentities	47	// [...]
#define	svc_deltapacketentities	48	// [...]
#define	svc_maxspeed		49	// maxspeed change, for prediction
#define	svc_entgravity		50	// gravity change, for prediction

// Hexen2 specifics
#define	svc_plaque		51
#define	svc_particle_explosion	52
#define	svc_set_view_tint	53
#define	svc_start_effect	54
#define	svc_end_effect		55
#define	svc_set_view_flags	56
#define	svc_clear_view_flags	57
#define	svc_update_inv		58
#define	svc_particle2		59
#define	svc_particle3		60	
#define	svc_particle4		61	
#define	svc_turn_effect		62
#define	svc_update_effect	63
#define	svc_multieffect		64
#define	svc_midi_name		65
#define	svc_raineffect		66
#define	svc_packmissile		67	// [byte] num [40 bits] xyz type 12 12 12 4
#define	svc_indexed_print	68	// same as svc_print, but sends an index in strings.txt instead of string
#define	svc_targetupdate	69	// [byte] angle [byte] pitch [byte] dist/4 - Hey, I got number 69!  Woo hoo!
#define	svc_name_print		70	// print player's name
#define	svc_sound_update_pos	71	// [short] ent+channel [coord3] pos
#define	svc_update_piv		72	// update players in view
#define	svc_player_sound	73	// sends weapon sound for invisible player
#define	svc_updatepclass	74	// [byte] [byte]
#define	svc_updatedminfo	75	// [byte] [short] [byte]
#define	svc_updatesiegeinfo	76	// [byte] [byte]
#define	svc_updatesiegeteam	77	// [byte] [byte]
#define	svc_updatesiegelosses	78	// [byte] [byte]
#define	svc_haskey		79	// [byte] [byte]
#define	svc_nonehaskey		80	// [byte] [byte]
#define	svc_isdoc		81	// [byte] [byte]
#define	svc_nodoc		82	// [byte] [byte]
#define	svc_playerskipped	83	// [byte]

//==============================================

//
// client to server
//
#define	clc_bad			0
#define	clc_nop 		1
//	clc_doublemove		2
#define	clc_move		3	// [[usercmd_t]
#define	clc_stringcmd		4	// [string] message
#define	clc_delta		5	// [byte] sequence number, requests delta compression of message
#define	clc_tmove		6	// teleport request, spectator only
#define	clc_inv_select		7
#define	clc_get_effect		8	// [byte] effect id

//==============================================

// playerinfo flags from server
// playerinfo always sends: playernum, flags, origin[] and framenumber

#define	PF_MSEC			(1<<0)
#define	PF_COMMAND		(1<<1)
#define	PF_VELOCITY1		(1<<2)
#define	PF_VELOCITY2		(1<<3)
#define	PF_VELOCITY3		(1<<4)
#define	PF_MODEL		(1<<5)
#define	PF_SKINNUM		(1<<6)
#define	PF_EFFECTS		(1<<7)
#define	PF_WEAPONFRAME		(1<<8)		// only sent for view player
#define	PF_DEAD			(1<<9)		// don't block movement any more
#define	PF_CROUCH		(1<<10)		// offset the view height differently
//#define	PF_NOGRAV		(1<<11)		// don't apply gravity for prediction
#define	PF_EFFECTS2		(1<<11)		// player has high byte of effects set...
#define	PF_DRAWFLAGS		(1<<12)
#define	PF_SCALE		(1<<13)
#define	PF_ABSLIGHT		(1<<14)
#define	PF_SOUND		(1<<15)		// play a sound in the weapon channel

//==============================================

// if the high bit of the client to server byte is set, the low bits are
// client move cmd bits
// ms and angle2 are always sent, the others are optional
#define	CM_ANGLE1	(1<<0)
#define	CM_ANGLE3	(1<<1)
#define	CM_FORWARD	(1<<2)
#define	CM_SIDE		(1<<3)
#define	CM_UP		(1<<4)
#define	CM_BUTTONS	(1<<5)
#define	CM_IMPULSE	(1<<6)
#define	CM_MSEC		(1<<7)

//==============================================

// the first 16 bits of a packetentities update holds 9 bits
// of entity number and 7 bits of flags
#define	U_ORIGIN1	(1<<9)
#define	U_ORIGIN2	(1<<10)
#define	U_ORIGIN3	(1<<11)
#define	U_ANGLE2	(1<<12)
#define	U_FRAME		(1<<13)
#define	U_REMOVE	(1<<14)		// REMOVE this entity, don't add it
#define	U_MOREBITS	(1<<15)

// if MOREBITS is set, these additional flags are read in next
#define	U_ANGLE1	(1<<0)
#define	U_ANGLE3	(1<<1)
#define	U_SCALE		(1<<2)	
#define	U_COLORMAP	(1<<3)
#define	U_SKIN		(1<<4)
#define	U_EFFECTS	(1<<5)
#define	U_MODEL16	(1<<6)
#define	U_MOREBITS2	(1<<7)

// if MOREBITS2 is set, then send the 3rd byte

#define	U_MODEL		(1<<16)
#define	U_SOUND		(1<<17)
#define	U_DRAWFLAGS	(1<<18)
#define	U_ABSLIGHT	(1<<19)


//==============================================

// Bits to help send server info about the client's edict variables
#define SC1_HEALTH		(1<<0)		// changes stat bar
#define SC1_LEVEL		(1<<1)		// changes stat bar
#define SC1_INTELLIGENCE	(1<<2)		// changes stat bar
#define SC1_WISDOM		(1<<3)		// changes stat bar
#define SC1_STRENGTH		(1<<4)		// changes stat bar
#define SC1_DEXTERITY		(1<<5)		// changes stat bar
//#define SC1_WEAPON			(1<<6)		// changes stat bar
#define SC1_TELEPORT_TIME	(1<<6)		// can't airmove for 2 seconds
#define SC1_BLUEMANA		(1<<7)		// changes stat bar
#define SC1_GREENMANA		(1<<8)		// changes stat bar
#define SC1_EXPERIENCE		(1<<9)		// changes stat bar
#define SC1_CNT_TORCH		(1<<10)		// changes stat bar
#define SC1_CNT_H_BOOST		(1<<11)		// changes stat bar
#define SC1_CNT_SH_BOOST	(1<<12)		// changes stat bar
#define SC1_CNT_MANA_BOOST	(1<<13)		// changes stat bar
#define SC1_CNT_TELEPORT	(1<<14)		// changes stat bar
#define SC1_CNT_TOME		(1<<15)		// changes stat bar
#define SC1_CNT_SUMMON		(1<<16)		// changes stat bar
#define SC1_CNT_INVISIBILITY	(1<<17)		// changes stat bar
#define SC1_CNT_GLYPH		(1<<18)		// changes stat bar
#define SC1_CNT_HASTE		(1<<19)		// changes stat bar
#define SC1_CNT_BLAST		(1<<20)		// changes stat bar
#define SC1_CNT_POLYMORPH	(1<<21)		// changes stat bar
#define SC1_CNT_FLIGHT		(1<<22)		// changes stat bar
#define SC1_CNT_CUBEOFFORCE	(1<<23)		// changes stat bar
#define SC1_CNT_INVINCIBILITY	(1<<24)		// changes stat bar
#define SC1_ARTIFACT_ACTIVE	(1<<25)
#define SC1_ARTIFACT_LOW	(1<<26)
#define SC1_MOVETYPE		(1<<27)
#define SC1_CAMERAMODE		(1<<28)
#define SC1_HASTED		(1<<29)
#define SC1_INVENTORY		(1<<30)
#define SC1_RINGS_ACTIVE	(1<<31)

#define SC2_RINGS_LOW		(1<<0)
#define SC2_AMULET		(1<<1)
#define SC2_BRACER		(1<<2)
#define SC2_BREASTPLATE		(1<<3)
#define SC2_HELMET		(1<<4)
#define SC2_FLIGHT_T		(1<<5)
#define SC2_WATER_T		(1<<6)
#define SC2_TURNING_T		(1<<7)
#define SC2_REGEN_T		(1<<8)
#define SC2_HASTE_T		(1<<9)
#define SC2_TOME_T		(1<<10)
#define SC2_PUZZLE1		(1<<11)
#define SC2_PUZZLE2		(1<<12)
#define SC2_PUZZLE3		(1<<13)
#define SC2_PUZZLE4		(1<<14)
#define SC2_PUZZLE5		(1<<15)
#define SC2_PUZZLE6		(1<<16)
#define SC2_PUZZLE7		(1<<17)
#define SC2_PUZZLE8		(1<<18)
#define SC2_MAXHEALTH		(1<<19)
#define SC2_MAXMANA		(1<<20)
#define SC2_FLAGS		(1<<21)

// This is to mask out those items that need to generate a stat bar change
#define SC1_STAT_BAR	0x01ffffff
#define SC2_STAT_BAR	0x0

// This is to mask out those items in the inventory (for inventory changes)
#define SC1_INV		0x01fffc00
#define SC2_INV		0x00000000

//==============================================

// a sound with no channel is a local only sound
// the sound field has bits 0-2: channel, 3-12: entity
#define	SND_VOLUME	(1<<15)
#define	SND_ATTENUATION	(1<<14)

#define DEFAULT_SOUND_PACKET_VOLUME		255
#define DEFAULT_SOUND_PACKET_ATTENUATION	1.0

// svc_print messages have an id, so messages can be filtered
#define	PRINT_LOW		0
#define	PRINT_MEDIUM		1
#define	PRINT_HIGH		2
#define	PRINT_CHAT		3	// also go to chat buffer

//
// temp entity events
//
#define	TE_SPIKE			0
#define	TE_SUPERSPIKE			1
#define	TE_GUNSHOT			2
#define	TE_EXPLOSION			3
#define	TE_TAREXPLOSION			4
#define	TE_LIGHTNING1			5
#define	TE_LIGHTNING2			6
#define	TE_WIZSPIKE			7
#define	TE_KNIGHTSPIKE			8
#define	TE_LIGHTNING3			9
#define	TE_LAVASPLASH			10
#define	TE_TELEPORT			11
#define	TE_BLOOD			12
#define	TE_LIGHTNINGBLOOD		13

// hexen 2
#define	TE_STREAM_CHAIN			25
#define	TE_STREAM_SUNSTAFF1		26
#define	TE_STREAM_SUNSTAFF2		27
#define	TE_STREAM_LIGHTNING		28
#define	TE_STREAM_COLORBEAM		29
#define	TE_STREAM_ICECHUNKS		30
#define	TE_STREAM_GAZE			31
#define	TE_STREAM_FAMINE		32

#define	TE_BIGGRENADE			33
#define	TE_CHUNK			34
#define	TE_HWBONEPOWER			35
#define	TE_HWBONEPOWER2			36
#define	TE_METEORHIT			37
#define	TE_HWRAVENDIE			38
#define	TE_HWRAVENEXPLODE		39
#define	TE_XBOWHIT			40

#define	TE_CHUNK2			41
#define	TE_ICEHIT			42
#define	TE_ICESTORM			43
#define	TE_HWMISSILEFLASH		44
#define	TE_SUNSTAFF_CHEAP		45
#define	TE_LIGHTNING_HAMMER		46
#define	TE_DRILLA_EXPLODE		47
#define	TE_DRILLA_DRILL			48

#define	TE_HWTELEPORT			49
#define	TE_SWORD_EXPLOSION		50

#define	TE_AXE_BOUNCE			51
#define	TE_AXE_EXPLODE			52
#define	TE_TIME_BOMB			53
#define	TE_FIREBALL			54
#define	TE_SUNSTAFF_POWER		55
#define	TE_PURIFY2_EXPLODE		56
#define	TE_PLAYER_DEATH			57
#define	TE_PURIFY1_EFFECT		58
#define	TE_TELEPORT_LINGER		59
#define	TE_LINE_EXPLOSION		60
#define	TE_METEOR_CRUSH			61
//MISSION PACK
#define	TE_STREAM_LIGHTNING_SMALL	62

#define	TE_ACIDBALL			63
#define	TE_ACIDBLOB			64
#define	TE_FIREWALL			65
#define	TE_FIREWALL_IMPACT		66
#define	TE_HWBONERIC			67
#define	TE_POWERFLAME			68
#define	TE_BLOODRAIN			69
#define	TE_AXE				70
#define	TE_PURIFY2_MISSILE		71
#define	TE_SWORD_SHOT			72
#define	TE_ICESHOT			73
#define	TE_METEOR			74
#define	TE_LIGHTNINGBALL		75
#define	TE_MEGAMETEOR			76
#define	TE_CUBEBEAM			77
#define	TE_LIGHTNINGEXPLODE		78
#define	TE_ACID_BALL_FLY		79
#define	TE_ACID_BLOB_FLY		80
#define	TE_CHAINLIGHTNING		81

/*
==========================================================

  ELEMENTS COMMUNICATED ACROSS THE NET

==========================================================
*/

#define	MAX_CLIENTS	32

#define	UPDATE_BACKUP	64	// copies of entity_state_t to keep buffered
				// must be power of two
#define	UPDATE_MASK	(UPDATE_BACKUP-1)

// entity_state_t is the information conveyed from the server
// in an update message
typedef struct
{
	int		number;			// edict index

	int		flags;			// nolerp, etc
	vec3_t	origin;
	vec3_t	angles;
	int		modelindex;
	int		frame;
	int		colormap;
	int		skinnum;
	int		effects;
	int		scale;			// for Alias models
	int		drawflags;		// for Alias models
	int		abslight;		// for Alias models
	int		wpn_sound;		// for cheap playing of sounds
} entity_state_t;


#define	MAX_PACKET_ENTITIES	64	// doesn't count nails
typedef struct
{
	int		num_entities;
	entity_state_t	entities[MAX_PACKET_ENTITIES];
} packet_entities_t;

typedef struct usercmd_s
{
	byte	msec;
	vec3_t	angles;
	short	forwardmove, sidemove, upmove;
	byte	buttons;
	byte	impulse;
	byte	light_level;
} usercmd_t;

#endif	/* __H2W_PROTOCOL_H */

