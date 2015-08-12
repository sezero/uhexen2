/*
 * client.h -- client main header
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

#ifndef __H2W_CLIENT_H
#define __H2W_CLIENT_H

typedef struct
{
	char		name[16];
	qboolean	failedload;		// the name isn't a valid skin
	cache_user_t	cache;
} skin_t;

// player_state_t is the information needed by a player entity
// to do move prediction and to generate a drawable entity
typedef struct
{
	int		messagenum;		// all player's won't be updated each frame

	double		state_time;		// not the same as the packet time,
							// because player commands come asyncronously
	usercmd_t	command;		// last command for prediction

	vec3_t		origin;
	vec3_t		viewangles;		// only for demos, not from server
	vec3_t		velocity;
	int		weaponframe;

	int		modelindex;
	int		frame;
	int		skinnum;
	int		effects;
	int		drawflags;
	int		scale;
	int		abslight;

	int		flags;			// dead, gib, etc

	float		waterjumptime;
	int		onground;		// -1 = in air, else pmove entity number
	int		oldbuttons;
} player_state_t;


#define	MAX_SCOREBOARDNAME	16
typedef struct player_info_s
{
	int		userid;
	char		userinfo[MAX_INFO_STRING];

	// scoreboard information
	char		name[MAX_SCOREBOARDNAME];
	float		entertime;
	int		frags;
	int		ping;

	// skin information
	int		topcolor;
	int		bottomcolor;
	int		playerclass;
	int		level;
	int		spectator;
	byte		translations[VID_GRADES*256];
	skin_t		*skin;
	int		modelindex;
	qboolean	Translated;
	int		siege_team;
	qboolean	shownames_off;
} player_info_t;


typedef struct
{
	// generated on client side
	usercmd_t	cmd;		// cmd that generated the frame
	double		senttime;	// time cmd was sent off
	int		delta_sequence;	// sequence number to delta from, -1 = full update

	// received from server
	double		receivedtime;	// time message was received, or -1
	player_state_t	playerstate[MAX_CLIENTS];	// message received that reflects performing
							// the usercmd
	packet_entities_t	packet_entities;
	qboolean	invalid;		// true if the packet_entities delta was invalid
} frame_t;


typedef struct
{
	int		destcolor[3];
	int		percent;		// 0-256
} cshift_t;

#define	CSHIFT_CONTENTS		0
#define	CSHIFT_DAMAGE		1
#define	CSHIFT_BONUS		2
#define	CSHIFT_POWERUP		3
#define	CSHIFT_INTERVENTION	4
#define	NUM_CSHIFTS		5

//
// client_state_t should hold all pieces of the client state
//

#define	MAX_DLIGHTS		32
typedef struct
{
	int		key;		// so entities can reuse same entry
	vec3_t		origin;
	float		radius;
	float		die;		// stop lighting after this time
	float		decay;		// drop this each second
	float		minlight;	// don't add when contributing less
	float		color[4];
	qboolean	dark;		// subtracts light instead of adding
} dlight_t;

typedef struct
{
	int		length;
	char		map[MAX_STYLESTRING];
} lightstyle_t;

#define	MAX_EFRAGS		512

#define	MAX_DEMOS		8
#define	MAX_DEMONAME		16

typedef enum
{
	ca_disconnected,	// full screen console with no connection
	ca_demostart,		// starting up a demo
	ca_connected,		// netchan_t established, waiting for svc_serverdata
	ca_onserver,		// processing data lists, donwloading, etc
	ca_active		// everything is in, so frames can be rendered
} cactive_t;

typedef enum
{
	dl_none,
	dl_model,
	dl_sound,
	dl_skin,
	dl_single
} dltype_t;		// download type

//
// the client_static_t structure is persistant through an arbitrary number
// of server connections
//
typedef struct
{
// connection information
	cactive_t	state;

// network stuff
	netchan_t	netchan;

// private userinfo for sending to masterless servers
	char		userinfo[MAX_INFO_STRING];

	char		servername[MAX_OSPATH];	// name of server from original connect

	FILE		*download;		// file transfer from server
	char		downloadtempname[MAX_OSPATH];
	char		downloadname[MAX_OSPATH];
	int		downloadnumber;
	dltype_t	downloadtype;
	int		downloadpercent;

// demo loop control
	int		demonum;		// -1 = don't play demos
	char		demos[MAX_DEMOS][MAX_DEMONAME];	// when not playing

// demo recording info must be here, because record is started before
// entering a map (and clearing client_state_t)
	qboolean	demorecording;
	qboolean	demoplayback;
	qboolean	timedemo;
	FILE		*demofile;
	float		td_lastframe;		// to meter out one message a frame
	int		td_startframe;		// host_framecount at start
	float		td_starttime;		// realtime at second frame of timedemo

	float		latency;		// rolling average
} client_static_t;

extern client_static_t	cls;

//
// the client_state_t structure is wiped completely at every
// server signon
//
typedef struct
{
	int		servercount;		// server identification for prespawns

	char		serverinfo[MAX_SERVERINFO_STRING];

	int		parsecount;		// server message counter
	int		validsequence;		// this is the sequence number of the last good
							// packetentity_t we got.  If this is 0, we can't
							// render a frame yet
	int		movemessages;		// since connecting to this server
							// throw out the first couple, so the player
							// doesn't accidentally do something the 
							// first frame

	int		protocol;
	int		spectator;

	double		last_ping_request;	// while showing scoreboard
	double		last_servermessage;

// sentcmds[cl.netchan.outgoing_sequence & UPDATE_MASK] = cmd
	frame_t		frames[UPDATE_BACKUP];

// information for local display
	int		stats[MAX_CL_STATS];	// health, etc
	int		inv_order[MAX_INVENTORY];
	int		inv_count, inv_startpos, inv_selected;
	float		item_gettime[32];	// cl.time of aquiring item, for blinking
	float		faceanimtime;		// use anim frame if cl.time < this

	entvars_t	v;		// NOTE: not every field will be update
					// you must specifically add them in
					// functions SV_WriteClientdatatToMessage()
					// and CL_ParseClientdata()

	cshift_t	cshifts[NUM_CSHIFTS];	// color shifts for damage, powerups
	cshift_t	prev_cshifts[NUM_CSHIFTS];	// and content types

	char		puzzle_pieces[8][10];	// puzzle piece names

// the client maintains its own idea of view angles, which are
// sent to the server each frame.  And only reset at level change
// and teleport times
	vec3_t		viewangles;

// the client simulates or interpolates movement to get these values
	double		time;			// this is the time value that the client
						// is rendering at.  always <= realtime
	vec3_t		simorg;
	vec3_t		simvel;
	vec3_t		simangles;

	float		idealroll;
	float		rollvel;

// pitch drifting vars
	float		idealpitch;
	float		pitchvel;
	qboolean	nodrift;
	float		driftmove;
	double		laststop;

	byte		light_level;

	float		crouch;			// local amount for smoothing stepups

	qboolean	paused;			// send over by server

	float		punchangle;		// temporary view kick from weapon firing

// intermissions: setup by CL_SetupIntermission() and run by SB_IntermissionOverlay()
	int		intermission;		// don't change view angle, full screen, etc
	int		completed_time;		// latched at intermission start
	int		message_index;
	int		intermission_flags;
	const char	*intermission_pic;
	int		lasting_time;
	int		intermission_next;

//
// information that is static for the entire time connected to a server
//
	char		model_name[MAX_MODELS][MAX_QPATH];
	char		sound_name[MAX_SOUNDS][MAX_QPATH];

	struct qmodel_s	*model_precache[MAX_MODELS];
	struct sfx_s	*sound_precache[MAX_SOUNDS];

	char		mapname[40];
	char		levelname[40];		// for display on solo scoreboard
	int		playernum;

// refresh related state
	struct qmodel_s	*worldmodel;		// cl_entitites[0].model
	struct efrag_s	*free_efrags;
	int		num_entities;		// stored bottom up in cl_entities array
	int		num_statics;		// stored top down in cl_entitiers

	int		cdtrack;		// cd audio
	char		midi_name[128];		// midi file name

	entity_t	viewent;		// weapon model

	struct EffectT	Effects[MAX_EFFECTS];

	unsigned int	PIV;			// players in view

// all player information
	player_info_t	players[MAX_CLIENTS];
} client_state_t;


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
// cvars
//
extern	cvar_t	cl_upspeed;
extern	cvar_t	cl_forwardspeed;
extern	cvar_t	cl_backspeed;
extern	cvar_t	cl_sidespeed;

extern	cvar_t	cl_movespeedkey;

extern	cvar_t	cl_yawspeed;
extern	cvar_t	cl_pitchspeed;

extern	cvar_t	cl_anglespeedkey;

extern	cvar_t	cl_shownet;

extern	cvar_t	cfg_unbindall;

extern	cvar_t	cl_pitchdriftspeed;
extern	cvar_t	lookspring;
extern	cvar_t	lookstrafe;
extern	cvar_t	sensitivity;

extern	cvar_t	m_pitch;
extern	cvar_t	m_yaw;
extern	cvar_t	m_forward;
extern	cvar_t	m_side;

extern	cvar_t	playerclass;
extern	cvar_t	spectator;


#define	MAX_STATIC_ENTITIES	256		// torches, etc

extern	client_state_t	cl;

// FIXME, allocate dynamically
extern	entity_state_t	cl_baselines[MAX_EDICTS];
extern	efrag_t		cl_efrags[MAX_EFRAGS];
extern	entity_t	cl_static_entities[MAX_STATIC_ENTITIES];
extern	lightstyle_t	cl_lightstyle[MAX_LIGHTSTYLES];
extern	dlight_t	cl_dlights[MAX_DLIGHTS];

extern	const int	color_offsets[MAX_PLAYER_CLASS];

//=============================================================================

extern	float		server_version;		// version of server we connected to

//
// cl_main
//
dlight_t *CL_AllocDlight (int key);
void	CL_DecayLights (void);

void CL_Init (void);

void CL_ClearState (void);

void CL_SendConnectPacket (void);

void CL_Disconnect (void);
void CL_NextDemo (void);
qboolean CL_DemoBehind(void);

#define	MAX_VISEDICTS		512
extern	int		cl_numvisedicts, cl_oldnumvisedicts;
extern	entity_t	*cl_visedicts, *cl_oldvisedicts;
extern	entity_t	cl_visedicts_list[2][MAX_VISEDICTS];

//
// cl_cmd
//
void Cmd_ForwardToServer (void);
void CL_Cmd_Init (void);

//
// cl_input
//
typedef struct
{
	int		down[2];	// key nums holding it down
	int		state;		// low bit is down state
} kbutton_t;

extern	kbutton_t	in_mlook, in_klook;
extern	kbutton_t	in_strafe;
extern	kbutton_t	in_speed;

extern	int		in_impulse;

void CL_InitInput (void);
void CL_SendCmd (void);
void CL_BaseMove (usercmd_t *cmd);

//
// cl_demo.c
//
void CL_StopPlayback (void);
qboolean CL_GetMessage (void);

void CL_Stop_f (void);
void CL_Record_f (void);
void CL_ReRecord_f (void);
void CL_PlayDemo_f (void);
void CL_TimeDemo_f (void);

void CL_WriteDemoCmd (const usercmd_t *pcmd);

//
// cl_interlude.c
//
#define	INTERMISSION_NOT_CONNECTED	(1<<0)	/* can not use cl.time, use realtime */
#define	INTERMISSION_NO_MENUS		(1<<1)	/* don't allow drawing the menus */
#define	INTERMISSION_NO_MESSAGE		(1<<2)	/* doesn't need a valid message index */
#define	INTERMISSION_PRINT_TOP		(1<<3)	/* print centered in top half of screen */
#define	INTERMISSION_PRINT_TOPMOST	(1<<4)	/* print at top-most side of the screen */
		/* without either of the above two, prints centered on the whole screen */
