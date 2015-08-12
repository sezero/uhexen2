/*
 * hwsv/server.h
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

#ifndef __H2W_SERVER_H
#define __H2W_SERVER_H

#define	QW_SERVER

#define	MAX_MASTERS		8	// max recipients for heartbeat packets

#define	MAX_SIGNON_BUFFERS	8

typedef enum
{
	ss_dead,			// no map loaded
	ss_loading,			// spawning level edicts
	ss_active			// actively running
} server_state_t;

// some qc commands are only valid before the server has finished
// initializing (precache commands, static sounds / objects, etc)

typedef struct
{
	qboolean	active;		// false when server is going down
	server_state_t	state;		// precache commands are only valid during load

	double		time;

	int		lastcheck;	// used by PF_checkclient
	double		lastchecktime;	// for monster ai
	double		next_PIV_time;	// Last Player In View time

	char		name[64];	// map name
	char		midi_name[128];	// midi file name
	byte		cd_track;	// cd track number
	int		current_skill;

	char		startspot[64];
	char		modelname[MAX_QPATH];	// maps/<name>.bsp, for model_precache[0]
	struct qmodel_s	*worldmodel;
	const char	*model_precache[MAX_MODELS];	// NULL terminated
	const char	*sound_precache[MAX_SOUNDS];	// NULL terminated
	const char	*lightstyles[MAX_LIGHTSTYLES];
	struct qmodel_s	*models[MAX_MODELS];

	struct EffectT	Effects[MAX_EFFECTS];

	int		num_edicts;	// increases towards MAX_EDICTS
	edict_t		*edicts;	// can NOT be array indexed, because
					// edict_t is variable sized, but can
					// be used to reference the world ent

	byte		*pvs, *phs;	// fully expanded and decompressed

	// added to every client's unreliable buffer each frame, then cleared
	sizebuf_t	datagram;
	byte		datagram_buf[MAX_DATAGRAM];

	// added to every client's reliable buffer each frame, then cleared
	sizebuf_t	reliable_datagram;
	byte		reliable_datagram_buf[MAX_MSGLEN];

	// the multicast buffer is used to send a message to a set of clients
	sizebuf_t	multicast;
	byte		multicast_buf[MAX_MSGLEN];

	// the master buffer is used for building log packets
	sizebuf_t	master;
	byte		master_buf[MAX_DATAGRAM];

	// the signon buffer will be sent to each client as they connect
	// includes the entity baselines, the static entities, etc
	// large levels will have >MAX_DATAGRAM sized signons, so
	// multiple signon messages are kept
	sizebuf_t	signon;
	int		num_signon_buffers;
	int		signon_buffer_size[MAX_SIGNON_BUFFERS];
	byte		signon_buffers[MAX_SIGNON_BUFFERS][MAX_DATAGRAM];
} server_t;


#define	NUM_SPAWN_PARMS		16

typedef enum
{
	cs_free,	// can be reused for a new connection
	cs_zombie,	// client has been disconnected, but don't reuse
			// connection for a couple seconds

	cs_connected,	// has been assigned to a client_t, but not in game yet
	cs_spawned	// client is fully in game
} client_state_t;

typedef struct
{
	// received from client

	// reply
	double			senttime;
	float			ping_time;
	packet_entities_t	entities;
} client_frame_t;

typedef struct client_s
{
	client_state_t	state;

	int		protocol;

	int		spectator;	// non-interactive

	qboolean	sendinfo;	// at end of frame, send info to all
					// this prevents malicious multiple broadcasts
	qboolean	portals;	// They have portals mission pack installed
	int		userid;		// identifying number
	char		userinfo[MAX_INFO_STRING];	// infostring

	usercmd_t	lastcmd;	// for filling in big drops and partial predictions
	double		localtime;	// of last message
	int		oldbuttons;

	float		maxspeed;	// localized maxspeed
	float		entgravity;	// localized ent gravity

	edict_t		*edict;		// EDICT_NUM(clientnum+1)
	char		name[32];	// for printing to other people
					// extracted from userinfo
	int		playerclass;
	int		siege_team;
	int		next_playerclass;
	int		messagelevel;	// for filtering printed messages

	// the datagram is written to after every frame, but only cleared
	// when it is sent out to the client.  overflow is tolerated.
	sizebuf_t	datagram;
	byte		datagram_buf[MAX_DATAGRAM];

	double		connection_started;	// or time of disconnect for zombies
	qboolean	send_message;		// set on frames a datagram arived on

	// spawn parms are carried from level to level
	float		spawn_parms[NUM_SPAWN_PARMS];

	// client known data for deltas
	int		old_frags;

	int		stats[MAX_CL_STATS];

	client_frame_t	frames[UPDATE_BACKUP];	// updates can be deltad from here

	FILE		*download;	// file being downloaded
	int		downloadsize;	// total bytes
	int		downloadcount;	// bytes sent

	int		spec_track;	// entnum of player tracking

	double		whensaid[10];	// JACK: For floodprots
	int		whensaidhead;	// Head value for floodprots
	double		lockedtill;

	//===== NETWORK ============
	int		chokecount;
	int		delta_sequence;	// -1 = no compression
	netchan_t	netchan;

	entvars_t	old_v;
	qboolean	send_all_v;

	unsigned int	PIV, LastPIV;	// people in view
	qboolean	skipsend;	// Skip sending this frame, guaranteed to send next frame
} client_t;

// a client can leave the server in one of four ways:
// dropping properly by quiting or disconnecting
// timing out if no valid messages are received for timeout.value seconds
// getting kicked off by the server operator
// a program error, like an overflowed reliable buffer

//=============================================================================


#define	STATFRAMES	100
typedef struct
{
	double		active;
	double		idle;
	int		count;
	int		packets;

	double		latched_active;
	double		latched_idle;
	int		latched_packets;
} svstats_t;


typedef struct
{
	int		spawncount;		// number of servers spawned since start,
						// used to check late spawns
	client_t	clients[MAX_CLIENTS];
	int		serverflags;		// episode completion information
	qboolean	changelevel_issued;	// cleared when at SV_SpawnServer

	double		last_heartbeat;
	int		heartbeat_sequence;
	svstats_t	stats;

	char		info[MAX_SERVERINFO_STRING];

	// log messages are used so that fraglog processes can get stats
	int		logsequence;	// the message currently being filled
	double		logtime;	// time of last swap
	sizebuf_t	log[2];
	byte		log_buf[2][MAX_DATAGRAM];
} server_static_t;

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

#define	MULTICAST_ALL		0
#define	MULTICAST_PHS		1
#define	MULTICAST_PVS		2

#define	MULTICAST_ALL_R		3
#define	MULTICAST_PHS_R		4
#define	MULTICAST_PVS_R		5

//============================================================================

extern	cvar_t	sv_mintic, sv_maxtic;
extern	cvar_t	sv_maxspeed;
extern	cvar_t	sv_highchars;

extern	netadr_t	master_adr[MAX_MASTERS];	// address of the master server

extern	cvar_t	spawn;
extern	cvar_t	teamplay;
extern	cvar_t	skill;
extern	cvar_t	deathmatch;
extern	cvar_t	coop;
extern	cvar_t	randomclass;
extern	cvar_t	maxclients;
extern	cvar_t	damageScale;
extern	cvar_t	meleeDamScale;
extern	cvar_t	shyRespawn;
extern	cvar_t	spartanPrint;
extern	cvar_t	manaScale;
extern	cvar_t	tomeMode;
extern	cvar_t	tomeRespawn;
extern	cvar_t	w2Respawn;
extern	cvar_t	altRespawn;
extern	cvar_t	fixedLevel;
extern	cvar_t	autoItems;
extern	cvar_t	dmMode;
extern	cvar_t	easyFourth;
extern	cvar_t	patternRunner;
extern	cvar_t	fraglimit;
extern	cvar_t	timelimit;
extern	cvar_t	noexit;

extern	server_static_t	svs;		// persistant server info
extern	server_t	sv;		// local server

extern	client_t	*host_client;

extern	edict_t		*sv_player;

extern	char		localmodels[MAX_MODELS][8];	// inline model names for precache

extern	char		localinfo[MAX_LOCALINFO_STRING+1];

extern	int		host_hunklevel;
extern	FILE		*sv_logfile;
extern	FILE		*sv_fraglogfile;

extern	unsigned int	defLosses;	// Defenders losses in Siege
extern	unsigned int	attLosses;	// Attackers Losses in Siege
#define	SV_PROGS_HAVE_SIEGE	(sv_globals.defLosses)	/* progs have siege fields: >= v0.14 */

