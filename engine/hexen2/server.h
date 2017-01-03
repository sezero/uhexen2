/*
 * hexen2/server.h
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

#ifndef __HX2_SERVER_H
#define __HX2_SERVER_H

typedef struct
{
	int			maxclients;
	int			maxclientslimit;
	struct client_s	*clients;		// [maxclients]
	int			serverflags;	// episode completion information
	qboolean	changelevel_issued;	// cleared when at SV_SpawnServer
} server_static_t;

//=============================================================================

typedef enum
{
	ss_loading,
	ss_active
} server_state_t;

typedef struct
{
	qboolean	active;		// false if only a net client

	qboolean	paused;
	qboolean	loadgame;	// handle connections specially

	double		time;

	int		lastcheck;	// used by PF_checkclient
	double		lastchecktime;

	char		name[64];	// map name
	char		midi_name[128];	// midi file name
	byte		cd_track;	// cd track number

	char		startspot[64];
	char		modelname[MAX_QPATH];	// maps/<name>.bsp, for model_precache[0]
	struct qmodel_s	*worldmodel;
	const char	*model_precache[MAX_MODELS];	// NULL terminated
	struct qmodel_s	*models[MAX_MODELS];
	const char	*sound_precache[MAX_SOUNDS];	// NULL terminated
	const char	*lightstyles[MAX_LIGHTSTYLES];
	struct EffectT	Effects[MAX_EFFECTS];

	client_state2_t	*states;
	int		num_edicts;
	edict_t		*edicts;	// can NOT be array indexed, because
					// edict_t is variable sized, but can
					// be used to reference the world ent

	server_state_t	state;		// some actions are only valid during load

	sizebuf_t	datagram;
	byte		datagram_buf[NET_MAXMESSAGE];

	sizebuf_t	reliable_datagram;	// copied to all clients at end of frame
	byte		reliable_datagram_buf[NET_MAXMESSAGE];

	sizebuf_t	signon;
	byte		signon_buf[NET_MAXMESSAGE];
} server_t;


#define	NUM_PING_TIMES		16
#define	NUM_SPAWN_PARMS		16

typedef struct client_s
{
	qboolean	active;		// false = client is free
	qboolean	spawned;	// false = don't send datagrams
	qboolean	dropasap;	// has been told to go to another level
	qboolean	sendsignon;	// only valid before spawned

	double		last_message;	// reliable messages must be sent
					// periodically

	struct qsocket_s *netconnection; // communications handle

	usercmd_t	cmd;		// movement
	vec3_t		wishdir;	// intended motion calced from cmd

	sizebuf_t	message;	// can be added to at any time,
					// copied and clear once per frame
	byte		msgbuf[MAX_MSGLEN];

	sizebuf_t	datagram;
	byte		datagram_buf[NET_MAXMESSAGE];

	edict_t		*edict;		// EDICT_NUM(clientnum+1)
	char		name[32];	// for printing to other people
	int		colors;
	float		playerclass;

	float		ping_times[NUM_PING_TIMES];
	int		num_pings;	// ping_times[num_pings%NUM_PING_TIMES]

	// spawn parms are carried from level to level
	float		spawn_parms[NUM_SPAWN_PARMS];

	// client known data for deltas
	int		old_frags;
	entvars_t	old_v;
	qboolean	send_all_v;

	byte		current_frame, last_frame;
	byte		current_sequence, last_sequence;

// mission pack, objectives strings
	unsigned int	info_mask, info_mask2;
} client_t;


//=============================================================================

//
// edict->movetype values
//
#define	MOVETYPE_NONE		0		// never moves
#define	MOVETYPE_ANGLENOCLIP	1
#define	MOVETYPE_ANGLECLIP	2
#define	MOVETYPE_WALK		3		// gravity
#define	MOVETYPE_STEP		4		// gravity, special edge handling
#define	MOVETYPE_FLY		5
#define	MOVETYPE_TOSS		6		// gravity
#define	MOVETYPE_PUSH		7		// no clip to world, push and crush
#define	MOVETYPE_NOCLIP		8
#define	MOVETYPE_FLYMISSILE	9		// extra size to monsters
#define	MOVETYPE_BOUNCE		10
//#ifdef QUAKE2
#define	MOVETYPE_BOUNCEMISSILE	11		// bounce w/o gravity
#define	MOVETYPE_FOLLOW		12		// track movement of aiment
//#endif
#define	MOVETYPE_PUSHPULL	13		// pushable/pullable object
#define	MOVETYPE_SWIM		14		// should keep the object in water

//
// edict->solid values
//
#define	SOLID_NOT		0		// no interaction with other objects
#define	SOLID_TRIGGER		1		// touch on edge, but not blocking
#define	SOLID_BBOX		2		// touch on edge, block
#define	SOLID_SLIDEBOX		3		// touch on edge, but not an onground
#define	SOLID_BSP		4		// bsp clip, touch on edge, block
#define	SOLID_PHASE		5		// won't slow down when hitting entities flagged as FL_MONSTER

//
// edict->deadflag values
//
#define	DEAD_NO			0
#define	DEAD_DYING		1
#define	DEAD_DEAD		2

#define	DAMAGE_NO		0		// Cannot be damaged
#define	DAMAGE_YES		1		// Can be damaged
#define	DAMAGE_NO_GRENADE	2		// Will not set off grenades

//
// edict->flags
//
#define	FL_FLY			1
#define	FL_SWIM			2
#define	FL_CONVEYOR		4
#define	FL_CLIENT		8
#define	FL_INWATER		16
#define	FL_MONSTER		32
#define	FL_GODMODE		64
#define	FL_NOTARGET		128
#define	FL_ITEM			256
#define	FL_ONGROUND		512
#define	FL_PARTIALGROUND	1024	// not all corners are valid
#define	FL_WATERJUMP		2048	// player jumping out of water
#define	FL_JUMPRELEASED		4096	// for jump debouncing
#define	FL_FLASHLIGHT		8192
#define	FL_ARCHIVE_OVERRIDE	1048576
#define	FL_ARTIFACTUSED		16384
#define	FL_MOVECHAIN_ANGLE	32768	// when in a move chain, will update the angle
/* the following three are for monster_pentacles of the mission pack */
#define	FL_HUNTFACE		65536	// Makes monster go for enemy view_ofs thwn moving
#define	FL_NOZ			131072	// Monster will not automove on Z if flying or swimming
#define	FL_SET_TRACE		262144	// Trace will always be set for this monster (pentacles)