#define	INTERMISSION_PRINT_WHITE	(1<<5)	/* print in white, not in red */
#define	INTERMISSION_PRINT_DELAY	(1<<6)	/* delay message print for ca. 2.5s */

void CL_SetupIntermission (int n);

//
// cl_parse.c
//
void CL_ParseServerMessage (void);
qboolean CL_CheckOrDownloadFile (const char *filename);

//
// cl_cam.c
//
void CL_InitCam (void);
void Cam_Reset (void);
void Cam_Track (usercmd_t *cmd);
void Cam_FinishMove (usercmd_t *cmd);

//
// view
//
void V_StartPitchDrift (void);
void V_StopPitchDrift (void);

void V_RenderView (void);
void V_UpdatePalette (void);
void V_Register (void);
void V_ParseDamage (void);
void V_SetContentsColor (int contents);

void V_ParseTarget(void);

extern	float		v_targAngle;
extern	float		v_targPitch;
extern	float		v_targDist;

//
// cl_effect
//
void CL_InitEffects (void);
void CL_ClearEffects (void);
void CL_EndEffect (void);
void CL_ParseEffect (void);
void CL_ParseMultiEffect (void);
void CL_UpdateEffects (void);
void CL_TurnEffect (void);
void CL_ReviseEffect (void);