//===========================================================

//
// sv_main.c
//
void SV_Shutdown (void);
void SV_Frame (float time);
void SV_FinalMessage (const char *message);
void SV_DropClient (client_t *drop);

int SV_CalcPing (client_t *cl);
void SV_FullClientUpdate (client_t *client, sizebuf_t *buf);

int SV_ModelIndex (const char *name);

qboolean SV_CheckBottom (edict_t *ent);
qboolean SV_movestep (edict_t *ent, vec3_t move, qboolean relink, qboolean noenemy, qboolean set_trace);

void SV_WriteClientdataToMessage (client_t *client, sizebuf_t *msg);

void SV_MoveToGoal (void);

void SV_SaveSpawnparms (void);

void SV_Physics_Client (edict_t	*ent);

void SV_ExecuteUserCommand (const char *s);
void SV_InitOperatorCommands (void);

void SV_SendServerinfo (client_t *client);
void SV_ExtractFromUserinfo (client_t *cl);


void Master_Heartbeat (void);
void Master_Packet (void);

//
// sv_init.c
//
void SV_SpawnServer (const char *server, const char *startspot);
void SV_FlushSignon (void);

const char *SV_GetLevelname (void);

//
// sv_phys.c
//
void SV_ProgStartFrame (void);
void SV_Physics (void);
void SV_CheckVelocity (edict_t *ent);
void SV_AddGravity (edict_t *ent, float scale);
qboolean SV_RunThink (edict_t *ent);
void SV_Physics_Toss (edict_t *ent);
void SV_RunNewmis (void);
void SV_Impact (edict_t *e1, edict_t *e2);
void SV_SetMoveVars (void);

