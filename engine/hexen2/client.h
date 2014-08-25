/*
 * hexen2/client.h -- client main header
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

#ifndef __HX2_CLIENT_H
#define __HX2_CLIENT_H

#define	MAX_SCOREBOARDNAME	32
typedef struct
{
	char		name[MAX_SCOREBOARDNAME];
	float		entertime;
	int		frags;
	int		colors;			// two 4 bit fields
	byte		translations[VID_GRADES*256];
	float		playerclass;
} scoreboard_t;

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

#define	NAME_LENGTH		64

//
// client_state_t should hold all pieces of the client state
//

#define	SIGNONS			4	// signon messages to receive before connected

#define	MAX_DLIGHTS		32
typedef struct
{
	vec3_t		origin;
	float		radius;
	float		die;		// stop lighting after this time
	float		decay;		// drop this each second
	float		minlight;	// don't add when contributing less
	int		key;
	qboolean	dark;		// subtracts light instead of adding
	float		color[4];	// LordHavoc: colored lights support
} dlight_t;

typedef struct
{
	int		length;
	char		map[MAX_STYLESTRING];
} lightstyle_t;

#define	MAX_EFRAGS		640

#define	MAX_MAPSTRING		2048
#define	MAX_DEMOS		8
#define	MAX_DEMONAME		16

typedef enum
{
	ca_dedicated,		// a dedicated server with no ability to start a client
	ca_disconnected,	// full screen console with no connection
	ca_connected,		// valid netcon, talking to a server
	ca_active = ca_connected	// simply an alias for hexenworld compatibility
} cactive_t;

//
// the client_static_t structure is persistant through an arbitrary number
// of server connections
//
typedef struct
{
	cactive_t	state;

// personalization data sent to server
	char		spawnparms[MAX_MAPSTRING];	// to restart a level

// demo loop control
	int		demonum;		// -1 = don't play demos
	char		demos[MAX_DEMOS][MAX_DEMONAME];	// when not playing

// demo recording info must be here, because record is started before
// entering a map (and clearing client_state_t)
	qboolean	demorecording;
	qboolean	demoplayback;
	qboolean	timedemo;
	int		forcetrack;		// -1 = use normal cd track
	FILE		*demofile;
//	FILE		*introdemofile;
	int		td_lastframe;		// to meter out one message a frame
	int		td_startframe;		// host_framecount at start
	float		td_starttime;		// realtime at second frame of timedemo

// connection information
	int		signon;			// 0 to SIGNONS
	struct qsocket_s	*netcon;
	sizebuf_t	message;		// writing buffer to send to server

} client_static_t;

extern client_static_t	cls;

//
// the client_state_t structure is wiped completely at every
// server signon
//
typedef struct
{
	int		movemessages;		// since connecting to this server
						// throw out the first couple, so the player
						// doesn't accidentally do something the 
						// first frame

	usercmd_t	cmd;			// last command sent to the server

// information for local display
	int		stats[MAX_CL_STATS];	// health, etc
	int		inv_order[MAX_INVENTORY];
	int		inv_count, inv_startpos, inv_selected;
	int		items;		// inventory bit flags
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
// sent to the server each frame.  The server sets punchangle when
// the view is temporarliy offset, and an angle reset commands at the start
// of each level and after teleporting.

	vec3_t		mviewangles[2];		// during demo playback viewangles is lerped
						// between these
	vec3_t		viewangles;

	vec3_t		mvelocity[2];		// update by server, used for lean+bob
						// (0 is newest)
	vec3_t		velocity;		// lerped between mvelocity[0] and [1]

	vec3_t		punchangle;		// temporary offset

	float		idealroll;
	float		rollvel;

// pitch drifting vars
	float		idealpitch;
	float		pitchvel;
	qboolean	nodrift;
	float		driftmove;
	double		laststop;

	float		viewheight;
	float		crouch;			// local amount for smoothing stepups

	qboolean	paused;			// send over by server
	qboolean	onground;
	qboolean	inwater;

// intermissions: setup by CL_SetupIntermission() and run by SB_IntermissionOverlay()
	int		intermission;		// don't change view angle, full screen, etc
	int		completed_time;		// latched at intermission start
	int		message_index;
	int		intermission_flags;
	const char	*intermission_pic;
	int		lasting_time;
	int		intermission_next;

	double		mtime[2];		// the timestamp of last two messages
	double		time;			// clients view of time, should be between
						// servertime and oldservertime to generate
						// a lerp point for other data

	double		oldtime;		// previous cl.time, time-oldtime is used
						// to decay light values and smooth step ups

	float		last_received_message;	// (realtime) for net trouble icon

//
// information that is static for the entire time connected to a server
//
	struct qmodel_s	*model_precache[MAX_MODELS];
	struct sfx_s	*sound_precache[MAX_SOUNDS];

	char		mapname[40];
	char		levelname[40];		// for display on solo scoreboard
	int		viewentity;		// cl_entitites[cl.viewentity] = player
	int		maxclients;
	int		gametype;

// refresh related state
	struct qmodel_s	*worldmodel;		// cl_entitites[0].model
	struct efrag_s	*free_efrags;
	int		num_entities;		// held in cl_entities array
	int		num_statics;		// held in cl_staticentities array
	entity_t	viewent;		// the gun model
	struct EffectT	Effects[MAX_EFFECTS];

	int		cdtrack, looptrack;	// cd audio
	char		midi_name[128];		// midi file name
	byte		current_frame, last_frame, reference_frame;
	byte		current_sequence, last_sequence;
	byte		need_build;

// frag scoreboard
	scoreboard_t	*scores;	// [cl.maxclients]

// light level at player's position including dlights
// this is sent back to the server each frame
// architectually ugly but it works
	int		light_level;

	client_frames2_t frames[3];	// 0 = base, 1 = building, 2 = 0 & 1 merged
	short		RemoveList[MAX_CLIENT_STATES], NumToRemove;

// mission pack, objectives strings
	unsigned int	info_mask, info_mask2;
} client_state_t;


//
// cvars
//
extern	cvar_t	cl_name;
extern	cvar_t	cl_color;
extern	cvar_t	cl_playerclass;

extern	cvar_t	cl_upspeed;
extern	cvar_t	cl_forwardspeed;
extern	cvar_t	cl_backspeed;
extern	cvar_t	cl_sidespeed;

extern	cvar_t	cl_movespeedkey;

extern	cvar_t	cl_yawspeed;
extern	cvar_t	cl_pitchspeed;

extern	cvar_t	cl_anglespeedkey;

extern	cvar_t	cl_shownet;
extern	cvar_t	cl_nolerp;

extern	cvar_t	cfg_unbindall;

extern	cvar_t	cl_pitchdriftspeed;
extern	cvar_t	lookspring;
extern	cvar_t	lookstrafe;
extern	cvar_t	sensitivity;

extern	cvar_t	m_pitch;
extern	cvar_t	m_yaw;
extern	cvar_t	m_forward;
extern	cvar_t	m_side;


#define	MAX_STATIC_ENTITIES	256		// torches, etc

extern	client_state_t	cl;

// FIXME, allocate dynamically
extern	efrag_t		cl_efrags[MAX_EFRAGS];
extern	entity_t	cl_entities[MAX_EDICTS];
extern	entity_t	cl_static_entities[MAX_STATIC_ENTITIES];
extern	lightstyle_t	cl_lightstyle[MAX_LIGHTSTYLES];
extern	dlight_t	cl_dlights[MAX_DLIGHTS];

//=============================================================================

//
// cl_main
//
dlight_t *CL_AllocDlight (int key);
void	CL_DecayLights (void);

void CL_Init (void);

void CL_ClearState (void);

void CL_EstablishConnection (const char *host);
void CL_SignonReply (void);

int  CL_ReadFromServer (void);

void CL_Disconnect (void);
void CL_Disconnect_f (void);
void CL_NextDemo (void);

#define	MAX_VISEDICTS		256
extern	int		cl_numvisedicts;
extern	entity_t	*cl_visedicts[MAX_VISEDICTS];

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
extern	qboolean	info_up;

void CL_InitInput (void);
void CL_SendCmd (void);
void CL_BaseMove (usercmd_t *cmd);
void CL_SendMove (const usercmd_t *cmd);

//
// cl_demo.c
//
void CL_StopPlayback (void);
int CL_GetMessage (void);

void CL_Stop_f (void);
void CL_Record_f (void);
void CL_PlayDemo_f (void);
void CL_TimeDemo_f (void);

extern	qboolean	intro_playing;	/* whether the mission pack intro is playing */
extern	qboolean	skip_start;	/* for the mission pack intro */
extern	int		num_intro_msg;	/* for the mission pack intro */
					/* skip_start and num_intro_msg are not used at present - O.S */

//
// cl_string.c
//
extern	int	puzzle_string_count;

void CL_LoadPuzzleStrings (void);
const char *CL_FindPuzzleString (const char *shortname);

/* mission pack objectives strings */
extern	int	info_string_count;

void CL_LoadInfoStrings (void);
const char *CL_GetInfoString (int idx);

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

extern	int		cl_protocol;	/* protocol version used by the server */

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

//
// cl_effect
//
void CL_InitEffects (void);
void CL_ClearEffects (void);
void CL_EndEffect (void);
void CL_ParseEffect (void);
void CL_UpdateEffects (void);

//
// cl_tent
//
void CL_InitTEnts (void);
void CL_ClearTEnts (void);
void CL_ParseTEnt (void);
void CL_UpdateTEnts (void);

//
// chase
//
extern	cvar_t	chase_active;

void Chase_Init (void);
void Chase_Reset (void);
void Chase_Update (void);

#endif	/* __HX2_CLIENT_H */