//
// cl_tent
//
void CL_InitTEnts (void);
void CL_ClearTEnts (void);
void CL_ParseTEnt (void);
void CL_UpdateTEnts (void);

void CL_UpdateHammer(entity_t *ent, int edict_num);
void CL_UpdateBug(entity_t *ent);
void CL_UpdateIceStorm(entity_t *ent, int edict_num);
void CL_UpdatePoisonGas(entity_t *ent, int edict_num);
void CL_UpdateAcidBlob(entity_t *ent, int edict_num);
void CL_UpdateOnFire(entity_t *ent, int edict_num);
void CL_UpdatePowerFlameBurn(entity_t *ent, int edict_num);

//
// cl_ents.c
//
void CL_EmitEntities (void);
void CL_SetUpPlayerPrediction (qboolean dopred);
void CL_SetSolidPlayers (int playernum);
void CL_SetSolidEntities (void);
void CL_ClearProjectiles (void);
void CL_ClearMissiles (void);
void CL_ParseProjectiles (void);
void CL_ParsePackMissiles (void);
void CL_ParsePacketEntities (qboolean delta);
void CL_ParsePlayerinfo (void);
void CL_SavePlayer (void);

//
// cl_pred.c
//
void CL_InitPrediction (void);
void CL_PredictMove (void);
void CL_PredictUsercmd (player_state_t *from, player_state_t *to, usercmd_t *u, qboolean spectate);

//
// skin.c
//
typedef struct
{
	char		manufacturer;
	char		version;
	char		encoding;
	char		bits_per_pixel;
	unsigned short	xmin,ymin,xmax,ymax;
	unsigned short	hres,vres;
	unsigned char	palette[48];
	char		reserved;
	char		color_planes;
	unsigned short	bytes_per_line;
	unsigned short	palette_type;
	char		filler[58];
	unsigned char	data;	// unbounded
} pcx_t;

void Skin_Find (player_info_t *sc);
byte *Skin_Cache (skin_t *skin);
void Skin_Skins_f (void);
void Skin_AllSkins_f (void);
void Skin_NextDownload (void);

//
// globals for Siege:
//
extern	qboolean	cl_siege;	// whether this is a Siege game
extern	byte		cl_fraglimit;
extern	float		cl_timelimit;
extern	float		cl_server_time_offset;
extern	unsigned int	defLosses;	// Defender losses
extern	unsigned int	attLosses;	// Attacker losses
extern	int		cl_keyholder;
extern	int		cl_doc;		// Defender of Crown (Excalibur)

#endif	/* __H2W_CLIENT_H */