//
// sv_send.c
//
extern unsigned int	clients_multicast;

void SV_SendClientMessages (void);

void SV_Multicast (vec3_t origin, int to);
void SV_MulticastSpecific (unsigned int clients, qboolean reliable);
void SV_StartSound (edict_t *entity, int channel, const char *sample, int volume, float attenuation);
void SV_StopSound (edict_t *entity, int channel);
void SV_UpdateSoundPos (edict_t *entity, int channel);
void SV_StartParticle (vec3_t org, vec3_t dir, int color, int count);
void SV_StartParticle2 (vec3_t org, vec3_t dmin, vec3_t dmax, int color, int effect, int count);
void SV_StartParticle3 (vec3_t org, vec3_t box, int color, int effect, int count);
void SV_StartParticle4 (vec3_t org, float radius, int color, int effect, int count);
void SV_StartRainEffect (vec3_t org, vec3_t e_size, int x_dir, int y_dir, int color, int count);
void SV_ClientPrintf (client_t *cl, int level, const char *fmt, ...) __attribute__((__format__(printf,3,4)));
void SV_BroadcastPrintf (int level, const char *fmt, ...) __attribute__((__format__(__printf__,2,3)));
void SV_BroadcastCommand (const char *fmt, ...) __attribute__((__format__(__printf__,1,2)));
void SV_SendMessagesToAll (void);
void SV_FindModelNumbers (void);

//
// sv_user.c
//
void SV_ExecuteClientMessage (client_t *cl);
void SV_UserInit (void);

//
// svonly.c
//
typedef enum
{
	RD_NONE,
	RD_CLIENT,
	RD_PACKET
} redirect_t;

void SV_BeginRedirect (redirect_t rd);
void SV_EndRedirect (void);

//
// sv_ccmds.c
//
//void SV_Status_f (void);

//
// sv_ents.c
//
void SV_WriteEntitiesToClient (client_t *client, sizebuf_t *msg);
void SV_WriteInventory (client_t *host_cl, edict_t *ent, sizebuf_t *msg);

//
// sv_effect.c
//
void SV_ParseEffect (sizebuf_t *sb);
void SV_SendEffect (sizebuf_t *sb, int idx);
void SV_SaveEffects (FILE *FH);
void SV_LoadEffects (FILE *FH);

#endif	/* __H2W_SERVER_H */