#define	FL_CLASS_DEPENDENT	2097152	// model will appear different to each player
#define	FL_SPECIAL_ABILITY1	4194304	// has 1st special ability
#define	FL_SPECIAL_ABILITY2	8388608	// has 2nd special ability

#define	FL2_CROUCHED		4096

//
// Built-in Spawn Flags
//
#define	SPAWNFLAG_NOT_PALADIN		0x00000100
#define	SPAWNFLAG_NOT_CLERIC		0x00000200
#define	SPAWNFLAG_NOT_NECROMANCER	0x00000400
#define	SPAWNFLAG_NOT_THEIF		0x00000800
#define	SPAWNFLAG_NOT_EASY		0x00001000
#define	SPAWNFLAG_NOT_MEDIUM		0x00002000
#define	SPAWNFLAG_NOT_HARD		0x00004000
#define	SPAWNFLAG_NOT_DEATHMATCH	0x00008000
#define	SPAWNFLAG_NOT_COOP		0x00010000
#define	SPAWNFLAG_NOT_SINGLE		0x00020000
/* SPAWNFLAG_NOT_DEMON is NOT used: ED_LoadFromFile
 * checks SPAWNFLAG_NOT_NECROMANCER for CLASS_DEMON !! */
#define	SPAWNFLAG_NOT_DEMON		0x00040000

//
// server flags
//
#define	SFL_EPISODE_1		1
#define	SFL_EPISODE_2		2
#define	SFL_EPISODE_3		4
#define	SFL_EPISODE_4		8
#define	SFL_NEW_UNIT		16
#define	SFL_NEW_EPISODE		32
#define	SFL_CROSS_TRIGGERS	65280

//============================================================================

extern	cvar_t	teamplay;
extern	cvar_t	skill;
extern	cvar_t	deathmatch;
extern	cvar_t	coop;
extern	cvar_t	randomclass;
extern	cvar_t	fraglimit;
extern	cvar_t	timelimit;

extern	server_static_t	svs;		// persistant server info
extern	server_t	sv;		// local server

extern	int		sv_protocol;	// protocol version to use

extern	client_t	*host_client;

extern	edict_t		*sv_player;

//===========================================================

void SV_Init (void);
void SV_UserInit (void);

void SV_StartParticle (vec3_t org, vec3_t dir, int color, int count);
void SV_StartParticle2 (vec3_t org, vec3_t dmin, vec3_t dmax, int color, int effect, int count);
void SV_StartParticle3 (vec3_t org, vec3_t box, int color, int effect, int count);
void SV_StartParticle4 (vec3_t org, float radius, int color, int effect, int count);
void SV_StartSound (edict_t *entity, int channel, const char *sample, int volume, float attenuation);
void SV_StopSound (edict_t *entity, int channel);
void SV_UpdateSoundPos (edict_t *entity, int channel);

void SV_DropClient (qboolean crash);

void SV_Edicts (const char *Name);

void SV_SendClientMessages (void);
void SV_ClearDatagram (void);

int SV_ModelIndex (const char *name);

void SV_SetIdealPitch (void);

void SV_AddUpdates (void);

void SV_ClientThink (void);
void SV_AddClientToServer (struct qsocket_s	*ret);

void SV_ClientPrintf (unsigned int unused, const char *fmt, ...) FUNC_PRINTF(2,3);
void SV_BroadcastPrintf (const char *fmt, ...) FUNC_PRINTF(1,2);

void SV_Physics (void);

qboolean SV_CheckBottom (edict_t *ent);
qboolean SV_movestep (edict_t *ent, vec3_t move, qboolean relink, qboolean noenemy,
					  qboolean set_trace);

void SV_WriteClientdataToMessage (client_t *client, edict_t *ent, sizebuf_t *msg);

void SV_MoveToGoal (void);

void SV_CheckForNewClients (void);
void SV_RunClients (void);
void SV_SaveSpawnparms (void);
void SV_SpawnServer (const char *server, const char *startspot);

const char *SV_GetLevelname (void);

void SV_ParseEffect (sizebuf_t *sb);
void SV_UpdateEffects (sizebuf_t *sb);
void SV_SaveEffects (FILE *FH);
void SV_LoadEffects (FILE *FH);

#endif	/* __HX2_SERVER_H */

